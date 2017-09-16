CC = gcc
CFLAGS = -g -Wall

ALL = on-off
all: ${ALL}

on-off: on-off.o
		$(CC) $^ -o $@ -lpthread -lrt

clean:
		${RM} *.o ${ALL}
