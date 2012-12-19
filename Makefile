CC=clang
AR=ar
CFLAGS=-Wall -O2 -fPIC 
LDFLAGS=
OBJECTS=dstr.o
LIBRARY=libdstr
LIBRARY_A=$(LIBRARY).a
LIBRARY_SO=$(LIBRARY).so

vpath %.c ./src

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@ 

$(LIBRARY_A): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

$(LIBRARY_SO): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$@.1 -o $@ $(OBJECTS)

static: $(LIBRARY_A)
shared: $(LIBRARY_SO)

all:: static shared

clean:
	rm -rf *.so
	rm -rf *.a
	rm -rf *.o