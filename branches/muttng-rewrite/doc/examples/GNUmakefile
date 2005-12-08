all:
	@ $(MAKE) -f ./GNUmakefile.compile_cpp all
	@ $(MAKE) -f ./GNUmakefile.compile_c all

clean:
	@ $(MAKE) -f ./GNUmakefile.compile_cpp clean extraclean
	@ $(MAKE) -f ./GNUmakefile.compile_c clean extraclean

test:
	@ $(MAKE) -f ./GNUmakefile.compile_cpp test
	@ $(MAKE) -f ./GNUmakefile.compile_c test
