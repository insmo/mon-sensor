
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "arduino/serial.h"
#include "arduino/pins.h"

#define FOSC  8000000

#define LED_1 2
#define XBEE 4


static inline void xbee_hibrinate_enable() {
    pin_high(XBEE);
}

static inline void xbee_hibrinate_disable() {
    pin_low(XBEE);
    _delay_ms(100);
}

static int uart_putchar(char c, FILE *stream) {
    /* Wait for empty transmit buffer */
    while (!serial_writeable())
            ;

    /* Write to the USART I/O Data Register(UDR). */
    serial_write(c);
    return 0;
}

static uint8_t uart_getchar(void) {
    /* Wait for data to be recieved */
    while(!serial_readable())
        ;
    return serial_read();
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

__attribute__((noreturn)) int main () {
    double c = 24.0;

    /* setup uart */
    serial_baud_4800();
    serial_mode_8n1(); 
    serial_transmitter_enable();
    stdout = &mystdout;

    pin_mode_output(XBEE);
    pin_mode_output(LED_1);

    while(1) {
        
        xbee_hibrinate_disable();
        pin_high(LED_1);

        printf("i%2.1f,", 4.0);
        printf("v%2.1f,", 12.0);
        printf("c%2.1f,", 24.0);
        printf("lux%2.1f", 3.0);
        printf(";");

        while(!serial_writeable())
            ;

        // xbee_hibrinate_enable();
        pin_low(LED_1);
        _delay_ms(500);
    }
}

