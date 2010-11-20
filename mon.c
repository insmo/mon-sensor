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
#define PHOTO 5

/* voltage */
#define V_BIT 0.00322265625 /* 3.3V / 1024-bit */
#define V_REF 3.3

/* lux */
#define LUX_REL 500         /* RL = 500 / lux Kohm */
#define LUX_RF 1000         /* lux V R2 resistor */

enum events {
    EV_NONE = 0,
    EV_UPDATE = 1 << 0,
    EV_ADC = 1 << 1
};

volatile uint8_t events = EV_NONE;

static inline uint8_t vtolux(uint8_t vout) {
    return LUX_REL / (LUX_RF * (V_REF - vout) / vout);
}

static inline uint8_t btov(uint8_t bit) {
    return V_BIT * bit;
}

static inline void xbee_hibernate_enable() { 
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

// static void read_data(char *s, uint8_t data) {
// }

uint16_t ms;
timer2_interrupt_a() {
    ms++;
}

__attribute__((noreturn)) int main () {
    uint16_t data = 0;

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

            pin_high(SENSORS);
            adc_pin_select(PHOTO);
            adc_enable();

            sleep(noise_reduction);            

            data = adc_data();
            //read_data(buf, data);

            adc_disable();
            pin_low(SENSORS);

            events |= EV_UPDATE;
            continue;
        }

        if (events & EV_UPDATE) {
            events &= ~EV_UPDATE;
            pin_high(LED_1);

            serial_transmitter_enable();
            xbee_hibernate_disable();

            printstr("lux");
            print_hex16(data);
            printstr(";\0");

            _delay_ms(1000);
            serial_transmitter_disable();
            xbee_hibernate_enable();
            continue;
        }
    }
}
