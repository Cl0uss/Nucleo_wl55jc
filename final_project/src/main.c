#include "connector.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACKSIZE 256

static void i2cSensors(void) {
    while (1) { k_sleep(K_SECONDS(5)); }
}

K_THREAD_DEFINE(uartThread, STACKSIZE, gpsSensor, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(i2cThread, STACKSIZE, i2cSensors, NULL, NULL, NULL, 1, 0, 0);

void main(void)
{
    printk("Minimal GPS test build\n");
    while (1) { k_sleep(K_SECONDS(10)); }
}
