#include "rbuf.h"

void rbuf_init(rbuf_t *buf) {
    // todo: make atomic
    buf->in = buf->buf;
    buf->out = buf->buf;
}

uint8_t rbuf_count(rbuf_t *buf) {
    // todo: make atomic
    return buf->count;
}

uint8_t rbuf_full(rbuf_t *buf) {
    if (rbuf_count(buf) == RBUF)
        return 1;
    return 0;
}

uint8_t rbuf_empty(rbuf_t *buf) {
    if (rbuf_count(buf) == 0)
        return 1;
    return 0;
}

void rbuf_insert(rbuf_t *buf, uint8_t data) {
    *buf->in = data;

    if (++buf->in == &buf->buf[RBUF])
        buf->in = buf->buf;

    // todo: make atomic
    buf->count++;
}

uint8_t rbuf_remove(rbuf_t *buf) {
    uint8_t data = *buf->out;

    if (++buf->out == &buf->buf[RBUF])
        buf->out = buf->buf;

    // todo: make atomic
    buf->count--;

    return data;
}
