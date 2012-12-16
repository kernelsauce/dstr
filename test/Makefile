CC=gcc
CFLAGS=
CPP=g++
CPPFLAGS=
LD=gcc
LDFLAGS=

OBJECTS=dstr_test.o ../src/dstr.o
TARGET=dstr_test
INCLUDE=-I../src
LIB=

all: $(TARGET)

clean:
	-rm $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)

%.o: %.cpp Makefile
	$(CPP) $(CPPFLAGS) -o $@ -c $< $(INCLUDE)