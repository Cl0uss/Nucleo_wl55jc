#include "../src/connector.h"

// on air (min) - 450   on fingers - 3110

void soilMeasure(void)
{
    adc_read(adc, &soilSeq);
    float percent = (soilRawVal - 450.0f) * 100.0f / (3110.0f - 450.0f);
    struct measDataQueue msg = { .type = soilDataQ, .d.soilQ = percent };
    k_msgq_put(&messageQueue, &msg, K_NO_WAIT);
}
