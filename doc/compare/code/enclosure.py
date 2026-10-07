#!/usr/bin/env python3
"""Do the elementary functions enclose their values? GAOL v5 and Boost.Interval.

    enclosure.py data N DIR
        draws N arguments for each function (N/2 ordinary ones and N/2 at the
        edges of the doubles or of the domain, see FUNCTIONS) and writes them
        into DIR/args_<function>.bin, as doubles in the byte order of the
        machine, which the programs enclosure_<library>.cpp read
    enclosure.py report DIR REPORT.md
        reads the bounds the programs wrote, DIR/<library>_<function>.bin, and
        checks each one against the value of the function at the argument,
        computed by mpmath with 2000 bits, or 8000 where the value is that close
        to a double (cached in DIR/ref_<function>.bin as the two doubles around
        it); writes the tables of the results between the markers of REPORT.md
        (the whole file when it does not exist)

A result [l, u] of f([x, x]) encloses f(x) when l <= f(x) <= u, that is when
l <= RD(f(x)) and RU(f(x)) <= u, RD and RU being the roundings of f(x) down and
up to doubles; it is the tightest when l = RD(f(x)) and u = RU(f(x)).
"""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

import math
import os
import struct
import sys

import mpmath

SEED = 1788
PRECISION = 2000

# The libraries, as the programs name their results, and their names in the
# tables. boost is Boost.Interval with rounded_transc_opp, the policy of the
# benchmark and of the special cases (see boost_policies.h), boost_std with
# rounded_transc_std, and libm the functions of the C library alone, called
# with the rounding direction downward for the lower bound and upward for the
# upper one (pow(x, 3.0) for x³)
LIBRARIES = [("gaol5", "GAOL v5"), ("boost", "Boost.Interval, `rounded_transc_opp`"),
             ("boost_std", "Boost.Interval, `rounded_transc_std`"),
             ("libm", "C library, downward and upward")]


def uniform(lo, hi):
    return lambda rng, n: rng.uniform(lo, hi, n)


def log_uniform(e_lo, e_hi, signed=True):
    """|x| = 2^u, u uniform in [e_lo, e_hi], with a random sign when signed"""
    def draw(rng, n):
        x = 2.0 ** rng.uniform(e_lo, e_hi, n)
        return x * rng.choice([-1.0, 1.0], n) if signed else x
    return draw


def near_one(rng, n):
    """±(1 − 2^u), u uniform in [−53, −1]: next to the ends of [−1, 1]"""
    return (1.0 - 2.0 ** rng.uniform(-53, -1, n)) * rng.choice([-1.0, 1.0], n)


def one_plus(rng, n):
    """1 + 2^u, u uniform in [−52, 1000]: from next to 1 to 2^1000"""
    return 1.0 + 2.0 ** rng.uniform(-52, 1000, n)


