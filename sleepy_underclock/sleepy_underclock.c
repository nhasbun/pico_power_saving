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
#include <hardware/vreg.h>
#include <pico/stdlib.h>
#include <hardware/pll.h>
#include "pico/sleep.h"


void measure_freqs(void) {
    uint32_t f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint32_t f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint32_t f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint32_t f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint32_t f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint32_t f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint32_t f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint32_t f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
}


void underclock() {
    pll_init(pll_usb, 1, 750 * MHZ, 7, 7);
    set_sys_clock_pll(750 * MHZ, 7, 7);
    vreg_set_voltage(VREG_VOLTAGE_0_90);

    clock_configure(clk_peri,
                    0, // Only AUX mux on ADC
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    15306 * KHZ,
                    15306 * KHZ);
}


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

    underclock();

    stdio_init_all();
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

    underclock();

    stdio_init_all();
    printf("Clocks recovered!\n");
    printf("Awaken!\n");

    uint8_t led_value = 1;

    clock_stop(clk_usb);
    clock_stop(clk_adc);


    for (;;) {
        gpio_put(LED_PIN, led_value);
        led_value = !led_value;
        measure_freqs();
        printf("=================================\n");
        sleep_ms(1000);
    }
    return 0;
}