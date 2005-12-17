include $(CURDIR)/../GNUmakefile.config.mine
include $(CURDIR)/../GNUmakefile.whereis
include $(CURDIR)/../GNUmakefile.config

ALL_LANGUAGES=en de
ifeq ($(DOCLANGUAGES),all)
BUILD_LANGUAGES=$(ALL_LANGUAGES)
else
BUILD_LANGUAGES=$(DOCLANGUAGES)
endif
TEXDIRS=$(TEXINPUTS):./../tex:./../examples:

mk_ind=touch $(1).idx && $(MAKEINDEX) -s muttng.ist -o $(1).ind $(1).idx

all: prepare

depend: doxygen

prepare: common/manual-full.xml common/manual-user.xml
	@ for l in $(ALL_LANGUAGES) ; do \
	  cp $^ tex/muttng.ist css/muttng.css manual_$$l/ ; \
	  cp css/muttng.css homepage_$$l ; \
	done

manual:
	@ for l in $(BUILD_LANGUAGES) ; do \
	  $(call doxslt_s,manual_$$l/manual-user.xml,xsl/manual2docbook.xsl,manual_$$l/manual-docbook.xml,$$l) ; \
	  $(call doxslt_m,manual_$$l/manual-docbook.xml,xsl/docbook2chunkhtml.xsl,manual_$$l/,$$l) ; \
	  $(call doxslt_s,manual_$$l/manual-docbook.xml,xsl/docbook2html.xsl,manual_$$l/manual.html,$$l) ; \
	  $(call doxslt_s,manual_$$l/manual-user.xml,xsl/manual2tex.xsl,manual_$$l/manual.tex,$$l) ; \
        done
ifneq ($(TIDY),)
	@ for l in $(BUILD_LANGUAGES) ; do \
	  $(TIDY) -q -i -m -utf8 manual_$$l/*.html >/dev/null 2>&1 || true ; \
	done
endif
ifneq ($(PDFLATEX),)
	@ for l in $(BUILD_LANGUAGES) ; do \
	  cd manual_$$l ; \
	  TEXINPUTS="$(TEXDIRS)" $(PDFLATEX) -interaction=nonstopmode -file-line-error-style manual.tex ; \
	  TEXINPUTS="$(TEXDIRS)" $(PDFLATEX) -interaction=nonstopmode -file-line-error-style manual.tex ; \
	  cd .. ; \
	done
ifneq ($(MAKEINDEX),)
	@ for l in $(BUILD_LANGUAGES) ; do \
	  cd manual_$$l ; \
	  TEXINPUTS="$(TEXDIRS)" $(call mk_ind,manual) ; \
	  cd .. ; \
	done
endif
	@ for l in $(BUILD_LANGUAGES) ; do \
	  cd manual_$$l ; \
	  TEXINPUTS="$(TEXDIRS)" $(PDFLATEX) -interaction=nonstopmode -file-line-error-style manual.tex ; \
	  cd .. ; \
	done
endif

doxygen:
	@ $(call doxslt_s,manual_en/manual-full.xml,xsl/manual2doxygen.xsl,./../src/related/manual.h,en)
	@ $(call doxslt_s,homepage_en/homepage.xml,xsl/manual2doxygen.xsl,./../src/related/mainpage.h,en)

homepage:
	@ for l in $(BUILD_LANGUAGES) ; do \
	  $(call doxslt_s,homepage_$$l/homepage.xml,xsl/manual2docbook.xsl,homepage_$$l/homepage-docbook.xml,$$l) ; \
	  $(call doxslt_s,homepage_$$l/homepage-docbook.xml,xsl/homepage2xhtml.xsl,homepage_$$l/index.html,$$l) ; \
	done
ifneq ($(TIDY),)
	@ for l in $(BUILD_LANGUAGES) ; do \
	  $(TIDY) -q -i -m -utf8 homepage_$$l/*.html >/dev/null 2>&1 || true ; \
	done
endif

clean:
	$(MAKE) -C examples clean
	@ for l in $(ALL_LANGUAGES) ; do \
	  rm -rf manual_$$l/*.html manual_$$l/*.aux manual_$$l/*.idx manual_$$l/*.lof manual_$$l/*.log manual_$$l/*.lol manual_$$l/*.out manual_$$l/*.pdf manual_$$l/*.tex manual_$$l/*.toc manual_$$l/*.ilg manual_$$l/*.ind manual_$$l/*.lot manual_$$l/*.ilg manual_$$l/manual-docbook.xml ; \
	  rm -rf homepage_$$l/*.html ; \
	done

test:
	$(MAKE) -C examples test
