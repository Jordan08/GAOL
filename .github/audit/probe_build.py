"""probe_build.py WORK LABEL: for the CMake, autotools and meson builds configured in WORK, the flags of
gaol_interval.cpp and of a C source of CORE-MATH (sin.c), and the configuration the probes show, as JSON."""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
import json, os, re, shlex, subprocess, sys
work, label = sys.argv[1], sys.argv[2]
probe = os.path.abspath(os.path.join(work, 'probe.cpp'))
probe_c = os.path.abspath(os.path.join(work, 'probe.c'))
result = {'label': label}

def first_error(log):
    lines = open(log, errors='replace').read().splitlines()
    for l in lines:
        if re.search(r'\berror\b|ERROR|Error:', l):
            return l.strip()[:300]
    return (lines[-1] if lines else '')[:300]

def analyse(kind, suffix, needle, make_n, make_dir, probe_file, drop_include):
    """The flags a build gives the source whose path ends with suffix, and what the probe shows with them."""
    out = {}
    if kind in ('cmake', 'meson'):
        db = json.load(open(os.path.join(work, kind, 'compile_commands.json')))
        c = [e for e in db if e['file'].replace('\\', '/').endswith(suffix)][0]
        tokens = c.get('arguments') or [x.strip('"') for x in shlex.split(c['command'], posix=(os.name != 'nt'))]
        cwd = c['directory']
    else:
        text = open(os.path.join(work, make_n)).read().replace('\\\n', ' ')
        line = [l for l in text.splitlines() if '--mode=compile' in l and needle in l][0]
        # automake writes `test -f 'x.c' || echo '$(srcdir)/'`x.c for a source it may find in the sources
        # of a VPATH build: the source is the last word, which is dropped below
        line = re.sub(r'`[^`]*`', '', line)
        tokens = shlex.split(line.split('--mode=compile', 1)[1].split('&&')[0])
        cwd = os.path.join(work, 'autotools', make_dir)
    args, skip = [], False
    for t in tokens[1:]:
        if skip:
            skip = False
            continue
        if t in ('-o', '-MF', '-MT', '-MQ') or (drop_include and t in ('-include', '/FI')):
            skip = True
            continue
        if t == '-c' or t.startswith('-M') or t.endswith(suffix.rsplit('/', 1)[-1]) or '$' in t:
            continue
        args.append(t)
    out['compiler'] = tokens[0]
    out['flags'] = [t for t in args if not t.startswith(('-I', '-L', '-isystem'))]
    p = subprocess.run([tokens[0]] + args + ['-E', '-P', probe_file], cwd=cwd, capture_output=True, text=True)
    if p.returncode != 0:
        out['error'] = 'probe: ' + ' | '.join(l.strip()[:200] for l in p.stderr.strip().splitlines()[-4:]) if p.stderr.strip() else 'probe failed'
    macros, ifs = {}, {}
    for l in p.stdout.splitlines():
        m = re.match(r'\s*GAOLPROBE_(\w+) (?:= (.*)|undefined)\s*$', l)
        if m:
            macros[m.group(1)] = m.group(2).strip() if m.group(2) is not None else None
            continue
        m = re.match(r'\s*GAOLPROBEIF_(\w+) (true|false)\s*$', l)
        if m:
            ifs[m.group(1)] = m.group(2) == 'true'
    out['macros'], out['ifs'] = macros, ifs
    return out

for kind in ('cmake', 'autotools', 'meson'):
    entry = {}
    result[kind] = entry
    status = open(os.path.join(work, kind + '.status')).read().strip()
    if status != '0':
        entry['error'] = first_error(os.path.join(work, kind + '.log'))
        continue
    try:
        entry.update(analyse(kind, 'gaol/gaol_interval.cpp', 'gaol_interval.cpp', 'autotools.make-n.txt', 'gaol', probe, False))
    except Exception as e:
        entry['error'] = 'no compilation command: %r' % e
        continue
    # The C sources, those of CORE-MATH, which the three builds are to compile alike (GAOL v5)
    try:
        entry['c'] = analyse(kind, 'binary64/sin/sin.c', 'binary64/sin/sin.c', 'autotools.make-n-c.txt', '3rd', probe_c, True)
    except Exception as e:
        entry['c'] = {'error': 'no compilation command of the C sources: %r' % e}
    # The macros the build wrote into gaol/gaol_configuration.h (GAOL v5)
    header = os.path.join(work, kind, 'gaol', 'gaol_configuration.h')
    if os.path.exists(header):
        entry['configuration'] = {m.group(1): m.group(2).strip() for m in
                                  re.finditer(r'^#define (\w+)[ \t]*(.*)$', open(header).read(), re.M)
                                  if m.group(1) != 'GAOL_CONFIGURATION_H'}
    v = subprocess.run([entry['compiler'], '--version'], capture_output=True, text=True)
    entry['version'] = (v.stdout.splitlines() or [''])[0]
print(json.dumps(result, indent=1))
