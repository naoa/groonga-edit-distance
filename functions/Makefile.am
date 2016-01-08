AM_CFLAGS =					\
	$(GROONGA_CFLAGS)

AM_LDFLAGS =					\
	-avoid-version				\
	-module					\
	-no-undefined

LIBS =						\
	$(GROONGA_LIBS)

functions_plugins_LTLIBRARIES =
functions_plugins_LTLIBRARIES += edit_distance.la

functions_plugins_SOURCE =			\
	edit_distance.c
