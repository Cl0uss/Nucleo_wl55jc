#include "../src/connector.h"

void distanceMeasure() {
    
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
        uint32_t distanceCm = usec / 58;

        distanceCm = (distanceCm > 450.0f) ? 0 : distanceCm;
        distanceCm = (distanceCm > 99.0f) ? 99.0f : distanceCm;

        distanceVal = distanceCm;
}
