CC = g++
CFLAGS = -g -Wall -Werror -fno-stack-protector -I../include
LDFLAGS = -pthread

LIB=../jsoncpp.a
OBJECTS = json_value.o json_writer.o json_reader.o
.PHONY: clean all


all: clean $(OBJECTS) $(LIB)

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIB):
	ar rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f $(LIB)