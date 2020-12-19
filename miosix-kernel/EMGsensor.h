#ifndef EMGsensor_H
#define EMGsensor_H

#include <miosix.h>
#include <queue.h>

using namespace std;

namespace EMGsensormod
{
    // Sensor mode
    enum Mode
    {
        POLLING = 0,
        INTERRUPT,
        DMA
    };
} // namespace EMGsensormod

class EMGsensor
{
public:
    /* Constructor */
    EMGsensor();
    /* Secondary constructor */
    EMGsensor(EMGsensormod::Mode mode);
    /* Distructor */
    ~EMGsensor();
    /* Read ADC1 value from data register using polling */
    uint16_t readPolling();
    /* Get value from synch queue */
    uint16_t getValue();

private:
    // Variables
    /* List of emg sensor values */
    miosix::Queue<uint16_t, 20> emgValues;

    // Functions
    /* Enables ADC */
    void initADC();
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