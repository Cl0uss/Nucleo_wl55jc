#include "connector.h"

const struct device *i2c;
const struct device *uart;
const struct device *adc;

uint8_t registers[2];

enum Mode {TEST, NORMAL, ADVANCED};

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

    adc = DEVICE_DT_GET(DT_NODELABEL(adc1));
    if (!device_is_ready(adc)) {
        printk("adc not ready\n");
        return;
    }
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
    while (true){
    rgbMeasure();
    k_msleep(5000);

    accelerometerMeasure();
    k_msleep(5000);

    temperatureMeasure();
    k_msleep(5000);

    gpsMeasure();
    k_msleep(5000);

    soilMeasure();
    k_msleep(5000);

    }
}

K_THREAD_DEFINE(measureThread,4096,measures,NULL,NULL,NULL,1,0,0);

void main(void) {
    
    k_thread_suspend(measureThread);

    i2cInit();
    uartInit();
    adcInit();
    rgbInit();
    accelerometerInit();
    k_thread_resume(measureThread);

    while (true){
        k_msleep(10000);
    }


}