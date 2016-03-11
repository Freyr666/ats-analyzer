CC = gcc

CFLAGS = -Wall -O3 -march=native -fPIC -c
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0`

LDFLAGS = -shared
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 glib-2.0`

all: videoanalysis_api.o videoanalysis.o
	@$(CC) $(LDFLAGS) videodata.o videoanalysis.o -o ../../../build/libvideoanalysis.so

videoanalysis_api.o:
	@$(CC) $(CFLAGS) videodata.c -o videodata.o

videoanalysis.o:
	@$(CC) $(CFLAGS) gstvideoanalysis.c -o videoanalysis.o

clean:
	rm *.o
