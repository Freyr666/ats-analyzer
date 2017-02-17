CC = g++

CFLAGS = -Wall -O3 -march=native -c
CFLAGS += $(pkg-config --cflags gstreamermm-1.0 glibmm-2.4)

LDFLAGS = 
LDFLAGS += $(pkg-config --libs gstreamermm-1.0 glibmm-2.4)

SOURCES = main.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) ats3-backend

ats3-backend: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(OBJECTS) -o ../../build/ats3-backend

.c.o:
	@$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
