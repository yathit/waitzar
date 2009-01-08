## Source directory

h_sources = \
	    fontconv.h \
            fontmap.h \
            lib.h \
            SentenceList.h \
            regex.h \
            WordBuilder.h

cc_sources = \
            fontconv.cpp \
            fontmap.cpp \
            lib.cpp \
            regex.cpp \
            WordBuilder.cpp \
            SentenceList.cpp 

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES=libwaitzar.la
libwaitzar_la_SOURCES= $(h_sources) $(cc_sources)

EXTRA_DIST = wz.ver


#Version symbol names
libwaitzar_la_LDFLAGS = \
	-export-dynamic \
	-version-info $(GENERIC_LIBRARY_VERSION) \
	-Wl,--version-script,$(top_srcdir)/$(GENERIC_LIBRARY_NAME)/wz.ver -Wl,-O1


