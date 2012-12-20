CC=gcc
AR=ar
CFLAGS=-DDSTR_MEM_SECURITY -DDSTR_MEM_CLEAR -Wall -g -fPIC -I./src
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
test: $(LIBRARY_SO) install
	gcc $(CFLAGS) -L. -ldstr -lcunit ./test/dstr_test.c -o dstr_test
	./dstr_test

clean:
	rm -rf *.so
	rm -rf *.a
	rm -rf *.o
	rm -rf dstr_test