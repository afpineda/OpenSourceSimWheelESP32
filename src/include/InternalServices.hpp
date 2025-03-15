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
#include <cassert>
#include <typeinfo>
#include <vector>

//-------------------------------------------------------------------
// Global macros
//-------------------------------------------------------------------

/// @brief Macro to mark a mock implementation for void methods
#define MOCK \
    {        \
    }

/// @brief Macro to mark a mock implementation for non-void methods
#define MOCK_R(value) \
    {                 \
        return value; \
    }

/// @brief Macro to declare an internal service class
#define SERVICE(ClassName) \
    ClassName:             \
public                     \
    DependencyManager<ClassName>

/// @brief Macro to declare a void static method in a service class
#define VOID_SINGLETON_INVOKER(Decl, Call) \
    static void Decl                       \
    {                                      \
        getInstance()->Call;               \
    }

/// @brief Macro to declare a non-void static method in a service class
#define SINGLETON_INVOKER(ReturnType, Decl, Call) \
    static ReturnType Decl                        \
    {                                             \
        return getInstance()->Call;               \
    }

/// @brief Reserved for future use
#define ALL_INVOKER(Decl, Call)             \
    static void Decl                        \
    {                                       \
        auto instances = getAllInstances(); \
        for (auto instance : instances)     \
            instance->Call;                 \
    }

/// @brief Reserved for future use
#define BOTH_INVOKER(Decl, Call)            \
    static void Decl                        \
    {                                       \
        getInstance()->Call;                \
        auto instances = getAllInstances(); \
        for (auto instance : instances)     \
            instance->Call;                 \
    }

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
    /// @brief Type of the set of injected instances
    typedef std::vector<Service *> InstanceSet;

    /**
     * @brief Get a singleton instance of the service provider
     *
     * @return Service* A pointer to the service provider.
     *                  **Do not delete**.
     */
    static Service *getInstance()
    {
        if constexpr ((std::is_abstract<Service>::value) || (!std::is_default_constructible<Service>::value))
        {
            assert((_singleton != nullptr) && "Provider not injected to Service");
        }
        else
        {
            if (_singleton == nullptr)
                _singleton = new Service();
        }
        return _singleton;
    }

    /**
     * @brief Get all instances of the service provider
     *
     * @return InstanceSet Vector of pointers to service providers.
     *                     **Do not delete** them.
     */
    static const InstanceSet getAllInstances() { return _instanceSet; }

    /**
     * @brief Inject an instance of the service provider to the service interface
     *
     * @tparam Provider Service interface
     * @param instance Instance of a service provider
     * @param multipleInstances When true, @p instance is retrieved with getAllInstances().
     *                          When false, @p instance is retrieved with getInstance().
     */
    template <typename Provider>
    static void inject(Provider *instance, bool multipleInstances = false)
    {
        static_assert(std::is_base_of<Service, Provider>::value, "Provider is not derived from Service");
        assert((instance != nullptr) && "A null provider is not allowed");
        if (multipleInstances)
        {
            _instanceSet.push_back(instance);
        }
        else
        {
            assert((_singleton == nullptr) && "Provider already injected to service");
            _singleton = instance;
        }
    }

    /**
     * @brief Remove all injected service providers (for testing)
     *
     * @warning For testing only. Do not call in production code.
     */
    static void reset()
    {
        if (_singleton != nullptr)
        {
            delete _singleton;
            _singleton = nullptr;
        }
        for (auto instance : _instanceSet)
            delete instance;
        _instanceSet.clear();
    }

    virtual ~DependencyManager() = default;

private:
    inline static InstanceSet _instanceSet = {};
    inline static Service *_singleton = nullptr;
};

//-------------------------------------------------------------------
// Input service
//-------------------------------------------------------------------

/**
 * @brief Input hardware services
 *
 */
class SERVICE(InputService)
{
public:
    /**
     * @brief Force auto-calibration of all axes (analog clutch paddles)
     *
     */
    virtual void recalibrateAxes() MOCK;

