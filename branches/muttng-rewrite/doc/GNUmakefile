LANGUAGES=en

all: vars funcs

manual:
	@ for l in $(LANGUAGES) ; do \
	  echo "xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual.xml" ; \
	  xsltproc --xinclude -o $$l/manual-docbook.xml xsl/manual2docbook.xsl $$l/manual.xml ; \
	  echo "xsltproc -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml" ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/manual.html xsl/docbook2html.xsl $$l/manual-docbook.xml ; \
	  xsltproc --nonet --stringparam html.stylesheet muttng.css -o $$l/ xsl/docbook2chunkhtml.xsl $$l/manual-docbook.xml ; \
	  cp css/muttng.css $$l/ ; \
        done

vars:
	xsltproc -o ../src/muttng/config/vars.h xsl/options2doxygen.xsl en/var_descr.xml

funcs:
	xsltproc -o ../src/muttng/event/funcs.h xsl/functions2doxygen.xsl en/func_descr.xml
