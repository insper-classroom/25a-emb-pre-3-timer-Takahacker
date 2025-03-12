#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int LED_PIN_R = 4;

volatile int flag_f_r = 0;
volatile bool flag_alarm = false;
volatile bool timer_fired = false;


int64_t alarm_callback(alarm_id_t id, void *user_data) {
    if (!flag_alarm) { 
        timer_fired = true;
    }
    return 0;
}

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R) {
        if (events & GPIO_IRQ_EDGE_FALL) { 
            flag_f_r = 1;
            flag_alarm = false;
            add_alarm_in_ms(500, alarm_callback, NULL, false);
        }
        if (events & GPIO_IRQ_EDGE_RISE) { 
            flag_f_r = 0;
            flag_alarm = true;
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, false); 

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, btn_callback);

    bool led_state = false;

    while (true) {
        if (timer_fired) {
            led_state = !led_state;
            gpio_put(LED_PIN_R, led_state);
            timer_fired = false;
        }
        sleep_ms(10);  
    }
}