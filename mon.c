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
#include "arduino/timer2.h"
#include "arduino/wdt.h"
#include "arduino/adc.h"

#define FOSC  800000

/* digital */
#define LED_1 2
#define XBEE 3
#define SENSORS 4

/* analog */
#define PHOTO 5

/* voltage */
#define V_BIT 0.00322265625 /* 3.3V / 1024-bit */
#define V_REF 3.3

/* lux */
#define LUX_REL 500         /* RL = 500 / lux Kohm */
#define LUX_RF 1000         /* lux V R2 resistor */

#define sleep(mode)\
    cli();\
    sleep_mode_##mode();\
    sleep_enable();\
    sleep_bod_disable();\
    sei();\
    sleep_cpu();\
    sleep_disable();

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

static inline uint8_t vtolux(uint8_t vout) {
    return LUX_REL / (LUX_RF * (V_REF - vout) / vout);
}

static inline uint8_t btov(uint8_t bit) {
    return V_BIT * bit;
}

static inline void wait_on_print() {
again:
    if (output.printing) {
        sleep(idle);
        goto again;
	}
}

static inline void xbee_hibernate_enable() { 
    pin_mode_output(XBEE);
    pin_high(XBEE);
    pin_low(LED_1);
    _delay_ms(100);
}

static inline void xbee_hibernate_disable() { 
    pin_mode_input(XBEE);
    pin_high(XBEE);
    pin_high(LED_1);
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
    if (++wdt_ctr == 210) { // 210 is ~30m
        wdt_ctr = 0;
        events |= EV_ADC;
    }
}

adc_interrupt() {
}

static void read_data(char *s, uint8_t data) {
}

uint16_t ms;
timer2_interrupt_a() {
    ms++;
}

__attribute__((noreturn)) int main () {
    uint8_t data;
    char buf[10];

    /* watchdog init */
    cli();
    wdt_reset();
    wdt_enable(WDTO_8S);
    wdt_interrupt_enable();
    sei();

    /* serial init */
    serial_baud_4800();
	serial_mode_8n1();
    serial_transmitter_enable();

    /* status led init */
    pin_mode_output(LED_1);
    pin_high(LED_1);
    _delay_ms(500);

    /* timer2 init to 1ms */ 
    timer2_mode_ctc();
    timer2_clock_d64(); 
    timer2_compare_a_set(125); 
    timer2_interrupt_a_enable();
    //timer2_interrupt_ovf_enable();
    

    /* ADC init */
    adc_reference_internal_1v1();
	adc_clock_d128();
    adc_interrupt_enable();
    
    /* set global interrrupts */
    sei(); 

    /* send welcome message */
    xbee_hibernate_disable();
    prints("initializing \0");
    xbee_hibernate_enable();

    wdt_reset();

    while(1) {
        pin_low(LED_1);

        if (events == EV_NONE){
            sleep(power_down);
            continue;
        }
        
        if (events & EV_ADC) {
            events &= ~EV_ADC;

	        adc_pin_select(PHOTO);
	        adc_enable();

            sleep(noise_reduction);            

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
