SUBDIRS=mailbox crypto message cache test util

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

all: signal.h subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o libmuttng.o

libmuttng.a: debug.o libmuttng.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o) $(wildcard ./crypto/*.o) \
	  $(wildcard ./message/*.o) $(wildcard ./cache/*.o) $(wildcard ./util/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

signal.h: signal.pl
	./signal.pl > ./signal.h
test: libmuttng.a subdirtest

depend: subdirdepend

include ./../../GNUmakefile.subdirs
