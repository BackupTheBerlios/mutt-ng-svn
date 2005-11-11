SUBDIRS=mailbox

include $(CURDIR)/../../GNUmakefile.config.default

all: subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a abstract_class.o

libmuttng.a: abstract_class.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

depend: subdirdepend

include ./../../GNUmakefile.subdirs
