CFLAGS=-g -Wall -Wextra $(shell pkg-config --cflags libusb-1.0)
LDFLAGS=$(shell pkg-config --libs libusb-1.0)
CC=gcc

# $(shell pkg-config --cflags libusb-1.0) $(shell pkg-config --libs libusb-1.0)
all:
	$(CC) -o main main.c $(CFLAGS) $(LDFLAGS)