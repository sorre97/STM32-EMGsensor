#include "EMGsensor.h"
#include "MSX_HAL.h"
//#include "stm32f401xe.h"

/**
 * EMGsensor()
 * Constructor initializes the peripherals in POLLING mode using default pins
 * Redirects to secondary constructor
 * Mode: POLLING
 */
EMGsensor::EMGsensor() : EMGsensor::EMGsensor(EMGsensormod::POLLING) {}

/**
 * EMGsensor(Mode mode)
 * Constructor initializes the peripherals in selected mode using default pins if not selected
 * Input: Mode mode. Selects operative mode of the sensor
 * Pins: Vin(analog) -> PA0 (ADC1 channel 0)
 * Mode: POLLING, INTERRUPT, DMA
 */
EMGsensor::EMGsensor(EMGsensormod::Mode mode)
{
    initADC(); // enables ADC
    __MSX_HAL_MASK_SET(GPIOA->MODER, GPIO_MODER_MODER0); // PA0 set as analog

    if (mode == EMGsensormod::POLLING) // POLLING
    {
        EMGsensor::configurePolling();
    }
    else if (mode == EMGsensormod::INTERRUPT) // INTERRUPT
    {
        EMGsensor::configureInterrupt();
    }
    else //DMA
    {
        EMGsensor::configureDMA();
    }
}

EMGsensor::~EMGsensor() {}

/* Sensor configuration */
/**
 * initADC()
 * Enables ADC clock, sets cycles of conversion, turns ADC power on, selects channel
 * Access: private
 */
void EMGsensor::initADC()
{
    __MSX_HAL_MASK_SET(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);   // enabling ADC clock
    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_ADON);            // turning ADC on   
    __MSX_HAL_MASK_CLEAR(ADC1->SR, ADC_SR_EOC);             // resetting EOC state
    __MSX_HAL_MASK_CLEAR(ADC1->SQR3, ADC_SQR3_SQ1);         // channel 0 select
    __MSX_HAL_MASK_CLEAR(ADC1->CR1, ADC_CR1_RES);           // 12 bit resolution
    __MSX_HAL_MASK_CLEAR(ADC1->SQR1, ADC_SQR1_L);           // single conversion
    __MSX_HAL_MASK_SET(ADC1->SMPR1, ADC_SMPR2_SMP0);        // 480 cycles for channel 0
}

/**
 * configurePolling()
 * Configures ADC for polling
 * Access: private
 */
void EMGsensor::configurePolling()
{
    // no need for further configurations
    // left for possible future extension
    return;
}

/**
 * configureInterrupt()
 * Configures ADC for interrupt using EOCIE flag
 * Access: private
 */
void EMGsensor::configureInterrupt()
{
    __MSX_HAL_MASK_SET(ADC1->CR1, ADC_CR1_EOCIE); // enabling interrupt at EOC

    NVIC_EnableIRQ(ADC_IRQn);           // enabling NVIC for ADC
    NVIC_SetPriority(ADC_IRQn, 15);     // low priority 15

    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_CONT);   // continue conversion mode
    __MSX_HAL_MASK_CLEAR(ADC1->CR2, ADC_CR2_EOCS); // EOCS sequential conversion flag (for continuos conversions)  
    __MSX_HAL_MASK_SET(ADC->CCR, ADC_CCR_ADCPRE);  // Prescaler by 8
}


/* TODO */
void EMGsensor::configureDMA()
{
    return;
}

/**
 * readValue()
 * This function reads converted ADC value in the data register and returns it
 * Access: public
 * Return type: uint16_t
 */
uint16_t EMGsensor::readPolling()
{
    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_SWSTART);
    while (!__MSX_HAL_MASK_CHECK(ADC1->SR, ADC_SR_EOC))
        ;
    return ADC1->DR;
}

/**
 * getValue()
 * Wrapper function of get for Queue class
 * This function gets a value from the queue and returns it
 * If the queue is empty, the thread is put to sleep until notify
 * Access: public
 * Return type: uint16_t
 */
uint16_t EMGsensor::getValue()
{
    uint16_t emgValue = 0;
    EMGsensor::emgValues.get(emgValue);
    return emgValue;
}

/**
 * IRQgetValue()
 * getValue variant to be used inside IRQ
 * Wrapper function of IRQget for Queue class
 * This function gets a value from the queue and returns it
 * If the queue is empty, the thread is put to sleep until notify
 * Access: public
 * Return type: uint16_t
 */
uint16_t EMGsensor::IRQgetValue()
{
    uint16_t emgValue = 0;
    EMGsensor::emgValues.IRQget(emgValue);
    return emgValue;
}

/**
 * queueSize()
 * Wrapper function of size for Queue class
 * This function returns the size of the queue
 * Access: public
 * Return type: unsigned int
 */
unsigned int EMGsensor::queueSize()
{
    unsigned int size = 0;
    size = EMGsensor::emgValues.size();
    return size;
}

/**
 * putValue()
 * Wrapper function of put for Queue class
 * This function puts an ADC converted value into queue of values.
 * If the queue is full, the thread is put to sleep until notify
 * Access: public
 * Return type: void
 */
void EMGsensor::putValue(const uint16_t &val)
{
    EMGsensor::emgValues.put(val);
}

/**
 * IRQputValue()
 * putValue variant to be used inside IRQ
 * Wrapper function of IRQput for Queue class
 * This function puts an ADC converted value into queue of values.
 * This function returns true only if the queue is not full
 * Access: public
 * Return type: bool
 */
bool EMGsensor::IRQputValue(const uint16_t &val)
{
    bool valid = EMGsensor::emgValues.IRQput(val);
    return valid;
}

/**
 * isQueueFull()
 * Wrapper function of isFull for Queue class
 * This function returns true only if the queue is full
 * Access: public
 * Return type: bool
 */
bool EMGsensor::isQueueFull()
{
    bool full = EMGsensor::emgValues.isFull();
    return full;
}

/**
 * isQueueEmpty()
 * Wrapper function of isEmpty for Queue class
 * This function returns true only if the queue is empty
 * Access: public
 * Return type: bool
 */
bool EMGsensor::isQueueEmpty()
{
    bool empty = EMGsensor::emgValues.isEmpty();
    return empty;
}