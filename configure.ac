dnl Process this file with autoconf to produce a configure script.

# Save the original $CFLAGS so we can distinguish whether the user set those
# in the environment, or whether autoconf added -O and -g options:
ORIGINAL_CFLAGS="$CFLAGS"

dnl --------------------------------
dnl Initialization macros (sets PACKAGE and VERSION)
dnl --------------------------------

AC_INIT(gvideo, 0.1.0, pj.assis@gmail.com)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE(disable)
AM_INIT_AUTOMAKE(gnu dist-bzip2 )
AC_CONFIG_MACRO_DIR([m4])

dnl -----------------------------------------------
dnl Package release
dnl -----------------------------------------------

PACKAGE_MAJOR_VERSION=0
PACKAGE_MINOR_VERSION=1
PACKAGE_MICRO_VERSION=0

PACKAGE_VERSION=$PACKAGE_MAJOR_VERSION.$PACKAGE_MINOR_VERSION.$PACKAGE_MICRO_VERSION
PACKAGE_RELEASE=$PACKAGE_MAJOR_VERSION.$PACKAGE_MINOR_VERSION
AC_SUBST(PACKAGE_RELEASE)
AC_SUBST(PACKAGE_VERSION)


dnl -----------------------------------------------
dnl libgvideo name and version number
dnl -----------------------------------------------

GVIDEO_LIBRARY_NAME=libgvideo
AC_SUBST(GVIDEO_LIBRARY_NAME)
GVIDEO_LD_NAME=gvideo
AC_SUBST(GVIDEO_LD_NAME)

#release versioning
GVIDEO_MAJOR_VERSION=0
GVIDEO_MINOR_VERSION=1
GVIDEO_MICRO_VERSION=0

#API version (SONAME)
GVIDEO_API_VERSION=$GVIDEO_MAJOR_VERSION.$GVIDEO_MINOR_VERSION
AC_SUBST(GVIDEO_API_VERSION)

#shared library versioning
GVIDEO_LIBRARY_VERSION=$GVIDEO_MAJOR_VERSION:$GVIDEO_MINOR_VERSION:$GVIDEO_MICRO_VERSION
#
#             current:revision:age
#                |       |     |
#                |       |     +- increment if interfaces have been added
#                |       |        set to zero if interfaces have been removed
#                                  or changed
#                |       +- increment if source code has changed
#                |          set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed

AC_SUBST(GVIDEO_LIBRARY_VERSION)

dnl -----------------------------------------------
dnl libgvaudio name and version number
dnl -----------------------------------------------

GVAUDIO_LIBRARY_NAME=libgvaudio
AC_SUBST(GVAUDIO_LIBRARY_NAME)
GVAUDIO_LD_NAME=gvaudio
AC_SUBST(GVAUDIO_LD_NAME)

GVAUDIO_MAJOR_VERSION=0
GVAUDIO_MINOR_VERSION=1
GVAUDIO_MICRO_VERSION=0

#API version (SONAME)
GVAUDIO_API_VERSION=$GVAUDIO_MAJOR_VERSION.$GVAUDIO_MINOR_VERSION
AC_SUBST(GVAUDIO_API_VERSION)

#shared library versioning
GVAUDIO_LIBRARY_VERSION=$GVAUDIO_MAJOR_VERSION:$GVAUDIO_MINOR_VERSION:$GVAUDIO_MICRO_VERSION
AC_SUBST(GVAUDIO_LIBRARY_VERSION)

dnl -----------------------------------------------
dnl libgvrender name and version number
dnl -----------------------------------------------

GVRENDER_LIBRARY_NAME=libgvrender
AC_SUBST(GVRENDER_LIBRARY_NAME)
GVRENDER_LD_NAME=gvrender
AC_SUBST(GVRENDER_LD_NAME)

GVRENDER_MAJOR_VERSION=0
GVRENDER_MINOR_VERSION=1
GVRENDER_MICRO_VERSION=0

#API version (SONAME)
GVRENDER_API_VERSION=$GVRENDER_MAJOR_VERSION.$GVRENDER_MINOR_VERSION
AC_SUBST(GVRENDER_API_VERSION)

#shared library versioning
GVRENDER_LIBRARY_VERSION=$GVRENDER_MAJOR_VERSION:$GVRENDER_MINOR_VERSION:$GVRENDER_MICRO_VERSION
AC_SUBST(GVRENDER_LIBRARY_VERSION)

dnl -----------------------------------------------
dnl libgvencoder name and version number
dnl -----------------------------------------------

GVENCODER_LIBRARY_NAME=libgvencoder
AC_SUBST(GVENCODER_LIBRARY_NAME)
GVENCODER_LD_NAME=gvencoder
AC_SUBST(GVENCODER_LD_NAME)

GVENCODER_MAJOR_VERSION=0
GVENCODER_MINOR_VERSION=1
GVENCODER_MICRO_VERSION=0

