#include <stdio.h>
#include <stdint.h>

#define _BV(bit) (1 << (bit))
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

static void print_hex4(uint8_t v, char *c) {
    v &= 0xF;
    if (v < 10)
        *c = '0' + v;
    else
        *c = 'A' - 10 + v;
}

static void print_hex8(uint8_t v, char *c) {
    print_hex4(v >> 4, c);
    print_hex4(v & 0x0F, c+1);
}

static void print_hex16(uint16_t v, char *c) {
    print_hex8(v >> 8, c);
    print_hex8(v & 0xFF, c+2);
}

unsigned int hex2int(char *a, unsigned int len) {
    int i;
    unsigned int val = 0;

    for (i = 0; i < len; i++) {
        if(a[i] <= 57)
            val += (a[i] - 48)*(1 << (4 *(len - 1 - i)));
        else
            val += (a[i] - 55)*(1 << (4 *(len - 1 - i)));
    }
    return val;
}

static void r(char *c) {
    int i;
    for (i = 0; i < 4; *c='\0', i++);
}

int main(const int argc, char **argv) {
    int i;
    char c[4];
    uint8_t d8;
    uint16_t d16;

    d8 = 255;
    print_hex8(d8, c);
    test("uint8 to char ");
    test_cond(hex2int(c, 2) == 255);

#if VERBOSE 
    bin(*c); bin(*c+1);
    printf("0x%c%c, %d dec\n", *(c+0), *(c+1), hex2int(c, 2));
#endif

    d8 = 65; r(c);
    print_hex8(d8, c);
    test("uint8 to char ");
    test_cond(hex2int(c, 2) == 65 && *c == '4' && *(c+1) == '1');

#if VERBOSE 
    bin(*c); bin(*c+1);
    printf("0x%c%c, %d dec\n", *(c+0), *(c+1), hex2int(c, 2));
#endif

    d16 = 65535; r(c);
    print_hex16(d16, c);
    test("uint16 to char ");
    test_cond(hex2int(c, 4) == 65535);

#if VERBOSE
    bin(*c); bin(*c+1); bin(*c+2); bin(*&c+3);
    printf("0x%c%c%c%c %d\n", *(c+0), *(c+1), *(c+2), *(c+3), hex2int(c, 4));
#endif

    if (fails == 0)
        printf("\n%s: %d PASSED\n", *argv, tests);
    else 
        printf("\n%s: %d PASSED, %d FAILED\n", *argv, tests - fails, fails);
}
