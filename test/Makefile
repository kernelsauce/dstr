CC=gcc
CFLAGS=-g -Wall 
CPP=g++
CPPFLAGS=
LD=gcc
LDFLAGS=-g -Wall

OBJECTS=dstr_test.o ../src/dstr.o
TARGET=dstr_test
INCLUDE=-I../src
LIB=-L/usr/lib -lcunit

all: $(TARGET)

clean:
	-rm $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)

%.o: %.cpp Makefile
	$(CPP) $(CPPFLAGS) -o $@ -c $< $(INCLUDE)