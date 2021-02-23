

// includes
#include <cstdio>
#include <miosix.h>
#include <miosix/kernel/scheduler/scheduler.h>
#include <thread>
#include "EMGsensor.h"
//#include "stm32f401xe.h"

// namespaces
using namespace std;
using namespace miosix;

// static variables
static EMGsensor emgSensor(EMGsensormod::INTERRUPT);
static Thread *waiting=nullptr;


void __attribute__((naked)) ADC_IRQHandler()
{
    saveContext();
    asm volatile("bl _Z18ADC_IRQHandlerImplv");
    restoreContext();
}

void __attribute__((used)) ADC_IRQHandlerImpl()
{
    NVIC_ClearPendingIRQ(ADC_IRQn);     
    ADC1->SR &= ~(ADC_SR_EOC);          // resetting EOC state
    
    if(waiting==nullptr) return;
    waiting->IRQwakeup();

    if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority()) 
    {   
        Scheduler::IRQfindNextThread();
    }

    waiting=nullptr;
}


/*
* samplingLoop()
* This function is runned by the sampling thread is used to sample data coming from 
* the sensor. The ADC readout is launched everytime in interrupt mode.
* While no values are converted, this thread goes to sleep.
* Everytime the ADC converts a value, the EOCIE bit is set to 1 triggering
* the interrupt on ADC_IRQHandler that wakes up this thread and the new data is put 
* inside the synch queue of values inside EMGSensor class
*/
void samplingLoop()
{
    while(1)
    {
        ADC1->CR2 |= ADC_CR2_SWSTART;        // start ADC (For now, started by software. Next: started by TIM)
        
        FastInterruptDisableLock dLock;
        waiting=Thread::IRQgetCurrentThread();

        // Sleep until new samples arrive...
        while(waiting)
        {
            Thread::IRQwait();
            FastInterruptEnableLock eLock(dLock);
            Thread::yield();
        }

        uint16_t emgVal = ADC1->DR;
        emgSensor.putValue(emgVal);
    }
}

int main()
{ 
    uint16_t emgVal = 0;
    thread samplingThread(samplingLoop); // launching sample thread

    while(1)
    {   
        // Get emg sensor value
        emgVal = emgSensor.getValue(); 
        printf("ADC value: %d\n", emgVal);
        //this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // should never reach here
    samplingThread.join();
    return 0;
}

