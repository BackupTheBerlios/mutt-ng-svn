include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

# tools contains binaries to build so it must be last
SUBDIRS=event ui config tools

all: muttng.o subdirs

clean: subdirclean
	rm -rf muttng.o

depend: subdirdepend

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

test:
	@echo no tests implemented yet

include $(CURDIR)/../../GNUmakefile.subdirs
