DIR_ROOT=$(abspath .)
DIR_BIN=$(DIR_ROOT)/bin
DIR_SRC=$(DIR_ROOT)/src
DIR_FLE=$(DIR_SRC)/file

CPPS=$(DIR_SRC)/connection.cpp $(DIR_FLE)/file.cpp $(DIR_FLE)/filemanagement.cpp  $(DIR_FLE)/syncdir.cpp

CC=g++
EXE=.exe
DEBUGGER=gdb
FLAGS= -std=c++17 -lpthread -I$(DIR_SRC) -I$(DIR_FLE)

PORT=4000
ADDR=127.0.0.1
USERNAME=roberto

s: serverc
	$(DIR_BIN)/server$(EXE)

c: clientc
	$(DIR_BIN)/client$(EXE) $(USERNAME) $(ADDR) $(PORT)

dir: 
	mkdir -p bin

server: 
	$(CC) -c $(DIR_SRC)/server.cpp $(DIR_FLE)/file.cpp $(FLAGS)

client: 
	$(CC) -c $(DIR_SRC)/client.cpp $(DIR_FLE)/file.cpp $(FLAGS)

connection: 
	$(CC) -c $(DIR_SRC)/connection.cpp $(DIR_FLE)/file.cpp $(FLAGS)

clientc: dir client connection
	$(CC) $(FLAGS) -o $(DIR_BIN)/client$(EXE) $(DIR_SRC)/clientui.cpp $(DIR_SRC)/client.cpp $(CPPS)

serverc: dir server connection
	$(CC) $(FLAGS) -o $(DIR_BIN)/server$(EXE) $(DIR_SRC)/serverui.cpp $(DIR_SRC)/server.cpp $(CPPS)
	
clean:
	rm -rf bin && find . \( -name '*.o' -o -name '*.exe' \) -type f -delete