#include "../src/connector.h"

//  (min) - 200   wet water (max) - 3600

void soilMeasure(void){

    adc_read(adc, &soilSeq);
    //printk("soilRawVal = %d\n",soilRawVal);
    soilValue = (soilRawVal - 200.0f) * 100.0f / (3600.0f - 200.0f);
}
