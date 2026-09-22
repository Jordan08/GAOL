# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
#
doxygen = find_program('doxygen', required: false)
if not doxygen.found()
  warning('Doxygen not found, no documentation generated!')
else
  cdata = configuration_data()
  cdata.set('VERSION', meson.project_version())
  cdata.set('TOP_SRCDIR', meson.current_source_dir())
  cdata.set('TOP_BUILDDIR', meson.current_build_dir())

  if find_program('dot', required: false).found()
    # In the real world this would set the variable
    # to YES. However we set it to NO so that the
    # list of generated files is always the same
    # so tests always pass.
    cdata.set('HAVE_DOT', 'YES')
  else
    cdata.set('HAVE_DOT', 'NO')
  endif

  doxyfile = configure_file(
    input: 'gaol_doxygen.cfg.in',
    output: 'gaol_doxygen.cfg',
    configuration: cdata,
    install: false,
  )

  datadir = join_paths(get_option('datadir'), 'doc', 'gaol')

  html_target = custom_target(
    'html-gaol-docs',
    build_by_default: false,
    input: doxyfile,
    output: 'html',
    command: [doxygen, doxyfile],
    install: true,
    install_dir: datadir,
  )
endif

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
