# Makefile for CPE464 tcp test code
# written by Hugh Smith - April 2019

CC= g++
CFLAGS= -g -Wall
LIBS = 

OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o IOcontrol.o serverObj.o clientTable.o client.o

.PHONY: $(OBJS)

all:   cclient server

cclient: cclient.cpp $(OBJS)
	$(CC) $(CFLAGS) -o cclient cclient.cpp  $(OBJS) $(LIBS)

server: server.cpp $(OBJS)
	$(CC) $(CFLAGS) -o server server.cpp $(OBJS) $(LIBS)

networks.o:
	$(CC) $(CFLAGS) -c networks.c

gethostbyname.o:
	$(CC) $(CFLAGS) -c gethostbyname.c

pollLib.o:
	$(CC) $(CFLAGS) -c pollLib.c

safeUtil.o:
	$(CC) $(CFLAGS) -c safeUtil.c

IOcontrol.o:
	$(CC) $(CFLAGS) -c IOcontrol.cpp

serverObj.o:
	$(CC) $(CFLAGS) -c serverObj.cpp

client.o:
	$(CC) $(CFLAGS) -c client.cpp

clientTable.o:
	$(CC) $(CFLAGS) -c clientTable.cpp


.c.o:
	g++ -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f myServer myClient *.o



