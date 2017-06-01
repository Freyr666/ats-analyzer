CC = clang++

CFLAGS = -Wall -O0 -std=c++1y -g -c
CFLAGS += `pkg-config --cflags gstreamermm-1.0 glibmm-2.4 gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0`

LDFLAGS = 
LDFLAGS += `pkg-config --libs gstreamermm-1.0 glibmm-2.4 gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0`

HEADERS = chatterer.hpp

SOURCES = main.cpp probe.cpp settings.cpp \
          metadata.cpp parser.cpp options.cpp \
					graph.cpp context.cpp control.cpp	\
					json-schema-draft4.json.cpp json-uri.cpp json-validator.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) ats3-backend

ats3-backend: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(OBJECTS) -o ../../build/ats3-backend

.cpp.o: $(HEADERS)
	@$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
