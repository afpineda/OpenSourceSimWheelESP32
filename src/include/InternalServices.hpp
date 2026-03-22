/**
 * @file InternalServices.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Internal service interfaces for dependency injection
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InternalTypes.hpp"
#include <cstdint>
#include <type_traits>

//-------------------------------------------------------------------
// Dependency manager
//-------------------------------------------------------------------

/**
 * @brief Dependency Manager
 *
 * @tparam Service Service interface
 */
template <typename Service>
struct DependencyManager
{
    /**
     * @brief Get the service provider implementing the service
     *
     * @return Service& Service provider
     */
    static Service &call()
    {
        if constexpr (!::std::is_default_constructible<Service>::value)
        {
            if (_singleton)
                return *_singleton;
        }
        else
        {
            if (!_singleton)
                _singleton = new Service();
            return *_singleton;
        }
        throw ::std::runtime_error("Service provider not injected");
    }

    /**
     * @brief Inject a service provider
     *
     * @tparam Provider Service provider class
     * @tparam _Args Constructor parameter types
     * @param __args Constructor parameters
     */
    template <class Provider, typename... _Args>
    static void inject(_Args &&...__args)
    {

        static_assert(
            std::is_base_of<Service, Provider>::value,
            "Provider does not implement Service");
        inject(new Provider(::std::forward<_Args>(__args)...));
    }

    /**
     * @brief Inject a service provider
     *
     * @param provider Service provider instance. Must live forever.
     */
    static void inject(Service *provider)
    {
        if (provider)
        {
            if (!_singleton)
                _singleton = provider;
            else
                throw ::std::runtime_error(
                    "Provider already injected to service");
        }
        else
            throw ::std::runtime_error("A null provider is not allowed");
    }

    /**
     * @brief Remove the injected service provider (for testing)
     *
     * @warning For testing only. Do not call in production code.
     *          May cause memory leaks.
     */
    static void reset()
    {
        _singleton = nullptr;
    }

private:
    inline static Service *_singleton = nullptr;
};

//-------------------------------------------------------------------
// Input service
//-------------------------------------------------------------------

/**
 * @brief Input hardware services
 *
 */
class InputService : public DependencyManager<InputService>
{
public:
    /**
     * @brief Force auto-calibration of all axes (analog clutch paddles)
     *
     */
    virtual void recalibrateAxes() {}

    /**
     * @brief Change polarity of left axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    virtual void reverseLeftAxis() {}

    /**
     * @brief Change polarity of right axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    virtual void reverseRightAxis() {}

    /**
     * @brief Multiply the pulse width of rotary encoders
     *
     * @note Valid values are saved to flash memory without delay.
     * @param multiplier A pulse width multiplier greater than zero.
     *                   Valid values are between 1 and 6.
     *                   Invalid values are ignored.
     * @param save True to save to flash memory
     */
    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save = true) {}

    /**
     * @brief Get the current pulse width multiplier for rotary encoders
     *
     * @return PulseWidthMultiplier A pulse width multiplier greater than zero.
     */
    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier()
    {
        return (PulseWidthMultiplier::X1);
    }

    /**
     * @brief Get current axis calibration data
     *
     * @note To be called from the storage namespace
     *
     * @param[out] minLeft minimum ADC reading for the left clutch
     * @param[out] maxLeft maximum ADC reading for the left clutch
     * @param[out] minRight minimum ADC reading for the right clutch
     * @param[out] maxRight maximum ADC reading for the right clutch
     * @return true if there are analog clutch paddles
     * @return true if there are no analog clutch paddles,
     *         so the output parameters were not set.
     */
    virtual bool getAxisCalibration(
        int &minLeft,
        int &maxLeft,
        int &minRight,
        int &maxRight) { return (false); }

    /**
     * @brief Set the Axis calibration data
     *
     * @note To be called from the storage namespace
     *
     * @param[in] minLeft minimum ADC reading for the left clutch
     * @param[in] maxLeft maximum ADC reading for the left clutch
     * @param[in] minRight minimum ADC reading for the right clutch
     * @param[in] maxRight maximum ADC reading for the right clutch
     * @param[in] save If true, save to persistent storage
     */
    virtual void setAxisCalibration(
        int minLeft,
        int maxLeft,
        int minRight,
        int maxRight,
        bool save = true) {}

    /**
     * @brief Get axes polarity
     *
     * @param[out] leftAxisReversed True if the left axis is reversed
     * @param[out] rightAxisReversed True if the right axis is reversed
     */
    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) {}

    /**
     * @brief Set axes polarity
     *
     * @param leftAxisReversed True if the left axis is reversed
     * @param rightAxisReversed True if the right axis is reversed
     * @param save If true, save to persistent storage
     */
    virtual void setAxisPolarity(
        bool leftAxisReversed,
        bool rightAxisReversed,
        bool save = true) {}

    /**
     * @brief Repeat last input event
     *
     */
    virtual void update() {}
};

