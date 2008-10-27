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
            SentenceList.cpp \
            ExternalModelTester.cpp

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES= libwaitzar-1.0.la
libwaitzar_1_0_la_SOURCES= $(h_sources) $(cc_sources)
libwaitzar_1_0_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

