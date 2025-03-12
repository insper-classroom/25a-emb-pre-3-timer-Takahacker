#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/timer.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

volatile int flag_r = 0;
volatile int flag_g = 0;

volatile bool led_r_on = false;
volatile bool led_g_on = false;

bool timer_callback_r(struct repeating_timer *t) {
    led_r_on = !led_r_on;
    gpio_put(LED_PIN_R, led_r_on);
    return true;
}

bool timer_callback_g(struct repeating_timer *t) {
    led_g_on = !led_g_on;
    gpio_put(LED_PIN_G, led_g_on);
    return true;
}

void btn_callback(uint gpio, uint32_t events) {
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = to_ms_since_boot(get_absolute_time());

    if (interrupt_time - last_interrupt_time > 200) { 
        if (events == 0x4) {
            if (gpio == BTN_PIN_R)
                flag_r = 1;
            else if (gpio == BTN_PIN_G)
                flag_g = 1;
        }
        last_interrupt_time = interrupt_time;
    }
}

int main() {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, false);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_put(LED_PIN_G, false);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    struct repeating_timer timer_r;
    struct repeating_timer timer_g;

    while (true) {
        if (flag_r) {
            flag_r = 0;
            if (led_r_on) {
                cancel_repeating_timer(&timer_r);
                gpio_put(LED_PIN_R, false);
                led_r_on = false;
            } else {
                add_repeating_timer_ms(500, timer_callback_r, NULL, &timer_r);
            }
        }

        if (flag_g) {
            flag_g = 0;
            if (led_g_on) {
                cancel_repeating_timer(&timer_g);
                gpio_put(LED_PIN_G, false);
                led_g_on = false;
            } else {
                add_repeating_timer_ms(250, timer_callback_g, NULL, &timer_g);
            }
        }

        sleep_ms(10);
    }
}