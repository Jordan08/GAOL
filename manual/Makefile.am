# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
# Copyright (c) 2026 ENSTA, France
#
# By: Frederic Goualard, Jordan NININ
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

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

# The archive of the sources, make dist (GAOL v5): automake takes the sources,
# the headers and the scripts it knows from the variables of the Makefile.am,
# and EXTRA_DIST names the other files of the repository, here the sources of the manuals, v4 and v5, and the PDF files.
# make dist, package_source of CMake and meson dist hold the same files, those
# of the repository: a file added to the repository is added to this list too,
# or the job Archives
# of the sources of the CI, which compares the three archives, fails
EXTRA_DIST = \
	Manual_GAOLv5.pdf \
	build-pdf.sh \
	check_examples.py \
	meson.build \
	v4/Manual_GAOLv4.pdf \
	v4/gaol.bib \
	v4/gaol.tex \
	v4/include/lgpl.texi \
	v4/lgpl.tex \
	v4/manual.cls \
	v4/meson.build \
	v4/pdfcolor.tex \
	v4/pdftex.cfg \
	v4/relation-cos.pdf \
	v4/relation-cos.tex \
	v4/texinfo.tex \
	v5/chapters/01_interval_arithmetic.tex \
	v5/chapters/02_installation.tex \
	v5/chapters/03_using.tex \
	v5/chapters/04_rounding.tex \
	v5/chapters/05_creation.tex \
	v5/chapters/06_relations.tex \
	v5/chapters/07_arithmetic.tex \
	v5/chapters/08_functions.tex \
	v5/chapters/09_io.tex \
	v5/chapters/10_ieee1788.tex \
	v5/chapters/11_accuracy.tex \
	v5/chapters/12_computing.tex \
	v5/chapters/13_numbers_errors.tex \
	v5/chapters/14_tests_ci.tex \
	v5/chapters/15_migration.tex \
	v5/chapters/abstract.tex \
	v5/chapters/appendix_copying.tex \
	v5/chapters/appendix_documentation.tex \
	v5/chapters/conclusion.tex \
	v5/chapters/contributors.tex \
	v5/chapters/copyright.tex \
	v5/chapters/introduction.tex \
	v5/chapters/titlepage.tex \
	v5/gaol.bib \
	v5/gaol_def.tex \
	v5/lgpl.tex \
	v5/manual_gaolv5.tex \
	v5/meson.build \
	v5/relation-cos.pdf \
	v5/relation-cos.tex
