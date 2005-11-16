SUBDIRS=event ui config tools

include $(CURDIR)/../../GNUmakefile.config.default

all: muttng.o subdirs

clean: subdirclean
	rm -rf muttng.o

depend: subdirdepend

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

test:
	@echo no tests implemented yet

include ./../../GNUmakefile.subdirs
