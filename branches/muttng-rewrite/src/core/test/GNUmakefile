include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. $(CXXFLAGS_UNITPP)
LDFLAGS+=-L.. $(LDFLAGS_UNITPP) $(LDFLAGS_LIBICONV) $(LDFLAGS_LIBIDN)
LINKLIBS=test.o buffer_tests.o hash_tests.o list_tests.o io_tests.o \
	 conv_tests.o base64_tests.o qp_tests.o net_tests.o \
	 rx_tests.o ./../libcore.a

all: 
	@echo "run '$(MAKE) test' in order to compile the tests"

test: run_test
	./$^ -v

run_test: $(LINKLIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) run_test $(LINKLIBS)

include $(CURDIR)/../../../GNUmakefile.subdirs
