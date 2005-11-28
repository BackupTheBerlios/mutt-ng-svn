LANGS=de

all:
	@ true

clean:
	rm -rf *.gmo

srcdoc:
	@ true

test:
	@ true

depend:
	@ true

full:
	rm -rf mutt-ng.pot
	find ./../ \( -name '*.c' -or -name '*.cpp' \) > ./POTFILES
	$(MAKE) mutt-ng.pot
	$(MAKE) update
	
mutt-ng.pot:
	find ./../ \( -name '*.c' -or -name '*.cpp' \) > ./POTFILES
	xgettext --default-domain=mutt-ng --add-comments --keyword=_ --keyword=N_ \
	  --files-from=./POTFILES && \
	  test ! -f mutt-ng.po || \
	    (rm -rf mutt-ng.pot && mv mutt-ng.po mutt-ng.pot )

update:
	for cat in $(LANGS) ; do \
	  cat=`basename $$cat` ; \
	  lang=`echo $$cat | sed 's/\.gmo$$//'` ; \
	  echo "$$lang:"; \
	  if msgmerge $$lang.po mutt-ng.pot -o $$lang.new.po ; then \
	    mv -f $$lang.new.po $$lang.po ; \
	  else \
	    echo "msgmerge for $$lang $$cat failed!" ; \
	  fi ; \
	done
	$(MAKE) gmofiles

%.gmo: %.po
	rm -rf $@ && msgfmt --statistics -o $@ $<

gmofiles: GNUmakefile $(patsubst %,%.gmo,$(LANGS))
