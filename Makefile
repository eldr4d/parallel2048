CXXFLAGS = -std=c++11 -g3 -pthread 

LDFLAGS =  -lnsl 

OBJ_ALL = Board/BitBoard.o Board/BitUtils.o Utils.o
SRC_ALL = Board/BitBoard.cpp Board/BitUtils.cpp Utils.cpp

OBJ_SER = Communication/Comm.o Server.o 
SRC_SER = Communication/Comm.hpp Server.cpp 

OBJ_CLN = Communication/Client-comm.o Client.o NegaScout/Search.o
SRC_CLN = Communication/Client-comm.cpp Client.cpp NegaScout/Search.cpp

parallel:CXXFLAGS += -DPARALLELIMPL=true
single:CXXFLAGS += -DPARALLELIMPL=false
compare:CXXFLAGS += -DCOMPARE

parallel:clean all
single:clean all
compare:clean all

all:client server

clean:
	-rm $(OBJ_ALL) $(OBJ_SER) $(OBJ_CLN) server client

server:$(OBJ_ALL) $(OBJ_SER)
	$(CXX) $(CXXFLAGS) -o server $(OBJ_ALL) $(OBJ_SER) $(LDFLAGS)

client:$(OBJ_ALL) $(OBJ_CLN)
	$(CXX) $(CXXFLAGS) -o client $(OBJ_ALL) $(OBJ_CLN) $(LDFLAGS)

depend:
	makedepend $(CFLAGS) $(SRC_SER) $(SRC_CLN) $(SRC_ALL)

# DO NOT DELETE THIS LINE -- make depend depends on it.

Communication/Comm.o: /usr/include/netinet/in.h /usr/include/features.h
Communication/Comm.o: /usr/include/stdc-predef.h /usr/include/stdint.h
Communication/Comm.o: /usr/include/endian.h /usr/include/netdb.h
Communication/Comm.o: /usr/include/rpc/netdb.h /usr/include/unistd.h
Communication/Comm.o: /usr/include/getopt.h /usr/include/arpa/inet.h
Communication/Comm.o: /usr/include/string.h
Server.o: /usr/include/unistd.h /usr/include/features.h
Server.o: /usr/include/stdc-predef.h /usr/include/getopt.h
Server.o: Communication/Comm.hpp /usr/include/netinet/in.h
Server.o: /usr/include/stdint.h /usr/include/endian.h /usr/include/netdb.h
Server.o: /usr/include/rpc/netdb.h /usr/include/arpa/inet.h
Server.o: /usr/include/string.h Communication/Protocol.hpp Definitions.hpp
Server.o: Board/BitBoard.hpp Board/BitUtils.hpp Board/BitBoard.hpp
Communication/Client-comm.o: Communication/Client-comm.hpp
Communication/Client-comm.o: /usr/include/time.h /usr/include/features.h
Communication/Client-comm.o: /usr/include/stdc-predef.h
Communication/Client-comm.o: Communication/Comm.hpp /usr/include/netinet/in.h
Communication/Client-comm.o: /usr/include/stdint.h /usr/include/endian.h
Communication/Client-comm.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
Communication/Client-comm.o: /usr/include/unistd.h /usr/include/getopt.h
Communication/Client-comm.o: /usr/include/arpa/inet.h /usr/include/string.h
Communication/Client-comm.o: Communication/Protocol.hpp Definitions.hpp
Communication/Client-comm.o: Board/BitBoard.hpp Board/BitUtils.hpp
Client.o: NegaScout/TranspositionTable.hpp Communication/Protocol.hpp
Client.o: Definitions.hpp Board/BitBoard.hpp Board/BitUtils.hpp
Client.o: Board/BitUtils.hpp Communication/Client-comm.hpp
Client.o: /usr/include/time.h /usr/include/features.h
Client.o: /usr/include/stdc-predef.h Communication/Comm.hpp
Client.o: /usr/include/netinet/in.h /usr/include/stdint.h
Client.o: /usr/include/endian.h /usr/include/netdb.h /usr/include/rpc/netdb.h
Client.o: /usr/include/unistd.h /usr/include/getopt.h
Client.o: /usr/include/arpa/inet.h /usr/include/string.h
Client.o: Communication/Protocol.hpp Board/BitBoard.hpp
Client.o: NegaScout/MoveIterator.hpp NegaScout/Search.hpp
Client.o: Communication/Client-comm.hpp NegaScout/TranspositionTable.hpp
Client.o: ThreadPool/ThreadPool.hpp /usr/include/stdio.h /usr/include/libio.h
Client.o: /usr/include/_G_config.h /usr/include/wchar.h NegaScout/Search.hpp
NegaScout/Search.o: NegaScout/Search.hpp Communication/Client-comm.hpp
NegaScout/Search.o: NegaScout/TranspositionTable.hpp
NegaScout/Search.o: Communication/Protocol.hpp Definitions.hpp
NegaScout/Search.o: Board/BitBoard.hpp Board/BitUtils.hpp Board/BitUtils.hpp
NegaScout/Search.o: ThreadPool/ThreadPool.hpp /usr/include/stdio.h
NegaScout/Search.o: /usr/include/features.h /usr/include/stdc-predef.h
NegaScout/Search.o: /usr/include/libio.h /usr/include/_G_config.h
NegaScout/Search.o: /usr/include/wchar.h /usr/include/unistd.h
NegaScout/Search.o: /usr/include/getopt.h NegaScout/MoveIterator.hpp
Board/BitBoard.o: Board/BitBoard.hpp Board/BitUtils.hpp Definitions.hpp
Board/BitUtils.o: Board/BitUtils.hpp
Utils.o: Communication/Protocol.hpp Definitions.hpp Board/BitBoard.hpp
Utils.o: Board/BitUtils.hpp Board/BitBoard.hpp
