#include "arduino/serial.h"
#include "serial.h"
#include "sleep.h"

static struct {
    uint8_t head;
    uint8_t tail;
    char buf[RBUF];
} output;

static uint8_t out_readable() {
    return (output.head - output.tail) & (RBUF - 1);
}

static uint8_t out_writeable() {
    return ((output.tail - output.head - 1) & (RBUF - 1));
}

static uint8_t out_read(char *d) {
    if (out_readable()) {
        *d = output.buf[output.tail++];
        output.tail &= (RBUF - 1);
        return 1;
    }
    return 0;
}

static uint8_t out_write(char d) {
    if (out_writeable()) {
        output.buf[output.head++] = d;
        output.head &= (RBUF - 1);
        return 1;
    }
    return 0;
}

void wait_on_buf() {
    while (out_writeable() == 0) {
        sleep(idle);
    }
}

void print(char d) {
    wait_on_buf();

    out_write(d);
    serial_interrupt_dre_enable();
}

void printstr(char *d) {
    char c;
    while ((c = *d++))
        print(c);
}

void print_hex4(uint8_t v) {
    v &= 0xF;
    if (v < 10)
        print('0' + v);
    else
        print('A' - 10 + v);
}

void print_hex8(uint8_t v) {
    print_hex4(v >> 4);
    print_hex4(v & 0x0F);
}

void print_hex16(uint16_t v) {
    print_hex8(v >> 8);
    print_hex8(v & 0xFF);
}

inline void serial_buffer_reset() {
    output.head = 0;
    output.tail = 0;
}

serial_interrupt_dre() {
    char c;
    if (out_read(&c))
        serial_write(c);
    else 
        serial_interrupt_dre_disable();
}
