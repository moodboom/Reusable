# Generated automatically from Makefile.in by configure.
# Makefile for TPIE library.
# Use GNU make on this Makefile. It relies on the default rules.
# Use (GNU) "make -p" to see the default rules.
# $Id: Makefile,v 1.1 2008/07/28 17:08:02 m Exp $

ARFLAGS = rv
CXX = c++
MAKEDEPEND = c++ -M
RANLIB = ranlib

# The directory containing the library header files.
TPIEINC = ../../include
# The name of the library.
LIBRARY = libtpie.a

# CPPFLAGS is used by the default rules. Using "override" and "+="
# allows the user to prepend things to CPPFLAGS on the command line.
override CPPFLAGS += -DTPIE_LIBRARY -I$(TPIEINC)
# These options are set by the configure script.
override CPPFLAGS += -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64

ifeq ($(MINUSG),1)
override CPPFLAGS += -g
else
override CPPFLAGS += -O3# -DNDEBUG 
endif

ifeq ($(MINUSPG),1)
override CPPFLAGS += -pg
endif

# All sources must be listed here.
SRC =	logstream.C \
	tpie_log.C \
	mm_register.C \
	ami_device.C \
	ami_stream_single.C \
	mm_base.C \
	bte_stream_base.C \
	bit.C \
	bit_matrix.C \
	ami_matrix_blocks.C \
	ami_bit_permute.C \
	wall_timer.C \
	cpu_timer.C \
	ami_key.C \
	tpie_tempnam.C

OBJ =	$(SRC:.C=.o)

LIB_COMPONENTS = $(patsubst %.C,$(LIBRARY)(%.o),$(SRC))

(%): %
	$(AR) $(ARFLAGS) $@ $<

# Public targets.

all : $(LIBRARY)

depend: cleandepend 
	$(MAKEDEPEND) $(CPPFLAGS) $(SRC) >> Makefile.depend

clean: cleandepend
	-rm -f $(OBJ) $(LIBRARY)

distclean: clean

# Private targets.

cleandepend:
	-rm -f Makefile.depend

$(LIBRARY): $(LIB_COMPONENTS)

Makefile.depend: #$(SRC)
	rm -f Makefile.depend; \
	$(MAKEDEPEND) $(CPPFLAGS) $(SRC) >> Makefile.depend

include Makefile.depend

.PHONY : clean all depend cleandepend