//-------------------------------------------------------------------
// Input hub service
//-------------------------------------------------------------------

/**
 * @brief Input Hub service
 *
 */
class InputHubService : public DependencyManager<InputHubService>
{
public:
    /**
     * @brief Get the status of the security lock
     *
     * @return true If locked
     * @return false If not locked
     */
    virtual bool getSecurityLock()
    {
        return (false);
    }

    /**
     * @brief Get the clutch bite point
     *
     * @return uint8_t Bite point
     */
    virtual uint8_t getBitePoint()
    {
        return (CLUTCH_DEFAULT_VALUE);
    }

    /**
     * @brief Get the current clutch working mode
     *
     * @return ClutchWorkingMode Working mode
     */
    virtual ClutchWorkingMode getClutchWorkingMode()
    {
        return (ClutchWorkingMode::_DEFAULT_VALUE);
    }

    /**
     * @brief Get the current working mode of ALT buttons
     *
     * @return AltButtonsWorkingMode Working mode
     */
    virtual AltButtonsWorkingMode getAltButtonsWorkingMode()
    {
        return (AltButtonsWorkingMode::_DEFAULT_VALUE);
    }

    /**
     * @brief Get the current working mode of the directional pad
     *
     * @return DPadWorkingMode Working mode
     */
    virtual DPadWorkingMode getDPadWorkingMode()
    {
        return (DPadWorkingMode::_DEFAULT_VALUE);
    }

    /**
     * @brief Set the clutch bite point
     *
     * @param value Bite point
     * @param save True to save to flash memory
     */
    virtual void setBitePoint(uint8_t value, bool save = true)
    {
    }

    /**
     * @brief Set the clutch working mode
     *
     * @param mode Working mode
     * @param save True to save to flash memory
     */
    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save = true)
    {
    }

    /**
     * @brief Set the working mode of ALT buttons
     *
     * @param mode Working mode
     * @param save True to save to flash memory
     */
    virtual void setAltButtonsWorkingMode(
        AltButtonsWorkingMode mode,
        bool save = true) {}

    /**
     * @brief Set the directional pad working mode
     *
     * @param mode Working mode
     * @param save True to save to flash memory
     */
    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save = true)
    {
    }

    /**
     * @brief Set the status of the security lock
     *
     * @param value True to activate, false to deactivate
     * @param save True to save to flash memory
     */
    virtual void setSecurityLock(bool value, bool save = true)
    {
    }
};

//-------------------------------------------------------------------
// Input map service
//-------------------------------------------------------------------

/**
 * @brief Input map service
 *
 */
class InputMapService : public DependencyManager<InputMapService>
{
public:
    /**
     * @brief Map a firmware-defined input number to user-defined
     *
     * @param firmware_defined Firmware-defined input number
     * @param user_defined User-defined input number when alternate mode
     *                     is NOT engaged
     * @param user_defined_alt User-defined input number when alternate mode
     *                         IS engaged
     */
    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt) {}

    /**
     * @brief Get the current map Firmware-defined input number
     *
     * @param firmware_defined Firmware-defined input number
     * @param[out] user_defined User-defined input number when alternate mode
     *                          is NOT engaged
     * @param[out] user_defined_alt User-defined input number when alternate
     *                              mode IS engaged
     */
    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt)
    {
        user_defined = firmware_defined;
        user_defined_alt = firmware_defined + 64;
    };

    /**
     * @brief Revert the input map to factory defaults
     *
     */
    virtual void resetMap() {}
};

//-------------------------------------------------------------------
// Power service
//-------------------------------------------------------------------

/**
 * @brief Power service
 *
 */
class PowerService : public DependencyManager<PowerService>
{
public:
    /// @brief Command a system shutdown
    virtual void shutdown() {}
};

//-------------------------------------------------------------------
// Battery service
//-------------------------------------------------------------------

/**
 * @brief Battery service
 *
 */
class BatteryService : public DependencyManager<BatteryService>
{
public:
    /**
     * @brief Get the last known battery level (SoC)
     *
     * @return int Battery level in the range [0,100]
     */
    virtual int getLastBatteryLevel() { return (0); }

    /**
     * @brief Check if the device can be powered by a battery
     *
     * @note This method should always return the same value on each call
     *
     * @return true if this device can be powered by a battery
     * @return false if this device is always powered by a external power supply
     */
    virtual bool hasBattery() { return (false); }

