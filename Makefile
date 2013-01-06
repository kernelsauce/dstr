CC=gcc
AR=ar
CFLAGS= -Wall -O3 -fPIC -I./src
LDFLAGS=
OBJECTS=dstr.o
LIBRARY=libdstr
LIBRARY_A=$(LIBRARY).a
LIBRARY_SO=$(LIBRARY).so

vpath %.c ./src ./test

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@ 

$(LIBRARY_A): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

$(LIBRARY_SO): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$@.1 -o $@ $(OBJECTS)
	
static: $(LIBRARY_A)
shared: $(LIBRARY_SO)
install: $(LIBRARY_SO)
	sudo install libdstr.so /usr/lib/libdstr.so.1

#Test target depends on libcunit (libcunit1-dev on debian/ubuntu)
test: $(LIBRARY_SO) install
	$(CC) $(CFLAGS) ./test/dstr_test.c -o dstr_test -L./ -ldstr -lcunit

all: static shared

clean:
	rm -rf *.so
	rm -rf *.a
	rm -rf *.o
	rm -rf dstr_test
