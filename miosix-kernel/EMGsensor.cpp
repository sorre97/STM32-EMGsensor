#include "EMGsensor.h"
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
    initADC();                         // enables ADC
    GPIOA->MODER |= GPIO_MODER_MODER0; // PA0 set as analog

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
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // enabling ADC clock
    ADC1->SR &= ~(ADC_SR_EOC);          // resetting EOC state
    ADC1->SQR3 &= ~(ADC_SQR3_SQ1);      // channel 0 select
    ADC1->SQR1 &= ~(ADC_SQR1_L);        // single conversion
    ADC1->SMPR1 |= ADC_SMPR2_SMP0;      // 480 cycles for channel 0
    ADC1->CR2 |= (ADC_CR2_ADON);        // turning ADC on
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
    ADC1->CR1 |= ADC_CR1_EOCIE;         // enabling interrupt at EOC

    NVIC_EnableIRQ(ADC_IRQn);           // enabling NVIC for ADC
    NVIC_SetPriority(ADC_IRQn, 5);      // low priority 15

    //ADC1->CR2 |= (ADC_CR2_CONT);      // continue conversion mode
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
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC))
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
    uint16_t emgValue;
    EMGsensor::emgValues.get(emgValue);
    return emgValue;
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