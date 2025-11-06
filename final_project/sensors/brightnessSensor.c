#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

#include "brightnessSensor.h"

void brightnessSensor(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc1));
    if (!device_is_ready(adc_dev)) {
        printk("Error: ADC1 not ready!\n");
        return;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_SETTING,
        .reference        = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id       = ADC_CHANNEL_ID,
        .differential     = 0,
    };

    int ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret != 0) {
        printk("ADC channel setup failed (%d)\n", ret);
        return;
    }

    struct adc_sequence sequence = {
        .channels    = BIT(ADC_CHANNEL_ID),
        .buffer      = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    printk("Brightness sensor started...\n");

    while (1) {
        ret = adc_read(adc_dev, &sequence);
        if (ret == 0) {
            int16_t val = sample_buffer;
            printk("Light level (ADC): %d / 4095\n", val);
        } else {
            printk("ADC read error (%d)\n", ret);
        }

        k_msleep(1000);
    }
}
