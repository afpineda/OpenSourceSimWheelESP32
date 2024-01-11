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

// Related to hardware inputs

static DigitalPolledInput *digitalInputChain = nullptr;
static AnalogAxisInput *leftClutchAxis = nullptr;
static AnalogAxisInput *rightClutchAxis = nullptr;
static inputBitmap_t assignedInputsBitmap = 0ULL;

// Related to the polling task

static TaskHandle_t pollingTask = nullptr;
static bool forceUpdate = false;
#define SAMPLING_RATE_TICKS DEBOUNCE_TICKS * 2
#define POLLING_TASK_STACK_SIZE 1 * 1024

// Related to the decoupling queue

typedef struct
{
  inputBitmap_t rawInputBitmap;
  inputBitmap_t rawInputChanges;
  bool axesAvailable;
  clutchValue_t leftAxisValue;
  clutchValue_t rightAxisValue;
} decouplingEvent_t;

static QueueHandle_t decouplingQueue = nullptr;

#define EVENT_SIZE sizeof(decouplingEvent_t)

// Related to the hub task

#define HUB_STACK_SIZE 4 * 1024
static TaskHandle_t hubTask = nullptr;

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
  // inputBitmap_t combinedMask = DigitalPolledInput::getChainMask(digitalInputChain);
  decouplingEvent_t currentState, previousState;
  bool leftAxisAutocalibrated = false;
  bool rightAxisAutocalibrated = false;
  bool stateChanged;
  currentState.leftAxisValue = CLUTCH_NONE_VALUE;
  currentState.rightAxisValue = CLUTCH_NONE_VALUE;
  currentState.rawInputBitmap = 0ULL;
  currentState.axesAvailable = (leftClutchAxis != nullptr);
  previousState = currentState;

  // loop
  while (true)
  {
    // Read digital inputs
    currentState.rawInputBitmap =
        DigitalPolledInput::readInChain(previousState.rawInputBitmap, digitalInputChain);
    currentState.rawInputChanges = currentState.rawInputBitmap ^ previousState.rawInputBitmap;
    stateChanged = forceUpdate || (currentState.rawInputChanges);

    // Read analog inputs
    if (leftClutchAxis)
    {
      // Left clutch axis
      leftClutchAxis->read(currentState.leftAxisValue, leftAxisAutocalibrated);

      // Right clutch axis
      rightClutchAxis->read(currentState.rightAxisValue, rightAxisAutocalibrated);

      if (leftAxisAutocalibrated || rightAxisAutocalibrated)
        requestSaveAxisCalibration();

      stateChanged =
          stateChanged ||
          (currentState.leftAxisValue != previousState.leftAxisValue) ||
          (currentState.rightAxisValue != previousState.rightAxisValue);
    }

    // Check for a state change
    if (stateChanged)
    {
      // Push state into the decoupling queue
      xQueueSend(decouplingQueue, &currentState, 0);
      previousState = currentState;
    }

    // Prepare for next iteration
    forceUpdate = false;

    // wait for next sampling interval
    vTaskDelay(SAMPLING_RATE_TICKS);
  }
}

// ----------------------------------------------------------------------------
// Input hub
// ----------------------------------------------------------------------------

void hubLoop(void *unused)
{
  decouplingEvent_t currentState;

  while (true)
  {
    if (xQueueReceive(decouplingQueue, &currentState, portMAX_DELAY))
      inputHub::onRawInput(
          currentState.rawInputBitmap,
          currentState.rawInputChanges,
          currentState.leftAxisValue,
          currentState.rightAxisValue,
          currentState.axesAvailable);
  } // end while
}

// ----------------------------------------------------------------------------
// Configure inputs
// ----------------------------------------------------------------------------

void abortDueToCallAfterStart()
{
  log_e("inputs::add*() or inputs::set*() called after inputs::start()");
  abort();
}

// void abortDueToInvalidInputNumber()
// {
//   log_e("invalid input or pin numbers at inputs::add*() or inputs::set*()");
//   abort();
// }

// ----------------------------------------------------------------------------

void checkInputNumber(inputNumber_t number)
{
  inputBitmap_t bmp = BITMAP(number);
  if (number > MAX_INPUT_NUMBER)
  {
    log_e("Input number out of range");
    abort();
  }
  else if (bmp & assignedInputsBitmap)
  {
    log_e("Input number already in use");
    abort();
  }
  assignedInputsBitmap |= bmp;
}

void checkInputNumbers(int count, inputNumber_t numbers[])
{
  for (int i = 0; i < count; i++)
    checkInputNumber(numbers[i]);
}

// ----------------------------------------------------------------------------

