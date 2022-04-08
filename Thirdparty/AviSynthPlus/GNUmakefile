# Makefile

all: default

PREFIX=/usr/local

install:
	install -d $(PREFIX)/include/avisynth
	cp avs_core/include/avisynth.h $(PREFIX)/include/avisynth
	cp avs_core/include/avisynth_c.h $(PREFIX)/include/avisynth
	cp -R avs_core/include/avs $(PREFIX)/include/avisynth

uninstall:
	rm -f -R $(PREFIX)/include/avisynth