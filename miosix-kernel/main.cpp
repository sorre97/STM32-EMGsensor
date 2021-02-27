

// includes
#include <cstdio>
#include <miosix.h>
#include <miosix/kernel/scheduler/scheduler.h>
#include <thread>
#include <queue>
#include "EMGsensor.h"
#include "MSX_HAL.h"
//#include "stm32f401xe.h"

// private defines
#define DEBUG 1

// namespaces
using namespace std;
using namespace miosix;

// static variables
static EMGsensor emgSensor(EMGsensormod::INTERRUPT);
static Thread *waiting=nullptr;

void __attribute__((used)) ADC_IRQHandlerImpl()
{

    NVIC_ClearPendingIRQ(ADC_IRQn);   // clearing ADC IRQ
    __MSX_HAL_REG_CLEAR(TIM3->SR);    //Clear interrupt flag
    
    uint16_t emgVal = ADC1->DR;
    emgSensor.IRQputValue(emgVal);
    
    // For debug porpuse. If queue is full, led is on
    if(DEBUG)
    {
        if(emgSensor.isQueueFull()) { ledOn(); } 
        else { ledOff(); }
    }

    /*
    // Waking up thread
    if(waiting==nullptr) return;
    waiting->IRQwakeup();

    if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority()) 
    {   
        Scheduler::IRQfindNextThread();
    }

    waiting=nullptr;
    }
    */
}

void __attribute__((naked)) ADC_IRQHandler()
{
    saveContext();
    asm volatile("bl _Z18ADC_IRQHandlerImplv");
    restoreContext();
}

/*
TO BE CHANGED, THIS LOOP IS ONLY WOKEN UP EVERY N SAMPLES AND PERFORMS OPERATIONS WITH IT
* samplingLoop()
* This function is runned by the sampling thread is used to sample data coming from 
* the sensor. The ADC readout is launched everytime in interrupt mode.
* While no values are converted, this thread goes to sleep.
* Everytime the ADC converts a value, the EOCIE bit is set to 1 triggering
* the interrupt on ADC_IRQHandler that wakes up this thread and the new data is put 
* inside the synch queue of values inside EMGSensor class
*/
void DSPloop()
{
    while(1)
    {    
        FastInterruptDisableLock dLock;
        waiting=Thread::IRQgetCurrentThread();

        // Sleep until new N samples arrive...
        while(waiting)
        {
            Thread::IRQwait();
            FastInterruptEnableLock eLock(dLock);
            Thread::yield();
        }
    }
}

int main()
{ 
    uint16_t emgVal = 0;
    thread DSPThread(DSPloop); // launching DSP thread

    while(1)
    {   
        // Get emg sensor value
        emgVal = emgSensor.getValue();
        printf("%u\n", emgVal);

        //this_thread::sleep_for(std::chrono::milliseconds(4000));
    }

    // should never reach here
    DSPThread.join();
    return 0;
}

