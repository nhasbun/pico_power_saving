#include <stdio.h>
#include <hardware/uart.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <hardware/regs/rosc.h>
#include <hardware/structs/rosc.h>
#include <hardware/rosc.h>
#include <hardware/structs/scb.h>
#include "pico/sleep.h"

static void sleep_callback(void) {
    tight_loop_contents();
}

static void rtc_sleep(void) {
    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };

    // Alarm 10 seconds later
    datetime_t t_alarm = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 10
    };

    sleep_run_from_xosc();
    rtc_init();
    rtc_set_datetime(&t);

    printf("Sleeping for 5 seconds\n");
    uart_default_tx_wait_blocking();

    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}


int main() {
    stdio_init_all();
    sleep_ms(1000);

    printf("Hello Sleep!\n");

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t _scb_orig = scb_hw->scr;
    uint32_t _en0_orig = clocks_hw->sleep_en0;
    uint32_t _en1_orig = clocks_hw->sleep_en1;

    rtc_sleep();

    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);
    scb_hw->scr             = _scb_orig;
    clocks_hw->sleep_en0    = _en0_orig;
    clocks_hw->sleep_en1    = _en1_orig;
    clocks_init();

    stdio_init_all();
    printf("Clocks recovered!\n");
    printf("Awaken!\n");

    uint8_t led_value = 1;

    for (;;) {
        gpio_put(LED_PIN, led_value);
        led_value = !led_value;
        sleep_ms(500);
    }
    return 0;
}