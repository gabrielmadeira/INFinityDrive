DIR_ROOT=$(abspath .)
DIR_BIN=$(DIR_ROOT)/bin
DIR_SRC=$(DIR_ROOT)/src
DIR_FLE=$(DIR_SRC)/file

CPPS=$(DIR_SRC)/connection.cpp $(DIR_FLE)/file.cpp $(DIR_FLE)/filemanager.cpp  $(DIR_FLE)/syncdir.cpp

CC=g++
EXE=.exe
DEBUGGER=gdb
FLAGS= -std=c++17 -pthread -I$(DIR_SRC) -I$(DIR_FLE)

USR=zacefron
SP=4000
CP=5000
ADR=127.0.0.1
ADR1=$(ADR)
ADR2=$(ADR1)
ADR3=$(ADR2)
ADR4=$(ADR3)
ADR5=$(ADR4)

INF1=1 $(ADR1) $(shell echo $$(($(SP)+1)))
INF2=$(INF1) 2 $(ADR2) $(shell echo $$(($(SP)+2)))
INF3=$(INF2) 3 $(ADR3) $(shell echo $$(($(SP)+3)))
INF4=$(INF3) 4 $(ADR4) $(shell echo $$(($(SP)+4)))
INF5=$(INF4) 5 $(ADR5) $(shell echo $$(($(SP)+5)))

init: sc cc

s: sc sr

c: cc cr

s%:
	mkdir -p server && cp $(DIR_BIN)/server$(EXE) $(DIR_ROOT)/server/ && cd $(DIR_ROOT)/server/ && \
	$(DIR_ROOT)/server/server$(EXE) $(SP) $* $(INF$*)

b%: 
	mkdir -p backup$* && cp $(DIR_BIN)/server$(EXE) $(DIR_ROOT)/backup$*/ && cd $(DIR_ROOT)/backup$*/ && \
	$(DIR_ROOT)/backup$*/server$(EXE) $(shell echo $$(($(SP)+$*))) $*

c%:
	mkdir -p client$* && cp $(DIR_BIN)/client$(EXE) $(DIR_ROOT)/client$*/ && cd $(DIR_ROOT)/client$*/ && \
	$(DIR_ROOT)/client$*/client$(EXE) $(USR) $(shell echo $$(($(CP)+$*))) $(ADR) $(SP)

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
	
reset:
	rm -rf sync_dir && rm -rf clients && rm -rf backup[0-9]* && rm -rf client[0-9]* && rm -rf server \
	&& find . \( -name '*.o' \) -type f -delete

clean: reset
	rm -rf bin && find . \( -name '*.exe' \) -type f -delete