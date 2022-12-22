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
#include <Preferences.h>

// #include <FreeRTOS.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static inputNumber_t globalButtonCount = 0;
static TaskHandle_t pollingTask = nullptr;
static DigitalPolledInput *digitalInputChain = nullptr;
static AnalogAxisInput *leftClutchAxis = nullptr;
static AnalogAxisInput *rightClutchAxis = nullptr;
static inputBitmap_t leftClutchButtonBitmap = 0ULL;
static inputBitmap_t rightClutchButtonBitmap = 0ULL;
static inputBitmap_t clutchButtonsMask = ~0ULL;
static bool buttonMatrixAlreadySet = false;

// Related to the polling task and event queue
#define SAMPLING_RATE_TICKS DEBOUNCE_TICKS * 2
#define POLLING_TASK_STACK_SIZE 1 * 1024
#define EVENT_TYPE_AXIS 0
#define EVENT_TYPE_SWITCH 1
#define EVENT_SIZE sizeof(inputEvent_t) > sizeof(axisEvent_t) ? sizeof(inputEvent_t) : sizeof(axisEvent_t)

typedef struct
{
  uint8_t eventType;
  inputBitmap_t mask;
  inputBitmap_t state;
} inputEvent_t;

typedef struct
{
  uint8_t eventType;
  inputBitmap_t inputBitmap;
  inputBitmap_t inputMask;
  clutchValue_t value;
  uint8_t id;
} axisEvent_t;

// Related to the hub task
#define HUB_STACK_SIZE 4 * 1024
static TaskHandle_t hubTask = nullptr;
static QueueHandle_t eventQueue = nullptr;

// Related to axis calibration data
static esp_timer_handle_t autoSaveTimer = nullptr;
#define AXIS_NAMESPACE "axis"
#define KEY_MIN_CAL_DATA "a"
#define KEY_MAX_CAL_DATA "z"
#define LEFT_CLUTCH_INDEX 0
#define RIGHT_CLUTCH_INDEX 1

// ----------------------------------------------------------------------------
// Calibration data of analog axes
// ----------------------------------------------------------------------------

void saveAxisCalibration(Preferences *prefs, uint8_t index, int min, int max)
{
  char aux[6];
  snprintf(aux, 6, "%s%d", KEY_MIN_CAL_DATA, index);
  prefs->putInt(aux, min);
  snprintf(aux, 6, "%s%d", KEY_MAX_CAL_DATA, index);
  prefs->putInt(aux, max);
}

bool loadAxisCalibration(Preferences *prefs, uint8_t index, int *min, int *max)
{
  char aux[6];
  snprintf(aux, 6, "%s%d", KEY_MIN_CAL_DATA, index);
  if (prefs->isKey(aux))
  {
    *min = prefs->getInt(aux);
    snprintf(aux, 6, "%s%d", KEY_MAX_CAL_DATA, index);
    if (prefs->isKey(aux))
    {
      *max = prefs->getInt(aux);
      return true;
    }
  }
  return false;
}

void axisCalibrationAutoSaveCallback(void *param)
{
  Preferences prefs;
  int min, max;

  if (prefs.begin(AXIS_NAMESPACE, false))
  {
    leftClutchAxis->getCalibrationData(&min, &max);
    saveAxisCalibration(&prefs, LEFT_CLUTCH_INDEX, min, max);
    rightClutchAxis->getCalibrationData(&min, &max);
    saveAxisCalibration(&prefs, RIGHT_CLUTCH_INDEX, min, max);
    prefs.end();
  }
}

void requestSaveAxisCalibration()
{
  esp_timer_stop(autoSaveTimer);
  esp_timer_start_once(autoSaveTimer, DEFAULT_AUTOSAVE_us);
}

void inputs::recalibrateAxes()
{
  leftClutchAxis->resetCalibrationData();
  rightClutchAxis->resetCalibrationData();
}

// ----------------------------------------------------------------------------
// Input polling
// ----------------------------------------------------------------------------

