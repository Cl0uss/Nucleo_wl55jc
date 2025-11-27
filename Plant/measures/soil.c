#include "../src/connector.h"

// on earth (min) - 900   wet water (max) - 3600

void soilMeasure(void)
{
    adc_read(adc, &soilSeq);
    printk("soilRawVal = %d\n",soilRawVal);
    soilValue = (soilRawVal - 900.0f) * 100.0f / (3600.0f - 900.0f);
}
