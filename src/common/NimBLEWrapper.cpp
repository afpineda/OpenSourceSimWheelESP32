/**
 * @file NimBLEWrapper.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-12-14
 * @brief NimBLE custom wrapper
 *
 * @note This custom wrapper implements the functionality
 *       required by this project only.
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------------------
// Conditional compilation
//------------------------------------------------------------------------------

#define NDEBUG // Comment out while debugging

#include "NimBLEWrapper.hpp"

#if CONFIG_NIMBLE_ENABLED

//------------------------------------------------------------------------------
// Imports
//------------------------------------------------------------------------------

#include <cstring>
#include <algorithm>
#include <cassert>

#include "esp_mac.h"                     // esp_efuse_mac_get_default()
#include "nvs_flash.h"                   // nvs_flash_init()
#include "esp_bt.h"                      // BT controller
#include "nimble/nimble_port.h"          // nimble_port_init() and others
#include "nimble/nimble_port_freertos.h" // nimble_port_freertos_init()
#include "host/ble_hs.h"                 // ble_hs_cfg global variable
#include "host/util/util.h"              // BT address configuration
#include "services/gap/ble_svc_gap.h"    // ble_svc_gap_device_name_set()
#include "host/ble_hs_adv.h"             // Aadvertising
#include "services/gatt/ble_svc_gatt.h"  // GATT server
#include "os/os_mbuf.h"                  // Attribute values
#include "host/ble_hs_mbuf.h"            // Attribute values

// Note: not included in any header file (a mystery to me)
extern "C" void ble_store_config_init(void);

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

#define BLE_APPEARANCE_GAMEPAD 0x03C4
static constexpr const uint8_t PREFERRED_PHY = BLE_HCI_LE_PHY_2M_PREF_MASK;

//------------------------------------------------------------------------------
// ApiResult
//------------------------------------------------------------------------------

constexpr ApiResult &ApiResult::operator=(int value) noexcept
{
    code = value;
    return *this;
}

void ApiResult::abort_if(const char *txt) const
{
    if (code)
    {
        log_e("%s. Code: %x (%d)", txt, code, code);
        abort();
    }
}

void ApiResult::log_if(const char *txt) const noexcept
{
    if (code)
        log_i("%s. Code: %x (%d)", txt, code, code);
}

int ApiResult::to_attr_rc(bool readOrWrite) const noexcept
{
    if (readOrWrite)
        return code ? BLE_ATT_ERR_INSUFFICIENT_RES : 0;
    else
        return code ? BLE_ATT_ERR_UNLIKELY : 0;
}

//------------------------------------------------------------------------------
// BLEAdvertising
//------------------------------------------------------------------------------

void BLEAdvertising::start()
{
    assert(BLEDevice::ready);
    assert(!_connected);
    if (!ble_gap_adv_active())
    {
        // Don't know why we have to initialize
        // advertising on every attempt
        BLEAdvertising::init();
        // Start advertising
        ApiResult rc =
            ble_gap_adv_start(
                BLEDevice::address_type,
                nullptr,
                BLE_HS_FOREVER,
                &adv_params,
                ble_gap_event_fn,
                nullptr);
        rc.abort_if("ble_gap_adv_start() failed");
    }
};

void BLEAdvertising::stop() noexcept
{
    ApiResult rc = ble_gap_adv_stop();
    if (rc != BLE_HS_EALREADY)
        rc.log_if("ble_gap_adv_stop() failed");
}

void BLEAdvertising::disconnect() noexcept
{
    if (_connected)
    {
        ApiResult rc = ble_gap_terminate(
            _conn_handle,
            BLE_ERR_REM_USER_CONN_TERM);
        // if ((rc != BLE_HS_ENOTCONN) && (rc != BLE_HS_EALREADY))
        rc.log_if("ble_gap_terminate() failed");
    }
    else
        log_i("BLEAdvertising::disconnect(): not connected anyway");
}

int BLEAdvertising::ble_gap_event_fn(ble_gap_event *event, void *arg)
{
    // Note: most events are handled just for debugging purposes
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        log_i("GAP event: connection");
        // Notes:
        // - A successful connection event does not mean the link is established
        // - if the connection attempt fails, advertising is stopped.
        //   We repeat the onConnectionStatus event to give a chance
        //   to start advertising again.
        if ((event->connect.status) && (onConnectionStatus))
            onConnectionStatus(false);
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        log_i("GAP event: disconnection");
        _connected = false;
        _conn_handle = BLE_HS_CONN_HANDLE_NONE;
        if (onConnectionStatus)
            onConnectionStatus(false);
        break;
    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        log_d("GAP event: BLE_GAP_EVENT_CONN_UPDATE_REQ");
        break;
    case BLE_GAP_EVENT_L2CAP_UPDATE_REQ:
        log_d("GAP event: BLE_GAP_EVENT_L2CAP_UPDATE_REQ");
        break;
    case BLE_GAP_EVENT_TERM_FAILURE:
        log_d("GAP event: BLE_GAP_EVENT_TERM_FAILURE");
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        log_d("GAP event: advertising complete");
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        log_d("GAP event: BLE_GAP_EVENT_ENC_CHANGE");
        break;
    case BLE_GAP_EVENT_PASSKEY_ACTION:
        log_d("GAP event: BLE_GAP_EVENT_PASSKEY_ACTION");
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
    {
        log_i("GAP event: subscription to chr %d, reason %d",
              event->subscribe.attr_handle,
              event->subscribe.reason);
        bool yesOrNo = event->subscribe.cur_notify |
                       event->subscribe.cur_indicate;
        BLEBatteryService::onSubscriptionChange(
            event->subscribe.attr_handle,
            yesOrNo);
        BLEHIDService::onSubscriptionChange(
            event->subscribe.attr_handle,
            yesOrNo);
    }
    break;
    case BLE_GAP_EVENT_MTU:
        log_d("GAP event: MTU change");
        break;
    case BLE_GAP_EVENT_LINK_ESTAB:
        if (event->link_estab.status)
            log_i("Gap event: link establishment failed with status %d",
                  event->link_estab.status);
        else
        {
            _conn_handle = event->link_estab.conn_handle;
            _connected = true;
            log_i("GAP event: link established, handle %u", _conn_handle);
            if (onConnectionStatus)
                onConnectionStatus(true);
        }
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING:
        log_i("GAP event: repeat pairing");
        // Delete the old bond
        ble_gap_conn_desc conn_info;
        if (ble_gap_conn_find(
                event->repeat_pairing.conn_handle,
                &conn_info) == 0)
        {
            log_i("Deleting old bond");
            ble_store_util_delete_peer(&conn_info.peer_id_addr);
        }
        // continue with the pairing protocol
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
        log_i("GAP event: new PHY: rx=%d, tx=%d",
              event->phy_updated.rx_phy,
              event->phy_updated.tx_phy);
    default:
        log_d("GAP event: %d", event->type);
        break;
    }
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wall"

void BLEAdvertising::init()
{
    ApiResult rc;

    // Configure data to include in subsequent advertisements.
    // Advertised data:
    // - flags
    // - BLE appearance
    // - HID service UUID
    // - Battery service UUID
    // - Device Information service UUID
    static const ble_uuid16_t service_uuid_list[]{
        BLEBatteryService::uuid,
        BLEDeviceInfoService::uuid,
        BLEHIDService::uuid,
    };
    static constexpr const uint8_t service_uuid_list_size =
        sizeof(service_uuid_list) / sizeof(ble_uuid16_t);
    static const ble_hs_adv_fields adv_fields{
        .flags = BLE_HS_ADV_F_DISC_GEN |
                 BLE_HS_ADV_F_BREDR_UNSUP,
        .uuids16 = service_uuid_list,
        .num_uuids16 = service_uuid_list_size,
        .uuids16_is_complete = 1,
        .appearance = BLE_APPEARANCE_GAMEPAD,
        .appearance_is_present = 1,
    };
    rc = ble_gap_adv_set_fields(&adv_fields);
    assert(rc);

    // Configure scan response data:
    // - Device name
    const char *device_name = ble_svc_gap_device_name();
    assert(device_name);
    const size_t device_name_size = strlen(device_name);
    if (device_name_size > 0)
    {
        ble_hs_adv_fields scan_rsp_fields{};
        scan_rsp_fields.name = (const uint8_t *)device_name;
        scan_rsp_fields.name_len =
            ::std::min<uint8_t>(device_name_size, BLE_HS_ADV_MAX_FIELD_SZ);
        scan_rsp_fields.name_is_complete =
            (scan_rsp_fields.name_len == device_name_size);
        rc = ble_gap_adv_rsp_set_fields(&scan_rsp_fields);
        assert(rc);
        log_i(
            "BLE: Device name '%s' added to scan response",
            device_name);
    }
    else
        log_i("BLE: Device name is empty (not advertised)");
}

#pragma GCC diagnostic pop

//------------------------------------------------------------------------------
// BLEDevice
//------------------------------------------------------------------------------

void BLEDevice::init(const std::string &deviceName)
{
    assert(!_initialized);
    // Initialize flash memory
    // (required to store PHY calibration data)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the NIMBLE stack
    ESP_ERROR_CHECK(nimble_port_init());

    // Initialize the NimBLE host configuration:
    // callbacks and security
    ble_hs_cfg.reset_cb = BLEDevice::onReset;
    ble_hs_cfg.sync_cb = BLEDevice::onSync;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 0;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;

    // Initialize GATT server
    init_gatt_server();

    ApiResult rc = ble_svc_gap_device_name_set(deviceName.c_str());
    rc.abort_if("BLE: unable to set the device name");

    rc = ble_svc_gap_device_appearance_set(BLE_APPEARANCE_GAMEPAD);
    assert(rc);

    // Initialize storage for BLE cryptographic keys.
    // Bonding will not work without this.
    ble_store_config_init();

    // Start the NimBLE task
    nimble_port_freertos_init(BLEDevice::host_task);

    // Wait for the host and controller to sync.
    // Using active wait to avoid having a single-use semaphore.
    while (!ready)
        ble_npl_time_delay(1);
    _initialized = true;
    log_i("BLEDevice::init() done");
}

void BLEDevice::init_gatt_server()
{
    ApiResult rc;

    // Initialize GAP and GATT services
    ble_svc_gap_init();
    ble_svc_gatt_init();
    BLEBatteryService::init();
    BLEDeviceInfoService::init();
    BLEHIDService::init();

    // Note:
    // this array must be static !!!
    // NimBLE does not make a copy of the pointed data
    static const ble_gatt_svc_def gatt_svr_svcs[] = {
        BLEBatteryService::definition(),
        BLEDeviceInfoService::definition(),
        BLEHIDService::definition(),
        EMPTY_ble_gatt_svc_def,
    };

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    rc.abort_if("ble_gatts_count_cfg() failed");

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    rc.abort_if("ble_gatts_add_svcs() failed");
}

void BLEDevice::onReset(int reason)
{
    ready = false;
    log_i("BLEDevice::onReset() with reason %d", reason);
}

void BLEDevice::onSync()
{
    if (!ready)
    {
        // Infer address type
        ApiResult rc = ble_hs_id_infer_auto(0, &address_type);
        assert(rc);
        log_i("BLEDevice::onSync(): address type %hhu", address_type);

        // Configure preferred PHY
        rc = ble_gap_set_prefered_default_le_phy(PREFERRED_PHY, PREFERRED_PHY);
        if (rc.code != BLE_HS_EDONE)
            rc.log_if("ble_gap_set_prefered_default_le_phy() failed");

        // Signal BLE controller ready
        ready = true;
        ble_npl_time_delay(1); // Force context switch
    }
}

void BLEDevice::host_task(void *param)
{
    log_i("BLE Host Task started");
    nimble_port_run();
    nimble_port_freertos_deinit();
    log_i("BLE Host Task finished");
}

//------------------------------------------------------------------------------
// BLEDesc2908
//------------------------------------------------------------------------------

int BLEDesc2908::onRead(os_mbuf *buffer)
{
    uint8_t data[] =
        {
            _report_id,
            (uint8_t)_report_type,
        };
    ApiResult rc = os_mbuf_append(
        buffer,
        data,
        sizeof(data));
    rc.log_if("BLEDesc2908::onRead() failed");
    return rc.to_attr_rc(true);
}

ble_gatt_dsc_def BLEDesc2908::definition()
{
    ble_gatt_dsc_def result = EMPTY_ble_gatt_dsc_def;
    result.uuid = &uuid.u;
    result.att_flags = BLE_ATT_F_READ;
    result.access_cb = BLEAccessor<BLEDesc2908, true>::access_fn;
    result.arg = (void *)this;
    return result;
}

void BLEDesc2908::set(uint8_t report_id, HIDReportType report_type)
{
    _report_id = report_id;
    _report_type = report_type;
}

//------------------------------------------------------------------------------
// BLECharacteristic
//------------------------------------------------------------------------------

void BLECharacteristic::notify() const
{
    assert(
        (attr_handle != INVALID_HANDLE) &&
        "BLE: notify() called but attr_handle not set");
    if (subscribed)
    {
        // Important note:
        // ble_gatts_chr_updated() must not be called
        // if the client is not subscribed due to a bug
        // in NimBLE.
        log_d("BLE: notify handle %d", attr_handle);
        ble_gatts_chr_updated(attr_handle);
    }
    else
        log_d("BLE: notify() ignored on attr handle %d (not subscribed)",
              attr_handle);
}

void BLECharacteristic::setSubscription_if(
    uint16_t requested_attr_handle,
    bool status)
{
    assert(
        (attr_handle != INVALID_HANDLE) &&
        "BLE: setSubscription_if() called but attr_handle not set");
    if (requested_attr_handle == attr_handle)
    {
        subscribed = status;
        log_d("BLE: new notification status %d on attr handle %d",
              status,
              attr_handle);
    }
}

//------------------------------------------------------------------------------
// BatteryLevelChr
//------------------------------------------------------------------------------

ble_gatt_chr_def BatteryLevelChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY;
    result.access_cb = BLEAccessor<BatteryLevelChr>::access_fn;
    result.cpfd = _chr_cpfd_def;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int BatteryLevelChr::onRead(os_mbuf *buffer)
{
    ApiResult rc = os_mbuf_append(
        buffer,
        &value,
        sizeof(value));
    rc.log_if("BatteryLevelChr::onRead() failed");
    return rc.to_attr_rc(true);
}

void BatteryLevelChr::set(uint8_t new_value)
{
    if (new_value > 100)
        new_value = 100;
    value = new_value;
    notify();
}

//------------------------------------------------------------------------------
// BatteryStatusChr
//------------------------------------------------------------------------------

ble_gatt_chr_def BatteryStatusChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY;
    result.access_cb = BLEAccessor<BatteryStatusChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int BatteryStatusChr::onRead(os_mbuf *buffer)
{
    ApiResult rc = os_mbuf_append(
        buffer,
        &value,
        sizeof(value));
    rc.log_if("BatteryStatusChr::onRead() failed");
    return rc.to_attr_rc(true);
}

void BatteryStatusChr::set(const BatteryStatusChrData &new_value)
{
    value = new_value;
    notify();
}

//------------------------------------------------------------------------------
// BLEBatteryService
//------------------------------------------------------------------------------

const ble_gatt_svc_def BLEBatteryService::definition()
{
    chr_set[0] = batteryLevel.definition();
    chr_set[1] = batteryStatus.definition();
    chr_set[2] = EMPTY_ble_gatt_chr_def;
    ble_gatt_svc_def result = EMPTY_ble_gatt_svc_def;
    result.type = BLE_GATT_SVC_TYPE_PRIMARY;
    result.uuid = &uuid.u;
    result.characteristics = chr_set;
    return result;
}

void BLEBatteryService::init()
{
    // Nothing to do here.
    // Reserved for future use.
}

//------------------------------------------------------------------------------
// PnpInfoChr
//------------------------------------------------------------------------------

ble_gatt_chr_def PnpInfoChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<PnpInfoChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int PnpInfoChr::onRead(os_mbuf *buffer)
{
    const uint8_t pnpInfo[] = {
        BLE_VENDOR_SOURCE,
        static_cast<uint8_t>(vid),
        static_cast<uint8_t>(vid >> 8),
        static_cast<uint8_t>(pid),
        static_cast<uint8_t>(pid >> 8),
        static_cast<uint8_t>(PRODUCT_REVISION),
        static_cast<uint8_t>(PRODUCT_REVISION >> 8),
    };
    ApiResult rc = os_mbuf_append(buffer, &pnpInfo, sizeof(pnpInfo));
    rc.log_if("PnpInfoChr::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

//------------------------------------------------------------------------------
// SerialNumberChr
//------------------------------------------------------------------------------

ble_gatt_chr_def SerialNumberChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<SerialNumberChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int SerialNumberChr::onRead(os_mbuf *buffer)
{
    // Retrieve the serial number
    uint64_t serial_number = 0ULL;
    if (esp_efuse_mac_get_default((uint8_t *)(&serial_number)) != ESP_OK)
        log_i("BLE: unable to retrieve a serial number. Using zero.");
    char serialAsStr[9];
    memset(serialAsStr, 0, 9);
    snprintf(serialAsStr, 9, "%08llX", serial_number);
    // send serial number
    ApiResult rc = os_mbuf_append(buffer, serialAsStr, 9);
    rc.log_if("SerialNumberChr::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

//------------------------------------------------------------------------------
// ManufacturerChr
//------------------------------------------------------------------------------

ble_gatt_chr_def ManufacturerChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<ManufacturerChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int ManufacturerChr::onRead(os_mbuf *buffer)
{
    ApiResult rc = os_mbuf_append(buffer, name.c_str(), name.size());
    rc.log_if("ManufacturerChr::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

//------------------------------------------------------------------------------
// BLEDeviceInfoService
//------------------------------------------------------------------------------

const ble_gatt_svc_def BLEDeviceInfoService::definition()
{
    chr_set[0] = pnpInfo.definition();
    chr_set[1] = serialNumber.definition();
    chr_set[2] = manufacturer.definition();
    chr_set[3] = EMPTY_ble_gatt_chr_def;
    ble_gatt_svc_def result = EMPTY_ble_gatt_svc_def;
    result.type = BLE_GATT_SVC_TYPE_PRIMARY;
    result.uuid = &uuid.u;
    result.characteristics = chr_set;
    return result;
}

void BLEDeviceInfoService::init()
{
    // Nothing to do here.
    // Reserved for future use.
}

//------------------------------------------------------------------------------
// FeatureReportChr
//------------------------------------------------------------------------------

FeatureReportChr::FeatureReportChr(uint8_t report_id, size_t size) noexcept
{
    desc2908.set(report_id, HIDReportType::feature);
    report_size = size;
    log_d("Creating HID feature report id %hhu, size %u", report_id, size);
}

ble_gatt_chr_def FeatureReportChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    desc_def[0] = desc2908.definition();
    desc_def[1] = EMPTY_ble_gatt_dsc_def;
    result.uuid = &uuid.u;
    result.flags =
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC |
        BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC;
    result.access_cb = BLEHIDService::report_access_fn;
    result.val_handle = &attr_handle;
    result.descriptors = desc_def;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

//------------------------------------------------------------------------------
// OutputReportChr
//------------------------------------------------------------------------------

OutputReportChr::OutputReportChr(uint8_t report_id, size_t size) noexcept
{
    desc2908.set(report_id, HIDReportType::output);
    report_size = size;
    log_d("Creating HID output report id %hhu, size %u", report_id, size);
}

ble_gatt_chr_def OutputReportChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    desc_def[0] = desc2908.definition();
    desc_def[1] = EMPTY_ble_gatt_dsc_def;
    result.uuid = &uuid.u;
    result.flags =
        BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC |
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC |
        BLE_GATT_CHR_F_WRITE_NO_RSP;
    result.access_cb = BLEHIDService::report_access_fn;
    result.val_handle = &attr_handle;
    result.descriptors = desc_def;
    result.arg = (void *)this;
    return result;
}

//------------------------------------------------------------------------------
// InputReportChr
//------------------------------------------------------------------------------

InputReportChr::InputReportChr(uint8_t report_id, size_t size) noexcept
{
    desc2908.set(report_id, HIDReportType::input);
    report_size = size;
    log_d("Creating HID input report id %hhu, size %u", report_id, size);
}

ble_gatt_chr_def InputReportChr::definition()
{
    assert(report_size > 0);
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    desc_def[0] = desc2908.definition();
    desc_def[1] = EMPTY_ble_gatt_dsc_def;
    result.uuid = &uuid.u;
    result.flags =
        BLE_GATT_CHR_F_READ |
        BLE_GATT_CHR_F_READ_ENC |
        BLE_GATT_CHR_F_NOTIFY;
    result.access_cb = BLEHIDService::report_access_fn;
    result.val_handle = &attr_handle;
    result.descriptors = desc_def;
    result.arg = (void *)this;
    return result;
}

//------------------------------------------------------------------------------
// InfoChr
//------------------------------------------------------------------------------

ble_gatt_chr_def HIDInfoChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<HIDInfoChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int HIDInfoChr::onRead(os_mbuf *buffer)
{
    static const uint8_t hid_info[] = {
        0x11,
        0x01,
        0x00, // Country code
        0x01, // Spec release number (flags)
    };
    ApiResult rc = os_mbuf_append(buffer, hid_info, sizeof(hid_info));
    rc.log_if("HIDInfoChr::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

//------------------------------------------------------------------------------
// HIDControlChr
//------------------------------------------------------------------------------

ble_gatt_chr_def HIDControlChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_WRITE_NO_RSP;
    result.access_cb = BLEAccessor<HIDControlChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

//------------------------------------------------------------------------------
// HIDReportMapChr
//------------------------------------------------------------------------------

ble_gatt_chr_def HIDReportMapChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<HIDReportMapChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int HIDReportMapChr::onRead(os_mbuf *buffer)
{
    ApiResult rc = os_mbuf_append(
        buffer,
        hid_descriptor,
        sizeof(hid_descriptor));
    rc.log_if("HIDReportMapChr::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

//------------------------------------------------------------------------------
// HIDProtocolModeChr
//------------------------------------------------------------------------------

ble_gatt_chr_def HIDProtocolModeChr::definition()
{
    ble_gatt_chr_def result = EMPTY_ble_gatt_chr_def;
    result.uuid = &uuid.u;
    result.flags = BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_READ;
    result.access_cb = BLEAccessor<HIDProtocolModeChr>::access_fn;
    result.val_handle = &attr_handle;
    result.arg = (void *)this;
    return result;
}

int HIDProtocolModeChr::onRead(os_mbuf *buffer)
{
    ApiResult rc = os_mbuf_append(buffer, &value, sizeof(value));
    rc.log_if("BLEHIDService::ProtocolModeCh::onReadCallback() failed");
    return rc.to_attr_rc(true);
}

int HIDProtocolModeChr::onWrite(void *data, uint16_t data_size)
{
    if (data_size != sizeof(value))
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    ::std::memcpy(&value, data, data_size);
    return 0;
}

//------------------------------------------------------------------------------
// BLEHIDService
//------------------------------------------------------------------------------

int BLEHIDService::report_access_fn(
    uint16_t conn_handle,
    uint16_t attr_handle,
    ble_gatt_access_ctxt *ctxt,
    void *arg)
{
    assert(arg);
    HIDReportType report_type =
        static_cast<HIDReportChr *>(arg)->desc2908.report_type();
    uint8_t report_id =
        static_cast<HIDReportChr *>(arg)->desc2908.report_id();
    log_d(
        "Access to HID report ID %hd, type %hhd, op=%d",
        report_id,
        (uint8_t)report_type,
        ctxt->op);
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        uint16_t report_size = static_cast<HIDReportChr *>(arg)->report_size;
        assert(report_size > 0);
        uint8_t data[report_size];
        uint16_t used_size;
        switch (report_type)
        {
        case HIDReportType::feature:
            if (onGetFeatureReport)
                used_size = onGetFeatureReport(
                    report_id,
                    data,
                    report_size);
            else
                return BLE_ATT_ERR_UNLIKELY;
            break;
        case HIDReportType::output:
            /// Note: despite output reports are "write-only",
            /// the GATT specification requires characteristics
            /// having write permission to also grant read permission.
            /// So, this event is not an error condition.
            log_i("BLE: Read operation on output report %d", report_id);
            return BLE_ATT_ERR_READ_NOT_PERMITTED;
        case HIDReportType::input:
            if (onReadInputReport)
                used_size = onReadInputReport(
                    report_id,
                    data,
                    report_size);
            else
                return BLE_ATT_ERR_UNLIKELY;
            break;
        default:
            return BLE_ATT_ERR_UNLIKELY;
        }
        assert(used_size <= report_size);
        ApiResult rc = os_mbuf_append(
            ctxt->om,
            data,
            used_size);
        rc.log_if("BLEHIDService:report_access_fn -> read op failed");
        return rc.to_attr_rc(true);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        switch (report_type)
        {
        case HIDReportType::feature:
            if (onSetFeatureReport)
                onSetFeatureReport(
                    report_id,
                    ctxt->om->om_data,
                    ctxt->om->om_len);
            return 0;
        case HIDReportType::output:
            if (onWriteOutputReport)
                onWriteOutputReport(
                    report_id,
                    ctxt->om->om_data,
                    ctxt->om->om_len);
            return 0;
        case HIDReportType::input:
            log_e("BLE: attempt to write input report id %d", report_id);
            return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
        default:
            break;
        }
    }
    return BLE_ATT_ERR_UNLIKELY;
}

const ble_gatt_svc_def BLEHIDService::definition()
{
    int index = 0;
    // Mandatory HID characteristics
    chr_set[index++] = info.definition();
    chr_set[index++] = control.definition();
    chr_set[index++] = report_map.definition();
    chr_set[index++] = protocol_mode.definition();
    // HID report characteristics
    chr_set[index++] = input_report.definition();
    chr_set[index++] = capabilities_report.definition();
    chr_set[index++] = config_report.definition();
    chr_set[index++] = button_map_report.definition();
    chr_set[index++] = hardware_id_report.definition();
    chr_set[index++] = powertrain_report.definition();
    chr_set[index++] = ecu_report.definition();
    chr_set[index++] = race_control_report.definition();
    chr_set[index++] = gauges_report.definition();
    chr_set[index++] = pixel_report.definition();
    chr_set[index++] = simhub_hid_protocol_report.definition();
    chr_set[index++] = EMPTY_ble_gatt_chr_def;
    assert(
        ((sizeof(chr_set) / sizeof(ble_gatt_chr_def)) == index + 1) &&
        "Invalid size of BLEHIDService::chr_set array ");
    ble_gatt_svc_def result = EMPTY_ble_gatt_svc_def;
    result.type = BLE_GATT_SVC_TYPE_PRIMARY;
    result.uuid = &uuid.u;
    result.characteristics = chr_set;
    return result;
}

void BLEHIDService::init()
{
    // Nothing to do here.
    // Reserved for future use.
}

#endif // CONFIG_NIMBLE_ENABLED