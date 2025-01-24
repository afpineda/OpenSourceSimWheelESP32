/**
 * @file RotaryEncoderInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Use a rotary encoder's rotation, both clockwise and counter-clockwise,
 *        as inputs for a sim racing wheel or button box.
 *
 * @details Each single "detent" is translated into two events:
 *          a "virtual button" press and then release,
 *          with a short delay between them.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __ROTARYENCODERINPUT_H__
#define __ROTARYENCODERINPUT_H__

// #include <Arduino.h> // For debug
#include "SimWheelTypes.h"
#include "esp32-hal.h"
#include "esp_intr_alloc.h"
#include "PolledInput.h"

/**
 * @brief Input from a rotary encoder
 *
 */
class RotaryEncoderInput : public DigitalPolledInput
{
private:
  // Hardware related
  gpio_num_t clkPin, dtPin; // pins
  uint8_t code;             // State of decoding algoritm
  uint16_t sequence;        // Last sequence of states in "alternate encoding"

  // Firmware related
  inputNumber_t cwButtonNumber;
  inputNumber_t ccwButtonNumber;

  // Circular bits queue
  uint64_t bitsQueue;         // data: bit 1 = clockwise rotation, 0 = counter-clockwise rotation
  uint8_t bqHead;             // "pointer" (short of) to head
  uint8_t bqTail;             // "pointer" to tail
  uint8_t pressEventNotified; // a "virtual button" press event was notified at read(), so a release event must be notified next

private:
  friend void IRAM_ATTR isrh(void *instance);
  friend void IRAM_ATTR isrhAlternateEncoding(void *instance);

protected:
  /**
   * @brief Pulse multiplier for rotary encoders
   *
   * @note For read only. Do not overwrite.
   *       Always greater than zero.
   */
  static uint8_t pulseMultiplier;

  void incBitQueuePointer(uint8_t &pointer)
  {
    pointer = (pointer + 1) % sizeof(bitsQueue);
  };

  /**
   * @brief Push rotation event into the queue
   *
   * @param cwOrCcw True for clockwise rotation event, False for counter-clockwise.
   */
  void bitsQueuePush(bool cwOrCcw);

  /**
   * @brief Extract a rotation event from the queue
   *
   * @param[out] cwOrCcw The extracted event, if any
   * @return true if the queue was not empty, so @p cwOrCcw contains valid data.
   * @return false if the queue was empty, so @p cwOrCcw was not written.
   */
  bool bitsQueuePop(bool &cwOrCcw);

public:
  /**
   * @brief Construct a new Rotary Encoder Input object
   *
   * @param[in] clkPin GPIO pin attached to the "CLK" (or "A") pin of the encoder.
   * @param[in] dtPin GPIO pin attached to the "DT" (or "B") pin of the encoder.
   * @param[in] cwButtonNumber A number for the "virtual button" of a clockwise rotation event.
   * @param[in] ccwButtonNumber A number for the "virtual button" of a counter-clockwise rotation event.
   *                           If not given, `cwButtonNumber`+1 is used.
   * @param[in] useAlternateEncoding Set to true in order to use the signal encoding of
   *                                 ALPS RKJX series of rotary encoders, and the alike.
   * @param[in] nextInChain Another instance to build a chain, or nullptr
   *
   * @note Interal pullup resistors will be enabled when available.
   */
  RotaryEncoderInput(
      gpio_num_t clkPin,
      gpio_num_t dtPin,
      inputNumber_t cwButtonNumber,
      inputNumber_t ccwButtonNumber = UNSPECIFIED_INPUT_NUMBER,
      bool useAlternateEncoding = false,
      DigitalPolledInput *nextInChain = nullptr);

  /**
   * @brief Set a time multiplier for "pulse" events
   *
   * @param multiplier A time multiplier between 1 and 3.
   * @return true if the multiplier has changed.
   * @return false otherwise.
   */
  static bool setPulseMultiplier(uint8_t multiplier)
  {
    if ((multiplier > 0) && (multiplier < 4) && (pulseMultiplier != multiplier))
    {
      pulseMultiplier = multiplier;
      return true;
    }
    return false;
  };

  virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif