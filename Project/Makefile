cc=g++
server=ChatServer
client=ChatClient
INCLUDE=-I./include
LDFLAGS=-std=c++11 -lpthread -ljson
LIB_PATH=./lib/lib/libjson.a

.PHONY:all
all:$(server) $(client)

$(server):ChatServer.cc
	$(cc) -o $@ $^ $(INCLUDE) $(LIB_PATH) $(LDFLAGS) 

$(client):ChatClient.cc
	$(cc) -o $@ $^ $(INCLUDE) $(LIB_PATH) -lncurses  $(LDFLAGS) 

.PHONY:clean
clean:
	rm -f $(server) $(client)


