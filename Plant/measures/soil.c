#include "../src/connector.h"

#define SOIL_ADC_CHANNEL 0  

static int16_t soil_raw_value = 0;

void soilMeasure(void)
{
    static bool channel_configured = false;

    if (!device_is_ready(adc)) {
        printk("soilMeasure: adc not ready\n");
        k_msleep(500);
        return;
    }

    /* Один раз настраиваем канал АЦП */
    if (!channel_configured) {
        struct adc_channel_cfg soil_cfg = {
            .gain             = ADC_GAIN_1,
            .reference        = ADC_REF_INTERNAL,
            .acquisition_time = ADC_ACQ_TIME_DEFAULT,
            .channel_id       = SOIL_ADC_CHANNEL,
            .differential     = 0,
        };

        int err = adc_channel_setup(adc, &soil_cfg);
        if (err < 0) {
            printk("adc_channel_setup error: %d\n", err);
            k_msleep(1000);
            return;
        }
        channel_configured = true;
    }

    /* Описание одиночного измерения */
    struct adc_sequence seq = {
        .channels    = BIT(SOIL_ADC_CHANNEL),
        .buffer      = &soil_raw_value,
        .buffer_size = sizeof(soil_raw_value),
        .resolution  = adcRes,          // у тебя #define adcRes 12
    };

    int err = adc_read(adc, &seq);
    if (err < 0) {
        printk("adc_read error: %d\n", err);
    } else {
        printk("Soil moisture raw: %d\n", soil_raw_value);
    }

    /* Чтобы поток не жрал 100% CPU и не спамил лог */
    k_msleep(500);
}
