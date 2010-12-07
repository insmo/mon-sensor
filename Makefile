CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRDUDE = /home/simon/bin/avrdude
STTY = stty
SED = sed
NAME = mon
MCU = atmega328p
F_CPU = 8000000
FORMAT = ihex
PORT = /dev/ttyUSB0
BAUD_RATE = 38400
BIT_CLOCK = 19200 
PROGRAMMER = ftdi
ARDUINO_HEADERS = .
CFLAGS = -Os -g -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
         -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
         -Wall -Wextra -I$(ARDUINO_HEADERS)

.PHONY: all list tty osc
.PRECIOUS: %.o %.elf

all: $(NAME).hex

%.o: %.c %.h
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

main.o: %.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -c $< -o $@

%.elf: mon.o serial.o
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
	@$(AVRDUDE) -vD -c$(PROGRAMMER) -b$(BAUD_RATE) -p$(MCU) -P$(PORT) -e -Uflash:w:$<:i -v

list: $(NAME).lss

tty:
	@echo '  STTY -F$(PORT) cs8 parenb -parodd cstopb raw -echo 4800'
	@$(STTY) -F$(PORT) cs8 parenb -parodd cstopb raw -echo 4800

clean:
	rm -f *.o *.elf *.hex *.lss

osc_ext:
	$(AVRDUDE) -F -c$(PROGRAMMER) -p$(MCU) -P$(PORT) -U lfuse:w:0xe6:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m 

osc_32:
	$(AVRDUDE) -F -c$(PROGRAMMER) -p$(MCU) -P$(PORT) -U lfuse:w:0x65:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

osc_default:
	$(AVRDUDE) -F  -c$(PROGRAMMER) -p$(MCU) -P$(PORT) -u -e -U lfuse:w:0x62:m -U hfuse:w:0xd9:m 

osc_nodiv:
	$(AVRDUDE) -F -B$(BIT_CLOCK) -c$(PROGRAMMER) -p$(MCU) -P$(PORT) -u -e -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m 
