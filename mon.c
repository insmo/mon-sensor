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
#define SENSORS 4

/* analog */
#define PHOTO 0

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

enum events {
    EV_NONE = 0,
    EV_UPDATE = 1 << 0,
    EV_ADC = 1 << 1
};

volatile uint8_t events = EV_NONE;


static inline uint16_t btov(uint16_t bit) {
    return V_BIT * bit;
}

static uint16_t vtolux(uint16_t vout) {
    uint16_t v, lux_rf;
    float lux_rel;

    return 5 / (10 * ((3300.0 - vout) / vout));
    // return 5.0 / (10.0 * ((3300 - 3290) / 3290 ));
    // /* volt x 1000 %1.3f */
    // v = 3300 - vout;
    // printf("v: %d\n", v);
    // 
    // /* lux_rf r2 resistor / 100 */
    // lux_rf = 10 * v;
    // printf("rf: %d\n", lux_rf);

    // lux_rel = 5 / lux_rf;
    // printf("rel: %d\n", lux_rel);

    // /* ex 500/(1000 * ((3.3-3.299)/3.299)) == 1649 lux*/
    // //return LUX_REL / (LUX_RF * (V_REF - vout) / vout);
    // return lux_rel;
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

static void col_data(uint8_t pin, uint16_t *data) {
    adc_pin_select(pin);
    adc_enable();
    sleep(noise_reduction);            
    *data = adc_data();
    adc_disable();
}

uint8_t wdt_ctr;

wdt_interrupt() {
    wdt_reset();
    if (++wdt_ctr == 1) { // 210 is ~30m
        wdt_ctr = 0;
        events |= EV_ADC;
    }
}

adc_interrupt() {
}

uint16_t ms;
timer2_interrupt_a() {
    ms++;
}

__attribute__((noreturn)) int main () {
    uint16_t data = 0;
    char *p;
    char out[32];

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
    pin_mode_output(SENSORS);
    pin_high(SENSORS);
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

    xbee_hibernate_enable();

    wdt_reset();

    while(1) {
        pin_low(LED_1);

        if (events == EV_NONE){
            wdt_interrupt_enable();
            sleep(power_down);
            continue;
        }
        
        if (events & EV_ADC) {
            events &= ~EV_ADC;

            /* photo cell */
            pin_high(SENSORS);
            col_data(PHOTO, &data);
            pin_low(SENSORS);
            p = itoa(out, vtolux(data * V_BIT));
            p = buf_append(p, "lux");
            *p = '\0';

            events |= EV_UPDATE;
        }

        if (events & EV_UPDATE) {
            events &= ~EV_UPDATE;
            pin_high(LED_1);

            serial_transmitter_enable();
            xbee_hibernate_disable();

            print_str(out);
            wait_on_serial();

            xbee_hibernate_enable();
            serial_transmitter_disable();
        }
    }
}
