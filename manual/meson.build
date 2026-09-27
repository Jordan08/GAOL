# Meson build of GAOL: the manuals
#
# Created 2025-11-25 by Raphaël Chenouard (doc/meson.build)
# Modified in 2026 by Jordan NININ (GAOL v5): the manuals of GAOL v5 and of
# GAOL 4; the documentation Doxygen wrote from the sources (gaol_doxygen.cfg.in)
# is gone, as the Doxyfile of the root
#
# The manuals: meson compile -C <build directory> pdf (build-pdf.sh) builds
# v5/gaol.pdf, the manual of GAOL v5, and v4/gaol.pdf, the manual of GAOL 4.
# Their editions and dates are those of configure.ac (GAOL_V5_EDITION,
# GAOL_V5_UPDATED, and GAOL_EDITION, GAOL_UPDATED for GAOL 4); v5/meson.build
# and v4/meson.build write the gaol_version.tex of each
pdflatex = find_program('pdflatex', required: false)
bibtex = find_program('bibtex', required: false)
makeindex = find_program('makeindex', required: false)
if pdflatex.found() and bibtex.found() and makeindex.found()
  v4_version_data = configuration_data()
  v4_version_data.set('GAOL_EDITION', '4.6')
  v4_version_data.set('GAOL_UPDATED', '18 September 2026')
  v4_version_data.set('GAOL_VERSION', meson.project_version())
  v5_version_data = configuration_data()
  v5_version_data.set('GAOL_V5_EDITION', '5.0')
  v5_version_data.set('GAOL_V5_UPDATED', '22 September 2026')
  v5_version_data.set('GAOL_VERSION', meson.project_version())
  subdir('v4')
  subdir('v5')
  run_target(
    'pdf',
    command: [find_program('sh'), files('build-pdf.sh'),
              join_paths(meson.current_source_dir(), 'v5'), join_paths(meson.current_build_dir(), 'v5'),
              join_paths(meson.current_source_dir(), 'v4'), join_paths(meson.current_build_dir(), 'v4')],
  )
else
  warning('pdflatex, bibtex or makeindex not found: the manuals cannot be built')
endif
