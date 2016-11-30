CC      = gcc
CFLAGS  = -g -Wall
RM      = rm -f

default: all

all: iio_temp_mod

iio_temp_mod: main.c
	$(CC) $(CFLAGS) -o $@ $< -ljson-c -liio -I/usr/include/json/

clean:
	$(RM) iio_temp_mod
