SUBDIRS = src


check_povcore: xFORCE
	$(MAKE) -C src/pov-core/ check

check_core: xFORCE
	$(MAKE) -C src/core/ check

check_numerics: xFORCE
	$(MAKE) -C src/numerics/ check

RVARGS=
rendview: xFORCE
	../rendview-build/src/rendview -rdfile=../rendview/cfg/renderers-priv.par \
		-l-rd=povray3.5 -l-r-args+="+L../anivision/src/pov-scan/ +AM4 -display :0.0 -geometry --8+45" \
		-l-r-ifpattern=src/core/f%07d.pov -l-r-ofpattern=/tmp/f%07d \
		-l-rcont -l-cont $(RVARGS)

encode: rendview
	/usr/src/film/MPlayer/MPlayer-1.0pre3/mencoder \
	 	-mf w=320:h=240:fps=25:type=png \
		-ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=600:vqmin=2:vme=1:v4mv:dia=2:cmp=6:vpass=1 \
		-o /tmp/testfilm.avi mf:///tmp/"f0000*.png"
	/usr/src/film/MPlayer/MPlayer-1.0pre3/mencoder \
	 	-mf w=320:h=240:fps=25:type=png \
		-ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=600:vqmin=2:vme=1:v4mv:dia=2:cmp=6:vpass=2 \
		-o /tmp/testfilm.avi mf:///tmp/"f0000*.png"

encode2: rendview
	 /usr/src/film/MPlayer/MPlayer-1.0pre3/mencoder \
	 	-mf w=320:h=240:fps=25:type=png \
		-ovc copy -o /tmp/testfilm.avi mf:///tmp/"f0000*.png"

MPARGS=
play: xFORCE
	/usr/src/film/MPlayer/MPlayer-1.0pre3/mplayer -vo xv /tmp/testfilm.avi \
		-loop 0 -xy 2 $(MPARGS)

xFORCE:
	
