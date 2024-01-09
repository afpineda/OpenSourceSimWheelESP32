/**
 * @file Inputs.cpp
 *
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
#include "AnalogMultiplexerInput.h"
#include "ShiftRegistersInput.h"
#include <Preferences.h>

// #include "debugUtils.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static TaskHandle_t pollingTask = nullptr;
static DigitalPolledInput *digitalInputChain = nullptr;
static AnalogAxisInput *leftClutchAxis = nullptr;
static AnalogAxisInput *rightClutchAxis = nullptr;
static inputBitmap_t leftClutchButtonBitmap = 0ULL;
static inputBitmap_t rightClutchButtonBitmap = 0ULL;
static inputBitmap_t clutchButtonsMask = ~0ULL;
static bool forceUpdate = false;

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
  if (leftClutchAxis)
  {
    leftClutchAxis->resetCalibrationData();
    rightClutchAxis->resetCalibrationData();
  }
}

// ----------------------------------------------------------------------------
// Input polling
// ----------------------------------------------------------------------------

void inputs::update()
{
  forceUpdate = true;
}

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
    if ((newState != oldState) || forceUpdate)
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
      if (axisChanged || forceUpdate)
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
      if (axisChanged || forceUpdate)
      {
        axisEvent.id = RIGHT_CLUTCH_INDEX;
        axisEvent.inputBitmap = rightClutchAxis->bitmap;
        axisEvent.inputMask = rightClutchAxis->mask;
        xQueueSend(eventQueue, &axisEvent, SAMPLING_RATE_TICKS); // portMAX_DELAY);
      }

      if (leftAxisAutocalibrated || rightAxisAutocalibrated)
        requestSaveAxisCalibration();
    }
    forceUpdate = false;
    // wait for next sampling interval
    vTaskDelay(SAMPLING_RATE_TICKS);
  }
}

// ----------------------------------------------------------------------------
// Configure inputs
// ----------------------------------------------------------------------------

void abortDueToCallBeforeBegin()
{
  log_e("inputs::add*() or inputs::set*() called before inputs::begin() or after inputs::start()");
  abort();
}

void abortDueToInvalidInputNumber()
{
  log_e("invalid input or pin numbers at inputs::add*() or inputs::set*()");
  abort();
}

// ----------------------------------------------------------------------------

void inputs::addDigital(
    gpio_num_t pinNumber,
    inputNumber_t inputNumber,
    bool pullupOrPulldown,
    bool enableInternalPull)
{
  if ((!pollingTask) && (hubTask))
  {
    if (inputNumber <= MAX_INPUT_NUMBER)
      digitalInputChain = new DigitalButton(
          pinNumber,
          inputNumber,
          pullupOrPulldown,
          enableInternalPull,
          digitalInputChain);
    else
      abortDueToInvalidInputNumber();
  }
  else
    abortDueToCallBeforeBegin();
}

// ----------------------------------------------------------------------------

void inputs::addRotaryEncoder(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    inputNumber_t cwInputNumber,
    inputNumber_t ccwInputNumber,
    bool useAlternateEncoding)
{
  if ((!pollingTask) && (hubTask))
  {
    if ((cwInputNumber <= MAX_INPUT_NUMBER) &&
        (ccwInputNumber <= MAX_INPUT_NUMBER) &&
        (cwInputNumber != ccwInputNumber))
      digitalInputChain = new RotaryEncoderInput(
          clkPin, dtPin, cwInputNumber, ccwInputNumber, useAlternateEncoding, digitalInputChain);
    else
      abortDueToInvalidInputNumber();
  }
  else
    abortDueToCallBeforeBegin();
}

// ----------------------------------------------------------------------------

void inputs::addButtonMatrix(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray)
{
  if ((!pollingTask) && (hubTask))
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
    abortDueToCallBeforeBegin();
}

void inputs::addAnalogMultiplexer(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray)
{
  if ((!pollingTask) && (hubTask))
  {
    digitalInputChain = new AnalogMultiplexerInput(
        selectorPins,
        selectorPinCount,
        inputPins,
        inputPinCount,
        buttonNumbersArray,
        true,
        digitalInputChain);
  }
  else
    abortDueToCallBeforeBegin();
}

void inputs::addShiftRegisters(
    const gpio_num_t serialPin,
    const gpio_num_t loadPin,
    const gpio_num_t nextPin,
    inputNumber_t *buttonNumbersArray,
    const uint8_t switchCount)
{
  if ((!pollingTask) && (hubTask))
  {
    digitalInputChain = new ShiftRegistersInput(
        serialPin,
        loadPin,
        nextPin,
        buttonNumbersArray,
        switchCount,
        true,
        false,
        false,
        digitalInputChain);
  }
  else
    abortDueToCallBeforeBegin();
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
  else if ((!pollingTask) && (hubTask))
  {
    if ((leftClutchPin != rightClutchPin) &&
        (leftClutchInputNumber <= MAX_INPUT_NUMBER) &&
        (rightClutchInputNumber <= MAX_INPUT_NUMBER) &&
        (rightClutchInputNumber != leftClutchInputNumber))
    {
      rightClutchAxis = new AnalogAxisInput(rightClutchPin, rightClutchInputNumber);
      leftClutchAxis = new AnalogAxisInput(leftClutchPin, leftClutchInputNumber);
      capabilities::setFlag(CAP_CLUTCH_ANALOG);
    }
    else
      abortDueToInvalidInputNumber();
  }
  else
    abortDueToCallBeforeBegin();
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
  else if ((leftClutchInputNumber == rightClutchInputNumber) ||
           (leftClutchInputNumber > MAX_INPUT_NUMBER) ||
           (rightClutchInputNumber > MAX_INPUT_NUMBER))
  {
    abortDueToInvalidInputNumber();
  }
  else if ((!pollingTask) && (hubTask))
  {
    leftClutchButtonBitmap = BITMAP(leftClutchInputNumber);
    rightClutchButtonBitmap = BITMAP(rightClutchInputNumber);
    clutchButtonsMask = ~(leftClutchButtonBitmap | rightClutchButtonBitmap);
    capabilities::setFlag(CAP_CLUTCH_BUTTON);
  }
  else
    abortDueToCallBeforeBegin();
}

// ----------------------------------------------------------------------------
// Macros to send events
// ----------------------------------------------------------------------------

void inputs::notifyInputEvent(inputBitmap_t mask, inputBitmap_t state)
{
  inputEvent_t event;
  event.eventType = EVENT_TYPE_SWITCH;
  event.mask = mask;
  event.state = state;
  xQueueSend(eventQueue, &event, SAMPLING_RATE_TICKS); // portMAX_DELAY);
}

void inputs::notifyInputEventForTesting(uint8_t id, inputBitmap_t bitmap, inputBitmap_t mask, clutchValue_t value)
{
  axisEvent_t event;
  event.eventType = EVENT_TYPE_AXIS;
  event.inputBitmap = bitmap;
  event.inputMask = mask;
  event.value = value;
  event.id = id;
  xQueueSend(eventQueue, &event, SAMPLING_RATE_TICKS); // portMAX_DELAY);
}

// ----------------------------------------------------------------------------
// Input hub
// ----------------------------------------------------------------------------

void hubLoop(void *unused)
{
  inputBitmap_t globalState = 0;
  inputBitmap_t newState, changes, inputFilter;

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
          // Translate digital clutch inputs into analog axis values
          changes = globalState ^ newState;
          if (changes & leftClutchButtonBitmap)
            clutchState::setLeftAxis((newState & leftClutchButtonBitmap) ? CLUTCH_FULL_VALUE : CLUTCH_NONE_VALUE);
          if (changes & rightClutchButtonBitmap)
            clutchState::setRightAxis((newState & rightClutchButtonBitmap) ? CLUTCH_FULL_VALUE : CLUTCH_NONE_VALUE);
          inputFilter = clutchButtonsMask;
        }
        else
          inputFilter = ~0ULL;
        changes = globalState ^ newState;
        globalState = newState;
        inputHub::onStateChanged(globalState & inputFilter, changes & inputFilter);
      }
      else
      {
        // buffer[0] == EVENT_TYPE_AXIS
        if (clutchState::currentFunction == CF_BUTTON)
        {
          // Translate analog axis values into digital input
          if ((axisEvent->value) >= CLUTCH_3_4_VALUE)
            newState = (globalState & axisEvent->inputMask) | axisEvent->inputBitmap;
          else if ((axisEvent->value) <= CLUTCH_1_4_VALUE)
            newState = (globalState & axisEvent->inputMask);
          else
            newState = globalState;
          changes = globalState ^ newState;
          if (changes)
          {
            globalState = newState;
            inputHub::onStateChanged(globalState, changes);
          }
        }
        else
        {
          bool oldAltEnabled = clutchState::isALTRequested();
          clutchValue_t oldClutchValue = clutchState::combinedAxis;

          if (axisEvent->id)
            clutchState::setRightAxis(axisEvent->value);
          else
            clutchState::setLeftAxis(axisEvent->value);

          globalState = globalState & (axisEvent->inputMask);
          if (
              (clutchState::currentFunction == CF_AXIS) ||
              ((clutchState::currentFunction == CF_ALT) &&
               (oldAltEnabled != clutchState::isALTRequested())) ||
              ((clutchState::currentFunction == CF_CLUTCH) &&
               (oldClutchValue != clutchState::combinedAxis)))
            inputHub::onStateChanged(globalState, 0);
        }
      }

    } // end if
  }   // end while
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
    if ((leftClutchAxis) && (rightClutchAxis) && prefs.begin(AXIS_NAMESPACE, true))
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
