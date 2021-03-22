CC=gcc
LDFLAG=`pkg-config --libs glib-2.0` -lpcap
CFLAGS=-Wall -ggdb `pkg-config --cflags glib-2.0`

OBJECTS=list.o interfaces.o search.o sniff.o udp_send.o ethernet.o mac.o \
        select.o results.o tcp_send.o time.o ports.o
PROG=search

all: $(OBJECTS)
	$(CC) -o $(PROG) $(OBJECTS) $(LDFLAG)

clean:
	rm -f $(OBJECTS) $(PROG)
