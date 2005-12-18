include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../.. $(CXXFLAGS_UNITPP) $(CXXFLAGS_ICONV) -DLIBMUTTNG_TEST
LDFLAGS+=$(LDFLAGS_UNITPP) $(LDFLAGS_ICONV)
LINKOBJS=test.o signal_tests.o url_tests.o conn_tests.o header_tests.o rfc2047_tests.o
LINKLIBS=./../libmuttng.a ./../../core/libcore.a

all: 
	@echo "run '$(MAKE) test' in order to compile the tests"

test: run_test
	./$^ -v

run_test: $(LINKOBJS) $(LINKLIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDFLAGS_LIBICONV) $(LDFLAGS_SSL)

clean:
	$(RM) run_test $(LINKOBJS)

depend:
	@true

include $(CURDIR)/../../../GNUmakefile.subdirs
