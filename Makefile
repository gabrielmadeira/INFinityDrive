DIR_ROOT=$(abspath .)
DIR_BIN=$(DIR_ROOT)/bin
DIR_SRC=$(DIR_ROOT)/src
DIR_FLE=$(DIR_SRC)/file

CPPS=$(DIR_SRC)/connection.cpp $(DIR_FLE)/file.cpp $(DIR_FLE)/filemanager.cpp  $(DIR_FLE)/syncdir.cpp

CC=g++
EXE=.exe
DEBUGGER=gdb
FLAGS= -std=c++17 -pthread -I$(DIR_SRC) -I$(DIR_FLE)

SP=4000
CP=5000
ADR=127.0.0.1
# INF=3 1 $(ADR) 4001 2 $(ADR) 4002 3 $(ADR) 4003
INF=1 1 $(ADR) 4001
USR=roberto

init: sc cc

s: sc sr

c: cc cr

sr:
	$(DIR_BIN)/server$(EXE) $(SP) $(INF)

b%: 
	mkdir -p $@ && cp $(DIR_BIN)/server$(EXE) $(DIR_ROOT)/$@/ && cd $(DIR_ROOT)/$@/ && \
	$(DIR_ROOT)/$@/server$(EXE) $(shell echo $$(($(SP)+$*))) $*

cr:
	$(DIR_BIN)/client$(EXE) $(USR) $(CP) $(ADR) $(SP)

dir: 
	mkdir -p bin

server: 
	$(CC) -c $(DIR_SRC)/server.cpp $(DIR_FLE)/file.cpp $(FLAGS)

client: 
	$(CC) -c $(DIR_SRC)/client.cpp $(DIR_FLE)/file.cpp $(FLAGS)

connection: 
	$(CC) -c $(DIR_SRC)/connection.cpp $(DIR_FLE)/file.cpp $(FLAGS)

cc: dir client connection
	$(CC) $(FLAGS) -o $(DIR_BIN)/client$(EXE) $(DIR_SRC)/clientui.cpp $(DIR_SRC)/client.cpp $(CPPS)

sc: dir server connection
	$(CC) $(FLAGS) -o $(DIR_BIN)/server$(EXE) $(DIR_SRC)/serverui.cpp $(DIR_SRC)/server.cpp $(CPPS)
	
clean:
	rm -rf bin && rm -rf sync_dir && rm -rf clients && rm -rf b* && find . \( -name '*.o' -o -name '*.exe' \) -type f -delete