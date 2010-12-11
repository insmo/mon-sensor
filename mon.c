#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>

#include "sleep.h"
#include "serial.h"
#include "arduino/serial.h"
#include "arduino/pins.h"
#include "arduino/timer2.h"
#include "arduino/wdt.h"
#include "arduino/adc.h"

#define FOSC  800000

/* digital */
#define LED_1 2
#define XBEE 3
#define PHOTO_SWITCH 4

/* analog */
#define PHOTO 5
#define CURRENT 4
#define VOLT 3

/* voltage */
#define V_BIT 32 /*0.00322265625 3.3V / 1024-bit */
#define V_REF 3300 /* 256x3.3 = 844, 16*3.3 = 52 */
 
/* 16*3.3 = 52
 * 1023x16 = 16368
 * 0.00322mVx10000 = 32
 *
 * 16368x52 = 851,136 (3.3*1023 = 3375.9)
 *
 * to_volt
 * 1023x32  = 32,736
 * 32,736 / 10,000 = 3.27
 *
 * 3.22x16 = 52
 * 1023x16 = 16368
*/

/* lux */
#define LUX_REL 500         /* RL = 500 / lux Kohm */
#define LUX_RF 1000         /* lux V R2 resistor */

static uint16_t vtolux(uint16_t vout);
static inline uint16_t btov(uint16_t bit);
static inline void xbee_hibernate_enable();
static inline void xbee_hibernate_disable();
static inline void run_adc(uint8_t pin, uint16_t *data);
static inline void reverse(char *s, uint8_t n);
static inline char *itoa(char *s, uint16_t n);
static inline char *buf_append(char *p, char *s);
static inline char *read_sensors(char *p);

enum events {
    EV_NONE = 0,
    EV_SEND = 1 << 0,
    EV_DATA = 1 << 1
};

volatile uint8_t events = EV_NONE;

static inline uint16_t btov(uint16_t bit) {
    return V_BIT * bit;
}

static uint16_t vtolux(uint16_t vout) {
    uint16_t v, lux_rf;
    float lux_rel;

    return 5 / (10 * ((3300.0 - vout) / vout));
}

static inline void xbee_hibernate_enable() { 
    _delay_ms(150);
    pin_mode_output(XBEE);
    pin_high(XBEE);
    pin_low(LED_1);
}

static inline void xbee_hibernate_disable() { 
    pin_mode_output(XBEE);
    pin_low(XBEE);
    pin_high(LED_1);
    _delay_ms(32);
}

static inline void run_adc(uint8_t pin, uint16_t *data) {
    uint8_t i;
    uint16_t tmp;

    adc_interrupt_enable();
    adc_pin_select(pin);
    adc_enable();
    
    for (i = 0, tmp = 0; i < 10; i++) {
        adc_start();
        while (!adc_interrupt_flag())
            sleep(noise_reduction);            
        tmp += adc_data();
    }

    *data = tmp/i;
    adc_interrupt_disable();
    adc_disable();
}

static inline char *read_sensors(char *p) {
    uint16_t data;

    pin_high(PHOTO_SWITCH);
    run_adc(PHOTO, &data);
    pin_low(PHOTO_SWITCH);
    p = buf_append(p, "lux");
    //p = itoa(p, vtolux(data * V_BIT));
    p = itoa(p, data);

    run_adc(CURRENT, &data);
    p = buf_append(p, "i");
    p = itoa(p, data);

    run_adc(VOLT, &data);
    p = buf_append(p, "v");
    p = itoa(p, data);

    return p;
}

static inline char *buf_append(char *p, char *s) {
    char c;
    while ((c = *s++)) {
        *p++ = c;
    }
    return p;
}

static inline void reverse(char *s, uint8_t n) {
    uint8_t c, i;
    for (i = 0; i < n; i++, n--) {
        c = s[i];
        s[i] = s[n];
        s[n] = c;
    }
}

static inline char *itoa(char *s, uint16_t n) {
    uint8_t i;

    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0); 
    
    reverse(s, i-1);
    return s+i;
}

volatile uint8_t wdt_ctr;

wdt_interrupt() {
    wdt_reset();
    if (++wdt_ctr == 1) { // 210 is ~30m
        wdt_ctr = 0;
        events |= EV_DATA;
    }
}

adc_interrupt() {
}

static volatile uint16_t ms;
timer2_interrupt_a() {
    ms++;
}

__attribute__((noreturn)) int main () {
    char out[32];
    char *p;
    uint16_t ctr;

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
    serial_buffer_reset();

    /* pin init */
    pin_mode_output(LED_1);
    pin_mode_output(PHOTO_SWITCH);
    pin_high(PHOTO_SWITCH);
    pin_high(LED_1);
    _delay_ms(500);

    /* timer2 init to 1ms */ 
    timer2_mode_ctc();
    timer2_clock_d64(); 
    timer2_compare_a_set(125); 
    timer2_interrupt_a_enable();
    //timer2_interrupt_ovf_enable();

    /* ADC init */
    adc_reference_internal_vcc();
    adc_clock_d128(); 
    adc_interrupt_enable();
    
    /* set global interrrupts */
    sei(); 
    wdt_reset();
    xbee_hibernate_enable();

    while(1) {
        pin_low(LED_1);

        if (events == EV_NONE){
            wdt_interrupt_enable();
            sleep(power_down);
            continue;
        }
        
        if (events & EV_DATA) {
            events &= ~EV_DATA;
            
            timer2_clock_reset();
            timer2_count_set(0);
            ms = 0;

            p = out;
            p = read_sensors(p);
            p = buf_append(p, "ctr");
            p = itoa(p, ctr);
            *p = '\0';
            
            events |= EV_SEND;
        }

        if (events & EV_SEND) {
            events &= ~EV_SEND;
            pin_high(LED_1);

            serial_transmitter_enable();
            xbee_hibernate_disable();

            print_str(out);
            wait_on_serial();

            xbee_hibernate_enable();
            serial_transmitter_disable();
            ctr = ms; 
        }
    }
}
