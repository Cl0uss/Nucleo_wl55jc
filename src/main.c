#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* === RGB LED aliases (определены в overlay) === */
#define LEDR DT_ALIAS(led0r)
#define LEDG DT_ALIAS(led0g)
#define LEDB DT_ALIAS(led0b)
#define BUTTON DT_ALIAS(sw1)

/* === Адрес и регистры сенсора TCS34725 === */
#define TCS34725_ADDR          0x29
#define TCS34725_COMMAND_BIT   0x80
#define TCS34725_ENABLE        0x00
#define TCS34725_ENABLE_PON    0x01
#define TCS34725_ENABLE_AEN    0x02
#define TCS34725_ATIME         0x01
#define TCS34725_CONTROL       0x0F
#define TCS34725_CDATAL        0x14

/* === Объявление устройств === */
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(LEDR, gpios);
static const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(LEDG, gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(LEDB, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON, gpios);
static const struct device *i2c_dev;

/* === Глобальные переменные === */
enum Mode { NORMAL, BLUE, OFF };
static enum Mode mode = NORMAL;
static struct gpio_callback button_cb_data;
static uint16_t clear_ref = 0;

/* === Управление RGB === */
static void set_rgb(bool r, bool g, bool b)
{
    gpio_pin_set_dt(&led_r, r);
    gpio_pin_set_dt(&led_g, g);
    gpio_pin_set_dt(&led_b, b);
}

/* === Запись в регистр TCS34725 === */
static void tcs34725_write(const struct device *i2c_dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { TCS34725_COMMAND_BIT | reg, val };
    i2c_write(i2c_dev, buf, sizeof(buf), TCS34725_ADDR);
}

/* === Усреднённое чтение CLEAR === */
static uint16_t tcs_read_clear_avg(int samples, int delay_ms)
{
    uint32_t sum = 0;
    uint8_t reg = TCS34725_COMMAND_BIT | TCS34725_CDATAL;
    uint8_t data[2];

    for (int i = 0; i < samples; i++) {
        if (i2c_write_read(i2c_dev, TCS34725_ADDR, &reg, 1, data, 2) != 0) {
            printk("I2C read error\n");
            return 0;
        }
        sum += ((uint16_t)data[1] << 8) | data[0];
        if (i < samples - 1)
            k_msleep(delay_ms);
    }
    return (uint16_t)(sum / samples);
}

/* === Инициализация сенсора === */
static void tcs34725_init(void)
{
    /* Включаем питание и разрешаем измерения */
    tcs34725_write(i2c_dev, TCS34725_ENABLE, TCS34725_ENABLE_PON);
    k_msleep(3);
    tcs34725_write(i2c_dev, TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

    /* Настраиваем время интеграции и усиление */
    tcs34725_write(i2c_dev, TCS34725_ATIME, 0xD5);  // ~101 ms
    tcs34725_write(i2c_dev, TCS34725_CONTROL, 0x00); // 1x gain

    k_msleep(150); // ждём первую интеграцию
    printk("TCS34725 initialized\n");
}

/* === Обработка кнопки === */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    bool pressed = gpio_pin_get_dt(&button);
    int64_t start = k_uptime_get();
    while (pressed) {
        pressed = gpio_pin_get_dt(&button);
        if (k_uptime_get() - start >=1000) {
            mode = OFF;
            return;
        }
    }
    mode = (mode == BLUE || mode == OFF) ? NORMAL : BLUE;
}

/* === MAIN === */
int main(void)
{
    /* Настройка устройств */
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c_dev)) {
        printk("I2C2 not ready!\n");
        return 0;
    }

    gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_g, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_b, GPIO_OUTPUT_INACTIVE);

    /* Настройка кнопки */
    gpio_pin_configure_dt(&button, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    /* Инициализируем сенсор */
    tcs34725_init();

    printk("System ready. Press SW1 (U13) to switch modes.\n");

    /* === Главный цикл === */
    while (1) {

        if (mode == NORMAL) {
            printk("=== NORMAL MODE ===\n");

            /* 1) Калибруем эталон освещённости */
            clear_ref = tcs_read_clear_avg(8, 10);
            if (clear_ref == 0) clear_ref = 1;

            printk("Reference CLEAR = %u\n", clear_ref);

            uint16_t low = clear_ref * 0.33f;
            uint16_t mid = clear_ref * 0.66f;

            /* 2) Цикл измерений каждые 2 секунды */
            while (mode == NORMAL) {
                uint16_t clear_now = tcs_read_clear_avg(4, 10);

                printk("\rClear_now = %u  ref = %u  low = %u  mid = %u-%u",
                       clear_now, clear_ref, (uint16_t)low, (uint16_t)low, (uint16_t)mid);

                if (clear_now < low)
                    set_rgb(1, 0, 0);   // RED
                else if (clear_now >= low && clear_now < mid)
                    set_rgb(1, 1, 0);   // YELLOW
                else
                    set_rgb(0, 1, 0);   // GREEN

                k_msleep(2000);
            }
            printk("\n");
        }
        else if (mode == BLUE) {
            printk("=== BLUE MODE ===\n");
            set_rgb(0, 0, 1); // BLUE ON
            while (mode == BLUE) {
                k_msleep(100);
            }
        }
        else if (mode == OFF) {
            printk("=== OFF MODE ===\n");
            set_rgb(0, 0, 0);  // выключаем все цвета
            while (mode == OFF) {
                k_msleep(100);  // ждём, пока пользователь снова нажмёт кнопку
            }
        }

    }
}