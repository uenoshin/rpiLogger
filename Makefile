PROGRAM1 = rpiLogger1
PROGRAM2 = rpiLogger2
CC = gcc
CFLAGS =  -Wall -pthread

.SUFFIXES: .c .o

#OBJS = pig_bme680.o bme680.o pig_adxl343.o
OBJS1 = pig_adxl343.o
OBJS2 = pigd_adxl343.o

.PHONY: all
all: $(PROGRAM1) $(PROGRAM2) 

rpiLogger1: $(OBJS1)
	gcc $(CFLAGS) -o $(PROGRAM1) $(OBJS1) -lpigpio -lrt

rpiLogger2: $(OBJS2)
	gcc $(CFLAGS) -o $(PROGRAM2) $(OBJS2) -lpigpiod_if2 -lrt

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm $(PROGRAM1) $(PROGRAM2) $(OBJS1) $(OBJS2)


