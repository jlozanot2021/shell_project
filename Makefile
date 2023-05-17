# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -Wshadow

LFLAGS= -g

# the build target executable:
TARGET = shell

all: $(TARGET)

$(TARGET): main.c $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -c   $(TARGET).c 
	$(CC) $(CFLAGS) -c   main.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o main.o 

clean:
	$(RM) $(TARGET) *.o