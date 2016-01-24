bin_PROGRAMS = scrap

# MDM Do I want -mt- here or not?  Pretty sure I do...
# it's possible to do something like this:
#   AM_LDFLAGS = -lboost_system$(BOOST_LIB_SUFFIX)
# Note that gentoo sets up this link for us, yay:
#  /usr/lib/libboost_regex-mt.so -> libboost_regex-mt-1_46.so
scrap_LDADD = /usr/lib/libboost_regex-mt.so

# sharethedj_SOURCES = main.cpp ../../Reusable/BitSet.cpp
scrap_SOURCES = main.cpp
