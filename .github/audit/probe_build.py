"""probe_build.py WORK LABEL: for the CMake, autotools and meson builds configured in WORK, the flags of
gaol_interval.cpp and the configuration the probe shows, as JSON."""
import json, os, re, shlex, subprocess, sys
work, label = sys.argv[1], sys.argv[2]
probe = os.path.abspath(os.path.join(work, 'probe.cpp'))
result = {'label': label}

def first_error(log):
    lines = open(log, errors='replace').read().splitlines()
    for l in lines:
        if re.search(r'\berror\b|ERROR|Error:', l):
            return l.strip()[:300]
    return (lines[-1] if lines else '')[:300]

for kind in ('cmake', 'autotools', 'meson'):
    entry = {}
    result[kind] = entry
    status = open(os.path.join(work, kind + '.status')).read().strip()
    if status != '0':
        entry['error'] = first_error(os.path.join(work, kind + '.log'))
        continue
    try:
        if kind in ('cmake', 'meson'):
            db = json.load(open(os.path.join(work, kind, 'compile_commands.json')))
            c = [e for e in db if e['file'].replace('\\', '/').endswith('gaol/gaol_interval.cpp')][0]
            tokens = c.get('arguments') or [x.strip('"') for x in shlex.split(c['command'], posix=(os.name != 'nt'))]
            cwd = c['directory']
        else:
            text = open(os.path.join(work, 'autotools.make-n.txt')).read().replace('\\\n', ' ')
            line = [l for l in text.splitlines() if '--mode=compile' in l and 'gaol_interval.cpp' in l][0]
            tokens = shlex.split(line.split('--mode=compile', 1)[1].split('&&')[0])
            cwd = os.path.join(work, 'autotools', 'gaol')
    except Exception as e:
        entry['error'] = 'no compilation command: %r' % e
        continue
    args, skip = [], False
    for t in tokens[1:]:
        if skip:
            skip = False
            continue
        if t in ('-o', '-MF', '-MT', '-MQ'):
            skip = True
            continue
        if t == '-c' or t.startswith('-M') or t.endswith('gaol_interval.cpp') or '$' in t:
            continue
        args.append(t)
    entry['compiler'] = tokens[0]
    entry['flags'] = [t for t in args if not t.startswith(('-I', '-L', '-isystem'))]
    p = subprocess.run([tokens[0]] + args + ['-E', '-P', probe], cwd=cwd, capture_output=True, text=True)
    if p.returncode != 0:
        entry['error'] = 'probe: ' + ' | '.join(l.strip()[:200] for l in p.stderr.strip().splitlines()[-4:]) if p.stderr.strip() else 'probe failed'
    macros, ifs = {}, {}
    for l in p.stdout.splitlines():
        m = re.match(r'\s*GAOLPROBE_(\w+) (?:= (.*)|undefined)\s*$', l)
        if m:
            macros[m.group(1)] = m.group(2).strip() if m.group(2) is not None else None
            continue
        m = re.match(r'\s*GAOLPROBEIF_(\w+) (true|false)\s*$', l)
        if m:
            ifs[m.group(1)] = m.group(2) == 'true'
    entry['macros'], entry['ifs'] = macros, ifs
    v = subprocess.run([tokens[0], '--version'], capture_output=True, text=True)
    entry['version'] = (v.stdout.splitlines() or [''])[0]
print(json.dumps(result, indent=1))
