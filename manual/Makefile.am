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

.PHONY: html pdf



PDFLATEX = @PDFLATEX@
BIBTEX = @BIBTEX@
MAKEINDEX = @MAKEINDEX@

# The manual of GAOL v5 is in v5, the manual of GAOL 4 in v4: each is built in
# the directory of the same name of the build tree, which holds its
# gaol_version.tex
LATEX_BUILT = v4/*.fux v4/*.idx v4/*.ind v4/*.fnd v4/*.aux v4/*.log v4/*.bbl \
	v4/*.blg v4/*.out v4/*.toc v4/*.ilg \
	v5/*.fux v5/*.idx v5/*.ind v5/*.fnd v5/*.aux v5/*.log v5/*.bbl \
	v5/*.blg v5/*.out v5/*.toc v5/*.ilg

CLEANFILES =  html/* $(LATEX_BUILT)
DISTCLEANFILES = html/* $(LATEX_BUILT)
MAINTAINERCLEANFILES = html/* $(LATEX_BUILT)

EXTRA_DIST = v5/gaol.bib v5/gaol.tex v5/gaol.pdf v5/manual.cls v5/lgpl.tex \
	v5/relation-cos.pdf v4/gaol.bib v4/gaol.tex v4/gaol.pdf v4/manual.cls \
	v4/lgpl.tex v4/relation-cos.pdf build-pdf.sh


all: html pdf

html: gaol_doxygen.cfg
if DOXYGEN
	doxygen ./gaol_doxygen.cfg
else
	@echo Sorry, doxygen does not seem to be present on your system
endif


# v5/gaol.pdf and v4/gaol.pdf
pdf:
if PDF_TO_LATEX
	PDFLATEX="$(PDFLATEX)" BIBTEX="$(BIBTEX)" MAKEINDEX="$(MAKEINDEX)" sh $(srcdir)/build-pdf.sh $(srcdir)/v5 v5 $(srcdir)/v4 v4
else
	@echo Sorry, no tool to compile a LaTeX file into a pdf file found
endif