    /**
     * @brief Check if the battery is detected
     *
     * @note If this method returns false, the battery level is unknown
     *
     * @return true if this device has a battery and was detected
     * @return false if this device has no battery or the battery
     *         is not detected
     *         (and the device is powered by a external power supply right now)
     *
     */
    virtual bool isBatteryPresent()
    {
        return (false);
    }

    /**
     * @brief Get the full battery status
     *
     * @param status Current battery status
     */
    virtual void getStatus(BatteryStatus &status) {}
};

//-------------------------------------------------------------------
// Battery calibration
//-------------------------------------------------------------------

/**
 * @brief Battery calibration service
 *
 */
class BatteryCalibrationService
    : public DependencyManager<BatteryCalibrationService>
{
public:
    /**
     * @brief Restart autocalibration algorithm.
     *
     */
    virtual void restartAutoCalibration()
    {
    }

    /**
     * @brief Get a percentage of battery charge based on calibration data.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     * @return int -1 if the battery has not been calibrated. Otherwise,
     *             a percentage in the range 0%-100%.
     */
    virtual int getBatteryLevel(int reading)
    {
        return (-1);
    }

    /**
     * @brief Get a percentage of battery charge using auto-calibration.
     *        Will provide incorrect battery levels (higher) until
     *        the battery is fully charged.
     *        Anyway, this algorithm is not accurate.
     *
     * @param reading An ADC reading of current battery(+) voltage
     *
     * @return int If auto-calibration is available,
     *             a percentage in the range 0%-100%.
     *             Otherwise, zero.
     *
     * @note Based on https://blog.ampow.com/lipo-voltage-chart/
     */
    virtual int getBatteryLevelAutoCalibrated(int reading)
    {
        return (0);
    }

    /**
     * @brief Get the count of calibration data items
     *
     * @return uint8_t Number of calibration data items
     */
    virtual uint8_t getCalibrationDataCount()
    {
        return (0);
    }

    /**
     * @brief Get calibration data
     *
     * @param index Index of the required datum
     * @return uint16_t Required datum
     */
    virtual uint16_t getCalibrationData(uint8_t index)
    {
        return (0);
    }

    /**
     * @brief Set calibration data
     *
     * @param index Index of this calibration data
     * @param data Data to set
     * @param save True to save to persistent storage.
     */
    virtual void setCalibrationData(
        uint8_t index, uint16_t data,
        bool save = true) {}

    /**
     * @brief Get the auto-calibration parameter
     *
     * @return int Auto-calibration parameter
     */
    virtual int getAutoCalibrationParameter()
    {
        return (-1);
    }

    /**
     * @brief Set the auto-calibration parameter
     *
     * @param value Value of the auto-calibration parameter
     * @param save True to save to persistent storage.
     */
    virtual void setAutoCalibrationParameter(int value, bool save = true)
    {
    }
};

//-------------------------------------------------------------------
// HID
//-------------------------------------------------------------------

/**
 * @brief HID service
 *
 */
class HidService : public DependencyManager<HidService>
{
public:
    /**
     * @brief Get the user-defined custom hardware ID
     *
     * @param[out] customVID Custom VID or zero if not configured
     * @param[out] customPID Custom PID or zero if not configured
     */
    virtual void getCustomHardwareID(
        uint16_t &customVID,
        uint16_t &customPID) {}

    /**
     * @brief Set the user-defiend custom hardware ID
     *
     * @note if both @p customVID and @p customPID are
     *       set to zero, the factory defaults will be
     *       used instead.
     *
     * @param customVID Custom VID
     * @param customPID Custom PID
     * @param save True to save to persistent storage
     */
    virtual void setCustomHardwareID(
        uint16_t customVID,
        uint16_t customPID,
        bool save = true) {}
};

//-------------------------------------------------------------------
// UI
//-------------------------------------------------------------------

/**
 * @brief User interface service
 *
 */
class UIService : public DependencyManager<UIService>
{
public:
    /**
     * @brief Get the maximum frames per seconds value
     *        required by UI instances
     *
     */
    virtual uint8_t getMaxFPS()
    {
        return (0);
    }
};

//-------------------------------------------------------------------
// Firmware
//-------------------------------------------------------------------

/**
 * @brief Global firmware service
 *
 */
class FirmwareService : public DependencyManager<FirmwareService>
{
    friend void firmwareSetIsRunningState(bool state);

public:
    /// @brief Check if the firmware daemons are already running
    /// @return true If running
    /// @return false If not running
    virtual bool isRunning()
    {
        return (_is_running);
    }

private:
    inline static bool _is_running = false;
};