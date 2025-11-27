#include "connector.h"

void rgbChange (int color) {
    gpio_pin_set_dt(&led_r, 0);
    gpio_pin_set_dt(&led_g, 0);
    gpio_pin_set_dt(&led_b, 0);

    switch (color) {
    case 0: {gpio_pin_set_dt(&led_r, 0);
             gpio_pin_set_dt(&led_r, 0);
             gpio_pin_set_dt(&led_r, 0); 
             break;}
    case 1: gpio_pin_set_dt(&led_r, 1); break;
    case 2: gpio_pin_set_dt(&led_g, 1); break;
    case 3: gpio_pin_set_dt(&led_b, 1); break;
    case 4: gpio_pin_set_dt(&led_r, 1); gpio_pin_set_dt(&led_b, 1); break;
    case 5: gpio_pin_set_dt(&led_g, 1); gpio_pin_set_dt(&led_b, 1); break;
    case 6: gpio_pin_set_dt(&led_r, 1); gpio_pin_set_dt(&led_g, 1); break;
    }
}