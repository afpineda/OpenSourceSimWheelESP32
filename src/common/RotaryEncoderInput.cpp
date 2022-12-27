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

#define RT_STACK_SIZE 1264
//#define RT_STACK_SIZE 2048

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
// state = xyXY
// where xy = previous input, XY = current input, x or X=CLK, y or Y=DT
//
// States and transitions (mermaid graph):
// graph TD
//  0011 --01--> 1101 --00--> 0100 --CCW--> 0000 
//  0000 --10--> 0010 --11--> 1011 --CCW--> 0011
//  0011 --10--> 1110 --00--> 1000 --CW--> 0000
//  0000 --01--> 0001 --11--> 0111 --CW--> 0011
//
// trasition = aaaabbbb
// where aaaa=previous state, bbbb=current state
//
// Valid transitions:
// 00111101
// 11010100 (CCW to 0000)
// 00000010
// 00101011 (CCW to 0011)
// 00111110
// 11101000 (CW to 0000)
// 00000001
// 00010111 (CW to 0011)

void IRAM_ATTR isrhAlternateEncoding(void *instance)
{
    // UBaseType_t lock = taskENTER_CRITICAL_FROM_ISR();
    UBaseType_t lock = portSET_INTERRUPT_MASK_FROM_ISR();
    RotaryEncoderInput *rotary = (RotaryEncoderInput *)instance;
    int clk = gpio_get_level(rotary->clkPin);
    int dt = gpio_get_level(rotary->dtPin);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(lock);
    // taskEXIT_CRITICAL_FROM_ISR(lock);

    uint8_t output = OUTPUT_NONE;
    uint8_t reading = ((clk << 1) | dt);
    uint8_t nextCode = ((rotary->code << 2) | reading) & 0b1111;
    uint8_t transition = (rotary->code << 4) | nextCode;

    if ((transition == 0b00111101) ||
        (transition == 0b11010100) ||
        (transition == 0b00000010) ||
        (transition == 0b00101011) ||
        (transition == 0b00111110) ||
        (transition == 0b11101000) ||
        (transition == 0b00000001) ||
        (transition == 0b00010111))
    {
        if (transition == 0b11010100) 
        {
            rotary->code = 0;
            output = OUTPUT_CW;
        }
        else if (transition == 0b00101011)
        {
            rotary->code = 0b11;
            output = OUTPUT_CW;
        }
        else if (transition == 0b11101000)
        {
            rotary->code = 0;
            output = OUTPUT_CCW;
        }
        else if (transition == 0b00010111)
        {
            rotary->code = 0b11;
            output = OUTPUT_CCW;
        }
        else
            rotary->code = nextCode;
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
            else {
                // Should not happen
                log_e("unknown event at rotaryDaemonLoop()");
                abort();
            }

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
    inputNumber_t ccwButtonNumber,
    bool useAlternateEncoding)
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
    if (useAlternateEncoding)
    {
        code = 0b11;
        isrhAlternateEncoding(this);
        isrhAlternateEncoding(this);
    }
    else
    {
        code = 0;
        isrh(this);
        isrh(this);
    }

    // Enable IRQ for dtPin
    ESP_ERROR_CHECK(gpio_set_intr_type(dtPin, GPIO_INTR_ANYEDGE));
    if (useAlternateEncoding)
        ESP_ERROR_CHECK(gpio_isr_handler_add(dtPin, isrhAlternateEncoding, (void *)this));
    else
        ESP_ERROR_CHECK(gpio_isr_handler_add(dtPin, isrh, (void *)this));
    ESP_ERROR_CHECK(gpio_intr_enable(dtPin));

    // Enable IRQ for clkPin
    ESP_ERROR_CHECK(gpio_set_intr_type(clkPin, GPIO_INTR_ANYEDGE));
    if (useAlternateEncoding)
        ESP_ERROR_CHECK(gpio_isr_handler_add(clkPin, isrhAlternateEncoding, (void *)this));
    else
        ESP_ERROR_CHECK(gpio_isr_handler_add(clkPin, isrh, (void *)this));
    ESP_ERROR_CHECK(gpio_intr_enable(clkPin));
};