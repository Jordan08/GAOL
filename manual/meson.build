# Meson build of GAOL: the manual
#
# Created 2025-11-25 by Raphaël Chenouard (doc/meson.build)
# Modified in 2026 by Jordan NININ (GAOL v5): the manual of GAOL v5; the
# documentation Doxygen wrote from the sources (gaol_doxygen.cfg.in) is gone, as
# the Doxyfile of the root; the manual of GAOL 4 is no longer built
#
# The manual: meson compile -C <build directory> pdf (build-pdf.sh) builds the
# manual of GAOL v5 and writes its PDF in manual/ of the sources,
# Manual_GAOLv5.pdf, not in the build directory, which keeps the files of
# LaTeX. The manual of GAOL 4 is not built: its PDF, manual/v4/Manual_GAOLv4.pdf,
# was built once (see build-pdf.sh), and v4/meson.build is not used. Its edition
# and its date are those of configure.ac (GAOL_V5_EDITION, GAOL_V5_UPDATED);
# v5/meson.build writes its gaol_version.tex
pdflatex = find_program('pdflatex', required: false)
bibtex = find_program('bibtex', required: false)
makeindex = find_program('makeindex', required: false)
if pdflatex.found() and bibtex.found() and makeindex.found()
  v5_version_data = configuration_data()
  v5_version_data.set('GAOL_V5_EDITION', '5.0')
  v5_version_data.set('GAOL_V5_UPDATED', '6 November 2026')
  v5_version_data.set('GAOL_VERSION', meson.project_version())
  subdir('v5')
  run_target(
    'pdf',
    command: [find_program('sh'), files('build-pdf.sh'),
              join_paths(meson.current_source_dir(), 'v5'), join_paths(meson.current_build_dir(), 'v5'),
              join_paths(meson.current_source_dir(), 'Manual_GAOLv5.pdf')],
  )
else
  warning('pdflatex, bibtex or makeindex not found: the manual cannot be built')
endif
