CC = gcc

CFLAGS = -Wall -O3 -march=native -fPIC -c
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-audio-1.0`

LDFLAGS = -shared -lebur128
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-audio-1.0 glib-2.0`

all: audioanalysis.o
	@$(CC) $(LDFLAGS) gstaudioanalysis.o -o ../../../build/libaudioanalysis.so

audioanalysis.o:
	@$(CC) $(CFLAGS) gstaudioanalysis.c -o gstaudioanalysis.o

clean:
	rm *.o
