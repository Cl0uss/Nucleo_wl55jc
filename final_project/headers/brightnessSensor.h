#define ADC_RESOLUTION      12
#define ADC_GAIN_SETTING    ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID       5  // PB1 / A0 / ADC1_IN5

static const struct device *adc_dev;
static int16_t sample_buffer;