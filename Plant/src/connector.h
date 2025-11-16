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

#define rgbAddr 0x29  
#define accAddr 0x1D
#define tempAndHumidAddr 0x40
#define adcRes 12

extern const struct device *adc;
extern const struct device *i2c;
extern const struct device *uart;


void rgbMeasure(void);
void accelerometerMeasure(void);
void temperatureMeasure(void);
void gpsMeasure(void);
void soilMeasure(void);