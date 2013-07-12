# This Makefile produces pdfs from tex sources via pdflatex
# The gifs are produced manually.

include common.mk
include ../../../Makefile.conf

PDFLATEX = pdflatex

SRCS += react1.tex
SRCS += react2.tex
SRCS += react3.tex
SRCS += init1.tex
SRCS += init2a.tex
SRCS += init2b.tex
SRCS += init3.tex
SRCS += library3.tex
SRCS += rosenbluth1.tex
SRCS += what1.tex what2.tex

PDFS = ${SRCS:.tex=.pdf}

pdfs:	$(PDFS)

$(PDFS): %.pdf: %.tex
	$(PDFLATEX) $*

all:	pdfs

clean:
	rm -rf *aux *log *pdf
