SUBDIRS=icons data
include $(abs_top_builddir)/src/rules.mk

LOCAL_CFLAGS=-DAPP_DATA_DIR="\"$(datadir)/ftk/bluetooth\"" -DAPP_LOCAL_DATA_DIR="\"$(srcdir)\""\
	-I$(abs_top_srcdir)/apps/common

lib_LTLIBRARIES=libapp-bluetooth.la
libapp_bluetooth_la_SOURCES=main.c ftk_app_bluetooth.c
libapp_bluetooth_la_LDFLAGS=$(MYLDFLAGS)
libapp_bluetooth_la_CFLAGS=$(LOCAL_CFLAGS)

noinst_PROGRAMS=bluetooth
bluetooth_SOURCES=main.c ftk_app_bluetooth.c
bluetooth_LDFLAGS=$(MYLDFLAGS)
bluetooth_CFLAGS=$(LOCAL_CFLAGS) -DHAS_MAIN

EXTRA_DIST=ftk_app_bluetooth.h
