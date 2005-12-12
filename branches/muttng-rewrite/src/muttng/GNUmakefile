SUBDIRS=event ui tools

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

all: muttng.o subdirs

clean: subdirclean
	rm -rf muttng.o

depend: subdirdepend

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

test:
	@echo no tests implemented yet

include $(CURDIR)/../../GNUmakefile.subdirs
