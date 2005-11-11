SUBDIRS=mailbox

include $(CURDIR)/../../GNUmakefile.config.default

all: subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o

libmuttng.a: debug.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

depend: subdirdepend

include ./../../GNUmakefile.subdirs
