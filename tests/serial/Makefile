CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRDUDE = /home/simon/bin/avrdude
STTY = stty
SED = sed
NAME = serial_test
MCU = atmega328p
F_CPU = 8000000
FORMAT = ihex
PORT = /dev/ttyUSB1
BAUD_RATE = 38400
PROGRAMMER = ftdi
ARDUINO_HEADERS = .
CFLAGS = -O2 -g -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
         -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
         -Wall -Wextra -I$(ARDUINO_HEADERS) \
         -Wl,-u,vfprintf -lprintf_flt -lm

.PHONY: all list tty osc
.PRECIOUS: %.o %.elf

all: $(NAME).hex

%.o: %.c %.h
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

main.o: serial_test.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

%.elf: %.o
	@echo '  LD $@'
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.hex: %.elf
	@echo '  OBJCOPY $@'
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom -S $< $@ && \
	  echo "  $$((0x$$($(OBJDUMP) -h $@ | \
	    $(SED) -n '6{s/^  0 \.sec1         //;s/ .*//;p}'))) bytes"

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo '  OBJDUMP > $@'
	@$(OBJDUMP) -h -S $< > $@

upload: $(NAME).hex
	@$(AVRDUDE) -vD -F -c$(PROGRAMMER) -b$(BAUD_RATE) -p$(MCU) -P$(PORT) -e -Uflash:w:$<:i

list: $(NAME).lss

tty:
	@echo '  STTY -F$(PORT) cs8 parenb -parodd cstopb raw -echo 9600'
	@$(STTY) -F$(PORT) cs8 parenb -parodd cstopb raw -echo 9600

clean:
	rm -f *.o *.elf *.hex *.lss

osc:
	$(AVRDUDE) -c $(POGRAMMER) -p $(MCU) -P $(PORT) -U lfuse:w:0xe6:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m 
