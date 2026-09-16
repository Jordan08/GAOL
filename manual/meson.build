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

pdflatex = find_program('pdflatex', required: false)
if pdflatex.found()
  # TODO
  # pdf_target = custom_target('pdf-gaol-docs',
  #   build_by_default: false,
  #   input: doxyfile,
  #   output: 'pdf',
  #   command: [pdflatex, ''],
  #   install: true,
  #   install_dir: datadir)
endif
