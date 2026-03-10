/**
 * @file NimBLEWrapper.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-12-14
 * @brief NimBLE custom wrapper (for HID implementation only)
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//------------------------------------------------------------------------------
// Conditional compilation
//------------------------------------------------------------------------------

#include <sdkconfig.h> // For conditional compilation
#include "SimWheelInternals.hpp" // BatteryStatusChrData

#if CONFIG_NIMBLE_ENABLED

//------------------------------------------------------------------------------
// Imports
//------------------------------------------------------------------------------

#include <cstdint>
#include <string>
#include <functional>
#include <initializer_list>
#include <type_traits>

#include "host/ble_att.h"  // ATT layer
#include "host/ble_gatt.h" // GATT layer
#include "host/ble_uuid.h" // UUIDs
#include "host/ble_gap.h"  // GAP layer
#include "host/ble_gap.h"  // GAP layer
#include "os/os_mbuf.h"    // Attribute buffers
#include "esp32-hal-log.h" // Logging

#include "HID_definitions.hpp"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

#define NIMBLE_HID_REPORT_COUNT 10

#define EMPTY_ble_gatt_cpfd { \
    .format = 0,              \
    .exponent = 0,            \
    .unit = 0,                \
    .name_space = 0,          \
    .description = 0}

#define EMPTY_ble_gatt_dsc_def { \
    .uuid = nullptr,             \
    .att_flags = 0,              \
    .min_key_size = 0,           \
    .access_cb = nullptr,        \
    .arg = nullptr}

#define EMPTY_ble_gatt_chr_def { \
    .uuid = nullptr,             \
    .access_cb = nullptr,        \
    .arg = nullptr,              \
    .descriptors = nullptr,      \
    .flags = 0,                  \
    .min_key_size = 0,           \
    .val_handle = nullptr,       \
    .cpfd = nullptr}

#define EMPTY_ble_gatt_svc_def { \
    .type = 0,                   \
    .uuid = nullptr,             \
    .includes = nullptr,         \
    .characteristics = nullptr}

//------------------------------------------------------------------------------
// Classes
//------------------------------------------------------------------------------

/**
 * @brief Result of a NimBLE function
 *
 */
struct ApiResult
{
    /// @brief Result code
    int code = 0;
    /// @brief Create a non-error result
    constexpr ApiResult() noexcept {};
    /// @brief Create a result code
    constexpr ApiResult(int value) noexcept : code{value} {};
    /// @brief Copy a result code
    constexpr ApiResult(const ApiResult &) noexcept = default;
    /// @brief Transfer a result code
    constexpr ApiResult(ApiResult &&) noexcept = default;

    /**
     * @brief Check if the result code is an error code
     *
     * @return true if not an error code
     * @return false if an error code
     */
    constexpr operator bool() const noexcept { return (code == 0); }

    /**
     * @brief Get the result code
     *
     * @return int Result code
     */
    constexpr explicit operator int() const noexcept { return code; }

    /**
     * @brief Copy-assign
     *
     * @return constexpr ApiResult& This instance
     */
    constexpr ApiResult &operator=(const ApiResult &) noexcept = default;

    /**
     * @brief Move-assign
     *
     * @return constexpr ApiResult& This instance
     */
    constexpr ApiResult &operator=(ApiResult &&) noexcept = default;

    /**
     * @brief Assign a new result code
     *
     * @param value New result code
     * @return constexpr ApiResult& This instance
     */
    constexpr ApiResult &operator=(int value) noexcept;

    /**
     * @brief Abort if the result code is an error code
     *
     * @param txt Error message
     */
    void abort_if(const char *txt) const;

    /**
     * @brief Log a debug message if the result code is an error code
     *
     * @param txt Debug message
     */
    void log_if(const char *txt) const noexcept;

    /**
     * @brief Transform this API result code into a BLE attribute
     *        operation result code
     *
     * @param readOrWrite True if a read operation, false otherwise.
     * @return int Either BLE_ATT_ERR_INSUFFICIENT_RES or zero
     */
    int to_attr_rc(bool readOrWrite) const noexcept;
}; // ApiResult

/**
 * @brief Device advertising (GAP)
 *
 */
struct BLEAdvertising
{
    friend struct BLEDevice;

    /// @brief Type of the status callback function
    using OnConnectionStatus = ::std::function<void(bool)>;

    /// @brief Callback function to be called when the connection status changes
    inline static OnConnectionStatus onConnectionStatus;

