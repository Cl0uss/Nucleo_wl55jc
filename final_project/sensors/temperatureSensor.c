#include "connector.h"
#include "temperatureSensor.h"

static inline float tempCalcRh(uint16_t srh) {
    return ((125.0f * (float)srh) / 65536.0f) - 6.0f;
}

static inline float tempCalcT(uint16_t st) {
    return ((175.72f * (float)st) / 65536.0f) - 46.85f;
}

static int tempReset(const struct device *i2c) {
    uint8_t cmd = tempCmdReset;
    int ret = i2c_write(i2c, &cmd, 1, tempAddr);
    if (ret == 0) {
        k_msleep(20);
    }
    return ret;
}

static int tempRead(float *temperature, float *humidity, const struct device *i2c) {
    int ret;
    uint8_t cmd;
    uint8_t buf[3];

    cmd = tempCmdMeasureRh;
    ret = i2c_write(i2c, &cmd, 1, tempAddr);
    if (ret) return ret;

    k_msleep(25);
    ret = i2c_read(i2c, buf, 3, tempAddr);
    if (ret) return ret;

    uint16_t srh = ((uint16_t)buf[0] << 8) | buf[1];
    *humidity = tempCalcRh(srh);

    cmd = tempCmdReadTempRh;
    ret = i2c_write_read(i2c, tempAddr, &cmd, 1, buf, 2);
    if (ret) return ret;

    uint16_t st = ((uint16_t)buf[0] << 8) | buf[1];
    *temperature = tempCalcT(st);

    return 0;
}

// --- Основная функция ---
void temperatureSensor() {
    if (tempReset(i2c) != 0) {
        printk("Si7021 reset failed!\n");
        return;
    }

    float t = 0.0f;
    float h = 0.0f;

    if (tempRead(&t, &h, i2c) == 0) {
        printf("\r\033[KTEMP/HUM: Temperature: %.1f °C,  Relative Humidity: %.1f%%", t, h);
        fflush(stdout);
    } else {
        printf("\r\033[KSi7021 I2C read error!");
        fflush(stdout);
    }
    k_msleep(200);
}