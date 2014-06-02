CXXFLAGS = -std=c++11 -g3 -pthread

LDFLAGS =  -lnsl 

OBJ_ALL = BitBoard.o BitUtils.o
SRC_ALL = BitBoard.cpp BitUtils.cpp

OBJ_SER = Communication/Comm.o Server.o 
SRC_SER = Communication/Comm.hpp Server.cpp 

OBJ_CLN = Communication/Client-comm.o Client.o
SRC_CLN = Communication/Client-comm.cpp Client.cpp ThreadPool.hpp TranspositionTable.hpp

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

Communication/Comm.o: /usr/include/stdio.h /usr/include/features.h
Communication/Comm.o: /usr/include/stdc-predef.h /usr/include/libio.h
Communication/Comm.o: /usr/include/_G_config.h /usr/include/wchar.h
Communication/Comm.o: /usr/include/stdlib.h /usr/include/alloca.h
Communication/Comm.o: /usr/include/netinet/in.h /usr/include/stdint.h
Communication/Comm.o: /usr/include/endian.h /usr/include/netdb.h
Communication/Comm.o: /usr/include/rpc/netdb.h /usr/include/errno.h
Communication/Comm.o: /usr/include/unistd.h /usr/include/getopt.h
Communication/Comm.o: /usr/include/arpa/inet.h /usr/include/string.h
Communication/Comm.o: /usr/include/xlocale.h
Server.o: /usr/include/unistd.h /usr/include/features.h
Server.o: /usr/include/stdc-predef.h /usr/include/getopt.h
Server.o: Communication/Comm.hpp /usr/include/stdio.h /usr/include/libio.h
Server.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
Server.o: /usr/include/alloca.h /usr/include/netinet/in.h
Server.o: /usr/include/stdint.h /usr/include/endian.h /usr/include/netdb.h
Server.o: /usr/include/rpc/netdb.h /usr/include/errno.h
Server.o: /usr/include/arpa/inet.h /usr/include/string.h
Server.o: /usr/include/xlocale.h Communication/Protocol.hpp BitBoard.hpp
Server.o: BitUtils.hpp BitBoard.hpp
Communication/Client-comm.o: Communication/Client-comm.hpp
Communication/Client-comm.o: /usr/include/time.h /usr/include/features.h
Communication/Client-comm.o: /usr/include/stdc-predef.h
Communication/Client-comm.o: /usr/include/xlocale.h Communication/Comm.hpp
Communication/Client-comm.o: /usr/include/stdio.h /usr/include/libio.h
Communication/Client-comm.o: /usr/include/_G_config.h /usr/include/wchar.h
Communication/Client-comm.o: /usr/include/stdlib.h /usr/include/alloca.h
Communication/Client-comm.o: /usr/include/netinet/in.h /usr/include/stdint.h
Communication/Client-comm.o: /usr/include/endian.h /usr/include/netdb.h
Communication/Client-comm.o: /usr/include/rpc/netdb.h /usr/include/errno.h
Communication/Client-comm.o: /usr/include/unistd.h /usr/include/getopt.h
Communication/Client-comm.o: /usr/include/arpa/inet.h /usr/include/string.h
Communication/Client-comm.o: Communication/Protocol.hpp BitBoard.hpp
Communication/Client-comm.o: BitUtils.hpp
Client.o: TranspositionTable.hpp BitUtils.hpp Communication/Client-comm.hpp
Client.o: /usr/include/time.h /usr/include/features.h
Client.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
Client.o: Communication/Comm.hpp /usr/include/stdio.h /usr/include/libio.h
Client.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
Client.o: /usr/include/alloca.h /usr/include/netinet/in.h
Client.o: /usr/include/stdint.h /usr/include/endian.h /usr/include/netdb.h
Client.o: /usr/include/rpc/netdb.h /usr/include/errno.h /usr/include/unistd.h
Client.o: /usr/include/getopt.h /usr/include/arpa/inet.h
Client.o: /usr/include/string.h Communication/Protocol.hpp BitBoard.hpp
Client.o: BitBoard.hpp ThreadPool/ThreadPool.hpp
TranspositionTable.o: BitUtils.hpp
BitBoard.o: BitBoard.hpp
BitUtils.o: BitUtils.hpp