    /// @brief Start advertising
    static void start();

    /// @brief Stop advertising
    static void stop() noexcept;

    /// @brief Check if a connection is established
    /// @return True if there is a connection
    static bool connected() noexcept { return _connected; }

    /// @brief Terminate connection (if any)
    static void disconnect() noexcept;

private:
    /// @brief Advertising parameters required by the stack
    inline static const ble_gap_adv_params adv_params{
        .conn_mode = BLE_GAP_CONN_MODE_UND, // Undirected, connectable
        .disc_mode = BLE_GAP_DISC_MODE_GEN, // General discoverable
        .itvl_min = 0,
        .itvl_max = 0,
        .channel_map = 0,
        .filter_policy = 0,
        .high_duty_cycle = 0,
    };

    /// @brief Current connection status
    inline static bool _connected = false;

    /// @brief Current connection handle
    inline static int16_t _conn_handle = BLE_HS_CONN_HANDLE_NONE;

    /// @brief Callback function for GAP events
    /// @param event Event
    /// @param arg Optional argument
    /// @return Result code
    static int ble_gap_event_fn(ble_gap_event *event, void *arg);

    /// @brief Initialize GAP
    static void init();
}; // struct BLEAdvertising

/**
 * @brief Bluetooth device
 *
 */
struct BLEDevice
{
    friend class BLEAdvertising;

    /**
     * @brief Initialization status
     *
     * @return true If initialized
     * @return false If not initialized
     */
    static bool initialized() { return _initialized; };

    /**
     * @brief Initialize the BLE stack
     *
     * @param deviceName Device name
     */
    static void init(const std::string &deviceName);

private:
    /// @brief Bluetooth address type configured for this device
    inline static uint8_t address_type = 0;

    /// @brief BLE controller availability
    inline static bool ready = false;

    /// @brief Initialization state
    inline static bool _initialized = false;

    /// @brief Initialize the GATT server
    static void init_gatt_server();

    /// @brief Callback to be called when the controller resets
    /// @param reason Reason code
    static void onReset(int reason);

    /// @brief Callback for host-controller synchronization
    static void onSync();

    /// @brief BLE daemon
    /// @param param Not used
    static void host_task(void *param);
}; // struct BLEDevice

/// @brief Interface for reading characteristic/descriptor values
struct BLEReadCallback
{
    virtual int onRead(os_mbuf *buffer) { return 0; };
};

/// @brief Interface for writing characteristic/descriptor values
struct BLEWriteCallback
{
    virtual int onWrite(void *data, uint16_t data_size) { return 0; };
};

/**
 * @brief API callback to access characteristic/descriptor values
 *
 * @tparam T Type of the instances passed as argument to access_fn()
 * @tparam is_descriptor True if T represents a descriptor
 */
template <
    typename T,
    bool is_descriptor = false>
struct BLEAccessor
{
    /// @brief T responds to read attempts
    static constexpr bool can_read =
        ::std::is_base_of<BLEReadCallback, T>::value;
    /// @brief T responds to write attemps
    static constexpr bool can_write =
        ::std::is_base_of<BLEWriteCallback, T>::value;

    static_assert(
        can_read || can_write,
        "BLE: Accessor does not read or write");

    /**
     * @brief Callback to access characteristic/descriptor values
     *
     * @param conn_handle Connection handle
     * @param attr_handle Attribute value
     * @param ctxt Context
     * @param arg Instance of T
     * @return int Result code
     */
    static int access_fn(
        uint16_t conn_handle,
        uint16_t attr_handle,
        ble_gatt_access_ctxt *ctxt,
        void *arg)
    {
        assert(arg);
        log_d(
            "BLEAccessor::access_fn(%d,%d,...), op=%d",
            conn_handle,
            attr_handle,
            ctxt->op);
        if constexpr (can_read)
        {
            if constexpr (is_descriptor)
            {
                if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC)
                    return static_cast<T *>(arg)->onRead(ctxt->om);
            }
            else
            {
                if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
                    return static_cast<T *>(arg)->onRead(ctxt->om);
            }
            log_e("Unexpected READ operation on attr_handle %d", attr_handle);
        }
        if constexpr (can_write)
        {

            if constexpr (is_descriptor)
            {
                if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_DSC)
                    return static_cast<T *>(arg)->onWrite(ctxt->om);
            }
            else
            {
                if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
                    return static_cast<T *>(arg)->onWrite(
                        ctxt->om->om_data,
                        ctxt->om->om_len);
            }
            log_e("Unexpected WRITE operation on attr_handle %d", attr_handle);
        }
        return BLE_ATT_ERR_UNLIKELY;
    }
};

