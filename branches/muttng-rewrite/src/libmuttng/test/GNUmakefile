CXXFLAGS+=-I.. -I../..
LDFLAGS+=-L.. -lunit++
LINKOBJS=test.o signal_tests.o url_tests.o conn_tests.o header_tests.o
LINKLIBS=./../libmuttng.a ./../../core/libcore.a

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

all: 
	@echo "run '$(MAKE) test' in order to compile the tests"

test: run_test
	./$^ -v

run_test: $(LINKOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKLIBS) $(LDFLAGS)

clean:
	$(RM) run_test $(LINKOBJS)

depend:
	@true

include $(CURDIR)/../../../GNUmakefile.subdirs
