# Les bancs de la tâche PK

Les bancs écrits le 4 novembre 2026 pour étudier la tâche PK (des `FE_INVALID`
avec Clang et les intervalles FPU), close sans code : voir `TODO_DONE.md` 3.17.
Ils servent à rejouer la mesure, par exemple avec Clang 21 ou sur un processeur
AVX-512, qui n'ont pas pu être essayés. Si un `FE_INVALID` revient avec les
drapeaux de GAOL, la tâche se rouvre.

| Fichier | Contenu |
| --- | --- |
| `pk_loops.cpp` | 184 boucles de `certainly_le()`, `certainly_leq()`, `certainly_ge()`, `certainly_geq()` et des opérateurs `<`, `<=`, `>`, `>=` : entre `x[i]` et `y[i]`, avec `interval(-7.5, c)` de chaque côté, avec une constante, avec un point, avec l'ensemble vide, sur des opérandes vides un sur trois ou cinq, ou tous. Chaque boucle ne doit pas lever `FE_INVALID` et doit donner le résultat de la relation calculée seule |
| `pk_const.cpp` | 192 boucles du même genre, entre `x[i]` et une constante (1,5, 0, -2,5, 1e300) de chaque côté, générées par un script qui n'a pas été gardé |
| `with_flags.sh` | Compile un banc avec les drapeaux de GAOL (`-frounding-math -fno-fast-math -ffp-contract=off`), avec plusieurs compilateurs et niveaux d'optimisation, et le lance. Attendu : 0 `FE_INVALID`, 0 faux |
| `without_flags.sh` | Compile un banc sans `-frounding-math` (non pris en charge), avec plusieurs jeux de drapeaux. Attendu avec Clang 18 et 20 : 120 boucles sur 184 et 192 sur 192 lèvent `FE_INVALID` ; GCC 13.3 n'en lève aucune, sauf avec `-fno-trapping-math` |
| `control.cpp` | Le témoin : une boucle `l[i] <= 1.5` sur des NaN, qui doit lever `FE_INVALID` et prouve que `fetestexcept()` voit bien le drapeau |

Les deux scripts prennent le répertoire d'un build FPU (`libgaol.a`) :

```sh
cmake -S . -B build-fpu -DGAOL_SIMD=OFF && cmake --build build-fpu -j2
todo-notes/pk/with_flags.sh build-fpu
SRC=pk_const.cpp todo-notes/pk/with_flags.sh build-fpu
CXXS="clang++-21 g++" todo-notes/pk/without_flags.sh build-fpu
```

`SRC` choisit le banc (`pk_loops.cpp` par défaut), `CXXS` les compilateurs
(`clang++-18 clang++-20 g++` par défaut) et `OUT` le répertoire des binaires
(un répertoire temporaire par défaut). Les lignes `-march=tigerlake` de
`without_flags.sh` ne se lancent que sur un processeur AVX-512.