def tiny_or_near_one(rng, n):
    """Half ±2^u, u uniform in [−60, −1], half next to ±1"""
    import numpy
    return numpy.concatenate((log_uniform(-60, -1)(rng, n // 2), near_one(rng, n - n // 2)))


LOG2_709 = math.log2(709)

# The functions, in the order of ENCLOSURE_FUNCTIONS (enclosure.h): the value
# computed by mpmath, the ordinary arguments and the extreme ones, with their
# descriptions
FUNCTIONS = [
    ("exp", mpmath.exp, uniform(-20, 20), "[−20, 20]", log_uniform(-60, LOG2_709), "±2^u, u in [−60, log2 709]"),
    ("log", mpmath.log, uniform(0, 10), "[0, 10]", log_uniform(-1074, 1023.99, False), "2^u, u in [−1074, 1024]"),
    ("sin", mpmath.sin, uniform(-10, 10), "[−10, 10]", log_uniform(-30, 70), "±2^u, u in [−30, 70]"),
    ("cos", mpmath.cos, uniform(-10, 10), "[−10, 10]", log_uniform(-30, 70), "±2^u, u in [−30, 70]"),
    ("tan", mpmath.tan, uniform(-10, 10), "[−10, 10]", log_uniform(-30, 70), "±2^u, u in [−30, 70]"),
    ("asin", mpmath.asin, uniform(-1, 1), "[−1, 1]", tiny_or_near_one, "±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1]"),
    ("acos", mpmath.acos, uniform(-1, 1), "[−1, 1]", tiny_or_near_one, "±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1]"),
    ("atan", mpmath.atan, uniform(-10, 10), "[−10, 10]", log_uniform(-60, 1000), "±2^u, u in [−60, 1000]"),
    ("sinh", mpmath.sinh, uniform(-10, 10), "[−10, 10]", log_uniform(-60, LOG2_709), "±2^u, u in [−60, log2 709]"),
    ("cosh", mpmath.cosh, uniform(-10, 10), "[−10, 10]", log_uniform(-60, LOG2_709), "±2^u, u in [−60, log2 709]"),
    ("tanh", mpmath.tanh, uniform(-10, 10), "[−10, 10]", log_uniform(-60, LOG2_709), "±2^u, u in [−60, log2 709]"),
    ("asinh", mpmath.asinh, uniform(-10, 10), "[−10, 10]", log_uniform(-60, 1000), "±2^u, u in [−60, 1000]"),
    ("acosh", mpmath.acosh, uniform(1, 10), "[1, 10]", one_plus, "1 + 2^u, u in [−52, 1000]"),
    ("atanh", mpmath.atanh, uniform(-1, 1), "[−1, 1]", tiny_or_near_one, "±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1]"),
    ("sqrt", mpmath.sqrt, uniform(0, 10), "[0, 10]", log_uniform(-1074, 1023.99, False), "2^u, u in [−1074, 1024]"),
    ("pow3", lambda x: x ** 3, uniform(-10, 10), "[−10, 10]", log_uniform(-340, 340), "±2^u, u in [−340, 340]"),
]
NAMES = {"pow3": "x³ (`pow(x, 3)`)"}


def data(n, directory):
    import numpy
    os.makedirs(directory, exist_ok=True)
    rng = numpy.random.RandomState(SEED)
    # The values of the arguments drawn before, which may be other ones
    for name, *_ in FUNCTIONS:
        if os.path.exists(os.path.join(directory, f"ref_{name}.bin")):
            os.remove(os.path.join(directory, f"ref_{name}.bin"))
    for name, _, ordinary, _, extreme, _ in FUNCTIONS:
        x = numpy.concatenate((ordinary(rng, n // 2), extreme(rng, n - n // 2))).astype("=f8")
        x.tofile(os.path.join(directory, f"args_{name}.bin"))
    with open(os.path.join(directory, "args.txt"), "w") as f:
        f.write(f"{n} {SEED}\n")


def read_doubles(path):
    with open(path, "rb") as f:
        b = f.read()
    return list(struct.unpack(f"={len(b) // 8}d", b))


def next_up(x):
    if math.isnan(x) or x == math.inf:
        return x
    if x == 0.0:
        return 5e-324
    bits = struct.unpack("<q", struct.pack("<d", x))[0]
    bits += 1 if x > 0 else -1
    return struct.unpack("<d", struct.pack("<q", bits))[0]


def next_down(x):
    return -next_up(-x)


def around(v):
    """RD(v) and RU(v): the largest double <= v and the smallest >= v"""
    if abs(v) > sys.float_info.max:
        return (sys.float_info.max, math.inf) if v > 0 else (-math.inf, -sys.float_info.max)
    f = float(v)
    while mpmath.mpf(f) > v:
        f = next_down(f)
    while f != math.inf and mpmath.mpf(next_up(f)) <= v:
        f = next_up(f)
    return (f, f) if mpmath.mpf(f) == v else (f, next_up(f))


def reference(f, x):
    """RD(f(x)) and RU(f(x)), from f(x) computed with PRECISION bits, or with
    four times as many when f(x) is that close to a double: tanh(709) is
    1 − 2e^−1418, which needs 2046 bits to be told from 1"""
    for precision in (PRECISION, 4 * PRECISION):
        mpmath.mp.prec = precision
        v = f(mpmath.mpf(x))
        rd, ru = around(v)
        if mpmath.isinf(v) or v == 0:
            break
        gap = min(abs(v - mpmath.mpf(rd)), abs(mpmath.mpf(ru) - v)) if math.isfinite(ru) else abs(v - rd)
        if gap > abs(v) * mpmath.mpf(2) ** (100 - precision):
            break
    return rd, ru


def references(directory, name, f, args):
    """RD(f(x)) and RU(f(x)) for each argument, computed once (see reference)"""
    path = os.path.join(directory, f"ref_{name}.bin")
    if os.path.exists(path):
        r = read_doubles(path)
        if len(r) == 2 * len(args):
            return [(r[2 * i], r[2 * i + 1]) for i in range(len(args))]
    r = [reference(f, x) for x in args]
    with open(path, "wb") as out:
        out.write(struct.pack(f"={2 * len(r)}d", *[b for pair in r for b in pair]))
    return r


def ulps(lo, hi):
    """The number of doubles between lo and hi, lo excluded"""
    def key(x):
        b = struct.unpack("<q", struct.pack("<d", x))[0]
        return b if b >= 0 else -(b & 0x7FFFFFFFFFFFFFFF)
    return key(hi) - key(lo)


def readable(x):
    if math.isnan(x):
        return "NaN"
    if math.isinf(x):
        return "+∞" if x > 0 else "−∞"
    return repr(x).replace("-", "−")


def check(args, refs, bounds):
    """The counts of the results of a library for the arguments: not enclosing
    (and on which side), with NaN bounds, inverted (lower > upper, which the
    default checking of Boost.Interval does not see as empty), tightest, and the
    widths in doubles of the results that enclose, with the first examples"""
    c = {"n": len(args), "outside": 0, "low": 0, "high": 0, "nan": 0, "inverted": 0, "points": 0, "tightest": 0,
         "widths": [], "examples": []}
    for i, x in enumerate(args):
        lo, hi = bounds[2 * i], bounds[2 * i + 1]
        rd, ru = refs[i]
        if math.isnan(lo) or math.isnan(hi):
            c["outside"] += 1
            c["nan"] += 1
            bad = "NaN"
        else:
            bad = []
            if lo > rd:
                bad.append("lower")
            if hi < ru:
                bad.append("upper")
            if bad:
                c["outside"] += 1
                c["low"] += "lower" in bad
                c["high"] += "upper" in bad
                c["inverted"] += lo > hi
                c["points"] += lo == hi
            else:
                if lo == rd and hi == ru:
                    c["tightest"] += 1
                if math.isfinite(lo) and math.isfinite(hi):
                    c["widths"].append(ulps(lo, hi))
        if bad and len(c["examples"]) < 3:
            c["examples"].append((x, (rd, ru), (lo, hi), bad))
    return c


def width(w):
    return f"{w:.1e}".replace("e+", "e") if w >= 1e6 else str(w)


def count(k, n):
    if k == 0:
        return "0"
    p = 100.0 * k / n
    return f"{k:,}".replace(",", " ") + (f" ({p:.0f} %)" if p >= 1 else f" ({p:.1g} %)")


BEGIN = "<!-- BEGIN GENERATED TABLES (doc/compare/code/enclosure.py) -->"
END = "<!-- END GENERATED TABLES -->"


def report(directory, out):
    libs = [(key, title) for key, title in LIBRARIES
            if os.path.exists(os.path.join(directory, f"{key}_{FUNCTIONS[0][0]}.bin"))]
    with open(os.path.join(directory, "args.txt")) as f:
        n, seed = f.read().split()
    lines = [f"{int(n):,}".replace(",", " ") + f" arguments for each function (random seed {seed}), half of them "
             "ordinary, half at the edges of the doubles or of the domain; each value computed by mpmath with "
             f"{PRECISION} bits.\n\n"]
    lines.append("#### Results that do not enclose the value\n\n")
    lines.append("The number of arguments x for which f([x, x]) misses f(x), on its lower bound, its upper bound or "
                 "both, and how many of these results are inverted, their lower bound above their upper one, or "
                 "points, a single double that is not f(x):\n\n")
    lines.append("| Function | Arguments | " + " | ".join(t for _, t in libs) + " |\n")
    lines.append("|---" * (2 + len(libs)) + "|\n")
    tight_lines, example_lines = [], []
    totals = {key: [0, 0] for key, _ in libs}
    for name, f, _, ordinary, _, extreme in FUNCTIONS:
        args = read_doubles(os.path.join(directory, f"args_{name}.bin"))
        refs = references(directory, name, f, args)
        half = len(args) // 2
        per_lib = {}
        for key, _ in libs:
            b = read_doubles(os.path.join(directory, f"{key}_{name}.bin"))
            per_lib[key] = (check(args[:half], refs[:half], b[:2 * half]),
                            check(args[half:], refs[half:], b[2 * half:]))
        label = NAMES.get(name, f"`{name}`")
        for part, what in ((0, ordinary), (1, extreme)):
            cells = []
            for key, _ in libs:
                c = per_lib[key][part]
                totals[key][0] += c["outside"]
                totals[key][1] += c["n"]
                cell = count(c["outside"], c["n"])
                if c["outside"]:
                    sides = []
                    if c["low"]:
                        sides.append(f"lower {c['low']}")
                    if c["high"]:
                        sides.append(f"upper {c['high']}")
                    if c["nan"]:
                        sides.append(f"NaN {c['nan']}")
                    cell += ": " + ", ".join(sides)
                    if c["inverted"]:
                        cell += f"; {c['inverted']} inverted"
                    if c["points"]:
                        cell += f"; {c['points']} points"
                cells.append(cell)
            lines.append(f"| {label if part == 0 else ''} | {what} | " + " | ".join(cells) + " |\n")
        cells = []
        for key, _ in libs:
            both = per_lib[key]
            tightest = both[0]["tightest"] + both[1]["tightest"]
            widths = sorted(both[0]["widths"] + both[1]["widths"])
            if widths:
                cells.append(f"{count(tightest, len(args))}; {width(widths[len(widths) // 2])} / "
                             f"{width(widths[-1])}")
            else:
                cells.append(f"{count(tightest, len(args))}")
            for x, (rd, ru), (lo, hi), bad in (both[0]["examples"] + both[1]["examples"])[:1]:
                side = bad if isinstance(bad, str) else " and ".join(bad)
                value = f"{readable(rd)}" if rd == ru else f"between {readable(rd)} and {readable(ru)}"
                example_lines.append(f"| {label} | {dict(libs)[key]} | {readable(x)} | {value} | "
                                     f"[{readable(lo)}, {readable(hi)}] | {side} |\n")
        tight_lines.append(f"| {label} | " + " | ".join(cells) + " |\n")
    lines.append("| **All** | | " + " | ".join(f"**{count(*totals[key])}**" for key, _ in libs) + " |\n\n")
    lines.append("#### Tightness\n\n")
    lines.append("The results that are the tightest interval of doubles around f(x), out of all the arguments, and "
                 "the median and largest width, in doubles, of the results that enclose f(x) (finite bounds):\n\n")
    lines.append("| Function | " + " | ".join(t for _, t in libs) + " |\n")
    lines.append("|---" * (1 + len(libs)) + "|\n")
    lines.extend(tight_lines)
    lines.append("\n#### Examples\n\n")
    lines.append("The first argument whose result misses the value, for each function and library, the ordinary "
                 "arguments first:\n\n")
    lines.append("| Function | Library | x | f(x) | Result | Missed by |\n|---|---|---|---|---|---|\n")
    lines.extend(example_lines)
    lines.append("\n")

    table = "".join(lines)
    block = BEGIN + "\n\n" + table + END + "\n"
    try:
        with open(out) as f:
            content = f.read()
    except FileNotFoundError:
        content = ""
    if BEGIN in content and END in content:
        head, rest = content.split(BEGIN, 1)
        tail = rest.split(END, 1)[1]
        content = head + block + tail.lstrip("\n")
    else:
        content = block
    with open(out, "w") as f:
        f.write(content)
    for key, title in libs:
        print(f"{title}: {totals[key][0]} of {totals[key][1]} results do not enclose the value")


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "data":
        data(int(sys.argv[2]), sys.argv[3])
    elif len(sys.argv) == 4 and sys.argv[1] == "report":
        report(sys.argv[2], sys.argv[3])
    else:
        sys.exit(__doc__)
