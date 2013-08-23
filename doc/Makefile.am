if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
	$(MAKE) -C latex
clean-local:
	rm -rf $(top_srcdir)/doc/man $(builddir)/doc/html $(builddir)/doc/latex

EXTRA_DIST = html

install-data-local:
	for f in `find $(builddir)/html` ; do \
	$(INSTALL_DATA) -D $$f \
		$(DESTDIR)$(datadir)/doc/$(PACKAGE)/$$f ; done
	$(INSTALL_DATA) -D $(builddir)/latex/refman.pdf  \
		$(DESTDIR)$(datadir)/doc/$(PACKAGE)/

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/doc/$(PACKAGE)/html \
		$(DESTDIR)$(datadir)/doc/$(PACKAGE)/refman.pdf



endif


if HAVE_PDFLATEX
doc_DATA = user_manual.pdf

docfiles = user_manual.pdf

MANNAME = user_manual
MANTEXSRC = $(MANNAME).tex
MANAUX = $(MANNAME).aux
MANPDF = $(MANNAME).pdf

CLEANFILES = $(MANPDF) $(MANNAME).log $(MANNAME).idx $(MANNAME).out $(MANNAME).toc $(MANAUX)

$(MANPDF): $(srcdir)/$(MANTEXSRC)
	TEXINPUTS=$(srcdir) $(PDFLATEX) $<

dist_doc_DATA = $(docfiles)

endif
