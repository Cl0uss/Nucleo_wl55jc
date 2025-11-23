#include "../src/connector.h"

void brightnessMeasure(){
    int err = adc_read(adc, &brightnessSeq);
    if (err < 0) {
        printk("Brightness adc_read error: %d\n", err);
    } else {
        printk("Brightness raw: %d\n", brightnessRawVal);
    }
    k_msleep(500);
}