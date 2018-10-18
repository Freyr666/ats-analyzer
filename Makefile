VERSION = 0.1.9

BACKSRC = ./backend/

all:
	[ -d build ] || mkdir build
	$(MAKE) -C $(BACKSRC)

clean:
	$(MAKE) clean -C $(BACKSRC)

install:
	cp ./build/ats3-backend /usr/bin

uninstall:
	rm /usr/bin/ats3-backend

tarboll:
	tar czvf /tmp/ats-analyzer-$(VERSION).tar.gz ../ats-analyzer
	mv /tmp/ats-analyzer-$(VERSION).tar.gz ./
