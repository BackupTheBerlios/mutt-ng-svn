SUBDIRS=mailbox crypto message cache test util transport

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

all: muttng_signal.h subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o libmuttng.o

libmuttng.a: debug.o libmuttng.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o) $(wildcard ./crypto/*.o) \
	  $(wildcard ./message/*.o) $(wildcard ./cache/*.o) $(wildcard ./util/*.o) \
	  $(wildcard ./transport/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

muttng_signal.h: signal.pl
	./signal.pl > ./muttng_signal.h

test: libmuttng.a subdirtest

depend: subdirdepend

include ./../../GNUmakefile.subdirs
