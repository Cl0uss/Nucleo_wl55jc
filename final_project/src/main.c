#include "connector.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdlib.h>

#define UART_NODE DT_NODELABEL(usart1)
static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

static char nmea_buf[128];
static int buf_pos = 0;

static void parse_gpgga(char *sentence) {
    // Пример: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    char *token;
    char *fields[15];
    int i = 0;

    token = strtok(sentence, ",");
    while (token && i < 15) {
        fields[i++] = token;
        token = strtok(NULL, ",");
    }

    if (i < 6) return; // не хватает данных

    char *utc_time = fields[1];
    char *lat_str = fields[2];
    char *lat_dir = fields[3];
    char *lon_str = fields[4];
    char *lon_dir = fields[5];
    char *fix_status = fields[6];

    if (fix_status[0] == '0') {
        printk("🛰  Нет фиксации спутников\n");
        return;
    }

    // Преобразуем координаты
    double lat_deg = atof(lat_str) / 100.0;
    double lon_deg = atof(lon_str) / 100.0;

    int lat_deg_int = (int)lat_deg;
    int lon_deg_int = (int)lon_deg;

    double lat_min = (lat_deg - lat_deg_int) * 100.0 / 60.0;
    double lon_min = (lon_deg - lon_deg_int) * 100.0 / 60.0;

    double lat = lat_deg_int + lat_min;
    double lon = lon_deg_int + lon_min;

    if (lat_dir[0] == 'S') lat = -lat;
    if (lon_dir[0] == 'W') lon = -lon;

    printk("📡 Координаты:\n");
    printk("   Широта: %.5f° %s\n", fabs(lat), lat_dir);
    printk("   Долгота: %.5f° %s\n", fabs(lon), lon_dir);
    printk("   Время (UTC): %.6s\n", utc_time);
}

static void parse_gprmc(char *sentence) {
    // Пример: $GPRMC,235947.00,A,3723.2475,N,12158.3416,W,0.13,309.62,120598,,,A*77
    char *token;
    char *fields[15];
    int i = 0;

    token = strtok(sentence, ",");
    while (token && i < 15) {
        fields[i++] = token;
        token = strtok(NULL, ",");
    }

    if (i < 10) return;

    char *status = fields[2];
    if (status[0] != 'A') {
        printk("🛰  Нет валидных данных (RMC)\n");
        return;
    }

    char *utc_time = fields[1];
    char *lat_str = fields[3];
    char *lat_dir = fields[4];
    char *lon_str = fields[5];
    char *lon_dir = fields[6];
    char *speed_knots = fields[7];
    char *date_str = fields[9];

    double speed_kmh = atof(speed_knots) * 1.852;

    printk("🕓 Время: %.6s UTC  |  Дата: %.6s\n", utc_time, date_str);
    printk("🚗 Скорость: %.2f км/ч\n", speed_kmh);
    printk("📍 Позиция: %s°%s, %s°%s\n", lat_str, lat_dir, lon_str, lon_dir);
}

void main(void)
{
    if (!device_is_ready(uart_dev)) {
        printk("UART1 not ready!\n");
        return;
    }

    printk("✅ GPS UART ready! Parsing NMEA...\n");

    uint8_t c;

    while (1) {
        if (uart_poll_in(uart_dev, &c) == 0) {
            if (c == '\n' || buf_pos >= sizeof(nmea_buf) - 1) {
                nmea_buf[buf_pos] = '\0';
                buf_pos = 0;

                if (strncmp(nmea_buf, "$GPGGA", 6) == 0) {
                    parse_gpgga(nmea_buf);
                } else if (strncmp(nmea_buf, "$GPRMC", 6) == 0) {
                    parse_gprmc(nmea_buf);
                }

            } else if (c != '\r') {
                nmea_buf[buf_pos++] = c;
            }
        } else {
            k_msleep(5);
        }
    }
}
