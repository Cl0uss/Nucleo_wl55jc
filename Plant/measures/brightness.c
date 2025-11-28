#include "../src/connector.h"

// brightness: 0% ≈ 30 | 100% = 4095 (formula: (value-30) / 40.65 )  (value - min) / (max - min) * 100

void brightnessMeasure(){
    
    int err = adc_read(adc, &brightnessSeq);
    if (err < 0) {
        printk("Brightness adc_read error: %d\n", err);
    } else {
        lightValue = (brightnessRawVal-30)/40.65;
    }
}