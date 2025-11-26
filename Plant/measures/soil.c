#include "../src/connector.h"

// on air (min) - 400   on fingers - 3110

void soilMeasure(void)
{
    adc_read(adc, &soilSeq);
    soilValue = (soilRawVal - 400.0f) * 100.0f / (3110.0f - 400.0f);
}
