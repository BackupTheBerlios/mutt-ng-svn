include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I..
LDFLAGS+=-L.. -lcore -lunit++ $(LDFLAGS_LIBICONV)
LINKLIBS=test.o buffer_tests.o hash_tests.o list_tests.o io_tests.o conv_tests.o base64_tests.o qp_tests.o

all: 
	@echo "run '$(MAKE) test' in order to compile the tests"

test: run_test
	./$^ -v

run_test: $(LINKLIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) run_test $(LINKLIBS)

include $(CURDIR)/../../../GNUmakefile.subdirs
