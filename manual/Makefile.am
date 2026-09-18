#-*-Makefile-*--------------------------------------------------------------
# gaol -- Just Another Interval Library
#--------------------------------------------------------------------------
# This file is part of the gaol distribution. Gaol was primarily 
# developed at the Swiss Federal Institute of Technology, Lausanne, 
# Switzerland, and is now developed at the Institut de Recherche 
# en Informatique de Nantes, France.
#
# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
#--------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated 
# COPYING file for information.
#--------------------------------------------------------------------------
# CVS: $Id: Makefile.am,v 1.15 2008/01/22 09:07:32 goualard Exp $
# Last modified: Fri Oct  6 10:55:46 2006 on pc-goualard.lina.sciences.univ-nantes.prive
# By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
#--------------------------------------------------------------------------

.PHONY: html gaol.pdf



PDFLATEX = @PDFLATEX@
BIBTEX = @BIBTEX@
MAKEINDEX = @MAKEINDEX@

CLEANFILES =  html/* *.fux *.idx *.ind *.fnd *.aux *.log *.bbl *.blg \
	*.out *.toc *.ilg pdflatex-*.log
DISTCLEANFILES = html/* *.fux *.idx *.ind *.fnd *.aux *.log *.bbl *.blg \
	*.out *.toc *.ilg pdflatex-*.log
MAINTAINERCLEANFILES = html/* *.fux *.idx *.ind *.fnd *.aux *.log *.bbl \
	*.blg *.out *.toc *.ilg pdflatex-*.log

EXTRA_DIST = gaol.bib gaol.tex gaol.pdf manual.cls lgpl.tex \
	relation-cos.pdf build-pdf.sh


all: html pdf

html: gaol_doxygen.cfg
if DOXYGEN
	doxygen ./gaol_doxygen.cfg
else
	@echo Sorry, doxygen does not seem to be present on your system
endif


pdf: gaol.pdf

gaol.pdf:
if PDF_TO_LATEX
	PDFLATEX="$(PDFLATEX)" BIBTEX="$(BIBTEX)" MAKEINDEX="$(MAKEINDEX)" sh $(srcdir)/build-pdf.sh $(srcdir) .
else
	@echo Sorry, no tool to compile a LaTeX file into a pdf file found
endif