#API version (SONAME)
GVENCODER_API_VERSION=$GVENCODER_MAJOR_VERSION.$GVENCODER_MINOR_VERSION
AC_SUBST(GVENCODER_API_VERSION)

#shared library versioning
GVENCODER_LIBRARY_VERSION=$GVENCODER_MAJOR_VERSION:$GVENCODER_MINOR_VERSION:$GVENCODER_MICRO_VERSION
AC_SUBST(GVENCODER_LIBRARY_VERSION)

dnl -----------------------------------------------
dnl gvideo command line app
dnl -----------------------------------------------

GVIDEO_GTK_NAME=gvideo
AC_SUBST(GVIDEO_NAME)

GVIDEO_MAJOR_VERSION=0
GVIDEO_MINOR_VERSION=1
GVIDEO_MICRO_VERSION=0

# app version
GVIDEO_VERSION=$GVIDEO_MAJOR_VERSION.$GVIDEO_MINOR_VERSION.$GVIDEO_MICRO_VERSION
AC_SUBST(GVIDEO_VERSION)

dnl -----------------------------------------------
dnl gvideo-gtk ui
dnl -----------------------------------------------

GVIDEO_GTK_NAME=gvideo-gtk
AC_SUBST(GVIDEO_GTK_NAME)

GVIDEO_GTK_MAJOR_VERSION=0
GVIDEO_GTK_MINOR_VERSION=1
GVIDEO_GTK_MICRO_VERSION=0

# app version
GVIDEO_GTK_VERSION=$GVIDEO_GTK_MAJOR_VERSION.$GVIDEO_GTK_MINOR_VERSION.$GVIDEO_GTK_MICRO_VERSION
AC_SUBST(GVIDEO_GTK_VERSION)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AM_SANITY_CHECK

AC_LANG_CPLUSPLUS

AC_PREFIX_DEFAULT(/usr/local)
AC_SUBST(CFLAGS)


dnl --------------------------------------------------------------------------
dnl Set gettext package name
dnl --------------------------------------------------------------------------
GETTEXT_GVIDEO = $PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [$PACKAGE])

dnl --------------------------------------------------------------------------
dnl Supported Languages
dnl --------------------------------------------------------------------------
ALL_LINGUAS="pt pt_BR de es fr da tr cs ja ru nl he eu"
AM_GLIB_GNU_GETTEXT([external])

dnl --------------------------------------------------------------------------
dnl check for libgvideo dependencies
dnl --------------------------------------------------------------------------

PKG_CHECK_MODULES(GVIDEOLIB, [libv4l2 libpng])
AC_SUBST(GVIDEOLIB_CFLAGS)
AC_SUBST(GVIDEOLIB_LIBS)

dnl --------------------------------------------------------------------------
dnl check for libgvaudio dependencies
dnl --------------------------------------------------------------------------

PKG_CHECK_MODULES(GVAUDIOLIB, [portaudio-2.0])
AC_SUBST(GVAUDIOLIB_CFLAGS)
AC_SUBST(GVAUDIOLIB_LIBS)

dnl -----------------------
dnl Check for pulseaudio
dnl -----------------------
AC_MSG_CHECKING(if you want to enable pulse support)
AC_ARG_ENABLE(pulse, AS_HELP_STRING([--enable-pulse],
		[enable pulseaudio support (default: disabled)]),
	[enable_pulse=yes],
	[enable_pulse=no])

AC_MSG_RESULT($enable_pulse)

if test $enable_pulse = yes; then
	PKG_CHECK_MODULES(PULSE, libpulse-simple >= 0.9.15, has_pulse=yes, has_pulse=no)
	AC_SUBST(PULSE_CFLAGS)
	AC_SUBST(PULSE_LIBS)
	if test "$has_pulse" = yes; then
	  AC_DEFINE(PULSEAUDIO,1,[set to 1 if pulseaudio installed])
	else 
	  AC_MSG_WARN(libpulse missing... pulse support will be disabled.)
	fi
fi

dnl --------------------------------------------------------------------------
dnl check for libgvrender dependencies
dnl --------------------------------------------------------------------------

AC_MSG_CHECKING(if you want to disable gvideo libgvrender support)
AC_ARG_ENABLE(gvrender, AS_HELP_STRING([--disable-libgvrender],
        [disable gvideo libgvrender support (default: enabled)]),
    [enable_libgvrender=no],
    [enable_libgvrender=yes])

AC_MSG_RESULT($enable_libgvrender)
if test $enable_libgvrender = yes; then
    PKG_CHECK_MODULES(GVRENDERLIB, [sdl >= 1.2.10])
    AC_SUBST(GVRENDERLIB_CFLAGS)
    AC_SUBST(GVRENDERLIB_LIBS)
fi

dnl --------------------------------------------------------------------------
dnl check for libgvencoder dependencies
dnl --------------------------------------------------------------------------

