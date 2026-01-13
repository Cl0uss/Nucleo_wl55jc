#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/adc.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#define rgbAddr 0x29  
#define accAddr 0x1D
#define tempAndHumidAddr 0x40
#define adcRes 12
#define soilAdcChannel 0
#define brightAdcChannel 5 

struct measDataQueue {
    char gpsQ[128];
};
extern struct measDataQueue gpsMsg;
//extern const struct device *adc;
extern const struct device *i2c;
extern const struct device *uart;
extern const struct device *port;

void rgbChange(int);
void rgbMeasure(void);
void accelerometerMeasure(void);
void temperatureMeasure(void);
void gpsMeasure(void);
void gps_init(void);
bool gps_get_latlon(int32_t *lat_e6, int32_t *lon_e6);
bool gps_is_real(void);
void distanceMeasure(void);
void soilMeasure(void);
void brightnessMeasure(void);
void gpsToHuman(int main);


extern float axisX;
extern float axisY;
extern float axisZ;
extern float lightValue;
extern uint16_t clear;
extern uint16_t red;
extern uint16_t green;
extern uint16_t blue;
extern float soilValue;
extern float tempValue;
extern float humValue;
extern uint32_t distanceVal;
extern uint32_t distanceVal;

extern const struct gpio_dt_spec led_r;
extern const struct gpio_dt_spec led_g;
