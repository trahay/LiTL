EXTRA_DIST =

if WITHDOC

if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp latex/refman.pdf

latex/refman.pdf:
	$(MAKE) -C latex


clean-local:
	rm -rf $(top_srcdir)/doc/man latex doxyfile.stamp

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(datadir)/doc/$(PACKAGE)/
	$(INSTALL_DATA) -D $(builddir)/latex/refman.pdf  \
		$(DESTDIR)$(datadir)/doc/$(PACKAGE)/

uninstall-local:
	rm -rf	$(DESTDIR)$(datadir)/doc/$(PACKAGE)/refman.pdf

endif

if HAVE_LATEXMK
doc_DATA = user_manual.pdf

docfiles = user_manual.pdf

MANNAME = user_manual
MANTEXSRC = $(MANNAME).tex
MANAUX = $(MANNAME).aux
MANPDF = $(MANNAME).pdf

CLEANFILES = $(MANPDF) $(MANNAME).log $(MANNAME).idx $(MANNAME).out \
             $(MANNAME).toc $(MANAUX) $(MANNAME).auxlock $(MANNAME).bbl \
             $(MANNAME).blg $(MANNAME).fdb_latexmk $(MANNAME).fls header.aux

$(MANPDF): $(MANTEXSRC)
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -bibtex $<

#dist_doc_DATA = $(docfiles)

endif

endif #WITHDOC

EXTRA_DIST += $(top_srcdir)/doc/tikz/event.recording.all.tex\
              $(top_srcdir)/doc/tikz/event.storage.all.line.packed.tex\
              $(top_srcdir)/doc/tikz/event.storage.trace.file.merge.tex\
              $(top_srcdir)/doc/tikz/event.storage.trace.file.tex \
              $(top_srcdir)/doc/references.bib
