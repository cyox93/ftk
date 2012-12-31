SUBDIRS=icons data
include $(abs_top_builddir)/src/rules.mk

LOCAL_CFLAGS=-DAPP_DATA_DIR="\"$(datadir)/ftk/movie\"" -DAPP_LOCAL_DATA_DIR="\"$(srcdir)\"" \
	-I$(abs_top_srcdir)/apps/common

lib_LTLIBRARIES=libapp-movie.la
libapp_movie_la_SOURCES=ftk_app_movie.c
libapp_movie_la_LDFLAGS=$(MYLDFLAGS)
libapp_movie_la_CFLAGS=$(LOCAL_CFLAGS)

noinst_PROGRAMS=movie
movie_SOURCES=main.c ftk_app_movie.c
movie_LDFLAGS=$(MYLDFLAGS)
movie_CFLAGS=$(LOCAL_CFLAGS) -DHAS_MAIN

EXTRA_DIST=ftk_app_movie.h
