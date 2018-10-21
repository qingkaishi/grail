CC	              = g++
CPPFLAGS          = -g -Wno-deprecated -O3 -c -std=c++11 -openmp
LDFLAGS	          = -O3 

# Server
SERVER_SOURCES    = Server.cpp Graph.cpp GraphUtil.cpp Grail.cpp TCSEstimator.cpp
SERVER_OBJECTS    = $(SERVER_SOURCES:.cpp=.o) interval_tree.o
SERVER_EXECUTABLE = server
# Client
CLIENT_SOURCES    = Client.cpp
CLIENT_OBJECTS    = $(CLIENT_SOURCES:.cpp=.o)
CLIENT_EXECUTABLE = client

# Server
all: $(SERVER_SOURCES) $(SERVER_EXECUTABLE)

# Client
all: $(CLIENT_SOURCES) $(CLIENT_EXECUTABLE)

$(SERVER_EXECUTABLE) : $(SERVER_OBJECTS)
	$(CC) $(LDFLAGS) $(SERVER_OBJECTS) -o $@

$(CLIENT_EXECUTABLE) : $(CLIENT_SOURCES)
	$(CC) $(LDFLAGS) $(CLIENT_SOURCES) -o $@

.cpp.o : 
	$(CC) $(CPPFLAGS) $< -o $@

interval_tree.o : interval_tree.cpp interval_tree.hpp templatestack.hpp
	g++ -c interval_tree.cpp 

.PHONY : clean
clean:
	-rm -f *.o server client
