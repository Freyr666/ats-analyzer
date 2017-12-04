CC = g++

CFLAGS = -Wall -Wno-deprecated -O0 -std=c++17 -g -c
CFLAGS += `pkg-config --cflags gstreamermm-1.0 glibmm-2.4 gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0 libzmq`

LDFLAGS = 
LDFLAGS += `pkg-config --libs gstreamermm-1.0 glibmm-2.4 gstreamer-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0 libzmq`

HEADERS = chatterer.hpp

SOURCES = main.cpp probe.cpp settings.cpp settings_chatterer.cpp \
          metadata.cpp parser.cpp streams.cpp streams_chatterer.cpp \
          graph.cpp graph_chatterer.cpp video_data.cpp context.cpp control.cpp	\
          wm.cpp wm_chatterer.cpp wm_container.cpp wm_treeview.cpp \
          wm_treeview_template.cpp wm_widget_video.cpp \
          pad.cpp branch.cpp root.cpp renderer.cpp \
          json-schema-draft4.json.cpp json-uri.cpp json-validator.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) ats3-backend

ats3-backend: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(OBJECTS) -o ../../build/ats3-backend

.cpp.o: $(HEADERS)
	@$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
