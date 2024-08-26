/**
 * @file Inputs.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `inputs` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */
#include "SimWheel.h"
#include "RotaryEncoderInput.h"
#include "PolledInput.h"
#include "ButtonMatrixInput.h"
#include "AnalogMultiplexerInput.h"
#include "ShiftRegistersInput.h"
#include "I2CExpanderInput.h"
#include <Preferences.h>
#include "i2cTools.h"

// #include "debugUtils.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to hardware inputs

static DigitalPolledInput *digitalInputChain = nullptr;
static AnalogAxisInput *leftClutchAxis = nullptr;
static AnalogAxisInput *rightClutchAxis = nullptr;

// Related to the polling task

static TaskHandle_t pollingTask = nullptr;
static bool forceUpdate = false;
#define SAMPLING_RATE_TICKS DEBOUNCE_TICKS * 2
#define POLLING_TASK_STACK_SIZE 2 * 1024

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

// Related to axis calibration data and polarity

static esp_timer_handle_t autoSaveTimer = nullptr;
#define AXIS_NAMESPACE "axis"
#define KEY_MIN_CAL_DATA "a"
#define KEY_MAX_CAL_DATA "z"
#define KEY_POLARITY "!"
#define LEFT_CLUTCH_INDEX 0
#define RIGHT_CLUTCH_INDEX 1

// Related to I2C bus
static std::vector<uint8_t> *i2cAddressesFromProbe = nullptr;

// ----------------------------------------------------------------------------
// Calibration data of analog axes
// ----------------------------------------------------------------------------

void saveAxisCalibration(Preferences &prefs, uint8_t index, int min, int max)
{
  char aux[6];
  snprintf(aux, 6, "%s%d", KEY_MIN_CAL_DATA, index);
  prefs.putInt(aux, min);
  snprintf(aux, 6, "%s%d", KEY_MAX_CAL_DATA, index);
  prefs.putInt(aux, max);
}

