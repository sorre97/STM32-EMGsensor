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
    //__MSX_HAL_MASK_SET(ADC1->SMPR1, ADC_SMPR2_SMP0);        // 480 cycles for channel 0
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
    NVIC_EnableIRQ(ADC_IRQn);           // enabling NVIC for ADC
    NVIC_SetPriority(ADC_IRQn, 5);     // medium priority 5

    __MSX_HAL_MASK_SET(ADC1->CR1, ADC_CR1_EOCIE);    // enabling interrupt at EOC
    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_EOCS);     // EOCS single conversion flag
    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_EXTEN_0);  // external trigger detection on rising edge
    __MSX_HAL_MASK_SET(ADC1->CR2, ADC_CR2_EXTSEL_3); // trigger on external event TIM3 TRGO

    EMGsensor::initTim();
}

/**
 * initTim()
 * Configures TIM3 to output trigger event with Xhz frequency
 * TIM3 frequency = 84Mhz / (counter+1 + prescaler+1)
 * Access: private
 */
void EMGsensor::initTim()
{
    {
        miosix::FastInterruptDisableLock dLock;
        __MSX_HAL_MASK_SET(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);   // enable TIM3 clock
        RCC_SYNC();
    }

    __MSX_HAL_MASK_CLEAR(TIM3->SMCR, TIM_SMCR_SMS); // internal clock
    __MSX_HAL_MASK_SET(TIM3->CR1, TIM_CR1_CKD_0);   // tDTS=tCK_INT


    __MSX_HAL_MASK_CLEAR(TIM3->CR1, TIM_CR1_CMS);   // edge align mode
    __MSX_HAL_MASK_CLEAR(TIM3->CR1, TIM_CR1_DIR);   // upcounting
    __MSX_HAL_MASK_SET(TIM3->CR2, TIM_CR2_MMS_1);   // update event as output trigger

    // 500Hz sampling
    __MSX_HAL_REG_SET(TIM3->PSC, 42-1);             // prescaler 
    __MSX_HAL_REG_SET(TIM3->ARR, 1000-1);           // counter register, SHOULD BE 2000, for some reason 42Mhz and not 84Mhz

    __MSX_HAL_MASK_SET(TIM3->EGR, TIM_EGR_UG);      // update ARR shadow register
    __MSX_HAL_REG_CLEAR(TIM3->SR);                  // clear interrupt flag caused by setting UG

    __MSX_HAL_MASK_SET(TIM3->CR1, TIM_CR1_CEN);     // enabling timer
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