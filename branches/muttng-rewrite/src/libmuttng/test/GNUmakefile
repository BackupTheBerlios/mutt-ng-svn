CXXFLAGS+=-I.. -I../..
LDFLAGS+=-L.. -lunit++
LINKLIBS=test.o signal_tests.o url_tests.o ../libmuttng.a  ../../core/libcore.a

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

all: 
	@echo "run '$(MAKE) test' in order to compile the tests"

test: run_test
	./$^ -v

run_test: $(LINKLIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) run_test $(LINKLIBS)

depend:
	@true

include $(CURDIR)/../../../GNUmakefile.subdirs
