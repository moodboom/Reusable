# $Id: Makefile,v 1.1 2008/07/28 17:08:02 m Exp $
#
# Makefile for TPIE test applications.
#
# Use gmake on this Makefile.  It relies on the default rules.
# Use gmake -p to see the default rules.

#SHELL = /bin/csh
CXX = c++
TPIEDIR = ..
MESADIR = /usr/pkg/Mesa

MINUSPG = 0

LIBPATH = -L$(TPIEDIR)/lib
LINKLIBS = -ltpie -lm
INCLUDEPATH  = -I$(TPIEDIR)/include/

LDLIBS = $(LIBPATH) $(LINKLIBS)
MAKEDEPEND = g++ -M

override CPPFLAGS += $(INCLUDEPATH)

ifeq ($(MINUSG),1)
override CPPFLAGS += -g
else
override CPPFLAGS += -O3  -DNDEBUG
endif

ifeq ($(MINUSPG),1)
override CPPFLAGS += -pg 
endif

# The various test applications.
APPS = buildtree queryrtree

# Sources to the test applications.
APPSRC = $(patsubst %,%.C,$(APPS))

# Other C++ sources for common support routines.
COMMONCXXSRC = hilbert.C hilbert4.C

CXXSRC = $(APPSRC) $(COMMONCXXSRC)

# Common objects that go into most apps.
COMMONOBJ = $(COMMONCXXSRC:.C=.o)

# Translation rules:
%:%.o
	$(LINK.cc) $^ $(LDLIBS) -o $@

# Public targets.

all: $(APPS)

# What objects each of the apps acually need.
testrtree: testrtree.o
buildtree: buildtree.o hilbert.o hilbert4.o
ascii2stream: ascii2stream.o
mbr: mbr.o
queryrtree: queryrtree.o

depend: cleandepend
	$(MAKEDEPEND) $(CPPFLAGS) $(CXXSRC) >> Makefile.depend

clean: cleandepend
	-rm -f $(CSRC:.c=.o) $(CXXSRC:.C=.o) $(APPS)

distclean: clean
	-rm -f *~ *.bak

backup:
	(set dn = `basename $$cwd `; cd ..; \
	tar cvf $$dn.tar $$dn/*.[CHch] $$dn/Makefile; gzip -f $$dn.tar)

# Private targets.

include Makefile.depend

cleandepend:
	-rm -f Makefile.depend

Makefile.depend:
	rm -f Makefile.depend; \
	$(MAKEDEPEND) $(CPPFLAGS) $(CXXSRC) >> Makefile.depend

.PHONY: clean depend cleandepend backup distclean all
