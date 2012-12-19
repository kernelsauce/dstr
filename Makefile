CC=gcc
AR=ar
CFLAGS=-Wall -O3 -fPIC 
LDFLAGS=
OBJECTS=dstr.o dstr_test.o
TESTS=
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

$(TEST): $(TESTS)
	$(CC) -L/usr/lib -lcunit -ldstr -o $@ $(TESTS)
	
static: $(LIBRARY_A)
shared: $(LIBRARY_SO)
test: $(TEST)

all:: static shared test

clean:
	rm -rf *.so
	rm -rf *.a
	rm -rf *.o
