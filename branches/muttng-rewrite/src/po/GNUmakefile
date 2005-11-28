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

include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config

mutt-ng.pot:
	find ./../ \( -name '*.c' -or -name '*.cpp' \) > ./POTFILES
ifneq ($(XGETTEXT),)
	$(XGETTEXT) --default-domain=mutt-ng --add-comments --keyword=_ --keyword=N_ \
	  --files-from=./POTFILES && \
	  test ! -f mutt-ng.po || \
	    (rm -rf mutt-ng.pot && mv mutt-ng.po mutt-ng.pot )
else
$(warning "xgettext(1) not found")
endif

update:
ifneq ($(MSGMERGE),)
	for cat in $(LANGS) ; do \
	  cat=`basename $$cat` ; \
	  lang=`echo $$cat | sed 's/\.gmo$$//'` ; \
	  echo "$$lang:"; \
	  if $(MSGMERGE) $$lang.po mutt-ng.pot -o $$lang.new.po ; then \
	    mv -f $$lang.new.po $$lang.po ; \
	  else \
	    echo "$(MSGMERGE) for $$lang $$cat failed!" ; \
	  fi ; \
	done
	$(MAKE) gmofiles
else
$(warning "msgmerge(1) not found")
endif

%.gmo: %.po
ifneq ($(MSGFMT),)
	rm -rf $@ && $(MSGFMT) --statistics -o $@ $<
else
$(warning "msgfmt(1) not found")
endif

gmofiles: GNUmakefile $(patsubst %,%.gmo,$(LANGS))