/// @brief HID report types (input/output/feature)
enum class HIDReportType : uint8_t
{
    input = 1,
    output = 2,
    feature = 3
};

/// @brief HID report meta-data descriptor (id and type)
struct BLEDesc2908 : BLEReadCallback
{

    /// @brief Descriptor UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2908);

    /**
     * @brief Get the descriptor definition as required by NimBLE
     *
     * @return ble_gatt_dsc_def Definition
     */
    ble_gatt_dsc_def definition();

    /**
     * @brief Set the HID report ID and type
     * @warning Must be called before BLE initialization
     *
     * @param report_type Report ID
     * @param report_type Report type
     */
    void set(uint8_t report_id, HIDReportType report_type);

    virtual int onRead(os_mbuf *buffer) override;

    /**
     * @brief Current report ID
     *
     * @return uint8_t Report ID
     */
    uint8_t report_id() { return _report_id; }

    /**
     * @brief Current report type
     *
     * @return HIDReportType Report type
     */
    HIDReportType report_type() { return _report_type; }

private:
    /// @brief Current report type
    HIDReportType _report_type = HIDReportType::feature;
    uint8_t _report_id = 0;
}; // BLEDesc2908

/// @brief Generic BLE characteristic
struct BLECharacteristic
{
    static constexpr const uint16_t INVALID_HANDLE = 0xFFFF;
    /**
     * @brief Retrieve the API definition of the characteristic
     *
     * @return ble_gatt_chr_def Definition.
     *         The life time of this value must match the lifetime
     *         of this object.
     */
    virtual ble_gatt_chr_def definition() = 0;

    /**
     * @brief Notify or indicate a change
     *        in the characteristic to the subscriber
     */
    void notify() const;

    /**
     * @brief Set the subscription status if it corresponds to this
     *        characteristic
     * @note Ignored if the attribute handle
     *       does not match @p requested_attr_handle
     *
     * @param requested_attr_handle Attribute handle.
     * @param status True on subscription, false otherwise
     */
    void setSubscription_if(uint16_t requested_attr_handle, bool status);

protected:
    /// @brief Subscription status
    bool subscribed = false;
    /**
     * @brief Attribute handle
     * @warning Must be filled by descendant classes
     *          to avoid having a constructor
     *          unless notify/indicate is not needed
     */
    uint16_t attr_handle = INVALID_HANDLE;
};

/// @brief Battery Level Characteristic
struct BatteryLevelChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A19);

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;

    /**
     * @brief Set and report the current battery level
     *
     * @param new_value Battery level in the range [0,100]
     */
    void set(uint8_t new_value);

    /**
     * @brief Get the last notified battery level
     *
     * @return uint8_t Battery level in the range [0,100]
     */
    uint8_t get() const noexcept { return value; }

private:
    inline static ble_gatt_cpfd _chr_cpfd_def[]{
        {
            .format = 4, // uint8
            .exponent = 0,
            .unit = 0x27AD,       // percentage
            .name_space = 0x01,   // Bluetooth SIG
            .description = 0x106, // main namespace
        },
        EMPTY_ble_gatt_cpfd,
    };
    uint8_t value{100};
    ble_gatt_dsc_def dsc_def[2]{};
};

/// @brief Battery Status characteristic
struct BatteryStatusChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2BED);

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;

    /**
     * @brief Set the value of the characteristic
     *
     * @param new_value New value
     */
    void set(const BatteryStatusChrData &new_value);

private:
    /// @brief Value of the characteristic
    BatteryStatusChrData value{};
};

/**
 * @brief Battery Service
 *
 */
struct BLEBatteryService
{
    /// @brief Service UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x180F);

    /// @brief Battery level characteristic
    inline static BatteryLevelChr batteryLevel{};
    /// @brief Battery status characteristic
    inline static BatteryStatusChr batteryStatus{};

    /**
     * @brief Service definition as required by NimBLE
     *
     * @return const ble_gatt_svc_def Definition
     */

    static const ble_gatt_svc_def definition();

    /// @brief Initialize
    static void init();

