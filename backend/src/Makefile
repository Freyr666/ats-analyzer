CC = gcc

CFLAGS = -Wall -O3 -march=native -c -g
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0`

LDFLAGS = 
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0 glib-2.0`

SOURCES = main.c parse_ts.c proc_branch.c
SOURCES += proc_tree.c proc_metadata.c

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) ats3-backend

ats3-backend: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(OBJECTS) -o ../../build/ats3-backend

.c.o:
	@$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
