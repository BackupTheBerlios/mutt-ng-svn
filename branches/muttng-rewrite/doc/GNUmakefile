LANGUAGES=en de

all: prepare doxygen

prepare: common/manual-full.xml common/manual-user.xml
	@ for l in $(LANGUAGES) ; do \
	  cp $^ manual_$$l/ ; \
	  cp css/muttng.css homepage_$$l ; \
	done

manual:
	@ for l in $(LANGUAGES) ; do \
	  xsltproc --xinclude -o manual_$$l/manual-docbook.xml xsl/manual2docbook.xsl manual_$$l/manual-user.xml ; \
	  xsltproc --nonet --stringparam l10n.gentext.default.language $$l --stringparam html.stylesheet muttng.css -o manual_$$l/manual.html xsl/docbook2html.xsl manual_$$l/manual-docbook.xml ; \
	  xsltproc --nonet --stringparam l10n.gentext.default.language $$l --stringparam html.stylesheet muttng.css -o manual_$$l/ xsl/docbook2chunkhtml.xsl manual_$$l/manual-docbook.xml ; \
	  tidy -q -i -m -utf8 manual_$$l/*.html >/dev/null 2>&1 || true ; \
        done

doxygen:
	xsltproc --xinclude -o ../src/related/manual.h xsl/manual2doxygen.xsl en/manual-full.xml
	xsltproc --xinclude -o ../src/related/mainpage.h xsl/manual2doxygen.xsl homepage_en/homepage.xml

homepage:
	@ for l in $(LANGUAGES) ; do \
	  xsltproc --xinclude -o homepage_$$l/homepage-docbook.xml xsl/manual2docbook.xsl homepage_$$l/homepage.xml ; \
	  xsltproc --nonet --stringparam l10n.gentext.default.language $$l --stringparam html.stylesheet muttng.css -o homepage_$$l/index.html xsl/docbook2html.xsl homepage_$$l/homepage-docbook.xml ; \
	  tidy -q -i -m -utf8 homepage_$$l/*.html >/dev/null 2>&1 || true ; \
	done

test:
	@ true
