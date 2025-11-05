#include "connector.h"              // только ради прототипа gpsSensor()
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define RX_BUF_SIZE 128
static char rx_buf[RX_BUF_SIZE];
static int rx_pos = 0;

void gpsSensor(void)
{
    /* берём именно USART1 (PB6/PB7, 9600) локальной переменной */
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(usart1));

    k_sleep(K_MSEC(300));  // дать ядру поднять драйвер

    if (!device_is_ready(dev)) {
        printk("USART1 not ready\n");
        return;
    }

    printk("USART1 ready, reading...\n");

    while (1) {
        unsigned char c;
        if (uart_poll_in(dev, &c) == 0) {
            if (c == '\n' || rx_pos >= RX_BUF_SIZE - 1) {
                rx_buf[rx_pos] = '\0';
                printk("%s\n", rx_buf);
                rx_pos = 0;
            } else {
                rx_buf[rx_pos++] = c;
            }
        }
        k_sleep(K_MSEC(5));
    }
}
