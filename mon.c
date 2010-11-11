#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>

#include "arduino/serial.h"
#include "arduino/pins.h"
#include "arduino/sleep.h"
#include "arduino/wdt.h"
#include "arduino/adc.h"

#define FOSC  800000

#define LED_1 2
#define LED_2 3
#define XBEE 4
#define PHOTO 5

enum events {
    EV_NONE = 0,
    EV_UPDATE = 1 << 0,
    EV_ADC = 1 << 1
};

static volatile struct {
	char *str;
	uint8_t printing;
} output;

volatile uint8_t events = EV_NONE;

static inline void sleep_uc() {
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();  
    sleep_disable();
}

static inline void wait_on_print() {
again:
    cli();
    sleep_mode_idle();
    if (output.printing) {
        sleep_uc();
        goto again;
	}
    sleep_mode_power_down();
	sei();
}

static inline void xbee_hibernate_enable() { 
    _delay_ms(100);
    pin_high(XBEE); 
}

static inline void xbee_hibernate_disable() { 
    pin_low(XBEE); 
    _delay_ms(100);
}

serial_interrupt_dre() {
	char *str = output.str;
	uint8_t c = *str;

	if (c == '\0') {
		serial_interrupt_dre_disable();
		output.printing = 0;
	} else {
		serial_write(c);
		output.str = str + 1;
	}
}

static inline void prints(char *s) {
    wait_on_print();

	output.str = s;
	output.printing = 1;

	serial_interrupt_dre_enable();
}

uint8_t wdt_ctr;

wdt_interrupt() {
    wdt_reset();
    if (++wdt_ctr == 210) { 
        wdt_ctr = 0;
        events |= EV_ADC;
    }
}

adc_interrupt() {
}

static void read_data(char *s, uint8_t data) {
}

__attribute__((noreturn)) int main () {
    uint8_t data;
    char buf[10];

    /* configure and set watchdog interrupt mode */
    cli();
    wdt_reset();
    wdt_enable(WDTO_8S);
    wdt_interrupt_enable();
    sei();

    /* Setup serial mode and transmission speed. 
     * Serial transmitter should be disabled
     * when not used to save power. */
    serial_baud_4800();
	serial_mode_8n1();
    serial_transmitter_enable();

    /* Set XBee control pin and put XBee to sleep. */
    pin_mode_output(XBEE);
    xbee_hibernate_disable();

    prints("A;\0");
    prints("B;\0");
    prints("A;\0");

    xbee_hibernate_enable();

    /* Set LED_1 to output, use it as an indicator of sleeping. */

    pin_mode_output(LED_1);
    pin_high(LED_1);

     /* Setup interrupt based timer to tick each 0.001 s (1 ms). The internal
     * timers clock frequency is based on the system F_CPU.
     *
     * 1. Clear on Timer Compare.
     * 2. Prescaler to 128(devide internal timer for lower precision).
     * 3. When timer hits 144, 1 ms has passed. 
     * 4. Enable compare match interrupt to wake the uC each 1 ms. */ 

    // timer2_mode_normal();
    // timer2_clock_d1024(); 
    // //timer2_compare_a_set(255); 
    // //timer2_interrupt_a_enable();
    // timer2_interrupt_ovf_enable();
    
    adc_reference_internal_1v1();
	adc_clock_d128();
    adc_interrupt_enable();
    
    /* Setup the global sleep mode to power-down which draws about 1u.
     * The watchdog interrupt is able to wake up the uC from the power-down. */

    sleep_mode_power_down();

    /* Enable global interrupts. */

    sei(); 

    while(1) {
        wdt_interrupt_enable();
        pin_low(LED_1);

        cli();
        if (events == EV_NONE){
            sleep_uc();
            continue;
        }
        sei();
        
        if (events & EV_ADC) {
            events &= ~EV_ADC;

	        adc_pin_select(PHOTO);
	        adc_enable();

            sleep_mode_noise_reduction();
            cli();
            sleep_uc();            
            sleep_mode_power_down();

            itoa(adc_data(), buf, 10);
            events |= EV_UPDATE;

            read_data(buf, data);
            adc_disable();

            continue;
        }

        if (events & EV_UPDATE) {
            events &= ~EV_UPDATE;
            pin_high(LED_1);

            serial_transmitter_enable();
            xbee_hibernate_disable();

            prints("lux");
            prints(buf);
            prints(";\0");

            serial_transmitter_disable();
            xbee_hibernate_enable();
            continue;
        }
    }
}

// typedef struct rbuf_t {
//     char buf[RBUF];
//     char *in;
//     char *out;
//     uint8_t count;
//     uint8_t printing;
// } rbuf_t;
// 
// rbuf_t out;
// 
// static inline void buf_init(rbuf_t *buf) {
//     buf->in = buf->buf;
//     buf->out = buf->buf;
// }
// 
// static inline void rbuf_insert(rbuf_t *buf, uint8_t data) {
//     *buf->in = data;
// 
//     if (++buf->in == &buf->buf[RBUF])
//         buf->in = buf->buf;
// 
//     // todo: make atomic
//     buf->count++;
// }
// 
// static inline char rbuf_remove(rbuf_t *buf) {
//     char data = *buf->out;
// 
//     if (++buf->out == &buf->buf[RBUF])
//         buf->out = buf->buf;
// 
//     // todo: make atomic
//     buf->count--;
// 
//     return data;
// }


// serial_interrupt_dre() {
//     if (out.count == 0) {
//         out.printing = 0;
//         serial_interrupt_dre_disable();
//     } else {
//         serial_write(rbuf_remove(&out));
//     }
// }
// 
// static inline void prints(char *s) {
//     uint8_t i = 0;
// 
//     while (s[i] != '\0') {
//         rbuf_insert(&out, s[i]);    
//         i++;
//     }
// 
//     if (out.printing == 0) {
//         out.printing = 1;
//         serial_interrupt_dre_enable();
//     }
// }

// uint16_t ms;
// 
// /* Timer2 compare on match interrupt */
// timer2_interrupt_a() {
//     ms++;
// }
// 
// static volatile struct Time {
//     uint8_t m;
//     uint8_t s;
//     uint8_t h;
// } timer2;
// 
// uint8_t ctr;
// timer2_interrupt_ovf() {
//     ctr++;
//     if (ctr >= 244) {
//         ctr = 0;
//         timer2.s += 8;
//         if (timer2.s >= 60) {
//             timer2.m++;
//             timer2.s -= 60;
//         }
//         if (timer2.m >= 60) {
//             timer2.h++;
//             timer2.m -= 60;
//         }
//         if (timer2.h == 24) {
//             timer2.h = 0;
//         }
//     }
// }
