#!/bin/sh
# Builds the manual, gaol.pdf, from gaol.tex:
#
#   sh manual/build-pdf.sh <directory of gaol.tex> <output directory>
#
# The output directory holds gaol_version.tex, the version, edition and date of
# the manual, which configure and meson generate from gaol_version.tex.in, and
# receives gaol.pdf. The make pdf of the autotools build and the target pdf of
# the meson build (-Dwith-doc=true) run it, and so does the continuous
# integration. Needs pdflatex, bibtex and makeindex, with the packages the
# class manual.cls loads (on Debian and Ubuntu: texlive-latex-recommended,
# texlive-latex-extra, texlive-fonts-recommended and lmodern).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
set -e
source_dir=$(cd "$1" && pwd)
mkdir -p "$2"
cd "$2"
if [ ! -f gaol_version.tex ]; then
  echo "gaol_version.tex is not in $2: configure GAOL with autotools or meson first" >&2
  exit 1
fi
# The sources are looked for in the directory of gaol.tex, after this one
export TEXINPUTS=".:$source_dir:"
export BIBINPUTS="$source_dir:"
export BSTINPUTS="$source_dir:"
latex="${PDFLATEX:-pdflatex} -interaction=nonstopmode -halt-on-error"
$latex gaol.tex > pdflatex-1.log || { tail -40 pdflatex-1.log; exit 1; }
${BIBTEX:-bibtex} gaol
${MAKEINDEX:-makeindex} gaol.idx -o gaol.ind
$latex gaol.tex > pdflatex-2.log || { tail -40 pdflatex-2.log; exit 1; }
$latex gaol.tex > pdflatex-3.log || { tail -40 pdflatex-3.log; exit 1; }
# A reference or a citation left undefined is an error
if grep -E "LaTeX Warning: (Reference|Citation) .* undefined" pdflatex-3.log; then
  exit 1
fi
grep -E "^Output written on" pdflatex-3.log
