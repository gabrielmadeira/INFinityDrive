DIR_ROOT=$(abspath .)
DIR_BIN=$(DIR_ROOT)/bin
DIR_SRC=$(DIR_ROOT)/src

CPPS=$(DIR_SRC)/server.cpp $(DIR_SRC)/server.cpp

CC=g++
DEBUGGER=gdb
FLAGS= -std=c++17 -lpthread

dir: 
	mkdir -p bin

%: %.cpp
	$(CC) $(FLAGS) -o $(DIR_BIN)/$(notdir $@)$(EXE) $@.cpp $(CPPS)

client: dir $(DIR_SRC)/interface $(DIR_SRC)/client

server: dir $(DIR_SRC)/server
	
clean:
	rm -rf bin && find . \( -name '*.o' -o -name '*.exe' \) -type f -delete