    /**
     * @brief Change polarity of left axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    virtual void reverseLeftAxis() MOCK;

    /**
     * @brief Change polarity of right axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    virtual void reverseRightAxis() MOCK;

    /**
     * @brief Multiply the pulse width of rotary encoders
     *
     * @note Valid values are saved to flash memory without delay.
     * @param multiplier A pulse width multiplier greater than zero.
     *                   Valid values are between 1 and 6.
     *                   Invalid values are ignored.
     */
    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) MOCK;

    /**
     * @brief Get the current pulse width multiplier for rotary encoders
     *
     * @return PulseWidthMultiplier A pulse width multiplier greater than zero.
     */
    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier() MOCK_R(PulseWidthMultiplier::X1);

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
        int &maxRight) MOCK_R(false);

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
        bool save) MOCK;

    /**
     * @brief Get axes polarity
     *
     * @param[out] leftAxisReversed True if the left axis is reversed
     * @param[out] rightAxisReversed True if the right axis is reversed
     */
    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) MOCK;

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
        bool save) MOCK;

    /**
     * @brief Repeat last input event
     *
     */
    virtual void update() MOCK;

    /// @cond

    struct call
    {
        VOID_SINGLETON_INVOKER(recalibrateAxes(), recalibrateAxes())
        VOID_SINGLETON_INVOKER(reverseLeftAxis(), reverseLeftAxis())
        VOID_SINGLETON_INVOKER(reverseRightAxis(), reverseRightAxis())
        VOID_SINGLETON_INVOKER(
            setRotaryPulseWidthMultiplier(PulseWidthMultiplier multiplier, bool save = true),
            setRotaryPulseWidthMultiplier(multiplier, save))
        SINGLETON_INVOKER(
            PulseWidthMultiplier,
            getRotaryPulseWidthMultiplier(),
            getRotaryPulseWidthMultiplier())
        SINGLETON_INVOKER(
            bool,
            getAxisCalibration(
                int &minLeft,
                int &maxLeft,
                int &minRight,
                int &maxRight),
            getAxisCalibration(minLeft, maxLeft, minRight, maxRight))
        VOID_SINGLETON_INVOKER(
            setAxisCalibration(
                int minLeft,
                int maxLeft,
                int minRight,
                int maxRight,
                bool save = true),
            setAxisCalibration(minLeft, maxLeft, minRight, maxRight, save))
        VOID_SINGLETON_INVOKER(
            getAxisPolarity(
                bool &leftAxisReversed,
                bool &rightAxisReversed),
            getAxisPolarity(
                leftAxisReversed,
                rightAxisReversed))
        VOID_SINGLETON_INVOKER(
            setAxisPolarity(
                bool leftAxisReversed,
                bool rightAxisReversed,
                bool save = true),
            setAxisPolarity(
                leftAxisReversed,
                rightAxisReversed,
                save))
        VOID_SINGLETON_INVOKER(update(), update());
    };

    /// @endcond

};

//-------------------------------------------------------------------
// Input hub service
//-------------------------------------------------------------------

/**
 * @brief Input Hub service
 *
 */
