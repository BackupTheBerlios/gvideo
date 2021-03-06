## Process this file with automake to produce Makefile.in

SUBDIRS = $(GVIDEO_LIBRARY_NAME) \
          $(GVRENDER_LIBRARY_NAME) \
          $(GVAUDIO_LIBRARY_NAME) \
          $(GVENCODER_LIBRARY_NAME) \
          $(GVTHREADS_LIBRARY_NAME) \
          $(GVIDEO_CMD_NAME) \
          $(GVIDEO_GTK_NAME) \
          $(GVIDEO_QT_NAME) \
          common \
          po \
          pixmaps

#Distribute these directories:
DIST_SUBDIRS = $(GVIDEO_LIBRARY_NAME) \
               $(GVRENDER_LIBRARY_NAME) \
               $(GVAUDIO_LIBRARY_NAME) \
               $(GVENCODER_LIBRARY_NAME) \
               $(GVTHREADS_LIBRARY_NAME) \
               $(GVIDEO_CMD_NAME) \
               $(GVIDEO_GTK_NAME) \
               $(GVIDEO_QT_NAME) \
               common \
               po \
               pixmaps

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pkgconfig/gvideo-0.1.pc \
                 pkgconfig/gvrender-0.1.pc

ACLOCAL_AMFLAGS = -I m4

pixmapsdir = ${datadir}/pixmaps/gvideo

pixmaps_DATA = \
		pixmaps/gvideo.png\
		pixmaps/camera.png\
		pixmaps/movie.png\
		pixmaps/save.png\
		pixmaps/image_controls.png\
		pixmaps/video_controls.png\
		pixmaps/audio_controls.png\
		pixmaps/images_folder.png\
		pixmaps/videos_folder.png\
		pixmaps/controls_folder.png\
		pixmaps/close.png

if DESKTOP
applicationdir = ${datadir}/applications
application_DATA = pixmaps/gvideo.desktop
endif

if DEBIAN_MENU
menudir = ${datadir}/menu
menu_DATA = pixmaps/gvideo
endif

gvideomandir = ${datadir}/man/man1
gvideoman_DATA = pixmaps/gvideo.1.gz

EXTRA_DIST = bootstrap.sh

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	@find $(srcdir)/src/ -name "*.cpp" -print | sort > $(srcdir)/po/POTFILES.in.2 ; \
	if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
	fi
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

update-gmo: check-gettext
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo: check-gettext
	touch po/*.po
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

$(gvideoman_DATA):
	cd pixmaps && $(MAKE) $(AM_MAKEFLAGS) .manpage

$(pixmaps_DATA):
	cd pixmaps && $(MAKE) $(AM_MAKEFLAGS) .png

if DEBIAN_MENU
$(menu_DATA):
	cd pixmaps && $(MAKE) $(AM_MAKEFLAGS) .menu
endif

if DESKTOP
# builds desktop menu entry
$(application_DATA): 
	cd pixmaps && $(MAKE) $(AM_MAKEFLAGS) .desktop
endif

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

.PHONY: check-gettext update-po update-gmo force-update-gmo
