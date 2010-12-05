#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define VERBOSE 1

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

static char *itoa(char *s, uint16_t n);
static void reverse(char *s, uint8_t n);

static void reverse(char *s, uint8_t n) {
    uint8_t c, i;
    for (i = 0; i < n; i++, n--) {
        c = s[i];
        s[i] = s[n];
        s[n] = c;
    }
}

static char *itoa(char *s, uint16_t n) {
    uint8_t i;
    i = 0;

    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0); 

    s[i] = '\0';
    reverse(s, i-1);
    return s+i;
}

int main(const int argc, char **argv) {
    char *c;
    char buf[10];
    uint8_t d8;
    uint16_t d16;

    d8 = 255;
    c = itoa(buf, d8);
    test("uint8 to char ");
    test_cond(strcmp(buf, "255") == 0);

    d16 = 111;
    c = itoa(c, d16);
    test("uint16 to char ");
    test_cond(strcmp(buf, "111") == 0);
    *c++ = 'l';
    *c = '\0';
    printf("%s \n", buf);


    d16 = 0;
    c = itoa(buf, d16);
    test("uint16 to char ");
    test_cond(strcmp(buf, "0") == 0);

    d16 = 65535;
    c = itoa(buf, d16);
    test("uint16 to char ");
    test_cond(strcmp(buf, "65535") == 0);

    if (fails == 0)
        printf("\n%s: %d PASSED\n", *argv, tests);
    else 
        printf("\n%s: %d PASSED, %d FAILED\n", *argv, tests - fails, fails);
}
