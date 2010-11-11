#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "arduino/pins.h"
#include "arduino/sleep.h"
#include "arduino/wdt.h"

#define FOSC  800000

#define DO 0
#define DI 1
#define LED_1 2
#define LED_2 1

uint8_t c = 0;

wdt_interrupt() {
    wdt_reset();
    wdt_interrupt_enable();
    c++;
    if (c == 7) {
         c = 0;
         pin_toggle(LED_2);
         _delay_ms(100);
         pin_toggle(LED_2);
         _delay_ms(100);
         pin_toggle(LED_2);
         _delay_ms(100);
         pin_toggle(LED_2);
    }

}

/* The sleep_cpu(); will make the uC sleep, it continues to execute after
 * the function call on wake-up. */
static void uc_sleep() {
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();  
    sleep_disable();
}

__attribute__((noreturn)) int main (void) {
    wdt_reset();
    wdt_interrupt_enable();
    wdt_change_clock(1024);

    pin_mode_output(LED_1);
    pin_mode_output(LED_2);

    pin_high(LED_1);
    _delay_ms(500);
    pin_low(LED_1);
    pin_low(LED_2);

    sleep_mode_power_down();
    
    sei();

    while(1) {
        uc_sleep();
        continue;
    }
}