class SERVICE(InputHubService)
{
public:
    virtual bool getSecurityLock() MOCK_R(false);
    virtual uint8_t getBitePoint() MOCK_R(CLUTCH_DEFAULT_VALUE);
    virtual ClutchWorkingMode getClutchWorkingMode() MOCK_R(ClutchWorkingMode::_DEFAULT_VALUE);
    virtual AltButtonsWorkingMode getAltButtonsWorkingMode() MOCK_R(AltButtonsWorkingMode::_DEFAULT_VALUE);
    virtual DPadWorkingMode getDPadWorkingMode() MOCK_R(DPadWorkingMode::_DEFAULT_VALUE);
    virtual void setBitePoint(uint8_t value, bool save) MOCK;
    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save) MOCK;
    virtual void setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save) MOCK;
    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) MOCK;
    virtual void setSecurityLock(bool value, bool save) MOCK;

    /// @cond

    struct call
    {
        SINGLETON_INVOKER(bool, getSecurityLock(), getSecurityLock())
        SINGLETON_INVOKER(uint8_t, getBitePoint(), getBitePoint())
        SINGLETON_INVOKER(ClutchWorkingMode, getClutchWorkingMode(), getClutchWorkingMode())
        SINGLETON_INVOKER(AltButtonsWorkingMode, getAltButtonsWorkingMode(), getAltButtonsWorkingMode())
        SINGLETON_INVOKER(DPadWorkingMode, getDPadWorkingMode(), getDPadWorkingMode())

        VOID_SINGLETON_INVOKER(setBitePoint(uint8_t value, bool save = true), setBitePoint(value, save))
        VOID_SINGLETON_INVOKER(
            setClutchWorkingMode(ClutchWorkingMode mode, bool save = true),
            setClutchWorkingMode(mode, save))
        VOID_SINGLETON_INVOKER(
            setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save = true),
            setAltButtonsWorkingMode(mode, save))
        VOID_SINGLETON_INVOKER(
            setDPadWorkingMode(DPadWorkingMode mode, bool save = true),
            setDPadWorkingMode(mode, save))
        VOID_SINGLETON_INVOKER(
            setSecurityLock(bool value, bool save = true),
            setSecurityLock(value, save))
    };

    /// @endcond

};

//-------------------------------------------------------------------
// Input map service
//-------------------------------------------------------------------

/**
 * @brief Input map service
 *
 */
class SERVICE(InputMapService)
{
public:
    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt) MOCK;
    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt)
    {
        user_defined = firmware_defined;
        user_defined_alt = firmware_defined + 64;
    };
    virtual void resetMap() MOCK;

    struct call
    {
        VOID_SINGLETON_INVOKER(
            setMap(uint8_t firmware_defined, uint8_t user_defined, uint8_t user_defined_alt),
            setMap(firmware_defined, user_defined, user_defined_alt))
        VOID_SINGLETON_INVOKER(
            getMap(uint8_t firmware_defined, uint8_t &user_defined, uint8_t &user_defined_alt),
            getMap(firmware_defined, user_defined, user_defined_alt))
        VOID_SINGLETON_INVOKER(resetMap(), resetMap())
    };
};

//-------------------------------------------------------------------
// Power service
//-------------------------------------------------------------------

/**
 * @brief Power service
 *
 */
class SERVICE(PowerService)
{
public:
    virtual void shutdown() MOCK;

    struct call
    {
        VOID_SINGLETON_INVOKER(shutdown(), shutdown())
    };
};

//-------------------------------------------------------------------
// Battery service
//-------------------------------------------------------------------

/**
 * @brief Battery service
 *
 */
class SERVICE(BatteryService)
{
public:
    virtual int getLastBatteryLevel() MOCK_R(UNKNOWN_BATTERY_LEVEL);
    struct call
    {
        SINGLETON_INVOKER(int, getLastBatteryLevel(), getLastBatteryLevel())
    };
};

//-------------------------------------------------------------------
// Battery calibration
//-------------------------------------------------------------------

/**
 * @brief Battery calibration service
 *
 */
class SERVICE(BatteryCalibrationService)
{
public:
    /**
     * @brief Restart autocalibration algorithm.
     *
     */
    virtual void restartAutoCalibration() MOCK;

    /**
     * @brief Get a percentage of battery charge based on calibration data.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     * @return int -1 if the battery has not been calibrated. Otherwise,
     *             a percentage in the range 0%-100%.
     */
    virtual int getBatteryLevel(int reading) MOCK_R(-1);

