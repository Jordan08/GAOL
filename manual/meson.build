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

# The manual, gaol.pdf: meson compile -C <build directory> pdf (build-pdf.sh).
# Its edition and date are those of configure.ac (GAOL_EDITION, GAOL_UPDATED)
pdflatex = find_program('pdflatex', required: false)
bibtex = find_program('bibtex', required: false)
makeindex = find_program('makeindex', required: false)
if pdflatex.found() and bibtex.found() and makeindex.found()
  version_data = configuration_data()
  version_data.set('GAOL_EDITION', '4.6')
  version_data.set('GAOL_UPDATED', '18 September 2026')
  version_data.set('GAOL_VERSION', meson.project_version())
  configure_file(
    input: 'gaol_version.tex.in',
    output: 'gaol_version.tex',
    configuration: version_data,
  )
  run_target(
    'pdf',
    command: [find_program('sh'), files('build-pdf.sh'), meson.current_source_dir(), meson.current_build_dir()],
  )
else
  warning('pdflatex, bibtex or makeindex not found: the manual cannot be built')
endif
