#ifndef EMGsensor_H
#define EMGsensor_H

#include <miosix.h>

using namespace std;

namespace EMGsensormod
{
    // Sensor mode
    enum Mode { 
        POLLING = 0,
        INTERRUPT,
        DMA 
    };
}

class EMGsensor
{
public:
    /* Constructor */
    EMGsensor();
    /* Secondary constructor */
    EMGsensor(EMGsensormod::Mode mode);
    /* Distructor */
    ~EMGsensor();
    /* Read ADC value */
    uint16_t readValue();

private:
    // Variables

    // Functions
    /* Configure EMG sensor in polling mode */
    void configurePolling();
    /* Configure EMG sensor in interrupt mode */
    void configureInterrupt();
    /* Configure EMG sensor in DMA mode */
    void configureDMA();
    /* Assignment operator */
    EMGsensor &operator=(const EMGsensor &other) = delete;
    /* Copy constructor */
    EMGsensor(const EMGsensor &other) = delete;
    
}; // End of EMGsensor class

#endif // End of EMGsensor_H