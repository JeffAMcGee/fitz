AUTOMAKE_OPTIONS = foreign

#don't make the kcontrol module - At the moment, Fitz ignores its settings
#SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = fitz.desktop

noinst_HEADERS = button.h client.h client.h fitz.h bar.h factory.h \
	buttoncache.h

kde_module_LTLIBRARIES = kwin3_fitz.la
kwin3_fitz_la_SOURCES = client.cpp button.cpp factory.cpp bar.cpp \
		buttoncache.cpp fakemouse.cpp
kwin3_fitz_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_fitz_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN) -lkdecore -lXtst
kwin3_fitz_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_fitz_la_METASOURCES)
