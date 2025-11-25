#include "../src/connector.h"

// brightness: 0% ≈ 30 | 100% = 4095 (formula: (value-30) / 40.65 )  (value - min) / (max - min) * 100

void brightnessMeasure(){
    int err = adc_read(adc, &brightnessSeq);
    if (err < 0) {
        printk("Brightness adc_read error: %d\n", err);
    } else {
        struct measDataQueue msg = { .type = lightDataQ, .d.lightQ = (brightnessRawVal-30)/40.65 };
        //struct measDataQueue msg = { .type = lightDataQ, .d.lightQ = brightnessRawVal };
        k_msgq_put(&messageQueue, &msg, K_NO_WAIT);
    }
}