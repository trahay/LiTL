# Copyright © Télécom SudParis, CNRS, INRIA, Université Bordeaux 1
# See COPYING in top-level directory.

all: user_manual

user_manual:
	pdflatex -interaction=nonstopmode user_manual.tex

bib: 
	pdflatex -interaction=nonstopmode user_manual.tex
	bibtex user_manual.aux
	pdflatex -interaction=nonstopmode user_manual.tex
	pdflatex -interaction=nonstopmode user_manual.tex

.PHONY: clean
clean:
	rm -f ._* *.log *.aux *.blg *.bbl *.toc *.snm *.nav *.out *.backup *~ *auxlock
