#!/bin/sh
# Builds a manual from its main file and writes its PDF where it is told to:
#
#   sh manual/build-pdf.sh <directory of the main file> <build directory> <PDF>
#
# The make pdf of the autotools build, the target pdf of the CMake build
# (-DWITH_DOC=ON) and the one of the meson build (-Dwith-doc=true) run it on the
# manual of GAOL v5, manual/v5, whose main file is manual_gaolv5.tex (the layout,
# which \input's the file of each chapter from chapters/), and write
# manual/Manual_GAOLv5.pdf, in the sources, not in the build directory. The
# manual of GAOL 4, manual/v4, whose main file is gaol.tex, is not built by any of
# them: its PDF, manual/v4/Manual_GAOLv4.pdf, was built once with this script
# (and a gaol_version.tex of edition 4.6, last updated 18 September 2026, version
# 4.3.0 in a directory of its own).
#
# LaTeX works in the build directory, which holds gaol_version.tex, the version,
# edition and date of the manual (configure, CMake and meson generate it from
# the gaol_version.tex.in of the manual), and receives the auxiliary files, named
# after the PDF (pdflatex -jobname=Manual_GAOLv5). The PDF itself is moved to
# the place given, so that no copy is left in the build directory. The manual
# of GAOL v5 can also be compiled by hand in its directory (pdflatex
# manual_gaolv5.tex: without gaol_version.tex, the title page takes the version
# of VERSION.txt and today's date). Needs pdflatex, bibtex and makeindex, with
# the packages that v5/gaol_def.tex (the manual of GAOL v5) and the class
# v4/manual.cls (the manual of GAOL 4) load (on Debian and Ubuntu:
# texlive-latex-recommended, texlive-latex-extra, texlive-fonts-recommended and
# lmodern).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
set -e
if [ $# -ne 3 ]; then
  echo "usage: sh build-pdf.sh <directory of the main file> <build directory> <PDF>" >&2
  exit 1
fi
source_dir=$(cd "$1" && pwd)
mkdir -p "$2"
build_dir=$(cd "$2" && pwd)
# The PDF is given by its directory and its name: its directory is made if it is
# missing, and the name gives the job name of LaTeX, so Manual_GAOLv5.pdf makes
# Manual_GAOLv5.aux, Manual_GAOLv5.toc...
pdf_dir=$(mkdir -p "$(dirname "$3")" && cd "$(dirname "$3")" && pwd)
pdf_name=$(basename "$3")
job=${pdf_name%.pdf}
latex="${PDFLATEX:-pdflatex} -interaction=nonstopmode -halt-on-error -jobname=$job"
cd "$build_dir"
if [ ! -f gaol_version.tex ]; then
  echo "gaol_version.tex is not in $build_dir: configure GAOL with autotools, CMake or meson first" >&2
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
${BIBTEX:-bibtex} "$job"
${MAKEINDEX:-makeindex} "$job.idx" -o "$job.ind"
$latex $main > pdflatex-2.log || { tail -40 pdflatex-2.log; exit 1; }
# The index of the first pass holds the page numbers of a text where the
# references were not resolved yet: it is made again, after each pass
${MAKEINDEX:-makeindex} "$job.idx" -o "$job.ind"
$latex $main > pdflatex-3.log || { tail -40 pdflatex-3.log; exit 1; }
# The page numbers of a long manual may need more than three passes
for pass in 4 5; do
  grep -q "Rerun to get cross-references right" pdflatex-$((pass - 1)).log || break
  ${MAKEINDEX:-makeindex} "$job.idx" -o "$job.ind"
  $latex $main > pdflatex-$pass.log || { tail -40 pdflatex-$pass.log; exit 1; }
  cp pdflatex-$pass.log pdflatex-3.log
done
# A reference or a citation left undefined is an error
if grep -E "LaTeX Warning: (Reference|Citation) .* undefined" pdflatex-3.log; then
  exit 1
fi
echo "$(grep -E "^Output written on" pdflatex-3.log)"
# The PDF goes where it was asked for, and leaves the build directory
mv -f "$job.pdf" "$pdf_dir/$pdf_name"
echo "$pdf_dir/$pdf_name"
