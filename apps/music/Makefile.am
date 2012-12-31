SUBDIRS=icons data
include $(abs_top_builddir)/src/rules.mk

LOCAL_CFLAGS=-DAPP_DATA_DIR="\"$(datadir)/ftk/music\"" -DAPP_LOCAL_DATA_DIR="\"$(srcdir)\"" \
	-I$(abs_top_srcdir)/apps/common

lib_LTLIBRARIES=libapp-music.la
libapp_music_la_SOURCES=ftk_app_music.c
libapp_music_la_LDFLAGS=$(MYLDFLAGS)
libapp_music_la_CFLAGS=$(LOCAL_CFLAGS)

noinst_PROGRAMS=music
music_SOURCES=main.c ftk_app_music.c
music_LDFLAGS=$(MYLDFLAGS)
music_CFLAGS=$(LOCAL_CFLAGS) -DHAS_MAIN

EXTRA_DIST=ftk_app_music.h
