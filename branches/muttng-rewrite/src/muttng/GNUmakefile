SUBDIRS=ui config tools

include $(CURDIR)/../../GNUmakefile.config.default

all: muttng.o subdirs

clean: subdirclean

depend: subdirdepend

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I.. -o $@ -c $<

include ./../../GNUmakefile.subdirs
