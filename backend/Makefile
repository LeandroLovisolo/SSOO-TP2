CXX = g++
CC = gcc
CXXFLAGS=-g -Wall -std=c++98 -Werror -fno-stack-protector -I./ -I./libs/jsoncpp/include
CFLAGS=-g -Wall -Werror -fno-stack-protector -I./ -I./libs/jsoncpp/include

LDFLAGS=-pthread

BINARY=server
JSONCPP=libs/jsoncpp/jsoncpp.a
OBJECTS=embarcacion.o elemento.o casilla.o tablero.o globales.o jugador.o modelo.o jsonificador.o decodificador.o evento.o tiro.o RWLock.o

.PHONY: clean all libs

all: $(OBJECTS) libs server

$(BINARY): server.cpp $(OBJECTS) $(JSONCPP)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(JSONCPP):
	$(MAKE) -C libs/jsoncpp/src

clean:
	rm -f $(OBJECTS)
	rm -f $(BINARY)
