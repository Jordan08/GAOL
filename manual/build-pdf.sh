#!/bin/sh
# Builds a manual, gaol.pdf, from its main file, or several manuals:
#
#   sh manual/build-pdf.sh <directory of the main file> <output directory> [...]
#
# the arguments going by pairs: manual/v5 is the manual of GAOL v5, whose main
# file is manual_gaolv5.tex (the layout, which \input's the file of each
# chapter from chapters/), manual/v4 the manual of GAOL 4, kept as it was, whose
# main file is gaol.tex. Each manual gives gaol.pdf (pdflatex -jobname=gaol), so
# that the auxiliary files are named gaol.* for both. Each output directory holds
# gaol_version.tex, the version, edition and date of its manual, which
# configure, CMake and meson generate from the gaol_version.tex.in of the manual,
# and receives gaol.pdf. The make pdf of the autotools build, the target pdf of
# the CMake build (-DWITH_DOC=ON) and the one of the meson build
# (-Dwith-doc=true) run it on both manuals, and so does the continuous
# integration. Needs pdflatex, bibtex and makeindex, with the
# packages that v5/gaol_def.tex (the manual of GAOL v5) and the class
# v4/manual.cls (the manual of GAOL 4) load (on Debian and Ubuntu:
# texlive-latex-recommended, texlive-latex-extra, texlive-fonts-recommended and
# lmodern).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
set -e
if [ $# -lt 2 ] || [ $(($# % 2)) -ne 0 ]; then
  echo "usage: sh build-pdf.sh <directory of the main file> <output directory> [...]" >&2
  exit 1
fi
latex="${PDFLATEX:-pdflatex} -interaction=nonstopmode -halt-on-error -jobname=gaol"
while [ $# -ge 2 ]; do
  (
    source_dir=$(cd "$1" && pwd)
    mkdir -p "$2"
    cd "$2"
    if [ ! -f gaol_version.tex ]; then
      echo "gaol_version.tex is not in $2: configure GAOL with autotools or meson first" >&2
      exit 1
    fi
    # The main file is manual_gaolv5.tex of GAOL v5, gaol.tex of GAOL 4
    main=gaol.tex
    if [ -f "$source_dir/manual_gaolv5.tex" ]; then
      main=manual_gaolv5.tex
    fi
    # The sources, the chapters/ of GAOL v5 included, are looked for in the
    # directory of the main file, after this one
    export TEXINPUTS=".:$source_dir:"
    export BIBINPUTS="$source_dir:"
    export BSTINPUTS="$source_dir:"
    $latex $main > pdflatex-1.log || { tail -40 pdflatex-1.log; exit 1; }
    ${BIBTEX:-bibtex} gaol
    ${MAKEINDEX:-makeindex} gaol.idx -o gaol.ind
    $latex $main > pdflatex-2.log || { tail -40 pdflatex-2.log; exit 1; }
    # The index of the first pass holds the page numbers of a text where the
    # references were not resolved yet: it is made again, after each pass
    ${MAKEINDEX:-makeindex} gaol.idx -o gaol.ind
    $latex $main > pdflatex-3.log || { tail -40 pdflatex-3.log; exit 1; }
    # The page numbers of a long manual may need more than three passes
    for pass in 4 5; do
      grep -q "Rerun to get cross-references right" pdflatex-$((pass - 1)).log || break
      ${MAKEINDEX:-makeindex} gaol.idx -o gaol.ind
      $latex $main > pdflatex-$pass.log || { tail -40 pdflatex-$pass.log; exit 1; }
      cp pdflatex-$pass.log pdflatex-3.log
    done
    # A reference or a citation left undefined is an error
    if grep -E "LaTeX Warning: (Reference|Citation) .* undefined" pdflatex-3.log; then
      exit 1
    fi
    echo "$2: $(grep -E "^Output written on" pdflatex-3.log)"
  )
  shift 2
done
