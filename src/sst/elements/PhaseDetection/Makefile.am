# -*- Makefile -*-
#
#

AM_CPPFLAGS += \
	$(MPI_CPPFLAGS)

compdir = $(pkglibdir)
comp_LTLIBRARIES = libPhaseDetection.la
libPhaseDetection_la_SOURCES = \
	WorkingSetSignature.h \
	WorkingSetSignature.cc

deprecated_libPhaseDetection_sources =

if !SST_ENABLE_PREVIEW_BUILD
libPhaseDetection_la_SOURCES += $(deprecated_libPhaseDetection_sources)
endif

EXTRA_DIST = \
    README
# TODO: Add test files to this list

deprecated_EXTRA_DIST =

if !SST_ENABLE_PREVIEW_BUILD
EXTRA_DIST += $(deprecated_EXTRA_DIST)
endif

libPhaseDetection_la_LDFLAGS = -module -avoid-version

install-exec-hook:
	$(SST_REGISTER_TOOL) SST_ELEMENT_SOURCE     PhaseDetection=$(abs_srcdir)
	$(SST_REGISTER_TOOL) SST_ELEMENT_TESTS      PhaseDetection=$(abs_srcdir)/tests




