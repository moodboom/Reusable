# Generated automatically from Makefile.in by configure.
# Makefile for TPIE test applications.
# Use gmake on this Makefile.  It relies on the default rules, and 
# Use gmake -p to see the default rules.
# $Id: Makefile,v 1.1 2008/07/28 17:08:01 m Exp $

CXX = c++
CC = gcc
MAKEDEPEND = c++ -M

TPIEINC = ../include
TPIELIBDIR = ../lib
TPIELIB = $(TPIELIBDIR)/libtpie.a

ifdef DEBUG_APPS
LOG_APPS = 1
ASSERT_DEBUG_APPS = 1
endif

CXXFLAGS = 

# CPPFLAGS is used by the default rules. Using "override" and "+="
# allows the user to prepend things to CPPFLAGS on the command line.
override CPPFLAGS +=  -I$(TPIEINC) -I./
# These options are set by the configure script.
override CPPFLAGS += -DHAVE_CONFIG_H

ifeq ($(MINUSG),1)
override CPPFLAGS += -g
else
override CPPFLAGS += -O3 -DNDEBUG
endif

ifeq ($(MINUSPG),1)
override CPPFLAGS += -pg
endif

LDLIBS =  -L$(TPIELIBDIR) -ltpie -lm #-liberty

CCFLAGS = $(CXXFLAGS)

# The various test applications.

APPS := build_PRtree query_PRtree  build_KDRtree query_KDRtree build_TGStree query_TGStree

# Sources to the test applications.
APPSRC = $(patsubst %,%.C,$(APPS))

# Other C++ sources for common support routines.
COMMONCXXSRC = 

OTHERCXXSRC = 

CXXSRC = $(APPSRC) $(COMMONCXXSRC) $(OTHERCXXSRC)

# Common objects that go into most apps.
COMMONOBJ = $(COMMONCXXSRC:.C=.o)

# C sources.
CSRC = 

# Translation rules:

%:%.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@


# Specific dependencies, explicit and automatic.

all: $(APPS)

depend: cleandepend 
	$(MAKEDEPEND) $(CPPFLAGS) $(CSRC) $(CXXSRC) >> Makefile.depend

clean: cleandepend
	-rm -f $(CSRC:.c=.o) $(CXXSRC:.C=.o) $(APPS)

distclean: clean


# What objects each of the apps acually needs.

build_semiRtree: build_semiRtree.o
query_semiRtree: query_semiRtree.o

ptest_ami_kdtree: ptest_ami_kdtree.o app_params.o

include Makefile.depend

Makefile.depend: #$(CSRC) $(CXXSRC)
	rm -f Makefile.depend; \
	$(MAKEDEPEND) $(CPPFLAGS) $(CSRC) $(CXXSRC) >> Makefile.depend

cleandepend:
	-rm -f Makefile.depend

.PHONY : clean all depend cleandepend distclean


