/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Everything about input from relative rotary encoders
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "RotaryEncoderInput.h"
#include "SimWheelTypes.h"
#include "SimWheel.h"

// This implementation is based on:
// https://www.best-microcontroller-projects.com/rotary-encoder.html

#define RT_STACK_SIZE 1024

#define OUTPUT_NONE 0
#define OUTPUT_CW 1
#define OUTPUT_CCW 2

// ----------------------------------------------------------------------------
// Interrupt service routine for rotary encoders
// ----------------------------------------------------------------------------

void IRAM_ATTR isrh(void *instance)
{
    static const uint8_t valid_code[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    // UBaseType_t lock = taskENTER_CRITICAL_FROM_ISR();
    UBaseType_t lock = portSET_INTERRUPT_MASK_FROM_ISR();
    RotaryEncoderInput *rotary = (RotaryEncoderInput *)instance;
    int clk = gpio_get_level(rotary->clkPin);
    int dt = gpio_get_level(rotary->dtPin);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(lock);
    // taskEXIT_CRITICAL_FROM_ISR(lock);

    uint8_t output = OUTPUT_NONE;

    rotary->code <<= 2;
    rotary->code = rotary->code | (dt << 1) | clk;
    rotary->code &= 0x0f;

    if (valid_code[rotary->code])
    {
        rotary->sequence <<= 4;
        rotary->sequence |= rotary->code;
        uint16_t aux = rotary->sequence & 0xff;
        if (aux == 0x2b)
            output = OUTPUT_CCW;
        else if (aux == 0x17)
            output = OUTPUT_CW;
    }

    if (output != OUTPUT_NONE)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(rotary->eventQueue, &output, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
            portYIELD_FROM_ISR();
    }
}

// ----------------------------------------------------------------------------
// Service task for rotary encoders
// ----------------------------------------------------------------------------

void rotaryDaemonLoop(void *instance)
{
    RotaryEncoderInput *rotary = (RotaryEncoderInput *)instance;
    uint8_t event;
    inputBitmap_t bitmap;
    while (true)
    {
        if (xQueueReceive(rotary->eventQueue, &event, portMAX_DELAY))
        {
            if (event == OUTPUT_CW)
            {
                // Clockwise rotation
                bitmap = BITMAP(rotary->cwButtonNumber);
            }
            else if (event == OUTPUT_CCW)
            {
                // Counter-clockwise rotation
                bitmap = BITMAP(rotary->ccwButtonNumber);
            }
            else
                // Should not happen
                abort();

            // Send button push event
            inputs::notifyInputEvent(rotary->mask, bitmap);
            // wait
            vTaskDelay(ROTARY_CLICK_TICKS);
            // Send button release event
            inputs::notifyInputEvent(rotary->mask, 0);

        } // end if
    }     // end while
}

// ----------------------------------------------------------------------------
// Implementation of class: RotaryEncoderInput
// ----------------------------------------------------------------------------

RotaryEncoderInput::RotaryEncoderInput(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    inputNumber_t cwButtonNumber,
    inputNumber_t ccwButtonNumber)
{
    // Check parameters
    GPIO_IS_VALID_GPIO(clkPin);
    GPIO_IS_VALID_GPIO(dtPin);
    if (clkPin == dtPin)
    {
        log_e("clkPin and dtPin must not match in RotaryEncoderInput::RotaryEncoderInput()");
        abort();
    }
    if (ccwButtonNumber == UNSPECIFIED_INPUT_NUMBER)
        ccwButtonNumber = cwButtonNumber + 1;
    if ((cwButtonNumber > MAX_INPUT_NUMBER) or (ccwButtonNumber > MAX_INPUT_NUMBER))
    {
        log_e("Invalid button number(s) in RotaryEncoderInput::RotaryEncoderInput()");
        abort();
    }

    // Initialize properties
    this->clkPin = clkPin;
    this->dtPin = dtPin;
    this->cwButtonNumber = cwButtonNumber;
    this->ccwButtonNumber = ccwButtonNumber;
    mask = ~(BITMAP(cwButtonNumber) | BITMAP(ccwButtonNumber));
    code = 0;
    sequence = 0;

    // Config task and queue
    eventQueue = xQueueCreate(64, sizeof(uint8_t));
    if (eventQueue == nullptr)
        abort();
    daemon = nullptr;
    xTaskCreate(rotaryDaemonLoop, "RotaryEnc", RT_STACK_SIZE, (void *)this, INPUT_TASK_PRIORITY, &daemon);
    if (daemon == nullptr)
        abort();

    // Config clkPin
    ESP_ERROR_CHECK(gpio_set_direction(clkPin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(clkPin, GPIO_PULLUP_ONLY));

    // Config dtPin
    ESP_ERROR_CHECK(gpio_set_direction(dtPin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(dtPin, GPIO_PULLUP_ONLY));

    // Initialize state
    isrh(this);
    isrh(this);

    // Enable IRQ for dtPin
    ESP_ERROR_CHECK(gpio_set_intr_type(dtPin, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(dtPin, isrh, (void *)this));
    ESP_ERROR_CHECK(gpio_intr_enable(dtPin));

    // Enable IRQ for clkPin
    ESP_ERROR_CHECK(gpio_set_intr_type(clkPin, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(clkPin, isrh, (void *)this));
    ESP_ERROR_CHECK(gpio_intr_enable(clkPin));
};