CC=g++
CFLAGS=-Wall -g3 -Ofast
CXX=-std=c++17


all: clean maqao-oneview

maqao-oneview: maqao-oneview.cpp
	${CC} ${CFLAGS} ${CXX} -o $@ $@.cpp

clean:
	rm -rf maqao-oneview *~
