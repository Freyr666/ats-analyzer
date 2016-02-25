VIDEOSRC = ./analyse/videoanalysis/src/
BACKSRC = ./backend/src/

all:
	[ -d build ] || mkdir build
	$(MAKE) -C $(BACKSRC)
	$(MAKE) -C $(VIDEOSRC)

clean:
	$(MAKE) clean -C $(BACKSRC)
	$(MAKE) clean -C $(VIDEOSRC)

install:
	cp ./build/ats3-backend /usr/bin
	cp ./build/libvideoanalysis.so /usr/lib64/gstreamer-1.0

uninstall:
	rm /usr/bin/ats3-backend
	rm /usr/lib64/gstreamer-1.0/libvideoanalysis.so
