CC = g++

CFLAGS = -Wall -Wextra -std=c++20

TARGET = hw2_chat_server

all: $(TARGET)

$(TARGET): hw2_chat_server.o
	$(CC) $(CFLAGS) hw2_chat_server.o -o $(TARGET)

hw2_chat_server.o: hw2_chat_server.cpp command.hpp
	$(CC) $(CFLAGS) -c hw2_chat_server.cpp

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean