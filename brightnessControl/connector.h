#define CONNECTOR_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

struct rgb {
    struct gpio_dt_spec r;
    struct gpio_dt_spec g;
    struct gpio_dt_spec b;
};


static void rgb_set(const struct rgb *bus, bool r, bool g, bool b)
{
    gpio_pin_set_dt(&bus->r, r);
    gpio_pin_set_dt(&bus->g, g);
    gpio_pin_set_dt(&bus->b, b);
}

extern bool saved;
extern struct rgb led;
extern const struct device *i2c;


void sensor(void);

