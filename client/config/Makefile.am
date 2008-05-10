AUTOMAKE_OPTIONS = foreign

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

noinst_HEADERS = config.h

kde_module_LTLIBRARIES = kwin_fitz_config.la
kwin_fitz_config_la_SOURCES = config.cpp configdialog.ui
kwin_fitz_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin_fitz_config_la_LIBADD = $(LIB_KDEUI)
kwin_fitz_config_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin_fitz_config_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_fitz_config.pot
