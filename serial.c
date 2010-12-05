#include "arduino/serial.h"
#include "serial.h"
#include "sleep.h"

static void reverse(char *s, uint8_t n);
static uint8_t out_readable();
static uint8_t out_writeable();
static uint8_t out_read(char *d);
static uint8_t out_write(char d);
static void wait_on_buf();

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

static void reverse(char *s, uint8_t n) {
    uint8_t c, i;
    for (i = 0; i < n; i++, n--) {
        c = s[i];
        s[i] = s[n];
        s[n] = c;
    }
}

char *itoa(char *s, uint16_t n) {
    uint8_t i;
    i = 0;

    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0); 
    
    reverse(s, i-1);
    return s+i;
}

char *buf_append(char *p, char *s) {
    while ((*p++ = *s++))
        ;
    return p;
}

static void wait_on_buf() {
    while (out_writeable() == 0) {
        sleep(idle);
    }
}

void wait_on_serial() {
again:
    if (!out_readable()) {
        sleep(idle);
        goto again;
    }
}

void print(char d) {
    wait_on_buf();

    out_write(d);
    serial_interrupt_dre_enable();
}

void print_str(char *d) {
    char c;
    while ((c = *d++))
        print(c);
    print('\0');
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
