SUBDIRS=mailbox crypto message cache

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

all: subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o libmuttng.o

libmuttng.a: debug.o libmuttng.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o) $(wildcard ./crypto/*.o) \
	  $(wildcard ./message/*.o) $(wildcard ./cache/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

test:
	@echo no tests implemented yet

depend: subdirdepend

include ./../../GNUmakefile.subdirs
