#include "connector.h"              // только ради прототипа gpsSensor()
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define RX_BUF_SIZE 128
static char rx_buf[RX_BUF_SIZE];
static int rx_pos = 0;


void process(void) {
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(usart1));

    k_sleep(K_MSEC(300));  // дать ядру поднять драйвер

    if (!device_is_ready(dev)) {
        printk("USART1 not ready\n");
        return;
    }

    struct uart_config cfg = {
    .baudrate = 9600,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
};

    uart_configure(dev, &cfg);

    printk("USART1 ready, reading...\n");

    while (1) {
        unsigned char c;
        if (uart_poll_in(dev, &c) == 0) {
            if (c == '\n' || rx_pos >= RX_BUF_SIZE - 1) {
                rx_buf[rx_pos] = '\0';
                printk("%s\n", rx_buf);
                rx_pos = 0;
            } 
            else {
                rx_buf[rx_pos++] = c;
            }
        }
        k_sleep(K_MSEC(5));
    }
}
void gpsSensor(void)
{
    process();
}
