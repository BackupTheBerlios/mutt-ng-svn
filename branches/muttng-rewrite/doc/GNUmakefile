LANGUAGES=en de_DE

all: prepare doxygen

prepare: common/manual-full.xml common/manual-user.xml
	@ for l in $(LANGUAGES) ; do \
	  cp $^ $$l/ ; \
	done
	@ for l in $(LANGUAGES) homepage ; do \
	  cp css/muttng.css $$l/ ; \
	done

manual:
	@ for l in $(LANGUAGES) ; do \
	  echo "xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual-user.xml" ; \
	  xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual-user.xml ; \
	  echo "xsltproc -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml" ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/ xsl/docbook2chunkhtml.xsl $$l/manual-docbook.xml ; \
	  tidy -q -i -m -utf8 $$l/*.html >/dev/null 2>&1 || true ; \
        done

doxygen:
	xsltproc --xinclude -o ../src/related/manual.h xsl/manual2doxygen.xsl en/manual-full.xml

.PHONY: homepage

homepage:
	xsltproc --xinclude -o homepage/homepage-docbook.xml xsl/manual2docbook.xsl homepage/homepage.xml ; \
	xsltproc --nonet --stringparam html.stylesheet muttng.css -o homepage/index.html xsl/docbook2html.xsl homepage/homepage-docbook.xml

test:
	@ true
