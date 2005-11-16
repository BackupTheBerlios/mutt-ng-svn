SUBDIRS=mailbox crypto message

include $(CURDIR)/../../GNUmakefile.config.default

all: subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a debug.o libmuttng.o

libmuttng.a: debug.o libmuttng.o
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o) $(wildcard ./crypto/*.o) $(wildcard ./message/*.o)
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

test:
	@echo no tests implemented yet

depend: subdirdepend

include ./../../GNUmakefile.subdirs
