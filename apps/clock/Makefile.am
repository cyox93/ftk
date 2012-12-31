SUBDIRS=icons data
include $(abs_top_builddir)/src/rules.mk

LOCAL_CFLAGS=-DAPP_DATA_DIR="\"$(datadir)/ftk/clock\"" -DAPP_LOCAL_DATA_DIR="\"$(srcdir)\""\
	-I$(abs_top_srcdir)/apps/common

lib_LTLIBRARIES=libapp-clock.la
libapp_clock_la_SOURCES=main.c ftk_app_clock.c
libapp_clock_la_LDFLAGS=$(MYLDFLAGS)
libapp_clock_la_CFLAGS=$(LOCAL_CFLAGS)

noinst_PROGRAMS=clock
clock_SOURCES=main.c ftk_app_clock.c
clock_LDFLAGS=$(MYLDFLAGS)
clock_CFLAGS=$(LOCAL_CFLAGS) -DHAS_MAIN

EXTRA_DIST=ftk_app_clock.h