void inputPollingLoop(void *param)
{
  inputBitmap_t newState;
  inputBitmap_t oldState = 0;
  inputBitmap_t combinedMask = DigitalPolledInput::getChainMask(digitalInputChain);
  bool axisChanged, leftAxisAutocalibrated, rightAxisAutocalibrated;
  axisEvent_t axisEvent;
  axisEvent.eventType = EVENT_TYPE_AXIS;

  // loop
  while (true)
  {
    // Digital inputs
    newState = DigitalPolledInput::readInChain(oldState, digitalInputChain);
    if (newState != oldState)
      inputs::notifyInputEvent(combinedMask, newState);
    oldState = newState;

    // Analog inputs
    if (leftClutchAxis)
    {
      // Left clutch axis
      leftClutchAxis->read(
          &(axisEvent.value),
          &axisChanged,
          &leftAxisAutocalibrated);
      if (axisChanged)
      {
        axisEvent.id = LEFT_CLUTCH_INDEX;
        axisEvent.inputBitmap = leftClutchAxis->bitmap;
        axisEvent.inputMask = leftClutchAxis->mask;
        xQueueSend(eventQueue, &axisEvent, SAMPLING_RATE_TICKS); // portMAX_DELAY);
      }

      // Right clutch axis
      rightClutchAxis->read(
          &(axisEvent.value),
          &axisChanged,
          &rightAxisAutocalibrated);
      if (axisChanged)
      {
        axisEvent.id = RIGHT_CLUTCH_INDEX;
        axisEvent.inputBitmap = rightClutchAxis->bitmap;
        axisEvent.inputMask = rightClutchAxis->mask;
        xQueueSend(eventQueue, &axisEvent, SAMPLING_RATE_TICKS); // portMAX_DELAY);
      }

      if (leftAxisAutocalibrated || rightAxisAutocalibrated)
        requestSaveAxisCalibration();
    }

    // wait for next sampling interval
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
    digitalInputChain = new DigitalButton(pinNumber, buttonNumber, pullupOrPulldown, enableInternalPull, digitalInputChain);
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
    digitalInputChain = new DigitalButton(pinNumber, inputNumber, pullupOrPulldown, enableInternalPull, digitalInputChain);
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
    new RotaryEncoderInput(clkPin, dtPin, buttonNumber, UNSPECIFIED_INPUT_NUMBER, useAlternateEncoding);
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
      digitalInputChain = new ButtonMatrixInput(
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
    digitalInputChain = new ButtonMatrixInput(
        selectorPins,
        selectorPinCount,
        inputPins,
        inputPinCount,
        buttonNumbersArray,
        UNSPECIFIED_INPUT_NUMBER,
        digitalInputChain);
  }
  else
    abortAtAdd();
}

void inputs::setAnalogClutchPaddles(
    const gpio_num_t leftClutchPin,
    const gpio_num_t rightClutchPin,
    const inputNumber_t leftClutchInputNumber,
    const inputNumber_t rightClutchInputNumber)
{
  if (rightClutchAxis || leftClutchAxis || leftClutchButtonBitmap || rightClutchButtonBitmap)
  {
    log_e("inputs::set*ClutchPaddles() called twice");
    abort();
  }
  else
  {
    if (leftClutchPin != rightClutchPin)
    {
      rightClutchAxis = new AnalogAxisInput(rightClutchPin, rightClutchInputNumber);
      leftClutchAxis = new AnalogAxisInput(leftClutchPin, leftClutchInputNumber);
      capabilities::setFlag(CAP_CLUTCH_ANALOG);
    }
    else
    {
      log_e("inputs::setAnalogClutchPaddles() called with the same left and right gpio pins");
      abort();
    }
  }
}

void inputs::setDigitalClutchPaddles(
    const inputNumber_t leftClutchInputNumber,
    const inputNumber_t rightClutchInputNumber)
{
  if (rightClutchAxis || leftClutchAxis || leftClutchButtonBitmap || rightClutchButtonBitmap)
  {
    log_e("inputs::set*ClutchPaddles() called twice");
    abort();
  }
  else if (leftClutchInputNumber == rightClutchInputNumber)
  {
    log_e("inputs::setDigitalClutchPaddles() called with the same left and right input numbers");
    abort();
  }
  else
  {
    leftClutchButtonBitmap = BITMAP(leftClutchInputNumber);
    rightClutchButtonBitmap = BITMAP(rightClutchInputNumber);
    clutchButtonsMask = ~(leftClutchButtonBitmap | rightClutchButtonBitmap);
    capabilities::setFlag(CAP_CLUTCH_BUTTON);
  }
}

// ----------------------------------------------------------------------------
// Input hub
// ----------------------------------------------------------------------------

void inputs::notifyInputEvent(inputBitmap_t mask, inputBitmap_t state)
{
  inputEvent_t event;
  event.eventType = EVENT_TYPE_SWITCH;
  event.mask = mask;
  event.state = state;
  xQueueSend(eventQueue, &event, SAMPLING_RATE_TICKS); // portMAX_DELAY);
}

void hubLoop(void *unused)
{
  inputBitmap_t globalState = 0;
  inputBitmap_t newState, changes;

  uint8_t buffer[EVENT_SIZE];
  inputEvent_t *switchEvent = (inputEvent_t *)buffer;
  axisEvent_t *axisEvent = (axisEvent_t *)buffer;

  while (true)
  {
    if (xQueueReceive(eventQueue, buffer, portMAX_DELAY))
    {
      if (buffer[0] == EVENT_TYPE_SWITCH)
      {
        newState = (globalState & switchEvent->mask) | switchEvent->state;
        if (clutchState::currentFunction != CF_BUTTON)
        {
          changes = globalState ^ newState;
          if (changes & leftClutchButtonBitmap)
            clutchState::setLeftAxis((newState & leftClutchButtonBitmap) ? CLUTCH_FULL_VALUE : CLUTCH_NONE_VALUE);
          if (changes & rightClutchButtonBitmap)
            clutchState::setRightAxis((newState & rightClutchButtonBitmap) ? CLUTCH_FULL_VALUE : CLUTCH_NONE_VALUE);
          newState &= clutchButtonsMask;
        }
        changes = globalState ^ newState;
        globalState = newState;
      }
      else
      {
        // EVENT_TYPE_AXIS
        if (axisEvent->id)
        {
          clutchState::setLeftAxis(axisEvent->value);
        }
        else
        {
          clutchState::setRightAxis(axisEvent->value);
        }
      }
      inputHub::onStateChanged(globalState, changes);
    } // end if
  }
}

// ----------------------------------------------------------------------------
// Initialization and start
// ----------------------------------------------------------------------------

void inputs::start()
{
  void *dummy;
  if (hubTask == nullptr)
  {
    log_e("inputs::start() called before inputs::begin()");
    abort();
  }
  if (pollingTask == nullptr)
  {
    // Load axis callibration data, if any
    Preferences prefs;
    if (prefs.begin(AXIS_NAMESPACE, true))
    {
      int min, max;
      if (loadAxisCalibration(&prefs, LEFT_CLUTCH_INDEX, &min, &max))
        leftClutchAxis->setCalibrationData(min, max);
      if (loadAxisCalibration(&prefs, RIGHT_CLUTCH_INDEX, &min, &max))
        rightClutchAxis->setCalibrationData(min, max);
      prefs.end();
    }

    // Create and run polling task
    xTaskCreate(inputPollingLoop, "PolledInputs", POLLING_TASK_STACK_SIZE, nullptr, INPUT_TASK_PRIORITY, &pollingTask);
    if (pollingTask == nullptr)
    {
      log_e("Unable to create polling task");
      abort();
    }
  }
}

void inputs::begin()
{
  if (hubTask == nullptr)
  {
    // create autosave timer
    esp_timer_create_args_t args;
    args.callback = &axisCalibrationAutoSaveCallback;
    args.arg = nullptr;
    args.name = nullptr;
    args.dispatch_method = ESP_TIMER_TASK;
    ESP_ERROR_CHECK(esp_timer_create(&args, &autoSaveTimer));

    // Enable interrupts for rotary encoders and create event queue
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    eventQueue = xQueueCreate(64, EVENT_SIZE);
    if (eventQueue == nullptr)
    {
      log_e("Unable to create event queue");
      abort();
    }

    // Create and run hub task
    xTaskCreate(hubLoop, "hub", HUB_STACK_SIZE, (void *)nullptr, INPUT_TASK_PRIORITY, &hubTask);
    if (hubTask == nullptr)
    {
      log_e("Unable to create inputHub task");
      abort();
    }
  }
}

// ----------------------------------------------------------------------------
