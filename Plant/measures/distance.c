#include "../src/connector.h"

void distanceMeasure() {
    

    while (1) {
        uint32_t start = 0, end = 0;

        gpio_pin_set(port, 5, 1);
        k_busy_wait(10);
        gpio_pin_set(port, 5, 0);

        while (gpio_pin_get(port, 6) == 0) {}
        start = k_cycle_get_32();

        while (gpio_pin_get(port, 6) == 1) {}
        end = k_cycle_get_32();

        uint32_t cycles = end - start;
        uint64_t usec = k_cyc_to_us_near64(cycles);
        uint32_t distance_cm = usec / 58;

        printk("Distance: %u cm (%llu us)\n", distance_cm, usec);

        k_msleep(500);
    }
}
