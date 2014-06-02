CXXFLAGS = -std=c++11 -g3

LDFLAGS =  -lnsl 

OBJ_ALL = BitBoard.o BitUtils.o
SRC_ALL = BitBoard.cpp BitUtils.cpp

OBJ_SER = Communication/Comm.o Server.o 
SRC_SER = Communication/Comm.hpp Server.cpp 

OBJ_CLN = Communication/Client-comm.o Client.o 
SRC_CLN = Communication/Client-comm.cpp Client.cpp 

all:client server

clean:
	rm $(OBJ_ALL) $(OBJ_SER) $(OBJ_CLN) server client

server:$(OBJ_ALL) $(OBJ_SER)
	$(CXX) $(CXXFLAGS) -o server $(OBJ_ALL) $(OBJ_SER) $(LDFLAGS)

client:$(OBJ_ALL) $(OBJ_CLN)
	$(CXX) $(CXXFLAGS) -o client $(OBJ_ALL) $(OBJ_CLN) $(LDFLAGS)

depend:
	makedepend $(CFLAGS) $(SRC_SER) $(SRC_CLN) $(SRC_ALL)

# DO NOT DELETE THIS LINE -- make depend depends on it.


