#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/kernel/stats.h>
#include <zephyr/arch/arch_interface.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/adc.h>
#include <math.h>

#define rgbAddr 0x29  
#define accAddr 0x1D
#define tempAndHumidAddr 0x40
#define adcRes 12
#define soilAdcChannel 0
#define brightAdcChannel 5 

extern const struct gpio_dt_spec led_r;
extern const struct gpio_dt_spec led_g;
extern const struct gpio_dt_spec led_b;

extern const struct device *adc;
extern const struct device *i2c;
extern const struct device *uart;

extern struct adc_channel_cfg soilCfg;
extern struct adc_sequence seq;
extern int16_t soilRawVal;

extern struct adc_channel_cfg brightnessCfg;
extern struct adc_sequence brightnessSeq;
extern int16_t brightnessRawVal;

extern bool permission;

enum measType {
    soilDataQ,
    lightDataQ,
    rgbDataQ,
    accDataQ,
    tempDataQ,
    gpsDataQ
};

struct measDataQueue {
    enum measType type;
    union {
        int16_t soilQ;
        int16_t lightQ;
        struct { uint16_t r,g,b; } rgbQ;
        struct { float x,y,z; } accQ;
        struct { float temp, hum; } tempQ;
        char gpsQ[128];
    } d;
};

extern struct k_msgq messageQueue;

void rgbChange(int);
void rgbMeasure(void);
void accelerometerMeasure(void);
void temperatureMeasure(void);
void gpsMeasure(void);
void soilMeasure(void);
void brightnessMeasure(void);