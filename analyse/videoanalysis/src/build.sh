#!/bin/bash

gcc -O3 -Wall -fPIC $(pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0) -c -o videoanalysis.o gstvideoanalysis.c

gcc -O3 -Wall -fPIC $(pkg-config --cflags glib-2.0) -c -o analysis.o analysis.c

gcc -O3 -Wall -fPIC $(pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0) -c -o videoanalysis_api.o videoanalysis_api.c

gcc -shared -o libvideoanalysis.so videoanalysis.o videoanalysis_api.o analysis.o  $(pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 glib-2.0)

rm *.o
