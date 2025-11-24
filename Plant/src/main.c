#include "connector.h"
#define nucleoLed DT_ALIAS(led2)

const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);
const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(ledb), gpios);

const struct device *i2c;
const struct device *uart;
const struct device *adc;

uint8_t registers[2];

int16_t soilRawVal;
struct adc_channel_cfg soilCfg;
struct adc_sequence seq;

int16_t brightnessRawVal;
struct adc_channel_cfg brightnessCfg;
struct adc_sequence brightnessSeq;


enum Mode {TEST, NORMAL, ADVANCED};
enum Mode mode = TEST;

K_MSGQ_DEFINE(messageQueue, sizeof(struct measDataQueue), 8, 4);

bool measureTime = false;

void rgbLedInit() {
    if (!device_is_ready(led_r.port) ||
        !device_is_ready(led_g.port) ||
        !device_is_ready(led_b.port)) {
        printk("LED ports not ready\n");
        return;
    }

    gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_g, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_b, GPIO_OUTPUT_INACTIVE);
}

void i2cInit() {

    i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c)) {
        printk("i2c not ready\n");
        return;
    }
}

void uartInit() {

    uart = DEVICE_DT_GET(DT_NODELABEL(usart1));
    if (!device_is_ready(uart)){
        printk("uart not ready\n");
        return;
    }
}

void adcInit() {
    soilRawVal = 0;

    adc = DEVICE_DT_GET(DT_NODELABEL(adc1));
    if (!device_is_ready(adc)) {
        printk("adc not ready\n");
        return;
    }

     soilCfg = (struct adc_channel_cfg){
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = soilAdcChannel,
        .differential     = 0,
    };

    int err = adc_channel_setup(adc, &soilCfg);
    if (err < 0) {
        printk("adc_channel_setup error: %d\n", err);
        return;
    }

    seq = (struct adc_sequence){
    .channels    = BIT(soilAdcChannel),
    .buffer      = &soilRawVal,
    .buffer_size = sizeof(soilRawVal),
    .resolution  = adcRes,          
    };


    brightnessRawVal = 0;

    brightnessCfg = (struct adc_channel_cfg){
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = brightAdcChannel,
        .differential     = 0,
    };

    err = adc_channel_setup(adc, &brightnessCfg);
    if (err < 0) {
        printk("adc_channel_setup brightness error: %d\n", err);
        return;
    }

    brightnessSeq = (struct adc_sequence){
        .channels    = BIT(brightAdcChannel),
        .buffer      = &brightnessRawVal,
        .buffer_size = sizeof(brightnessRawVal),
        .resolution  = adcRes,
    };

}

void registersInput(uint8_t first, uint8_t second) {
    
    registers[0] = first;
    registers[1] = second;
}

void rgbInit(){
    
    registersInput(0x80 | 0x00, 0x01);      // COMMAND BIT | ENABLE & PON
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
    k_msleep(3);

    registersInput(0x80 | 0x00, 0x3);       // COMMAND BIT | ENABLE & PON | AEN
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
    
    registersInput(0x80 | 0x01, 0xD5);      // COMMAND BIT | ATIME  & 101ms
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
}

void accelerometerInit(){
    
    registersInput(0x2A, 0x01);     //CTRL_REG1 & ACTIVE MODE
    i2c_write(i2c,registers,sizeof(registers),accAddr);

    registersInput(0x0E, 0x00);     //DYNAMIC RANGE & ±2g
    i2c_write(i2c,registers,sizeof(registers),accAddr);

}
void measures(){
    here:
    while (measureTime){
        brightnessMeasure();

        rgbMeasure();

        accelerometerMeasure();

        temperatureMeasure();

        soilMeasure();
        
        gpsMeasure();
        struct measDataQueue m;

    while (k_msgq_get(&messageQueue, &m, K_NO_WAIT) == 0) {
        switch (m.type) {
        case soilDataQ:
            printk("Soil: %d\n", m.d.soilQ);
            break;
        case lightDataQ:
            printk("Light: %d\n", m.d.lightQ);
            break;
        case rgbDataQ:
            if ( mode == TEST ){
                if (m.d.rgbQ.r > m.d.rgbQ.g && m.d.rgbQ.r > m.d.rgbQ.b) rgbChange(1);
                else if (m.d.rgbQ.g > m.d.rgbQ.r && m.d.rgbQ.g > m.d.rgbQ.b ) rgbChange(2);
                else rgbChange(3);
            }
            printk("RGB: R=%u G=%u B=%u\n",
                m.d.rgbQ.r, m.d.rgbQ.g, m.d.rgbQ.b);
            break;
        case accDataQ:
            printk("Acc: X=%.2f Y=%.2f Z=%.2f\n",
                m.d.accQ.x, m.d.accQ.y, m.d.accQ.z);
            break;
        case tempDataQ:
            printk("Temp: %.2f C  Hum: %.2f %%\n",
                m.d.tempQ.temp, m.d.tempQ.hum);
            break;
        case gpsDataQ:
            printk("GPS: %s\n\n", m.d.gpsQ);
            break;
        default:
            break;
        }
    }
    measureTime = false;
}
    goto here;

}

K_THREAD_DEFINE(measureThread,4096,measures,NULL,NULL,NULL,1,0,0);

void main(void) {

    k_thread_suspend(measureThread);


    rgbLedInit();
    i2cInit();
    uartInit();
    adcInit();
    rgbInit();
    accelerometerInit();
    k_thread_resume(measureThread);

    static const struct gpio_dt_spec blueLed = GPIO_DT_SPEC_GET(nucleoLed, gpios);
    gpio_pin_configure_dt(&blueLed, GPIO_OUTPUT_INACTIVE);

    while (true){
        if (mode == TEST)
        {
            gpio_pin_set_dt(&blueLed,1);
            k_msleep(2000);
            measureTime = true;
        } 
    }


}