/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Use a rotary encoder rotation, both clockwise and counter-clocwise,
 *        as inputs for a sim racing wheel or button box.
 *  
 * @section DESCRIPTION
 * 
 * Each single "detent" is translated into two events: a "virtual button" press and then release, 
 * with a short delay between them.
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *  
 */

#ifndef __ROTARYENCODERINPUT_H__
#define __ROTARYENCODERINPUT_H__

#include <Arduino.h>
#include "SimWheelTypes.h"
#include "esp32-hal.h"
#include "esp_intr_alloc.h"
//#include <FreeRTOS.h>

/**
 * @brief Fixed elapsed time between "virtual button" press and release
 * 
 */
#define ROTARY_CLICK_TICKS 50 / portTICK_RATE_MS

/**
 * @brief Input from a rotary encoder
 * 
 */
class RotaryEncoderInput
{
private:
  gpio_num_t clkPin, dtPin;
  QueueHandle_t eventQueue;
  TaskHandle_t daemon;
  uint8_t code;
  uint16_t sequence;
  inputNumber_t cwButtonNumber;
  inputNumber_t ccwButtonNumber;
  inputBitmap_t mask;

private:
  friend void IRAM_ATTR isrh(void *instance);
  friend void rotaryDaemonLoop(void *instance);

public:
  /**
   * @brief Construct a new Rotary Encoder Input object
   * 
   * @param[in] clkPin GPIO pin attached to the "CLK" (or "A") pin of the encoder. 
   * @param[in] dtPin GPIO pin attached to the "DT" (or "B") pin of the encoder.
   * @param[in] cwButtonNumber A number for the "virtual button" of a clockwise rotation event. 
   * @param[in] cwButtonNumber A number for the "virtual button" of a counter-clockwise rotation event. 
   *                           If not given, `cwButtonNumber`+1 is used.
   * The button number for the counter-clockwise rotation event is `cwButtonNumber+1`.
   * (interal pullup resistors will be enabled).
   */
  RotaryEncoderInput(gpio_num_t clkPin, gpio_num_t dtPin, inputNumber_t cwButtonNumber, inputNumber_t ccwButtonNumber = UNSPECIFIED_INPUT_NUMBER);
};

#endif