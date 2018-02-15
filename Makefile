VERSION = 0.1.9

VIDEOSRC = ./analyse/videoanalysis/src/
AUDIOSRC = ./analyse/audioanalysis/src/
BACKSRC = ./backend/

all:
	[ -d build ] || mkdir build
	$(MAKE) -C $(BACKSRC)
	$(MAKE) -C $(VIDEOSRC)
	$(MAKE) -C $(AUDIOSRC)

clean:
	$(MAKE) clean -C $(BACKSRC)
	$(MAKE) clean -C $(VIDEOSRC)
	$(MAKE) clean -C $(AUDIOSRC)

install:
	cp ./build/ats3-backend /usr/bin
	cp ./build/libvideoanalysis.so /usr/lib64/gstreamer-1.0
	cp ./build/libaudioanalysis.so /usr/lib64/gstreamer-1.0

uninstall:
	rm /usr/bin/ats3-backend
	rm /usr/lib64/gstreamer-1.0/libvideoanalysis.so
	rm /usr/lib64/gstreamer-1.0/libaudioanalysis.so

tarboll:
	tar czvf /tmp/ats-analyzer-$(VERSION).tar.gz ../ats-analyzer
	mv /tmp/ats-analyzer-$(VERSION).tar.gz ./
