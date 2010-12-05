#ifndef __SERIAL_H
#define __SERIAL_H

/* ringbuffer */
#define RBUF 64

void print(char d);
void print_str(char *d);
inline void serial_buffer_reset();
void wait_on_serial();

#endif
