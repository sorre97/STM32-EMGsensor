
/* includes */
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
#include "filtfilt.h"

/* private defines */
#define DEBUG 1

/* namespaces */
using namespace std;
using namespace miosix;

/* EMG sensor init */
static EMGsensor emgSensor(EMGsensormod::INTERRUPT);
static Thread *waiting=nullptr;

/* contants */
const unsigned int _BUFF_SIZE = 300;
const unsigned int _NUM_BUFF = 5;
const unsigned int _BUFF_SIZE2 = 300;
const unsigned int _NUM_BUFF2 = 3;

/* buffer ADC - DSPloop */
static BufferQueue<uint16_t, _BUFF_SIZE, _NUM_BUFF> bufferQueue;
static unsigned int currPos = 0;
static uint16_t supportBuffer[_BUFF_SIZE] = { 0 };

/* buffer DSPloop - UART */
static BufferQueue<float, _BUFF_SIZE2, _NUM_BUFF2> bufferQueue2;
static condition_variable consumer_cv, producer_cv;
static mutex m;

/* DSP filtfilt variables */
static vectord b_coeff_bandpass = { 0.199880906801133,0,-0.599642720403399,0,0.599642720403399,0,-0.199880906801133 };
static vectord a_coeff_bandpass = { 1,-2.13183455555828,1.47978011393210,-0.679740843101842,0.584825906895303,-0.218461835750097,-0.0211926261278646 };
static vectord b_coeff_bandstop = { 0.991153595101663,-3.77064677042227,5.56847615976590,-3.77064677042227,0.991153595101663 };
static vectord a_coeff_bandstop = { 1,-3.78739953308251,5.56839789935512,-3.75389400776205,0.982385450614124 };
static vectord input_signal;
static vectord y_filtfilt_out;
static vectord y_filtfilt_out2;

/**
* ADC_IRQHandlerImpl()
* Interrupt routine for ADC endOfConversionCallback. Each converted sample is
* stored inside a buffer to be sent to the DSP thread to be processed.
* As soon as the first buffer is filled, the DSPloop thread is woken up to
* start DSP on sampled data while the interrupt routine fills another buffer.
*/
void __attribute__((used)) ADC_IRQHandlerImpl()
{
    NVIC_ClearPendingIRQ(ADC_IRQn);   // clearing ADC IRQ
    __MSX_HAL_REG_CLEAR(TIM3->SR);    // clear interrupt flag

    uint16_t emgVal = ADC1->DR;       // retrieving ADC converted value
    

    // filling support buffer
    if(currPos < _BUFF_SIZE)
    {
        supportBuffer[currPos] = emgVal;
        ++currPos;
    }

    // buffer full and ready to be processed
    if(currPos >= _BUFF_SIZE)
    {
        uint16_t * buff = nullptr;
        bool valid = false;

        valid = bufferQueue.tryGetWritableBuffer(buff);

        if(valid)
        {   
            // copying support buffer into Nth buffer in queue
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

/**
* DSPloop()
* This thread is launched by the main thread. It is used to perform ditigal 
* signal processing on data coming from the ADC. When the ADC fills at lest one 
* buffer, this thread is woken up and processing starts. 
* Each processed value is then inserted into another buffer ready to be printed 
* by the UART controller by the main thread. This thread goes in wait state until
* either buffer is made available by the ADC_IRQ or no buffer shared with main thread
* is available for writing. Notifies the main thread after filling up a buffer.
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

            // wait until woken up by the ADC_IRQ
            while(!bufferQueue.tryGetReadableBuffer(buff, buff_size))
            {
                waiting->IRQwait();
                FastInterruptEnableLock eLock(dLock);
                Thread::yield();
            }
        }

        /** ADC -> DSP **/
        
        // reading values from buffer
        input_signal.insert(input_signal.end(), &buff[0], &buff[buff_size]);
        bufferQueue.bufferEmptied();
        
        // bandpass (30hz-300hz)
        filtfilt(b_coeff_bandpass, a_coeff_bandpass, input_signal, y_filtfilt_out);
        input_signal.clear();

        // bandstop (50hz)
        filtfilt(b_coeff_bandstop, a_coeff_bandstop, y_filtfilt_out, y_filtfilt_out2);
        y_filtfilt_out.clear();

        /** DSP -> USART **/
        float * buff2 = nullptr;
        
        // passing value to USART shared buffer
        {
            unique_lock<mutex> lock(m);
            while(!bufferQueue2.tryGetWritableBuffer(buff2))
            {
                producer_cv.wait(lock);
            }
        }

        for(size_t i = 0; i < _BUFF_SIZE2; ++i)
        {
            buff2[i] = y_filtfilt_out2[i];  
        }
    
        y_filtfilt_out2.clear();
        bufferQueue2.bufferFilled(_BUFF_SIZE2);
        consumer_cv.notify_all();
    }
}

/**
 * main()
 * This thread is responsible of communicating via USART converted values recieved
 * via the shared buffer with the DSP thread. The main thread waits on the condition
 * variable if no buffer are available. Notifies DSP thread after making available
 * at least one buffer by consuming it.
 */
int main()
{ 
    thread DSPThread(DSPloop); // launching DSP thread

    while(1)
    {  
        // main code
        const float * buff = nullptr;
        unsigned int buff_size = 0;

        // accessing read buffer
        {
            unique_lock<mutex> lock(m);
            while(!bufferQueue2.tryGetReadableBuffer(buff, buff_size))
            {
                consumer_cv.wait(lock);
            }
        }

        // UART serial communication
        for(size_t i = 0; i < buff_size; ++i)
        {
            printf("%.4f\n", buff[i]);
        }

        bufferQueue2.bufferEmptied();
        producer_cv.notify_all();
        
    }

    // should never reach here
    DSPThread.join();
    return 0;
}

