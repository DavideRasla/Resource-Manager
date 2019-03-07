CFLAGS=-Wall -Wextra -pedantic
CFLAGS+=-std=c11
CFLAGS+=-g

all: manager  client

manager: manager.o ports.o
client: client.o ports.o

clean:
	rm -f manager lpd client
	rm -f *.o
