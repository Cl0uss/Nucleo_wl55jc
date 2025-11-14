#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel/stats.h>
#include <zephyr/arch/arch_interface.h>
#include "connector.h"

const struct device *i2c;
uint8_t registers[2];

enum Mode {TEST, NORMAL, ADVANCED};



void registersInput(uint8_t first, uint8_t second) {
    registers[0] = first;
    registers[1] = second;
}

void rgbInit(){
    registersInput(0x80 | 0x00, 0x01);
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
    k_msleep(3);
    registersInput(0x80 | 0x00, 0x3);
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
}

void accelerometerInit(){
    return;
}

void tempInit(){
    return;
}


void gpsInit(){
    return;
}


void measures(){
    return;
}

K_THREAD_DEFINE (measureThread,4096,measures,NULL,NULL,NULL,0,0,0);

void main(void) {
    
    i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c)) {
        printk("i2c2 not ready");
    }

    while (true){

    }


}