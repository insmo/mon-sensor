#!/usr/bin/python2
import os
import sys
import subprocess
import shutil
import time

avrdude = '/home/simon/bin/avrdude'

bauds = (
    1200,
    2400,
    4800,
    9600,
    14400,
    19200,
    28800,
    38400,
    57600,
    76800,
    115200,
)

bits = list()

start = 1
while start < 9601:
    bits.append(start)
    start = start * 2

bits = bauds

args = (
    avrdude,
    '-F',
    '-c', 'ftdi',
    '-p', 'atmega328p',
    '-P', '/dev/ttyUSB0',
    '-e',
    '-u',
    '-U', 'lfuse:w:0x62:m',
    '-U', 'hfuse:w:0xd9:m',
)

ret = 1
while 1:
    ret = subprocess.call(['make', 'u'])
    # for bit in bits:
    #     print("bit: %s" % bit)
    #     for baud in bauds:
    #         print("baud: %s" % baud)
    #         ret = subprocess.call(args + ('-B %s' % bit, '-b %s' % baud))
    # break
    time.sleep(0.2)
