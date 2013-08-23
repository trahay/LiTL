if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_srcdir)/docs/man
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
