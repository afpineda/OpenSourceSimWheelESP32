/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `inputs` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */
#include "SimWheel.h"
#include "RotaryEncoderInput.h"
#include "PolledInput.h"
#include "ButtonMatrixInput.h"
//#include <FreeRTOS.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SAMPLING_RATE_TICKS DEBOUNCE_TICKS
#define POLLING_TASK_STACK_SIZE 1 * 1024

static inputNumber_t globalButtonCount = 0;
static TaskHandle_t pollingTask = nullptr;
static PolledInput *polledInputChain = nullptr;
static bool buttonMatrixAlreadySet = false;

#define HUB_STACK_SIZE 4 * 1024
typedef struct
{
  inputBitmap_t mask;
  inputBitmap_t state;
} inputEvent_t;

static TaskHandle_t hubTask = nullptr;
static QueueHandle_t eventQueue = nullptr;

// ----------------------------------------------------------------------------
// Input polling
// ----------------------------------------------------------------------------

void inputPollingLoop(void *param)
{
  inputBitmap_t newState;
  inputBitmap_t oldState = 0;
  inputBitmap_t combinedMask = PolledInput::getChainMask(polledInputChain);

  // loop
  while (true)
  {
    newState = PolledInput::readInChain(oldState, polledInputChain);
    if (newState != oldState)
      inputs::notifyInputEvent(combinedMask, newState);
    oldState = newState;
    vTaskDelay(SAMPLING_RATE_TICKS);
  }
}

// ----------------------------------------------------------------------------
// Configure inputs
// ----------------------------------------------------------------------------

inputNumber_t abortAtAdd()
{
  log_e("inputs::add*() or inputs::set*() called before inputs::begin()");
  abort();
  return UNSPECIFIED_INPUT_NUMBER;
}

// ----------------------------------------------------------------------------

inputNumber_t inputs::addDigital(
    gpio_num_t pinNumber,
    bool pullupOrPulldown,
    bool enableInternalPull)
{
  if ((pollingTask == nullptr))
  {
    int buttonNumber = globalButtonCount++;
    polledInputChain = new DigitalButton(pinNumber, buttonNumber, pullupOrPulldown, enableInternalPull, polledInputChain);
    return buttonNumber;
  }
  else
    return abortAtAdd();
}

void inputs::addDigitalExt(
    gpio_num_t pinNumber,
    inputNumber_t inputNumber,
    bool pullupOrPulldown,
    bool enableInternalPull)
{
  if ((pollingTask == nullptr))
    polledInputChain = new DigitalButton(pinNumber, inputNumber, pullupOrPulldown, enableInternalPull, polledInputChain);
  else
    abortAtAdd();
}

// ----------------------------------------------------------------------------

inputNumber_t inputs::addRotaryEncoder(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    bool useAlternateEncoding)
{
  if (pollingTask == nullptr)
  {
    int buttonNumber = globalButtonCount;
    new RotaryEncoderInput(clkPin, dtPin, buttonNumber,UNSPECIFIED_INPUT_NUMBER, useAlternateEncoding);
    globalButtonCount += 2;
    return buttonNumber;
  }
  else
    return abortAtAdd();
}

void inputs::addRotaryEncoderExt(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    inputNumber_t cwInputNumber,
    inputNumber_t ccwInputNumber,
    bool useAlternateEncoding)
{
  if (pollingTask == nullptr)
  {
    new RotaryEncoderInput(clkPin, dtPin, cwInputNumber, ccwInputNumber, useAlternateEncoding);
  }
  else
    abortAtAdd();
}

// ----------------------------------------------------------------------------

inputNumber_t inputs::setButtonMatrix(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount)
{
  if (pollingTask == nullptr)
  {
    if (buttonMatrixAlreadySet)
    {
      log_e("inputs::setButtonMatrix() called twice");
      abort();
      return UNSPECIFIED_INPUT_NUMBER;
    }
    else
    {
      int firstButtonNumber = globalButtonCount;
      polledInputChain = new ButtonMatrixInput(
          selectorPins,
          selectorPinCount,
          inputPins,
          inputPinCount,
          nullptr,
          firstButtonNumber);
      globalButtonCount += (inputPinCount * selectorPinCount);
      buttonMatrixAlreadySet = true;
      return firstButtonNumber;
    }
  }
  else
    return abortAtAdd();
}

void inputs::addButtonMatrixExt(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray)
{
  if (pollingTask != nullptr)
  {
    polledInputChain = new ButtonMatrixInput(
        selectorPins,
        selectorPinCount,
        inputPins,
        inputPinCount,
        buttonNumbersArray,
        UNSPECIFIED_INPUT_NUMBER,
        polledInputChain);
  }
  else
    abortAtAdd();
}

// ----------------------------------------------------------------------------
// Input hub
// ----------------------------------------------------------------------------

void inputs::notifyInputEvent(inputBitmap_t mask, inputBitmap_t state)
{
  inputEvent_t event;
  event.mask = mask;
  event.state = state;
  xQueueSend(eventQueue, &event, portMAX_DELAY);
}

void hubLoop(void *unused)
{
  inputBitmap_t globalState = 0;
  inputBitmap_t newState, changes;
  inputEvent_t event;
  while (true)
  {
    if (xQueueReceive(eventQueue, &event, portMAX_DELAY))
    {
      newState = (globalState & event.mask) | event.state;
      changes = globalState ^ newState;
      globalState = newState;
      inputHub::onStateChanged(newState, changes);
    } // end if
  }
}

// ----------------------------------------------------------------------------
// Initialization and start
// ----------------------------------------------------------------------------

void inputs::start()
{
  void *dummy;
  if ((hubTask == nullptr) || (globalButtonCount == 0))
  {
    log_e("inputs::start() called before inputs::begin() or inputs::add*()");
    abort();
  }
  if (pollingTask == nullptr)
  {
    xTaskCreate(inputPollingLoop, "PolledInputs", POLLING_TASK_STACK_SIZE, nullptr, INPUT_TASK_PRIORITY, &pollingTask);
    if (pollingTask == nullptr)
    {
      log_e("Unable to create polling daemon");
      abort();
    }
  }
}

void inputs::begin()
{
  if (hubTask == nullptr)
  {
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    eventQueue = xQueueCreate(64, sizeof(inputEvent_t));
    if (eventQueue == nullptr)
      abort();
    xTaskCreate(hubLoop, "hub", HUB_STACK_SIZE, (void *)nullptr, INPUT_TASK_PRIORITY, &hubTask);
    if (hubTask == nullptr)
    {
      log_e("Unable to create inputHub daemon");
      abort();
    }
  }
}

// ----------------------------------------------------------------------------
