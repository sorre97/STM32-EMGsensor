#include "EMGsensor.h"

/**
 * EMGsensor()
 * Constructor initializes the peripherals in POLLING mode using default pins
 * Pins: Vin(analog) -> PA0 (ADC1_0)
 * Mode: POLLING
 */
EMGsensor::EMGsensor() : EMGsensor::EMGsensor(EMGsensormod::POLLING) {}

/**
 * EMGsensor(Mode mode)
 * Constructor initializes the peripherals in selected mode using default pins if not selected
 * Input: Mode mode. Selects operative mode of the sensor
 * Pins: 
 * Mode: POLLING, INTERRUPT, DMA
 */
EMGsensor::EMGsensor(EMGsensormod::Mode mode)
{
    if (mode == EMGsensormod::POLLING)  // POLLING
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
/* TODO */
void EMGsensor::configurePolling()
{
    return;
}

/* TODO */
void EMGsensor::configureInterrupt()
{
    return;
}

/* TODO */
void EMGsensor::configureDMA()
{
    return;
}

/* Sensor reading */
/**
 * uint16_t readValue()
 * This function reads converted ADC value in the register and returns it
 * Access: public
 * Return type: uint16_t
 */
uint16_t EMGsensor::readValue()
{
    return 0;
}