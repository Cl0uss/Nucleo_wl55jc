#include "../src/connector.h"



void soilMeasure(void)
{

    int err = adc_read(adc, &seq);
    if (err != 0) {
        printk("adc_read error: %d\n", err);
    } else {
        struct measDataQueue msg = { .type = soilDataQ, .d.soilQ = soilRawVal };
        k_msgq_put(&messageQueue, &msg, K_NO_WAIT);
    }
}