void inputs::addDigital(
    gpio_num_t pinNumber,
    inputNumber_t inputNumber,
    bool pullupOrPulldown,
    bool enableInternalPull)
{
  if ((!pollingTask) && (!hubTask))
  {
    checkInputNumber(inputNumber);
    digitalInputChain = new DigitalButton(
        pinNumber,
        inputNumber,
        pullupOrPulldown,
        enableInternalPull,
        digitalInputChain);
  }
  else
    abortDueToCallAfterStart();
}

// ----------------------------------------------------------------------------

void inputs::addRotaryEncoder(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    inputNumber_t cwInputNumber,
    inputNumber_t ccwInputNumber,
    bool useAlternateEncoding)
{
  if ((!pollingTask) && (!hubTask))
  {
    checkInputNumber(cwInputNumber);
    checkInputNumber(ccwInputNumber);
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_ERR_INVALID_STATE)
      ESP_ERROR_CHECK(err);
    digitalInputChain = new RotaryEncoderInput(
        clkPin, dtPin, cwInputNumber, ccwInputNumber, useAlternateEncoding, digitalInputChain);
  }
  else
    abortDueToCallAfterStart();
}

// ----------------------------------------------------------------------------

void inputs::addButtonMatrix(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray)
{
  if ((!pollingTask) && (!hubTask))
  {
    checkInputNumbers(selectorPinCount * inputPinCount, buttonNumbersArray);
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
    abortDueToCallAfterStart();
}

void inputs::addAnalogMultiplexer(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray)
{
  if ((!pollingTask) && (!hubTask))
  {
    checkInputNumbers((1 << selectorPinCount) * inputPinCount, buttonNumbersArray);
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
    abortDueToCallAfterStart();
}

void inputs::addShiftRegisters(
    const gpio_num_t serialPin,
    const gpio_num_t loadPin,
    const gpio_num_t nextPin,
    inputNumber_t *buttonNumbersArray,
    const uint8_t switchCount)
{
  if ((!pollingTask) && (!hubTask))
  {
    checkInputNumbers(switchCount, buttonNumbersArray);
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
    abortDueToCallAfterStart();
}

void inputs::setAnalogClutchPaddles(
    const gpio_num_t leftClutchPin,
    const gpio_num_t rightClutchPin)
{
  if ((!pollingTask) && (!hubTask))
  {
    if ((leftClutchPin != rightClutchPin) && (rightClutchAxis == nullptr) && (leftClutchAxis == nullptr))
    {
      rightClutchAxis = new AnalogAxisInput(rightClutchPin);
      leftClutchAxis = new AnalogAxisInput(leftClutchPin);
      capabilities::setFlag(CAP_CLUTCH_ANALOG);
      capabilities::setFlag(CAP_CLUTCH_BUTTON, false);
    }
    else
    {
      log_e("inputs::setAnalogClutchPaddles() called twice or for two identical GPIO pins");
      abort();
    }
  }
  else
    abortDueToCallAfterStart();
}

// ----------------------------------------------------------------------------
// Macros to send events
// ----------------------------------------------------------------------------

void inputs::notifyInputEventForTesting(
    inputBitmap_t state,
    clutchValue_t leftAxisValue,
    clutchValue_t rightAxisValue)
{
  if (decouplingQueue)
  {
    decouplingEvent_t event;
    event.leftAxisValue = leftAxisValue;
    event.rightAxisValue = rightAxisValue;
    event.rawInputBitmap = state;
    event.axesAvailable = true;
    event.rawInputChanges = state;
    xQueueSend(decouplingQueue, &event, SAMPLING_RATE_TICKS); // portMAX_DELAY);
  }
}

// ----------------------------------------------------------------------------
// Initialization and start
// ----------------------------------------------------------------------------

void inputs::start()
{
  void *dummy;
  if (pollingTask == nullptr)
  {
    // Load axis calibration data, if any
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

    // create autosave timer
    esp_timer_create_args_t args;
    args.callback = &axisCalibrationAutoSaveCallback;
    args.arg = nullptr;
    args.name = nullptr;
    args.dispatch_method = ESP_TIMER_TASK;
    ESP_ERROR_CHECK(esp_timer_create(&args, &autoSaveTimer));

    // Create event queue
    decouplingQueue = xQueueCreate(64, EVENT_SIZE);
    if (decouplingQueue == nullptr)
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

    // Create and run polling task
    xTaskCreate(inputPollingLoop, "PolledInputs", POLLING_TASK_STACK_SIZE, nullptr, INPUT_TASK_PRIORITY, &pollingTask);
    if (pollingTask == nullptr)
    {
      log_e("Unable to create polling task");
      abort();
    }
  }
}

// ----------------------------------------------------------------------------
