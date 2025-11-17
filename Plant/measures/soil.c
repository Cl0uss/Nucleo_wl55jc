#include "../src/connector.h"



void soilMeasure(void)
{

    int err = adc_read(adc, &seq);
    if (err < 0) {
        printk("adc_read error: %d\n", err);
    } else {
        printk("Soil moisture raw: %d\n", soilRawVal);
    }

    k_msleep(500);
}