    /**
     * @brief Get a percentage of battery charge using auto-calibration.
     *        Will provide incorrect battery levels (higher) until
     *        the battery is fully charged.
     *        Anyway, this algorithm is not accurate.
     *
     * @param reading An ADC reading of current battery(+) voltage
     *
     * @return If auto-calibration is available, a percentage in the range 0%-100%.
     *         Otherwise, the constant `UNKNOWN_BATTERY_LEVEL`.
     *
     * @note Based on https://blog.ampow.com/lipo-voltage-chart/
     */
    virtual int getBatteryLevelAutoCalibrated(int reading) MOCK_R(UNKNOWN_BATTERY_LEVEL);

    /**
     * @brief Get the count of calibration data items
     *
     */
    virtual uint8_t getCalibrationDataCount() MOCK_R(0);

    /**
     * @brief Get calibration data
     *
     * @param index Index of the required datum
     * @return uint16_t Required datum
     */
    virtual uint16_t getCalibrationData(uint8_t index) MOCK_R(0);

    /**
     * @brief Set calibration data
     *
     * @param index Index of this calibration data
     * @param data Data to set
     * @param save True to save to persistent storage.
     */
    virtual void setCalibrationData(uint8_t index, uint16_t data, bool save) MOCK;

    /**
     * @brief Get the auto-calibration parameter
     *
     * @return int Auto-calibration parameter
     */
    virtual int getAutoCalibrationParameter() MOCK_R(-1);

    /**
     * @brief Set the auto-calibration parameter
     *
     * @param value Value of the auto-calibration parameter
     * @param save True to save to persistent storage.
     */
    virtual void setAutoCalibrationParameter(int value, bool save) MOCK;

    /// @cond

    struct call
    {
        VOID_SINGLETON_INVOKER(restartAutoCalibration(), restartAutoCalibration())
        SINGLETON_INVOKER(int, getBatteryLevel(int reading), getBatteryLevel(reading))
        SINGLETON_INVOKER(int, getBatteryLevelAutoCalibrated(int reading), getBatteryLevelAutoCalibrated(reading))
        SINGLETON_INVOKER(uint8_t, getCalibrationDataCount(), getCalibrationDataCount())
        SINGLETON_INVOKER(uint16_t, getCalibrationData(uint8_t index), getCalibrationData(index))
        VOID_SINGLETON_INVOKER(
            setCalibrationData(uint8_t index, uint16_t data, bool save = true),
            setCalibrationData(index, data, save))
        SINGLETON_INVOKER(int, getAutoCalibrationParameter(), getAutoCalibrationParameter())
        VOID_SINGLETON_INVOKER(
            setAutoCalibrationParameter(int value, bool save = true),
            setAutoCalibrationParameter(value, save))
    };

    /// @endcond
};

//-------------------------------------------------------------------
// HID
//-------------------------------------------------------------------

/**
 * @brief HID service
 *
 */
class SERVICE(HidService)
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
        uint16_t &customPID) MOCK;

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
        bool save) MOCK;

    struct call
    {
        VOID_SINGLETON_INVOKER(
            getCustomHardwareID(uint16_t &customVID, uint16_t &customPID),
            getCustomHardwareID(customVID, customPID))
        VOID_SINGLETON_INVOKER(
            setCustomHardwareID(uint16_t customVID, uint16_t customPID, bool save = true),
            setCustomHardwareID(customVID, customPID, save))
    };
};

//-------------------------------------------------------------------
// UI
//-------------------------------------------------------------------

/**
 * @brief User interface service
 *
 */
class SERVICE(UIService)
{
public:
    virtual uint8_t getMaxFPS() MOCK_R(0);

    struct call
    {
        SINGLETON_INVOKER(uint8_t, getMaxFPS(), getMaxFPS())
    };
};

//-------------------------------------------------------------------
// Firmware
//-------------------------------------------------------------------

/**
 * @brief Global firmware service
 *
 */
class SERVICE(FirmwareService)
{
    friend void firmwareSetIsRunningState(bool state);

public:
    virtual bool isRunning() MOCK_R(_is_running);

    struct call
    {
        SINGLETON_INVOKER(bool, isRunning(), isRunning())
    };

private:
    inline static bool _is_running = false;
};