include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

SUBDIRS=mailbox crypto message test util transport config
ifeq ($(WANT_CACHE),1)
SUBDIRS+=cache
endif
CXXFLAGS+=$(CXXFLAGS_SSL)

all: muttng_signal.h
	@ $(MAKE) subdirs
	@ $(MAKE) libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o libmuttng.o

libmuttng.a: debug.o libmuttng.o $(wildcard ./mailbox/*.o) $(wildcard ./crypto/*.o) $(wildcard ./message/*.o) $(wildcard ./cache/*.o) $(wildcard ./util/*.o) $(wildcard ./transport/*.o) $(wildcard ./config/*.o)
	$(AR) -cru $@ $^ 
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

muttng_signal.h: signal.pl
	./signal.pl > ./muttng_signal.h

test: libmuttng.a subdirtest

depend: subdirdepend

include ./../../GNUmakefile.subdirs
