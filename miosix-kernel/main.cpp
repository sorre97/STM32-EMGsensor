
#include <cstdio>
#include <miosix.h>
#include "EMGsensor.h"

using namespace std;
using namespace miosix;

int main()
{
    EMGsensor emgSensor(EMGsensormod::POLLING);
    uint16_t emgVal = 0;
    while(1)
    {
        emgVal = emgSensor.readValue();
        printf("ADC value: %d\n", emgVal);
    }
}
