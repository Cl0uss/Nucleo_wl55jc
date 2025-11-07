#include "connector.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

enum Mode mode = TEST;

static void i2cSensors(void) {

    while (true){
        k_msleep(1000);
    }
}

static void analogSensors(void) {
    brightnessSensor();
}

K_THREAD_DEFINE(uartThread, STACKSIZE, gpsSensor, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(i2cThread, STACKSIZE, i2cSensors, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(analogThread,STACKSIZE,analogSensors,NULL,NULL,NULL,1,0,0);

void main(void)
{
    k_thread_suspend(uartThread);
    k_thread_suspend(i2cThread);
    k_thread_suspend(analogThread);
    k_thread_resume(uartThread);
    while (1) { 
        k_msleep(1000); 
    }
}
