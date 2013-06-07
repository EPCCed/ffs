# This Makefile produces pdfs from tex sources via pdflatex
# The gifs are produced manually.

include common.mk
include ../../../Makefile.conf

PDFLATEX = pdflatex
HTMLDIR = /Volumes/kevin/html/ffs

SRCS += react1.tex
SRCS += init1.tex
SRCS += init2a.tex
SRCS += init2b.tex

PDFS = ${SRCS:.tex=.pdf}

pdfs:	$(PDFS)

$(PDFS): %.pdf: %.tex
	$(PDFLATEX) $*

all:	pdfs

install:
	install -m 0644 *.gif ../html/.
	cp -R ../html/* $(HTMLDIR)

uninstall:
	rm -rf $(HTMLDIR)/*html

clean:
	rm -rf *aux *log *pdf
