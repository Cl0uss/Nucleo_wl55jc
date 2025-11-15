#include "connector.h"

const struct device *i2c;
uint8_t registers[2];

enum Mode {TEST, NORMAL, ADVANCED};



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

void gpsInit(){
    return;
}

void measures(){
    //rgbMeasure();
    //accelerometerMeasure();
    temperatureMeasure();
}

K_THREAD_DEFINE(measureThread,4096,measures,NULL,NULL,NULL,1,0,0);
void main(void) {
    k_thread_suspend(measureThread);
    i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c)) {
        printk("i2c2 not ready");
    }
    rgbInit();
    accelerometerInit();
    k_thread_resume(measureThread);
    while (true){
        k_msleep(100);
    }


}