

// includes
#include <cstdio>
#include <miosix.h>
#include <miosix/kernel/scheduler/scheduler.h>
#include <thread>
#include <queue>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include "EMGsensor.h"
#include "MSX_HAL.h"

//#include "stm32f401xe.h"

// private defines
#define DEBUG 1

// namespaces
using namespace std;
using namespace miosix;

// contants
const unsigned int _BUFF_SIZE = 64;
const unsigned int _NUM_BUFF = 2;
const unsigned int _BUFF_SIZE2 = 1;
const unsigned int _NUM_BUFF2 = 10;

// static variables
static EMGsensor emgSensor(EMGsensormod::INTERRUPT);

static Thread *waiting=nullptr;

// buffer ADC - DSPloop
static BufferQueue<uint16_t, _BUFF_SIZE, _NUM_BUFF> bufferQueue;
static unsigned int currPos = 0;
static uint16_t supportBuffer[_BUFF_SIZE] = { 0 };

// buffer DSPloop - UART
static BufferQueue<float, _BUFF_SIZE2, _NUM_BUFF2> bufferQueue2;
static float supportBuffer2[_BUFF_SIZE2] = { 0 };
static unsigned int currPos2 = 0;
static condition_variable cv;
static mutex m;

/*
* ADC_IRQHandlerImpl()
* Interrupt routine for ADC endOfConversionCallback. Each converted sample is
* stored inside a buffer to be sent to the DSP thread to be processed.
* As soon as the first buffer is filled, the DSPloop thread is woken up to
* start DSP on sampled data.
*/
void __attribute__((used)) ADC_IRQHandlerImpl()
{
    NVIC_ClearPendingIRQ(ADC_IRQn);   // clearing ADC IRQ
    __MSX_HAL_REG_CLEAR(TIM3->SR);    // clear interrupt flag

    uint16_t emgVal = ADC1->DR;
    
    if(currPos < _BUFF_SIZE)
    {
        supportBuffer[currPos] = emgVal;
        ++currPos;
    }

    if(currPos >= _BUFF_SIZE)
    {
        uint16_t * buff = nullptr;
        bool valid = false;

        {
            FastInterruptDisableLock dLock;
            valid = bufferQueue.tryGetWritableBuffer(buff);
        }

        if(valid)
        {   
            for(size_t i = 0; i < _BUFF_SIZE; ++i)
            {
                buff[i] = supportBuffer[i];   
            }

            bufferQueue.bufferFilled(_BUFF_SIZE);

            // resetting condition
            currPos = 0;
            memset(supportBuffer, 0, sizeof(supportBuffer));

            // waking up thread
            waiting->IRQwakeup();
            if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority()) 
            {   
                Scheduler::IRQfindNextThread();
            }
        }
    }
}

void __attribute__((naked)) ADC_IRQHandler()
{
    saveContext();
    asm volatile("bl _Z18ADC_IRQHandlerImplv");
    restoreContext();
}

/*
* DSPloop()
* This thread is launched by the main thread. It is used to perform ditigal 
* signal processing on data coming from the ADC. When the ADC fills at lest 
* one buffer, this thread is woken up and processing starts. 
* Each processed value is then inserted into another buffer ready
* to be printed by the UART controller (in the main).
*/
void DSPloop()
{
    const uint16_t * buff = nullptr;
    unsigned int buff_size = 0;

    while(1)
    {
        {
            FastInterruptDisableLock dLock;
        
            waiting=Thread::IRQgetCurrentThread();

            while(!bufferQueue.tryGetReadableBuffer(buff, buff_size))
            {
                waiting->IRQwait();
                FastInterruptEnableLock eLock(dLock);
                Thread::yield();
            }
        }

        /** ADC -> DSP **/

        // example of DSP, average value
        float avg = 0.0;
        for(size_t i = 0; i < buff_size; ++i)
        {
            avg += buff[i];
        }
        avg = avg * 0.002; // avg = sum/500
        
        bufferQueue.bufferEmptied();

        /** DSP -> UART **/
        if(currPos2 < _BUFF_SIZE2)
        {
            supportBuffer2[currPos2] = avg;
            ++currPos2;
        }

        if(currPos2 >= _BUFF_SIZE2)
        {
            float * buff2 = nullptr;
            {
                unique_lock<mutex> lock(m);
                while(!bufferQueue2.tryGetWritableBuffer(buff2))
                {
                    cv.wait(lock);
                }
            }

            for(size_t i = 0; i < _BUFF_SIZE2; ++i)
            {
                buff2[i] = supportBuffer2[i];   
            }
            cv.notify_all();
            bufferQueue2.bufferFilled(_BUFF_SIZE2);

            // resetting condition
            currPos2 = 0;
            memset(supportBuffer2, 0, sizeof(supportBuffer2));
        }
    }
}

int main()
{ 
    thread DSPThread(DSPloop); // launching DSP thread

    while(1)
    {  
        // main code
        const float * buff = nullptr;
        unsigned int buff_size = 0;

        {
            unique_lock<mutex> lock(m);
            while(!bufferQueue2.tryGetReadableBuffer(buff, buff_size))
            {
                cv.wait(lock);
            }
        }

        for(size_t i = 0; i < buff_size; ++i)
        {
            printf("Avg: %.3f\n", buff[i]);
        }
        bufferQueue2.bufferEmptied();
    }

    // should never reach here
    DSPThread.join();
    return 0;
}

