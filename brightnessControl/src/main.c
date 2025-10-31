#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include "../connector.h"
#include <zephyr/kernel/stats.h>
#include <zephyr/arch/arch_interface.h>

// === Thread definitions ===
#define STACKSIZE 512
#define PRIORITY 5

// === Registers in RGB SENSOR ===
#define ADDR          0x29
#define COMMAND_BIT   0x80

#define ENABLE        0x00
#define ENABLE_PON    0x01
#define ENABLE_AEN    0x02

#define ATIME         0x01
#define CONTROL       0x0F
#define CDATAL        0x14

#define WAIT_TIME     0x03
#define WAIT_VALUE    0xAB


#define LEDR_NODE DT_ALIAS(ledr)
#define LEDG_NODE DT_ALIAS(ledg)
#define LEDB_NODE DT_ALIAS(ledb)
#define BUTTON DT_ALIAS(sw1)


// === Avalilable for both files ===
struct rgb led = {
    .r = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios),
    .g = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios),
    .b = GPIO_DT_SPEC_GET(DT_ALIAS(ledb), gpios),
};

const struct device *i2c;
bool saved = false;


static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON, gpios);
static struct gpio_callback button_cb_data;




enum Mode { NORMAL, BLUE, OFF};
enum Mode saved_last;
enum Mode mode = NORMAL;

void buffer_changer(uint8_t *buff, uint8_t first, uint8_t second) {
    buff[0] = first;
    buff[1] = second;
}


// == RGB Sensor initialisation === 
void tcs34725_init() {
    uint8_t buff[2];
    buffer_changer(buff, COMMAND_BIT | ENABLE, ENABLE_PON);
    i2c_write(i2c, buff, sizeof(buff), ADDR);

    k_msleep(3); //needed as written in documentation
    buffer_changer(buff, COMMAND_BIT | ENABLE, ENABLE_AEN | ENABLE_PON);
    i2c_write(i2c ,buff, sizeof(buff), ADDR);

    buffer_changer(buff, COMMAND_BIT | ATIME, 0xD5);
    i2c_write(i2c,buff,sizeof(buff),ADDR);

    buffer_changer(buff, COMMAND_BIT | CONTROL, 0x00);
    i2c_write(i2c,buff,sizeof(buff),ADDR);


    buffer_changer(buff, COMMAND_BIT | WAIT_TIME, WAIT_VALUE);
    i2c_write(i2c,buff,sizeof(buff),ADDR);

    k_msleep(150);
    printk("Sensor initialized\n");
}


void leds_init(const struct rgb *leds)
{
    gpio_pin_configure_dt(&leds->r, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&leds->g, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&leds->b, GPIO_OUTPUT_INACTIVE);
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    bool pressed = gpio_pin_get_dt(&button);
    int64_t start = k_uptime_get();

    while (pressed && mode != OFF) {
        pressed = gpio_pin_get_dt(&button);
        if (k_uptime_get() - start >= 1000) {
            mode = OFF;
            return;
        }
    }

    if (mode == OFF) mode = saved_last;
    else if (mode == NORMAL) mode = BLUE;
    else mode = NORMAL;
}


void normal_function(void) {sensor();}
K_THREAD_DEFINE (sensor_thread,STACKSIZE,normal_function,NULL,NULL,NULL,0,0,0);

void main (void) {
    k_thread_suspend(sensor_thread);

    // === Button initialisation ===
    gpio_pin_configure_dt(&button, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    leds_init(&led);

    // === i2c initialisation ===
    i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c)) {
        printk("i2c2 is not ready\n");
        return;
    }

    tcs34725_init();

    while (true) {
        if ( mode == NORMAL) {
            saved_last = NORMAL;
            printk("=== NORMAL MODE ===\n");
            k_thread_resume(sensor_thread);
            printk("thread activated\n");
            while (mode == NORMAL) {
                k_msleep(2000);
            }
            saved=false;
            k_thread_suspend(sensor_thread);
            printk("\nthread suspended\n");
        }

        else if ( mode == BLUE ) {
            saved_last = BLUE;
            printk("=== BLUE MODE ===\n");
            rgb_set(&led, 0, 0, 1); 
            while (mode == BLUE) k_msleep(100);
        }

        else if ( mode == OFF ) {
            printk("=== OFF MODE ===\n");
            rgb_set(&led, 0, 0, 0);
            while (mode == OFF) k_msleep(100);
        }
    }
}
