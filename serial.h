#ifndef __SERIAL_H
#define __SERIAL_H

/* ringbuffer */
#define RBUF 64

char *itoa(char *s, uint16_t n);
char *buf_append(char *p, char *s);
void print(char d);
void print_str(char *d);
inline void serial_buffer_reset();
void wait_on_serial();

#endif
