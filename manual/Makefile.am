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

# The manual of GAOL v5 is built in v5 of the build tree, which holds its
# gaol_version.tex; its PDF, Manual_GAOLv5.pdf, is written in manual/ of the
# sources, and make clean leaves it. The manual of GAOL 4 is not built
LATEX_BUILT = v5/*.fux v5/*.idx v5/*.ind v5/*.fnd v5/*.aux v5/*.log v5/*.bbl \
	v5/*.blg v5/*.out v5/*.toc v5/*.ilg

CLEANFILES = $(LATEX_BUILT)
DISTCLEANFILES = $(LATEX_BUILT)

# No html target: the documentation Doxygen wrote from the sources
# (gaol_doxygen.cfg.in) is gone, as the Doxyfile of the root (GAOL v5)
all: pdf


# The manual of GAOL v5: manual/Manual_GAOLv5.pdf of the sources (build-pdf.sh)
pdf:
if PDF_TO_LATEX
	PDFLATEX="$(PDFLATEX)" BIBTEX="$(BIBTEX)" MAKEINDEX="$(MAKEINDEX)" sh $(srcdir)/build-pdf.sh $(srcdir)/v5 v5 $(srcdir)/Manual_GAOLv5.pdf
else
	@echo Sorry, no tool to compile a LaTeX file into a pdf file found
endif
