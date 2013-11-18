# Directorio de la la librería googletest
GTEST_DIR = libs/gtest-1.6.0

CXX = g++
CC = gcc
CXXFLAGS=-g -Wall -std=c++98 -Werror -fno-stack-protector -I./ -I./libs/jsoncpp/include
CFLAGS=-g -Wall -Werror -fno-stack-protector -I./ -I./libs/jsoncpp/include

LDFLAGS=-pthread

BINARY=server
JSONCPP=libs/jsoncpp/jsoncpp.a
OBJECTS=embarcacion.o elemento.o casilla.o tablero.o globales.o jugador.o modelo.o jsonificador.o decodificador.o evento.o tiro.o RWLock.o

TESTS_BINARY=tests
TESTS_OBJECTS=gtest-all.o RWLock.o

.PHONY: clean all libs

all: $(BINARY) $(TESTS_BINARY)

$(BINARY): server.cpp $(OBJECTS) $(JSONCPP)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(JSONCPP):
	$(MAKE) -C libs/jsoncpp/src

$(TESTS_BINARY): tests.cpp $(TESTS_OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -I$(GTEST_DIR)/include -o $@ $^

gtest-all.o:
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR) -I$(GTEST_DIR)/include -c $(GTEST_DIR)/src/gtest-all.cc

clean:
	$(MAKE) -C libs/jsoncpp/src clean
	rm -f $(OBJECTS)
	rm -f $(BINARY)
	rm -f $(TESTS_OBJECTS)
	rm -f $(TESTS_BINARY)