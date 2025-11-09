#include "connector.h"
#include "rgbSensor.h"

enum Mode {TEST, NORMAL, ADVANCED};
enum Mode mode = TEST;

const struct device *i2c;

void bufferChanger(uint8_t *buff, uint8_t first, uint8_t second) {
    buff[0] = first;
    buff[1] = second;
}

void rgbInit() {
    uint8_t buff[2];

    bufferChanger(buff, rgbCommandBit | rgbEnable, rgbEnablePon);
    i2c_write(i2c,buff,sizeof(buff),rgbAddr);
    
    k_msleep(3);
    bufferChanger(buff, rgbCommandBit | rgbEnable, rgbEnableAen | rgbEnablePon);
    i2c_write(i2c,buff,sizeof(buff),rgbAddr);

    bufferChanger(buff, rgbCommandBit | rgbAtime, 0xD5);
    i2c_write(i2c,buff,sizeof(buff),rgbAddr);

    bufferChanger(buff, rgbCommandBit | rgbControl, 0x00);
    i2c_write(i2c,buff,sizeof(buff),rgbAddr);

    bufferChanger(buff, rgbCommandBit | rgbWaitTime, rgbWaitValue);
    i2c_write(i2c, buff, sizeof(buff), rgbAddr);
}


static void i2cSensors(void) {
    accelerometerSensor();
}

static void analogSensors(void) {
    brightnessSensor();
}




K_THREAD_DEFINE(uartThread,   1024, gpsSensor,     NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(i2cThread,    4096, i2cSensors,    NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(analogThread, 1024, analogSensors, NULL, NULL, NULL, 1, 0, 0);


void main(void) {
    k_thread_suspend(uartThread);
    k_thread_suspend(i2cThread);
    k_thread_suspend(analogThread);
 
    while (mode == TEST) {
        k_thread_resume(i2cThread);
        k_msleep(2000);
    }
}