AC_MSG_CHECKING(if you want to disable gvideo libgvencoder support)
AC_ARG_ENABLE(gvencoder, AS_HELP_STRING([--disable-libgvencoder],
        [disable gvideo libgvencoder support (default: enabled)]),
    [enable_libgvencoder=no],
    [enable_libgvencoder=yes])

AC_MSG_RESULT($enable_libgvencoder)
if test $enable_libgvencoder = yes; then
    PKG_CHECK_MODULES(GVENCODERLIB, [libavcodec])
    AC_SUBST(GVENCODERLIB_CFLAGS)
    AC_SUBST(GVENCODERLIB_LIBS)
fi

dnl --------------------------------------
dnl Check for avcodec.h installation path
dnl --------------------------------------

dnl Check for avcodec.h directly
dnl if all tests fail must add avcodec.h path to C_INCLUDE_PATH before running configure
AC_CHECK_HEADER([avcodec.h], [has_avcodec_h="yes"])

dnl Check old install path for avcodec.h (ffmpeg/avcodec.h)
AC_CHECK_HEADER([ffmpeg/avcodec.h], [has_ffmpeg_dir="yes"])

dnl Check new install path for avcodec.h (libavcodec/avcodec.h)
dnl if all the checks fail we include "libavcodec/avcodec.h"
AC_CHECK_HEADER([libavcodec/avcodec.h], [has_libavcodec_dir="yes"])

if test "$has_avcodec_h" = yes; then
  AC_DEFINE(HAS_AVCODEC_H,1,[Define if avcodec.h can be acessed directly]) 
fi
if test "$has_ffmpeg_dir" = yes; then
  AC_DEFINE(HAS_FFMPEG_AVCODEC_H,1,[Define if avcodec.h installed to ffmpeg dir]) 
fi
if test "$has_libavcodec_dir" = yes; then
  AC_DEFINE(HAS_LIBAVCODEC_AVCODEC_H,1,[Define if avcodec.h installed to libavcodec dir])
fi

dnl --------------------------------------------------------------------------
dnl check for gvideo dependencies
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to disable gvideo command line app)
AC_ARG_ENABLE(cmd, AS_HELP_STRING([--disable-cmd],
        [disable command line app build (default: enabled)]),
    [enable_cmd=no],
    [enable_cmd=yes])

AC_MSG_RESULT($enable_cmd)
dnl if test $enable_cmd = yes; then
    dnl PKG_CHECK_MODULES(GVIDEO, )
    dnl AC_SUBST(GVIDEO_CFLAGS)
    dnl AC_SUBST(GVIDEO_LIBS)
dnl fi

dnl --------------------------------------------------------------------------
dnl check for gvideo-gtk dependencies
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to disable gvideo-gtk support)
AC_ARG_ENABLE(gtk, AS_HELP_STRING([--disable-gtk],
        [disable gtk support (default: enabled)]),
    [enable_gtk=no],
    [enable_gtk=yes])

AC_MSG_RESULT($enable_gtk)
if test $enable_gtk = yes; then
    PKG_CHECK_MODULES(GVIDEOGTK, [gtkmm-2.4  glibmm-2.4 gthread-2.0])
    AC_SUBST(GVIDEOGTK_CFLAGS)
    AC_SUBST(GVIDEOGTK_LIBS)
fi

dnl --------------------------------------------------------------------------
dnl set/unset debian menu
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to enable debian menu)
AC_ARG_ENABLE(debian-menu, AS_HELP_STRING([--disable-debian-menu],
        [disable debian menu (default: enabled)]),
    [enable_debian_menu=no],
    [enable_debian_menu=yes])

AC_MSG_RESULT($enable_debian_menu)

AM_CONDITIONAL(DEBIAN_MENU, test "$enable_debian_menu" = yes)

dnl --------------------------------------------------------------------------
dnl set/unset desktop file
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to enable desktop file)
AC_ARG_ENABLE(desktop, AS_HELP_STRING([--disable-desktop],
		[disable desktop file (default: enabled)]),
	[enable_desktop=no],
	[enable_desktop=yes])

AC_MSG_RESULT($enable_desktop)

AM_CONDITIONAL(DESKTOP, test "$enable_desktop" = yes)

dnl --------------------------------------------------------------------------
dnl Check if the processor stores words with the most significant byte first 
dnl (like Motorola and SPARC, unlike Intel and VAX).
dnl --------------------------------------------------------------------------

AC_C_BIGENDIAN 

dnl -----------------------------------------------
dnl Generates Makefile's, configuration files and scripts
dnl -----------------------------------------------

AC_OUTPUT(Makefile \
          pkgconfig/gvideo-0.1.pc \
          pkgconfig/gvrender-0.1.pc \
          pkgconfig/gvaudio-0.1.pc \
          libgvideo/Makefile \
          libgvrender/Makefile \
          libgvaudio/Makefile \
          libgvencoder/Makefile \
          gvideo/Makefile \
          gvideo-gtk/Makefile \
          common/Makefile \
          pixmaps/Makefile \
          po/Makefile.in
)
