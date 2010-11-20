#ifndef __SERIAL_H
#define __SERIAL_H

/* ringbuffer */
#define RBUF 64

void print(char d);
void printstr(char *d);
void print_hex4(uint8_t v);
void print_hex8(uint8_t v);
void print_hex16(uint16_t v);
inline void serial_buffer_reset();

#endif
