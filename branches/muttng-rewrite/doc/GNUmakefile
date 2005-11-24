LANGUAGES=en

all: doxygen

manual:
	@ for l in $(LANGUAGES) ; do \
	  echo "xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual.xml" ; \
	  xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual.xml ; \
	  echo "xsltproc -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml" ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/ xsl/docbook2chunkhtml.xsl $$l/manual-docbook.xml ; \
	  cp css/muttng.css $$l/ ; \
	  tidy -q -i -m -utf8 $$l/*.html >/dev/null 2>&1 || true ; \
        done

doxygen:
	xsltproc --xinclude -o ../src/related/manual.h xsl/manual2doxygen.xsl en/manual.xml

test:
	@ true
