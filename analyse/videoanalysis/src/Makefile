CC = gcc

CFLAGS = -Wall -O3 -march=native -fPIC -Wall -c 
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0`

LDFLAGS = -shared -Wall
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 glib-2.0`

PY=python3

all: videoanalysis_api.o videoanalysis.o
	@$(CC) $(LDFLAGS) videodata.o videoanalysis.o -o ../../../build/libvideoanalysis.so

videoanalysis_api.o: analysis.h
	@$(CC) $(CFLAGS) videodata.c -o videodata.o

videoanalysis.o: analysis.h
	@$(CC) $(CFLAGS) gstvideoanalysis.c -o videoanalysis.o

analysis.h:
	@$(PY) generate_array.py analysis.h.template analysis.h

clean:
	rm *.o analysis.h
