CXXFLAGS+=-I..
LDFLAGS+=-L.. -lcore -lunit++
LINKLIBS=test.o buffer_tests.o hash_tests.o

include $(CURDIR)/../../../GNUmakefile.config.default

all: 
	@echo "run 'make test' in order to compile the tests"

test: $(LINKLIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	./$@ -v

clean:
	$(RM) test *.o

include $(CURDIR)/../../../GNUmakefile.subdirs
