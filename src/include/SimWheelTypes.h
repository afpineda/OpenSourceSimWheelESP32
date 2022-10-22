/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Types and constants used everywhere
 *
 * @note Some types are renamed just for portability
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __SIMWHEELTYPES_H__
#define __SIMWHEELTYPES_H__

#include <stdint.h>

/**
 * @brief A bit array which assembles the state of every button, being the least significant bit
 *        the button numbered as 0, and the most significant bit the button numbered as 31.
 *        A bit set to 1 means the button is pressed, 0 means not pressed.
 *        May be used to identify a number of buttons, too.
 */
typedef uint64_t inputBitmap_t;

/**
 * @brief A button number, starting from zero.
 *
 */
typedef uint8_t inputNumber_t;

#define UNSPECIFIED_INPUT_NUMBER 0xFF /// Input number is not required or is implicit or is unknown
#define MAX_INPUT_NUMBER 63 /// Maximun allowed input number, including itself

// Well-known input numbers for PC game controllers
#define JOY_A 0
#define JOY_B 1
#define JOY_X 2
#define JOY_Y 3
#define JOY_LB 4
#define JOY_RB 5
#define JOY_LSHIFT_PADDLE 4
#define JOY_RSHIFT_PADDLE 5
#define JOY_BACK 6
#define JOY_START 7
#define JOY_LTHUMBSTICK_CLICK 8
#define JOY_RTHUMBSTICK_CLICK 9


/**
 * @brief The value of a joystick's axis
 *
 */
typedef int8_t clutchValue_t;

#define CLUTCH_NONE_VALUE -127
#define CLUTCH_FULL_VALUE 127
#define CLUTCH_DEFAULT_VALUE 0

//#define clutchValuePercent(value) ((uint8_t)(value))/255

/**
 * @brief Value read from an ADC pin
 *
 */
typedef uint16_t analogReading_t;

/**
 * @brief Transform a button number to an input bitmap.
 *        For example, BITMAP(2) returns 0b00000100
 *
 */
#define BITMAP(n) (1ULL << static_cast<inputBitmap_t>(n))

/**
 * @brief Return a mask for a number of buttons (`count`) starting from `first`.
 *        A mask is a bit array where each bit determines if a button is to be used or not.
 *        1 means **not** used. 0 means in use.
 *        Masks are required to combine the state from multiple input bitmaps.
 *        For example, BITMASK(2,2) returns 0b(...)11110011 which means that
 *        buttons numbered 2 and 3 are in use.
 */
#define BITMASK(count, first) ~(((1ULL << static_cast<inputBitmap_t>(count)) - 1ULL) << static_cast<inputBitmap_t>(first))

/**
 * @brief Return the logical negation of a bit mask
 *
 */
#define NBITMASK(count, first) (((1ULL << static_cast<inputBitmap_t>(count)) - 1ULL) << static_cast<inputBitmap_t>(first))
//#define NBITMASK(count, first) (((1 << count) - 1) << first)

/**
 * @brief Debounce time for buttons, in system ticks
 *
 */
#define DEBOUNCE_TICKS 30 / portTICK_RATE_MS

/**
 * @brief Priority of background tasks 
 *
 */
#define INPUT_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define UART_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/**
 * @brief User-selected function of the clutch paddles
 *        CF_CLUTCH = F1-style clutch
 *        CF_ALT = Alternate button numbers (like ALT key in PC keybards)
 *        CF_BUTTON = a button like any other
 */
typedef enum
{
    CF_CLUTCH = 0,
    CF_ALT,
    CF_BUTTON
} clutchFunction_t;

/**
 * @brief Default delay (in microseconds) before clearing the OLED display after a screen shows up
 * 
 */
#define DEFAULT_UI_TIME_us 4*1000*1000

/**
 * @brief Default autosave delay (in microseconds)
 * 
 */
#define DEFAULT_AUTOSAVE_us 20*1000*1000

typedef enum {
    SCR_INFO_PRIORITY = 0, /// highest priority
    SCR_MENU_PRIORITY = 1,
    SCR_COMM_PRIORITY = 2,
    SCR_FRAMESERVER_PRIORITY = 3
} screenPriority_t;
#define SCR_PRIORITY_COUNT 4

/**
 * @brief Display types supported by the ss_oled library
 *        Must match `iType` parameter for the `oledInit` function.
 */
typedef enum {
  SSOLED_128x128 = 1,
  SSOLED_128x32,
  SSOLED_128x64,
  SSOLED_132x64,
  SSOLED_64x32,
  SSOLED_96x16,
  SSOLED_72x40
} displayType_t;

// OLED SETUP
#define SCREEN_RESET_PIN -1
#define SCREEN_SDA_PIN 21
#define SCREEN_SCL_PIN 22

/**
 * @brief Supported UI languages
 * 
 */
typedef enum {
    LANG_EN = 0, /// English
    LANG_ES /// Spanish
} language_t;

// Time to wait for connection before power off (in seconds)
#define AUTO_POWER_OFF_DELAY_SECS 60 

/**
 * @brief Supported power latch modes
 * 
 */
typedef enum {
    POWER_OPEN_DRAIN, /// Power on when open drain, power off when low voltage
    POWER_OFF_HIGH, /// Power on when low voltage, power off when high voltage
    POWER_OFF_LOW, /// Power on when high voltage, power off when low voltage 
} powerLatchMode_t;

/**
 * @brief Battery level to report when unknown (percentage)
 * 
 */
#define UNKNOWN_BATTERY_LEVEL 66

#endif