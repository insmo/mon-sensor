/* This implementation of the ring-buffer 
 * algorithm is a simplification of 
 * Dean Camera's ring-buffer library. */

#ifndef _RBUF_H_
#define _RBUF_H_

#include <stdint.h>

#define RBUF 255

typedef struct {
    uint8_t buf[RBUF];
    uint8_t *in;
    uint8_t *out;
    uint8_t count;
} rbuf_t;

void rbuf_init(rbuf_t *buf);
uint8_t rbuf_count(rbuf_t *buf);
uint8_t rbuf_full(rbuf_t *buf);
uint8_t rbuf_empty(rbuf_t *buf);
void rbuf_insert(rbuf_t *buf, uint8_t data);
uint8_t rbuf_remove(rbuf_t *buf);

#endif
