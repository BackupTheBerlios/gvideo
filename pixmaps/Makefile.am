#copies backup files if regular data files not found

pixs = \
	gvideo.png\
	camera.png\
	movie.png\
	save.png\
	image_controls.png\
	video_controls.png\
	audio_controls.png\
	images_folder.png\
	videos_folder.png\
	controls_folder.png\
	close.png

CONFIG_CLEAN_FILES = gvideo *.desktop* gvideo.1_ gvideo.1.gz gvideo.menu

clean-generic:
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

.manpage:
	if test -e gvideo.1; then\
		echo "compressing man page";\
		cp gvideo.1 gvideo.1_;\
		gzip gvideo.1;\
		cp gvideo.1_ gvideo.1;\
	else\
		echo "could not find man page";\
	fi

if DEBIAN_MENU
.menu:
		if test -e gvideo; then\
			echo "gvideo debian menu file exists";\
		else\
			echo "Creating debian menu file";\
			rm -f gvideo.menu;\
			touch gvideo.menu;\
			echo "?package(gvideo):\\" >> gvideo.menu;\
			echo "	needs=\"X11\"\\" >> gvideo.menu;\
			echo "	section=\"Applications/Video\"\\" >> gvideo.menu;\
			echo "	title=\"gvideo\"\\" >> gvideo.menu;\
			echo "	longtitle=\"GTK UVC Viewer - uvc video viewer and capturer\"\\" >> gvideo.menu;\
			echo "	command=\"gvideo\"\\" >> gvideo.menu;\
			echo "	icon=\"$(datadir)/pixmaps/gvideo/gvideo.png\"" >> gvideo.menu;\
			cp gvideo.menu gvideo;\
		fi
endif

if DESKTOP
.desktop:
	if test -e gvideo.desktop; then\
		echo "gvideo.desktop exists";\
	else\
		echo "creating gvideo.desktop";\
		rm -f gvideo.desktop_;\
		touch gvideo.desktop_;\
		echo "[Desktop Entry]" >> gvideo.desktop_;\
		echo "Version=1.0" >> gvideo.desktop_;\
		echo "Name=gvideo" >> gvideo.desktop_;\
		echo "GenericName=GTK UVC video viewer" >> gvideo.desktop_;\
		echo "Comment=A video viewer and capturer for the linux uvc driver" >> gvideo.desktop_;\
		echo "TryExec=gvideo" >> gvideo.desktop_;\
		echo "Exec=gvideo" >> gvideo.desktop_;\
		echo "Icon=$(datadir)/pixmaps/gvideo/gvideo.png" >> gvideo.desktop_;\
		echo "Terminal=false" >> gvideo.desktop_;\
		echo "Type=Application" >> gvideo.desktop_;\
		echo "Categories=Video;AudioVideo;" >> gvideo.desktop_;\
		echo "StartupNotify=true" >> gvideo.desktop_;\
		cp gvideo.desktop_ gvideo.desktop;\
	fi
endif

.png:
	for file in ${pixs}; do \
		if test -e $$file; then \
			echo "$$file exists";\
		else\
			echo "ERROR: $$file not found!";\
		fi;\
	done

.PHONY: .menu .desktop .png .manpage