    /**
     * @brief Handle characteristic subscriptions
     * @note Called from the GAP server when the client
     *       subscribes or cancels subscription to a characteristic.
     *
     * @param attr_handle Attribute handle
     * @param yesOrNo True on subscription, false otherwise.
     */
    static void onSubscriptionChange(
        uint16_t attr_handle,
        bool yesOrNo)
    {
        batteryLevel.setSubscription_if(attr_handle, yesOrNo);
        batteryStatus.setSubscription_if(attr_handle, yesOrNo);
    }

private:
    inline static ble_gatt_chr_def chr_set[3]{};
};

/// @brief Plug-and-play information characteristic
struct PnpInfoChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A50);

    /// @brief Vendor ID
    uint16_t vid = BLE_VENDOR_ID;
    /// @brief Product ID
    uint16_t pid = BLE_PRODUCT_ID;

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;
};

/// @brief Serial Number characteristic
struct SerialNumberChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A25);

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;
};

/// @brief Manufacturer's name characteristic
struct ManufacturerChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A29);
    /// @brief Current manufacturer's name
    ::std::string name{};

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;
};

/**
 * @brief Battery Service
 *
 */
struct BLEDeviceInfoService
{
    /// @brief Service UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x180A);

    /// @brief PNP Info characteristic
    inline static PnpInfoChr pnpInfo;

    /// @brief Serial Number characteristic
    inline static SerialNumberChr serialNumber;

    /// @brief Manufacturer's name characteristic
    inline static ManufacturerChr manufacturer;

    /// @brief Service definition as required by NimBLE
    static const ble_gatt_svc_def definition();

    /// @brief Initialize
    static void init();

private:
    inline static ble_gatt_chr_def chr_set[4]{};
};

/// @brief Base class for all HID report characteristics
struct HIDReportChr : BLECharacteristic
{
    friend class BLEHIDService;

    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A4D);

protected:
    BLEDesc2908 desc2908;
    ble_gatt_dsc_def desc_def[2]{};
    size_t report_size = 0;
};

/// @brief HID feature report characteristic
struct FeatureReportChr : HIDReportChr
{
    FeatureReportChr(uint8_t report_id, size_t size) noexcept;
    FeatureReportChr(const FeatureReportChr &) noexcept = default;
    FeatureReportChr(FeatureReportChr &&) noexcept = default;
    FeatureReportChr &operator=(const FeatureReportChr &) noexcept = default;
    FeatureReportChr &operator=(FeatureReportChr &&) noexcept = default;

    virtual ble_gatt_chr_def definition() override;
};

/// @brief HID feature report characteristic
struct OutputReportChr : HIDReportChr
{
    OutputReportChr(uint8_t report_id, size_t size) noexcept;
    OutputReportChr(const OutputReportChr &) noexcept = default;
    OutputReportChr(OutputReportChr &&) noexcept = default;
    OutputReportChr &operator=(const OutputReportChr &) noexcept = default;
    OutputReportChr &operator=(OutputReportChr &&) noexcept = default;

    virtual ble_gatt_chr_def definition() override;
};

/// @brief HID feature report characteristic
struct InputReportChr : HIDReportChr
{
    InputReportChr(uint8_t report_id, size_t size) noexcept;
    InputReportChr(const InputReportChr &) noexcept = default;
    InputReportChr(InputReportChr &&) noexcept = default;
    InputReportChr &operator=(const InputReportChr &) noexcept = default;
    InputReportChr &operator=(InputReportChr &&) noexcept = default;
    virtual ble_gatt_chr_def definition() override;
};

/// @brief HID information characteristic
struct HIDInfoChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A4A);

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;
};

/// @brief HID control point characteristic
/// @note Current implementation ignores write attemps without error
struct HIDControlChr : BLECharacteristic, BLEWriteCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A4C);

    virtual ble_gatt_chr_def definition() override;
};

/// @brief HID report map characteristic
struct HIDReportMapChr : BLECharacteristic, BLEReadCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A4B);

    virtual int onRead(os_mbuf *buffer) override;
    virtual ble_gatt_chr_def definition() override;
};

// TO-DO
// - Consider return BLE_ATT_ERR_VALUE_NOT_ALLOWED on write attempts
//   as "boot mode" does not make sense in this device
// - Check if this characteristic is mandatory or not

/// @brief HID protocol mode characteristic
struct HIDProtocolModeChr : BLECharacteristic, BLEReadCallback, BLEWriteCallback
{
    /// @brief Characteristic UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x2A4E);

