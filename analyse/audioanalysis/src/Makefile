CC = gcc

CFLAGS = -Wall -O3 -march=native -fPIC -c
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-audio-1.0`

LDFLAGS = -shared
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-audio-1.0 glib-2.0`

all: audioanalysis.o audiodata.o ebur.o
	@$(CC) $(LDFLAGS) gstaudioanalysis.o audiodata.o ebur.o -o ../../../build/libaudioanalysis.so

audioanalysis.o:
	@$(CC) $(CFLAGS) gstaudioanalysis.c -o gstaudioanalysis.o

audiodata.o:
	@$(CC) $(CFLAGS) audiodata.c -o audiodata.o

ebur.o:
	@$(CC) $(CFLAGS) ebur128.c -o ebur.o

clean:
	rm *.o
