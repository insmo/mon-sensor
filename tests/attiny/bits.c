#include <stdio.h>

#define _BV( bit ) ( 1<<(bit) )

void bin(short d) {
    int i;
    for(i = 7; i >= 0; i--) {
        if((1 << i) & d)
            printf("1");
        else printf("0");
    }
    printf("\n");
}

void main() {
    int i;
    short a, b, ISC00, ISC01;

    // b = 0x41;
    // printf("0x%x\n", b);

    a = 0x00;
    bin(a);

    ISC00 = 0;
    ISC01 = 1;

    bin(~(_BV(ISC01)) | _BV(ISC00));
    bin(a & ~(_BV(ISC01)) | _BV(ISC00));
}


