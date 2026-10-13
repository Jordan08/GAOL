#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: checks, with rationals, the bound K of the error of the
# integer powers x^n of pown_products() (gaol/gaol_interval.cpp), whose
# comment gives the induction: after j steps (squares and products by c) of
# the binary exponentiation reaching c^k = X, with u = 2^-52,
#
#     |l| <= b_k |X|,  b_k = (k - 1) u (1 + 2 (k - 1) u),
#     |h + l - X| <= a_kj |X|,  a_kj = 3 k j u^2,
#
# each step keeping these bounds, and then K = (8 m L + 16) u^2 for c^m, m of
# L bits, and K = 2 (8 m L + 16) u^2 + 64 u^2 + 2 m^3 u^3 for c^-m. The
# bounds of one step, which the comment writes, are those of the worst case
# of each rounding (u of its value, or 2^-1074, u^2/2 of 2^-969, below which
# no power goes there).
#
#     python3 pown_bound.py [--all N] [--random N] [--seed S]
#
# checks every k from 2 to N (3000 by default), N random k up to 2^31 (3000
# by default), and k = 2^31 - 1 and 2^30, for each j from 1 to 2 bitlen(k),
# and prints the largest ratio of the error to K found: below 1 where the
# bound holds. The exit status is 1 where an inequality fails.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-13 by Jordan NININ

import argparse
import random
import sys
from fractions import Fraction

U = Fraction(1, 2**52)
ETA = Fraction(1, 2**105)  # 2^-1074 relative to a power above 2^-969
LARGEST = 2**31


def b(k):
    return (k - 1)*U*(1 + 2*(k - 1)*U)


def a(k, j):
    return 3*k*j*U*U


def square(bk, ajk):
    """The bounds of the rest and of the error after a square, from those before it, as the comment writes
    them"""
    rest = (1 + U)**2*bk*(2 + 2*ajk + 3*bk) + (1 + U)*U*(1 + ajk + bk)**2 + ETA
    error = ajk*(2 + ajk) + U*bk*(2 + 2*ajk + 3*bk) + U*rest + ETA
    return rest, error


def product(bk, ajk):
    """The bounds of the rest and of the error after a product by c"""
    rest = (1 + U)*(bk + U*(1 + U)*(1 + ajk + bk)) + ETA
    error = ajk + U*rest + ETA
    return rest, error


def k_power(m):
    return (8*m*m.bit_length() + 16)*U*U


def k_inverse(m):
    return 2*k_power(m) + 64*U*U + 2*m**3*U**3


def inverse_error(m):
    """The bound of the error of q + q (r + r^2) relative to |q|, for c^-m"""
    L = m.bit_length()
    am = a(m, 2*(L - 1))
    bm = b(m)
    eta = Fraction(1, 2**113)  # 2^-1074 relative to |q| >= 2^-961
    rho = U + (1 + U)*bm/(1 - am - bm)  # |R|
    r = rho*(1 + U) + eta
    return ((U*rho + eta)*(1 + 2*r) + (U*r + eta)*(1 + r) + (U*(r*(1 + U) + eta)*(1 + r) + eta)
            + rho**3/(1 - rho) + am/(1 - rho))


def main():
    p = argparse.ArgumentParser(description='Checks the error bound of the integer powers of GAOL with rationals.')
    p.add_argument('--all', type=int, default=3000, help='every k from 2 to N (%(default)s)')
    p.add_argument('--random', type=int, default=3000, help='N random k up to 2^31 (%(default)s)')
    p.add_argument('--seed', type=int, default=1)
    args = p.parse_args()
    rng = random.Random(args.seed)
    ks = list(range(2, args.all + 1)) + [rng.randint(args.all + 1, LARGEST - 1) for _ in range(args.random)]
    ks += [LARGEST - 1, LARGEST // 2]
    failures = 0
    for k in ks:
        for j in range(1, 2*k.bit_length() + 1):
            bk, ajk = b(k), a(k, j)
            if 2*k <= LARGEST:
                rest, error = square(bk, ajk)
                if rest > b(2*k) or error > a(2*k, j + 1):
                    failures += 1
                    print('square from k = %d, j = %d: the bounds do not hold' % (k, j))
            rest, error = product(bk, ajk)
            if rest > b(k + 1) or error > a(k + 1, j + 1):
                failures += 1
                print('product from k = %d, j = %d: the bounds do not hold' % (k, j))
    worst_power = worst_inverse = Fraction(0)
    for m in ks + [LARGEST]:
        L = m.bit_length()
        am, bm = a(m, 2*(L - 1)), b(m)
        worst_power = max(worst_power, am/(1 - am - bm)/k_power(m))
        worst_inverse = max(worst_inverse, inverse_error(m)/k_inverse(m))
    print('%d values of k: %d inequalities fail; error/K at most %.3f for c^m, %.3f for c^-m'
          % (len(ks), failures, float(worst_power), float(worst_inverse)))
    return 0 if failures == 0 and worst_power <= 1 and worst_inverse <= 1 else 1


if __name__ == '__main__':
    sys.exit(main())
