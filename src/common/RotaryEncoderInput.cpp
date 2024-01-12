/**
 * @file RotaryEncoderInput.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Everything about input from relative rotary encoders
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "RotaryEncoderInput.h"
#include "SimWheelTypes.h"

// This implementation is based on:
// https://www.best-microcontroller-projects.com/rotary-encoder.html

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

    rotary->code <<= 2;
    rotary->code = rotary->code | (dt << 1) | clk;
    rotary->code &= 0x0f;

    if (valid_code[rotary->code])
    {
        rotary->sequence <<= 4;
        rotary->sequence |= rotary->code;
        uint16_t aux = rotary->sequence & 0xff;
        if (aux == 0x2b)
            // Counter-clockwise rotation event
            rotary->bitsQueuePush(false);
        else if (aux == 0x17)
            // Clockwise rotation event
            rotary->bitsQueuePush(true);
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
            // Clockwise rotation event
            rotary->code = 0;
            rotary->bitsQueuePush(true);
        }
        else if (transition == 0b00101011)
        {
            // Clockwise rotation event
            rotary->code = 0b11;
            rotary->bitsQueuePush(true);
        }
        else if (transition == 0b11101000)
        {
            // Counter-clockwise rotation event
            rotary->code = 0;
            rotary->bitsQueuePush(false);
        }
        else if (transition == 0b00010111)
        {
            // Counter-clockwise rotation event
            rotary->code = 0b11;
            rotary->bitsQueuePush(false);
        }
        else
            rotary->code = nextCode;
    }
}

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

RotaryEncoderInput::RotaryEncoderInput(
    gpio_num_t clkPin,
    gpio_num_t dtPin,
    inputNumber_t cwButtonNumber,
    inputNumber_t ccwButtonNumber,
    bool useAlternateEncoding,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    // Check parameters
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
    bitsQueue = 0ULL;
    bqHead = 0;
    bqTail = 0;
    pressEventNotified = false;

    // Config clkPin
    checkAndInitializeInputPin(clkPin, false, true);
    // Config dtPin
    checkAndInitializeInputPin(dtPin, false, true);

    // Initialize decoding state machine
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

// ----------------------------------------------------------------------------
// Read
// ----------------------------------------------------------------------------

inputBitmap_t RotaryEncoderInput::read(inputBitmap_t lastState)
{
    if (!pressEventNotified)
    {
        bool cwOrCcw;
        if (bitsQueuePop(cwOrCcw))
        {
            pressEventNotified = true;
            if (cwOrCcw)
                return BITMAP(cwButtonNumber);
            else
                return BITMAP(ccwButtonNumber);
        }
    }
    pressEventNotified = false;
    return 0;
}

// ----------------------------------------------------------------------------
// Circular bits queue
// ----------------------------------------------------------------------------

void RotaryEncoderInput::bitsQueuePush(bool cwOrCcw)
{
    uint8_t bqTailNext = bqTail;
    incBitQueuePointer(bqTailNext);
    if (bqTailNext != bqHead)
    {
        // Queue not full
        uint64_t aux = (1ULL << bqTail);
        bitsQueue &= (~aux);
        if (cwOrCcw)
            bitsQueue |= aux;
        bqTail = bqTailNext;
    } // Queue full, overflow
}

bool RotaryEncoderInput::bitsQueuePop(bool &cwOrCcw)
{
    bool isNotEmpty = (bqHead != bqTail);
    if (isNotEmpty)
    {
        uint64_t bitState = (1ULL << bqHead) & bitsQueue;
        cwOrCcw = (bitState != 0ULL);
        incBitQueuePointer(bqHead);
    }
    return isNotEmpty;
}