    virtual int onRead(os_mbuf *buffer) override;
    virtual int onWrite(void *data, uint16_t data_size) override;
    virtual ble_gatt_chr_def definition() override;

private:
    /// @brief Current protocol mode
    uint8_t value = 0x01; // Non-boot mode
};

/**
 * @brief HID service
 *
 */
struct BLEHIDService
{
    friend class FeatureReportChr;
    friend class OutputReportChr;
    friend class InputReportChr;

    /// @brief Service UUID
    inline static const ble_uuid16_t uuid = BLE_UUID16_INIT(0x1812);

    /// @brief Get-feature-report callback type
    using OnGetFeatureCallback =
        // params(reportID,[out]data,data_size)
        // return count of bytes written into "data"
        ::std::function<uint16_t(uint8_t, uint8_t *, uint16_t)>;
    /// @brief Set-feature-report callback type
    using OnSetFeatureCallback =
        // params(reportID,data,data_size)
        ::std::function<void(uint8_t, const uint8_t *, uint16_t)>;
    /// @brief Write-output-report callback type
    using OnOutputCallback = OnSetFeatureCallback;
    /// @brief Read-input-report callback type
    using OnInputCallback = OnGetFeatureCallback;

    /// @brief Global callback to get a feature report
    inline static OnGetFeatureCallback onGetFeatureReport = nullptr;
    /// @brief Global callback to set a feature report
    inline static OnSetFeatureCallback onSetFeatureReport = nullptr;
    /// @brief Global callback to write an output report
    inline static OnOutputCallback onWriteOutputReport = nullptr;
    /// @brief Global callback to read an input report
    inline static OnInputCallback onReadInputReport = nullptr;

    /// @brief HID information
    inline static HIDInfoChr info;
    /// @brief HID control
    inline static HIDControlChr control;
    /// @brief HID report map
    inline static HIDReportMapChr report_map;
    /// @brief HID protocol mode
    inline static HIDProtocolModeChr protocol_mode;

    inline static InputReportChr input_report{
        RID_INPUT_GAMEPAD,
        GAMEPAD_REPORT_SIZE};
    inline static FeatureReportChr capabilities_report{
        RID_FEATURE_CAPABILITIES,
        CAPABILITIES_REPORT_SIZE};
    inline static FeatureReportChr config_report{
        RID_FEATURE_CONFIG,
        CONFIG_REPORT_SIZE};
    inline static FeatureReportChr button_map_report{
        RID_FEATURE_BUTTONS_MAP,
        BUTTONS_MAP_REPORT_SIZE};
    inline static FeatureReportChr hardware_id_report{
        RID_FEATURE_HARDWARE_ID,
        HARDWARE_ID_REPORT_SIZE};
    inline static OutputReportChr powertrain_report{
        RID_OUTPUT_POWERTRAIN,
        POWERTRAIN_REPORT_SIZE};
    inline static OutputReportChr ecu_report{
        RID_OUTPUT_ECU,
        ECU_REPORT_SIZE};
    inline static OutputReportChr race_control_report{
        RID_OUTPUT_RACE_CONTROL,
        RACE_CONTROL_REPORT_SIZE};
    inline static OutputReportChr gauges_report{
        RID_OUTPUT_GAUGES,
        GAUGES_REPORT_SIZE};
    inline static OutputReportChr pixel_report{
        RID_OUTPUT_PIXEL,
        PIXEL_REPORT_SIZE};

    /**
     * @brief Service definition as required by NimBLE
     *
     * @return const ble_gatt_svc_def Definition
     */
    static const ble_gatt_svc_def definition();

    /// @brief Initialize
    static void init();

    /**
     * @brief Handle characteristic subscriptions
     * @note Called from the GAP server when the client
     *       subscribes or cancels subscription to a characteristic.
     *
     * @param attr_handle Attribute handle
     * @param yesOrNo True on subscription, false otherwise.
     */
    static void onSubscriptionChange(
        uint16_t attr_handle,
        bool yesOrNo)
    {
        input_report.setSubscription_if(attr_handle, yesOrNo);
    }

private:
    inline static ble_gatt_chr_def chr_set[4 + NIMBLE_HID_REPORT_COUNT + 1]{};
    static int report_access_fn(
        uint16_t conn_handle,
        uint16_t attr_handle,
        ble_gatt_access_ctxt *ctxt,
        void *arg);
};

#endif // CONFIG_NIMBLE_ENABLED
