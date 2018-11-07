#----------------------------------------------------------------------------------------------
#  VSYS BIF WS2018 Dittmann, Kreidl
#  date: 23.09.2018
#  makefile for client and server
#----------------------------------------------------------------------------------------------

CC = g++
#CFLAGS = -std=c++11 -g -D_BSD_SOURCE -D_XOPEN_SOURCE -Wall -o
CFLAGS = -std=c++11 -Wall -DLDAP_DEPRECATED -o

all: Server

Server: main.cpp 
	echo "compiling test: main.cpp..."
	${CC} ${CFLAGS} bin/Server *.cpp -lldap -llber -pthread


clean:
	echo "cleaning binaries..."
	#rm -rf bin
	rm -f bin/Server

