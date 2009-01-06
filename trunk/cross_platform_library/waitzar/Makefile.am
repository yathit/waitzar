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

##Trying to figure out package-name-doesnt-match-sonames error:
#libwaitzar_1_0_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)
libwaitzar_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)

