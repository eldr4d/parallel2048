CC = /usr/bin/g++
CFLAGS = -I. -O3 -I/usr/local/lib/g++-include

LDFLAGS =  -lnsl 

OBJ_ALL = Board.o
SRC_ALL = Board.cpp

OBJ_SER = Server.o
SRC_SER = Server.cpp

OBJ_CLN = Client-comm.o Client.o
SRC_CLN = Client-comm.cpp Client.cpp

all:client server

clean:
	rm $(OBJ_ALL) $(OBJ_SER) $(OBJ_CLN) server client

server:$(OBJ_ALL) $(OBJ_SER)
	$(CC) $(CFLAGS) -o server $(OBJ_ALL) $(OBJ_SER) $(LDFLAGS)

client:$(OBJ_ALL) $(OBJ_CLN)
	$(CC) $(CFLAGS) -o client $(OBJ_ALL) $(OBJ_CLN) $(LDFLAGS)

depend:
	makedepend $(CFLAGS) $(SRC_SER) $(SRC_CLN) $(SRC_ALL)

# DO NOT DELETE THIS LINE -- make depend depends on it.


