#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/arch/arch_interface.h>
#include <zephyr/kernel/stats.h>
#include <stdio.h>
#include <math.h>

void accelerometerSensor(); //i2c2
void brightnessSensor();    //analog 
void earthSensor();         //analog
void rgbSensor();           //i2c2
void temperatureSensor();   //i2c2
void gpsSensor();           //UART



extern const struct device *i2c;

