# Generated automatically from Makefile.in by configure.
# Makefile for the TPIE library.
# $Id: Makefile,v 1.1 2008/07/28 17:08:02 m Exp $

LIBRARY = libtpie.a
LIBSRCPATH = ./src/
RANLIB = ranlib

export

# Public targets.

all: $(LIBRARY)

clean:
	rm -f $(LIBSRCPATH)/Makefile.depend; \
	touch $(LIBSRCPATH)/Makefile.depend
	$(MAKE) -C $(LIBSRCPATH) $@

distclean: 
	-rm -f $(LIBRARY)
	$(MAKE) -C $(LIBSRCPATH) $@

# Private targets.

$(LIBRARY): $(LIBSRCPATH)$(LIBRARY)
	cp $< $@
	$(RANLIB) $@

.PHONY : clean all
