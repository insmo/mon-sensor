#include <stdio.h>
#include <stdint.h>

#define _BV(bit) (1 << (bit))
#define BUF 64
#define VERBOSE 0

static int tests = 0, fails = 0;
#define test(_s) { printf("#%02d ", ++tests); printf(_s); }
#define test_cond(_c) if(_c) printf("PASSED\n"); else {printf("FAILED\n"); fails++;}

void bin(short d) {
#if VERBOSE 
    int i;
    for(i = 7; i >= 0; i--) {
        if((1 << i) & d)
            printf("1");
        else printf("0");
    }
    printf("\n");
#endif
}

static struct {
	uint8_t head;
	uint8_t tail;
	uint8_t buf[BUF];
} output;

static uint8_t readable() {
    return (output.head - output.tail) & (BUF - 1);
}

static uint8_t writeable() {
    return ((output.tail - output.head - 1) & (BUF - 1));
}

static uint8_t read(uint8_t *d) {
    if (readable()) {
        *d = output.buf[output.tail++];
        output.tail &= (BUF - 1);
        return 1;
    }
    return 0;
}

static uint8_t write(uint8_t d) {
    if (writeable()) {
        output.buf[output.head++] = d;
        output.head &= (BUF - 1);
        return 1;
    }
    return 0;
}

void main() {
    int i;
    uint8_t c;

    output.head = 63;
    output.tail = 0;

    test("tail 0, head 63, readable() ");
    test_cond(readable());
    bin(readable());

    test("tail 0, head 63, writeable()");
    test_cond(writeable() == 0);
    bin(writeable());

    output.head = 62;
    output.tail = 0;

    test("tail 0, head 62, readable() ");
    test_cond(readable());
    bin(readable());

    test("tail 0, head 62, writeable() ");
    test_cond(writeable());
    bin(writeable());

    output.head = 1;
    output.tail = 1;

    test("tail 1, head 1, readable() ");
    test_cond(readable() == 0);
    bin(readable());

    test("tail 1, head 1, writeable() ");
    test_cond(writeable());
    bin(writeable());

    test("buf empty, read ");
    for (i = 0; read(&c); i++);
    test_cond(i == 0 && output.tail == 1 && output.head == 1);

    test("buf empty, write 63 chars ");
    for (i = 0; write(i++););
    test_cond(i == 64 && output.tail == 1 && output.head == 0);

    test("buf full, write ");
    for (i = 0; write(i); i++);
    test_cond(i == 0 && output.tail == 1 && output.head == 0);

    test("buf full, read 63 chars ");
    for (i = 0; read(&c); i++);
    test_cond(i == 63 && c == 62 && output.tail == 0 && output.head == 0);

    if (fails == 0)
        printf("\n%d PASSED\n", tests);
    else 
        printf("\n%d PASSED, %d FAILED\n", tests, fails);
}