bool loadAxisCalibration(Preferences &prefs, uint8_t index, int &min, int &max)
{
  char aux[6];
  snprintf(aux, 6, "%s%d", KEY_MIN_CAL_DATA, index);
  if (prefs.isKey(aux))
  {
    min = prefs.getInt(aux);
    snprintf(aux, 6, "%s%d", KEY_MAX_CAL_DATA, index);
    if (prefs.isKey(aux))
    {
      max = prefs.getInt(aux);
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
    saveAxisCalibration(prefs, LEFT_CLUTCH_INDEX, min, max);
    rightClutchAxis->getCalibrationData(&min, &max);
    saveAxisCalibration(prefs, RIGHT_CLUTCH_INDEX, min, max);

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
// Axis polarity
// ----------------------------------------------------------------------------

void loadAxisPolarity(Preferences &prefs, uint8_t index, bool &currentPolarity)
{
  char aux[6];
  snprintf(aux, 6, "%s%d", KEY_POLARITY, index);
  currentPolarity = prefs.getBool(aux, currentPolarity);
}

void saveAxisPolarity(uint8_t index, bool currentPolarity)
{
  Preferences prefs;
  if (prefs.begin(AXIS_NAMESPACE, false))
  {
    char aux[6];
    snprintf(aux, 6, "%s%d", KEY_POLARITY, index);
    prefs.putBool(aux, currentPolarity);
    prefs.end();
  }
}

void inputs::reverseLeftAxis()
{
  if (leftClutchAxis)
  {
    leftClutchAxis->reversed = !leftClutchAxis->reversed;
    saveAxisPolarity(LEFT_CLUTCH_INDEX, leftClutchAxis->reversed);
    inputs::update();
  }
}

void inputs::reverseRightAxis()
{
  if (rightClutchAxis)
  {
    rightClutchAxis->reversed = !rightClutchAxis->reversed;
    saveAxisPolarity(RIGHT_CLUTCH_INDEX, rightClutchAxis->reversed);
    inputs::update();
  }
}

void resetAxesPolarityForTesting()
{
  if (rightClutchAxis)
    rightClutchAxis->reversed = true;
  if (leftClutchAxis)
    leftClutchAxis->reversed = true;
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
  // Initialize
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

// ----------------------------------------------------------------------------

void inputs::addDigital(
    gpio_num_t pinNumber,
    inputNumber_t inputNumber)
{
  if ((!pollingTask) && (!hubTask))
  {
    digitalInputChain = new DigitalButton(
        pinNumber,
        inputNumber,
        true,
        true,
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

ButtonMatrixInputSpec &inputs::addButtonMatrix(
    const gpio_num_array_t selectorPins,
    const gpio_num_array_t inputPins)
{
  if ((!pollingTask) && (!hubTask))
  {
    ButtonMatrixInput *matrix = new ButtonMatrixInput(
        selectorPins,
        inputPins,
        digitalInputChain);
    digitalInputChain = matrix;
    return *matrix;
  }
  else
    abortDueToCallAfterStart();
}

Multiplexers8InputSpec &inputs::addAnalogMultiplexer(
    const gpio_num_array_t &selectorPins,
    const gpio_num_array_t &inputPins)
{
  if ((!pollingTask) && (!hubTask))
  {
    AnalogMultiplexerInput *mux = new AnalogMultiplexerInput(
        selectorPins,
        inputPins,
        true,
        digitalInputChain);
    digitalInputChain = mux;
    return *mux;
  }
  else
    abortDueToCallAfterStart();
}

ShiftRegisters8InputSpec &inputs::addShiftRegisters(
    const gpio_num_t serialPin,
    const gpio_num_t loadPin,
    const gpio_num_t nextPin,
    const uint8_t switchCount)
{
  if ((!pollingTask) && (!hubTask))
  {
    ShiftRegistersInput *sr =
        new ShiftRegistersInput(
            serialPin,
            loadPin,
            nextPin,
            switchCount,
            true,
            false,
            false,
            digitalInputChain);
    digitalInputChain = sr;
    return *sr;
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

uint8_t getI2CFullAddress(uint8_t I2CAddress, bool isFullAddress)
{
  if (isFullAddress)
    return I2CAddress;

  // Retrieve all 7-bit addresses found in the bus (only once)
  if (i2cAddressesFromProbe == nullptr)
  {
    i2cAddressesFromProbe = new std::vector<uint8_t>;
    i2c::probe(*i2cAddressesFromProbe);
  }
  uint8_t fullAddress = i2c::findFullAddress(*i2cAddressesFromProbe, I2CAddress);

  if (fullAddress == 0xFF)
  {

    log_e("No GPIO expander found with hardware address %x (hex)", I2CAddress);
    abort();
  }
  else if (fullAddress == 0xFE)
  {
    log_e("Unable to auto-detect full address of GPIO expander. Hardware address is %x (hex)", I2CAddress);
    abort();
  }
  return fullAddress;
}

PCF8574InputSpec &inputs::addPCF8574Digital(
    uint8_t I2CAddress,
    bool isFullAddress)
{
  if ((!pollingTask) && (!hubTask))
  {
    uint8_t fullAddress = getI2CFullAddress(I2CAddress, isFullAddress);
    PCF8574ButtonsInput *gpioExpander =
        new PCF8574ButtonsInput(
            fullAddress,
            false,
            digitalInputChain);
    digitalInputChain = gpioExpander;
    return *gpioExpander;
  }
  else
    abortDueToCallAfterStart();
}

MCP23017InputSpec &inputs::addMCP23017Digital(
    uint8_t I2CAddress,
    bool isFullAddress)
{
  if ((!pollingTask) && (!hubTask))
  {
    uint8_t fullAddress = getI2CFullAddress(I2CAddress, isFullAddress);
    MCP23017ButtonsInput *gpioExpander =
        new MCP23017ButtonsInput(
            fullAddress,
            false,
            digitalInputChain);
    digitalInputChain = gpioExpander;
    return *gpioExpander;
  }
  else
    abortDueToCallAfterStart();
}

void inputs::initializeI2C(gpio_num_t sdaPin, gpio_num_t sclPin)
{
  i2c::begin(sdaPin, sclPin);
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
      if (loadAxisCalibration(prefs, LEFT_CLUTCH_INDEX, min, max))
        leftClutchAxis->setCalibrationData(min, max);
      if (loadAxisCalibration(prefs, RIGHT_CLUTCH_INDEX, min, max))
        rightClutchAxis->setCalibrationData(min, max);
      loadAxisPolarity(prefs, LEFT_CLUTCH_INDEX, leftClutchAxis->reversed);
      loadAxisPolarity(prefs, RIGHT_CLUTCH_INDEX, rightClutchAxis->reversed);
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

    // Take note of used input numbers
    inputBitmap_t usedInputs = ~DigitalPolledInput::getChainMask(digitalInputChain);
    capabilities::availableInputs |= usedInputs;

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
