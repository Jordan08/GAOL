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
# By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
#--------------------------------------------------------------------------

.PHONY: pdf



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

CLEANFILES = $(LATEX_BUILT)
DISTCLEANFILES = $(LATEX_BUILT)

# No html target: the documentation Doxygen wrote from the sources
# (gaol_doxygen.cfg.in) is gone, as the Doxyfile of the root (GAOL v5)
all: pdf


# v5/gaol.pdf and v4/gaol.pdf
pdf:
if PDF_TO_LATEX
	PDFLATEX="$(PDFLATEX)" BIBTEX="$(BIBTEX)" MAKEINDEX="$(MAKEINDEX)" sh $(srcdir)/build-pdf.sh $(srcdir)/v5 v5 $(srcdir)/v4 v4
else
	@echo Sorry, no tool to compile a LaTeX file into a pdf file found
endif
