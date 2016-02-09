VIDEOSRC = ./analyse/videoanalysis/src/
BACKSRC = ./backend/src/

all:
	$(MAKE) -C $(BACKSRC)
	$(MAKE) -C $(VIDEOSRC)

clean:
	$(MAKE) clean -C $(BACKSRC)
	$(MAKE) clean -C $(VIDEOSRC)
