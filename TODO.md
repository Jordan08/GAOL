# À faire

Ce qui reste à faire sur GAOL v5 au commit `773d6a1` de `dev` (la fusion de #335
le 14 novembre 2026 à 14 h 53 UTC, la tâche PV, en une seule pull request, après
les registres de la tâche PU, `079687e` ; `configure-clean` est à `83e2f69`, la
fusion de #321, sans les notes de travail), et avant au commit `998e61c` de
`dev` (la fusion de #334 le 13 novembre à 22 h 09 UTC, la dernière des douze
pull requests de la tâche PU, #323 à #334, fusionnées depuis 16 h 43 UTC après
les registres de ses décisions, `07dca51`, et ceux de #319, #321 et #322,
`03f3783`), et avant au commit `f7046b9` de `dev` (la fusion de #322 à 10 h 53
UTC, après les registres de la relecture du 13 novembre et la fusion de #319,
`45efbd1`), et avant au commit `9d59e0c` de `dev` et de `configure-clean` (les
décisions de PJ.3, après `0ae1840` et la fusion de #318, `f006421`, la tâche
PJ.7, le 12 novembre 2026 à 19 h 56 UTC, après celle de #317, la suite de #316,
à 17 h 31 UTC, après celle de #316, la tâche PJ.2, à 15 h 30 UTC, après celle de
#315, la tâche PJ.1, à 15 h 20 UTC, le commit direct `7745279`, la fusion de
#314, hors des points, à 12 h 42 UTC, celle de #313, hors des points, à 12 h 31
UTC, le commit direct `7528253`, la fusion de #312, hors des points, à 7 h 53
UTC, le commit direct `f27c6c2`, celle de #309, hors des points, le 11 novembre
à 22 h 16 UTC, le commit direct `be73e24` et celles de #311, de #310 et de #308,
hors des points, de #307 et de #306, hors des points, de #305, hors des points,
de #302, la tâche PP, de #304 et de #303, les registres de la tâche PT, de #301,
la tâche PT, de #300, hors des points, de #298, de #299, de #297, de #296 et de
#295, de #294, le manuel dans le style de la thèse, et de #292, de #293, #291,
#290 et #289), en tâches PW à PZ (les constats de la relecture complète du 13
novembre), PH et PJ (PA à PG, PI, PL, PP, PQ, PS, PT, PU et PV sont faites, PK
et PM sont closes sans code : voir `TODO_DONE.md`). Ce fichier et l'issue #179
ont le même contenu. Ce qui est fait et décidé est dans `TODO_DONE.md` (l'issue
#177 n'est plus mise à jour), ce qui est reporté dans `TODO_AFTER.md` (issue
#178). Les anciennes issues #49 et #142 sont fermées, et `TODO_mistral.md` est
supprimé, comme `TODO_fin.md` : ses articles sont répartis dans les trois
registres, dont les tâches PS (faite par #297), PT (faite par #301 :
`TODO_DONE.md` 3.25) et PP (faite par #302 : `TODO_DONE.md` 2.3 et 7.25).

Les deux défauts bloquants de la relecture du 13 novembre sont corrigés :
`acos_rel()`, `asin_rel()` et `atan_rel()` rendaient parfois le vide alors que
l'intervalle contient des solutions (PU.1, #323), et deux threads qui
affichaient la même expression `-x` plantaient (PU.2, #324) ; une borne est
peut-être fausse encore pour un programme qui met lui-même `FPCR.AH` sur arm64
(issue #198 : `x / y` des intervalles NEON, cause non trouvée). Rien n'est à
envoyer à un autre projet ni à publier sans l'accord du mainteneur. Une pull
request est ouverte, vers `dev` : #336 (hors des points : les 207 jobs de la CI
regroupés en 59, un job de plus pour l'IPO sur le chemin SSE2, et la correction
de `x += d` et des opérations voisines avec `-flto`). #335 (la tâche PV : le
lecteur et les expressions, six points en une seule pull request sur la demande
du mainteneur, et quatre commits qu'il a demandés ensuite) est fusionnée dans
`dev` le 14 novembre à 14 h 53 UTC par le mainteneur ; ses registres sont faits
(`TODO_DONE.md` 2.3, 3.40, 4 et 7.38), et ce qu'elle laisse au manuel, à
`doc/differences.md` et au `ChangeLog` est dans PZ (PZ.3 et PZ.5 à PZ.7). Les
douze pull requests de la tâche PU, une par point, sont fusionnées dans `dev` le
13 novembre par le mainteneur : #323 (PU.1) à
16 h 43 UTC, #324 (PU.2) à 16 h 46, #325 (PU.4) à 17 h 03, #326 (PU.6) à
17 h 09, #327 (PU.7) à 21 h 09, #329 (PU.9) à 21 h 26, #328 (PU.8) à 21 h 29,
#331 (PU.5) à 21 h 38, #332 (PU.10) à 21 h 46, #330 (PU.3) à 21 h 54, #333
(PU.11) à 22 h 04 et #334 (PU.12) à 22 h 09 ; leurs registres sont faits
(`TODO_DONE.md` 2.3, 3.39, 4 et 7.37), et ce qu'elles laissent au manuel, à
`doc/differences.md` et au `ChangeLog` est dans PZ (PZ.5 et PZ.7). #319 (les
renvois aux registres récrits, la même archive des sources) et #322 (la CI sur
un push vers `dev`) sont fusionnées le 13 novembre dans `dev`, la branche créée
de `configure-clean` le 12 novembre et la base du développement, et #321 dans
`configure-clean`, la branche des versions, qui n'a plus les notes de travail :
ce fichier n'est que dans `dev` (process.md §1 et §13). #295 (PS.2), #296 (PM),
#297 (la tâche PS), #299 (PQ.2), #298 (PQ.1), #300 (hors des points : la recette
de compilation croisée GCC 12.2 pour aarch64 de `todo-notes/gcc12-aarch64/`) et
#301 (la tâche PT) sont fusionnées le 8 novembre, à 17 h 21, 17 h 29, 19 h 38,
20 h 31, 20 h 44, 21 h 38 et 23 h 09 UTC, #302 (la tâche PP : le cas N1 M de
`div_rel()` en SSE2) le 9 novembre à 22 h 03 UTC, #305 (hors des points : six
`friend` de `interval` retirés) le 10 novembre à 8 h 52 UTC, #306 (hors des
points : `gaol_interval_core.h` allégé, ses aides dans `gaol_interval_detail.h`)
à 17 h 55 UTC et #307 (hors des points : les en-têtes de tous les fichiers sur
un même modèle) à 21 h 04 UTC, puis, le 11 novembre, #308 (hors des points :
`gaol_interval_core.cpp` découpé par domaine) à 21 h 05 UTC, #310 (du
mainteneur : `.gitignore`) à 21 h 08 UTC, #311 (hors des points : l'en-tête
`<cstdint>`, que #310 avait remplacé par un `<stdint>` qui n'existe pas) à
21 h 36 UTC et #309 (hors des points : `make dist` dans les trois builds, le
workflow Release des paquets) à 22 h 16 UTC, puis, le 12 novembre, #312 (hors
des points : `MATH-CORE` retirée de la CI, `numbers` à 600 s dans le job macOS
26 arm64 LLVM Clang Debug ASan+UBSan et à 900 s dans celui de macOS 15 x86_64
GCC, `compile_commands.json` hors de l'archive de CMake), fusionnée à 7 h 53 UTC
par la session, sur la demande explicite du mainteneur, puis #313 (hors des
points : les textes qui citent macOS 14, PJ.6, et un commentaire de
`CMakeLists.txt`, PJ.7) à 12 h 31 UTC et #314 (hors des points : les textes de
#279, #281, #85, #306, #308, #309 et #312 dans `doc/differences.md` et le
`ChangeLog`, trois passages du manuel avec son PDF, PJ.7) à 12 h 42 UTC, par le
mainteneur ; leurs registres sont faits, puis #315 (PJ.1 : le rapport de
couverture, 95,1 % des lignes de `gaol/`) à 15 h 20 UTC et #316 (PJ.2 : les
temps remesurés, le manuel et son PDF) à 15 h 30 UTC, puis #317 (la suite de
#316, PJ.2 : le tableau de Visual C++ et de clang-cl par une expérience de CI,
`x.midpoint()`) à 17 h 31 UTC, par le mainteneur, alors que leur CI attendait
des runners de GitHub, puis #318 (PJ.7 : les restes de la synthèse, quatre
entrées de CI au `ChangeLog`) à 19 h 56 UTC, par le mainteneur, alors qu'un job
de macOS attendait encore un runner ; leurs registres sont faits ici
(`TODO_DONE.md` 2.3, 3.34, 3.35, 7.31 et 7.32).

## Comment lire ce fichier

- Une tâche (PW à PZ, PH, PJ) regroupe des points qui touchent le même code, les
  mêmes fichiers ou les mêmes builds de validation, pour les traiter dans la
  même session ; chaque point reste une pull request à part (process.md : un
  point, une branche), sauf quand la tâche dit qu'une seule suffit. Les tâches
  sont dans l'ordre conseillé. Critères : la sûreté des bornes d'abord ; ce qui
  coûte plus cher en attendant (une décision qui bloque) ; ce qui tue un
  programme ou rompt une promesse écrite avant ce qui ne fait que ralentir ; le
  petit et sans dépendance avant le gros ; la version (PJ) en dernier. La lettre
  PR n'est pas une tâche : le sigle dit déjà « pull request ».
- PH ne dépend d'aucune autre tâche, sauf quand le texte le dit (PJ suit PI, PS,
  PQ, PP, PT, PU, PV et les études de PG, qui sont faites, et PW à PZ : la
  version en dernier, les corrections faites, *décidé le 19 octobre*). PZ suit
  PU à PY, dont les corrections se reportent dans le manuel et les registres.
- Chaque point garde sa lettre et son ancien numéro, par exemple « 79 (AB) » :
  ce sont ceux des pull requests, des issues, des commentaires du code et de
  `TODO_DONE.md` (section 4 pour les lettres, section 5 pour les numéros faits).
  Tailles, estimées d'après les descriptions : S, moins d'une demi-journée ; M,
  un à deux jours ; L, plusieurs jours ou incertain.
- « Décidé le 22 octobre (Q13) » marque une décision du mainteneur, dans
  `TODO_DONE.md` (3.5 pour le 22 octobre, 3.6 pour le 23) : applique-la telle
  quelle. « À décider » marque une question sans réponse : pose-la au mainteneur
  avant d'écrire du code.
- La procédure est dans `process.md` ; son §9 dit comment tenir ces trois
  registres à jour après une fusion.

## Les tâches restantes

| Tâche | Contenu | Points | Taille |
| --- | --- | --- | --- |
| PW | Les builds et la CI | PW.1 à PW.11 | M |
| PX | Les tests sans dents | PX.1 à PX.5 | S |
| PY | Les textes faux hors du manuel, le code mort | PY.1 à PY.12 | M |
| PZ | Le manuel, `differences.md` et le `ChangeLog` | PZ.1 à PZ.7 | M |
| PH | Les signalements à d'autres projets | #125 à #138, #175 | accord |
| PJ | La publication de v5.0.0 (`master`, étiquette, annonce) | 34, 71 | S |

## PW. Les builds et la CI [M]

Les tâches PW à PZ viennent de la relecture complète du 13 novembre : 31
relecteurs indépendants sur l'archive de `9d59e0c`, après des contrôles
automatiques qui passent tous (builds, fichiers générés, PDF, exemples du
manuel), chaque constat revérifié par la session ; le mainteneur en a fait des
tâches le même jour (`TODO_DONE.md` 3.37), dont la tâche PU, faite par #323 à
#334 (`TODO_DONE.md` 2.3 et 3.39), et la tâche PV, faite par #335 en une seule
pull request (`TODO_DONE.md` 2.3 et 3.40). Le rapport, les rapports des
relecteurs et les programmes de preuve sont hors du dépôt, dans
`/home/jninin/Documents/WORK/DEV/GAOL/GAOL_review_2026-11-13/` (`rapport.md`).
Les numéros de ligne sont ceux de `9d59e0c` : au début d'un point, le relire
contre l'arbre, que PU et PV ont changé. « Démontré » : un programme ou un
calcul le montre ; « plausible » : raisonné, non montré. Chaque point est une
pull request, avec un test qui a des dents (process.md §5). Les fichiers de
`.github/workflows/` se poussent par SSH (process.md §10).

1. **`automatic_configuration` échoue sur une machine sans FMA** (important,
   démontré) : `tests/automatic_configuration.cmake` (lignes 389 à 396) exige
   que la compilation croisée avec `-DGAOL_MACHINE_RUNS_FMA=1` donne les
   drapeaux FMA de la configuration native, vides sur une telle machine (x86
   d'avant Haswell ou Piledriver, Atom, ARMv7 sans VFPv4) : la configuration est
   juste et le test échoue (process.md §5 : pas de test fragile). Comparer aux
   drapeaux que le compilateur prend, ou sauter cette vérification quand la
   machine n'exécute pas FMA.
2. **`make check` échoue avec `--enable-shared --disable-static`** (important,
   démontré) : `tests/Makefile.am` (lignes 41 à 43) construit
   `automatic_cleanup` sous Linux même quand la bibliothèque est partagée, ce
   que CMake (`tests/CMakeLists.txt`, ligne 100) et meson (`tests/meson.build`,
   ligne 36) écartent ; sans archive statique, libtool ignore `-static`, le test
   lie `libgaol.so` et échoue. Le commentaire des lignes 9 et 10 est faux dans
   ce cas. Un `AM_CONDITIONAL` sur `enable_static`.
3. **Des étapes de CI qui ne peuvent pas échouer** (important, démontré) :
   `linux.yml`, ligne 200, `./rounding_direction_pc | tail -1` sans `pipefail`
   (l'étape n'a pas le `set -o pipefail` de la ligne 188) : l'échec du seul test
   lié par le `gaol.pc` de CMake est perdu, dans les 26 jobs `cmake` ; de même
   `compare-performance.py ... | tee -a` (`containers.yml`, lignes 158 et 159 ;
   `performance.yml`, ligne 305) dans les jobs sans `shell: bash`.
4. **La locale à virgule et les tests reconstruits, annoncés et non faits**
   (important) : `doc/continuous-integration.md` (lignes 253 à 257) et les
   en-têtes de `build-systems.yml` (lignes 8 à 11), de `macos.yml` (lignes 4 à
   6) et de `windows.yml` (lignes 12 à 14) disent que chaque job qui lance les
   tests vérifie que `numbers` a lu sous une locale à virgule ; ne le vérifient
   pas `cygwin` (deux jobs de `windows.yml`, un de `build-systems.yml`),
   `clang-msvc`, `meson-clang-msvc`, `autotools-clang-msvc`, `universal`,
   `xcode`, `universal-autotools` et `universal-meson`. Les en-têtes de
   `windows.yml` (lignes 5 à 8) et de `build-systems.yml` (lignes 3 à 7)
   annoncent des tests reconstruits avec le GAOL installé que `msys2`,
   `clang-msvc`, `visual-studio-sde` et `meson-clang-msvc` ne font pas. *À
   décider* : ajouter les vérifications, ou nommer les exceptions, comme Alpine
   et manylinux.
5. **autotools et meson compilent les tests, les exemples et `gaol_bench` avec
   les drapeaux de la bibliothèque** (mineur, démontré) : `configure.ac` (lignes
   796 à 829) et `meson.build` (lignes 333 à 359) donnent à tout `-funroll-loops
   -fomit-frame-pointer -fexpensive-optimizations`, la visibilité cachée et
   `-Wall -Wconversion`, que CMake réserve à la bibliothèque
   (`GAOL_LIBRARY_FLAGS`, `CMakeLists.txt`, lignes 1200 à 1232) : `make perf` et
   `ninja perf` ne mesurent pas le binaire de CMake, et le commentaire de
   `tests/Makefile.am` (lignes 75 et 76) est faux. Les réserver à `gaol/` et
   `3rd/` (process.md §4, principe 6).
6. **L'audit des trois builds** (mineur, démontré) : un échec de la sonde C,
   celle de CORE-MATH, passe inaperçu (`.github/audit/probe_build.py`, lignes 53
   à 65, remplit `macros` même en erreur, et `compare.py`, lignes 85 à 87, ne
   teste que `'macros' not in c`) ; `audit.sh` (lignes 17 à 27) ne compare que
   les options par défaut, jamais `GAOL_PRESERVE_ROUNDING`, SIMD, FMA, les
   exceptions ni `GAOL_PREFER_AVX512`.
7. **Les délais** (mineur) : `numbers` n'a aucun délai avec autotools (300 s
   avec CMake et meson), ce que `doc/tests.md` (lignes 732 à 740) ne dit pas ;
   `examples/meson.build` (ligne 37) laisse aux exemples les 30 s de meson
   (`06_global_optimization` prend 12 s en Debug avec ASan sur l'i7).
8. **`-mno-daz-ftz` avec Clang** (mineur, démontré) : Clang 20 et 21 le
   prennent, Clang 18 non ; `doc/three-builds.md` (lignes 32 à 34 et 524 à 530),
   le commentaire de `CMakeLists.txt` (lignes 678 à 681) et l'expression
   exportée (ligne 1384), réservée à GNU, ne citent que GCC : un programme
   compilé par Clang 21 avec `gaol::gaol` perd cette protection (les bornes
   restent sûres, `round_upward_if_needed()` effaçant les modes).
9. **Des restes de petite portée** (mineur) : la boucle Rosetta du job
   `universal` sans `test "$count" -ge 20` (`macos.yml`, lignes 143 à 155) ;
   `VERSION.txt` lu sans le BOM, `\v` et `\f` que les builds acceptent
   (`release.yml`, lignes 82 et 151 ; `build-systems.yml`, ligne 826) ; le job
   « CMake 3.14.7 » ne vérifie pas la version qui tourne (`linux.yml`, lignes
   386 à 395) ; `CPACK_GENERATOR "TGZ;DEB"` sur tout Linux, même sans `dpkg`
   (`CMakeLists.txt`, lignes 1593 et 1594, plausible) ; les versions Mach-O de
   la dylib de libtool (6 et 6.0) contre 5.0.0 avec CMake et meson
   (`configure.ac`, ligne 156, plausible, non essayé sous macOS) ; `meson dist`
   ne donne les fichiers de `git archive` qu'avec un meson récent
   (`Makefile.am`, lignes 82 à 85, plausible pour 0.53) ; `gaol/stamp-h.in`
   toujours distribué (`gaol/Makefile.am`, ligne 146) et lu par aucun build ;
   les actions des workflows suivies par des étiquettes mobiles, sauf
   `setup-sde` (*à décider*).
10. **Des commentaires morts ou faux des builds** (mineur) : `configure.ac`
    (lignes 163 et 164) et `meson.build` (ligne 348) sur `--enable-optimize`,
    qui n'existe plus ; `meson.build` (lignes 468 à 476) sur `enable-arm64`, qui
    n'existe plus ; `gaol/meson.build` (ligne 112) sur `GAOL_CORE_MATH_LOG`, qui
    n'existe pas ; `Makefile.am` (lignes 31 et 32) « Requires automake 1.4 » ;
    `gaol/Makefile.am` (ligne 113) `$(gaol_HEADERS)`, indéfinie ;
    `CMakeLists.txt`, ligne 170 (`doc/coverage.md`, absent), lignes 818 et 819
    (configure ne teste que le drapeau `-msse3`), lignes 1421 et 1422
    (`if(FALSE)` vide) ; `tests/CMakeLists.txt` (lignes 139 et 140 : le job
    macOS x86_64 donne 900 s à `numbers`, non 600) ;
    `gaol/core/gaol_interval_internal.h`, ligne 43 (le `#if
    GAOL_USING_SSE2_INSTRUCTIONS` est lu avant `gaol_config.h` et n'est jamais
    vrai) ; trois tables de valeurs diffèrent de leur générateur d'une ligne
    vide après l'en-tête (`elementary_values.h`, `extended_precision_values.h`
    et `compoundm1_values.h`, retouchés par `afd838a`, #307) : régénérer les
    tables ou corriger les scripts.
11. **Un commit de registres sur `dev` lance toute la CI** (de #322) : depuis
    le 13 novembre, un push sur `dev` lance les workflows, environ 200 jobs,
    même pour un commit qui ne change que `TODO.md`, `TODO_DONE.md`,
    `TODO_AFTER.md`, `process.md`, `STEP.md` ou `todo-notes/`. *À décider* : un
    `paths-ignore` de ces fichiers sur le push des six workflows (non fait par
    #322).

## PX. Les tests sans dents [S]

Relevés par la relecture du 13 novembre (voir PW). Le test de PU.1 est fait par
#323.

1. **`tests/u128.cpp`** (important, démontré) : sans entier natif de 128 bits
   (i386, armhf, Visual C++, « where this code runs in earnest »), « mul64 is
   mul of the low halves » (lignes 199 à 201) compare `gaol_u128_mul64()` à
   lui-même (`gaol_u128_mul()` commence par lui, `gaol/math/gaol_u128.h`, ligne
   161), et le « schoolbook product » qu'annonce l'en-tête (lignes 18 à 21)
   n'existe pas ; les branches `_umul128` et `__umulh` de Visual C++ ne sont
   jamais comparées à une valeur indépendante. Ajouter un produit scolaire sur
   des chiffres de 16 bits.
2. **Des vérifications qui ne peuvent pas échouer** (mineur, démontré) : la
   boucle aléatoire de `cancel_minus` saute tout résultat `entire` sans le
   justifier, et pour les décalages 0 et +1 ses 100 000 résultats le sont
   (`tests/arithmetic.cpp`, lignes 2310 à 2323) ; `X.set_contains(X)`
   (`tests/interval_functions.cpp`, lignes 77 et 79 à 81 : un `.midpoint()`
   perdu) ; la borne droite de `log([-3, 4])` vérifiée d'un seul côté
   (`tests/non_arithmetic.cpp`, ligne 127) ; le sens d'arrondi vérifié juste
   après `round_upward()` (`tests/test_ibex_codac.cpp`, lignes 3382 et 3383) ;
   `TEST_PEQ` (non disjoints) pour la relecture d'un texte écrit
   (`tests/input_output.cpp`, lignes 169 et 170 : un point milieu passe).
3. **Des tolérances trop larges** (mineur, démontré) : `TEST_EQ` accepte une
   distance de Hausdorff de 1e-8 (`tests/unit_tests.h`, lignes 140 à 142) et ne
   vérifie aucun encadrement, dans `tests/trigonometric.cpp`,
   `tests/reverse_mappings.cpp`, `tests/constants.cpp` (ligne 50),
   `tests/constructor.cpp` (ligne 155) et les quotients exacts de
   `tests/arithmetic_operators.cpp` (un `acos` décalé de 5e-9 passe) ;
   `expect_close()` (`tests/elementary.cpp`, lignes 93 à 100) accepte une borne
   un double trop bas quand la valeur exacte est un double (une mutation de
   `sqrt` qui donne `[2 - 2^-52, 2]` pour `sqrt([4])` passe) ; l'unité du format
   général va jusqu'à dix unités du dernier chiffre (`tests/numbers.cpp`, lignes
   1093 à 1100) ; `pownRev(c, INT_MIN)` (`tests/ieee1788.cpp`, lignes 273 à 276)
   accepte un résultat aux bornes de c échangées ; 15^17 et 15^18 sont comparés
   au littéral arrondi (`tests/non_arithmetic.cpp`, lignes 80 à 83) ; la borne
   droite de `[1, 2] / (2^53 + 1)` n'est pas vérifiée (`test_integer_operands`
   de `tests/arithmetic_operators.cpp`).
4. **Du code mort et des tests vides** (mineur) : `expect_within()`, jamais
   appelée, et les branches identiques de `limit == 1` (`tests/elementary.cpp`,
   lignes 113 à 145) ; la branche `slack > 0.0` d'`expect_kept()`
   (`tests/other_functions.cpp`, ligne 952) ; `test_sin`, `test_asin_rel` et
   `test_sqrt_rel`, vides mais comptés comme réussis (`tests/trigonometric.cpp`,
   ligne 52, dont le commentaire « No test because it uses cos() » est faux ;
   `tests/reverse_mappings.cpp`, lignes 94 et 125) ; des cas répétés à la place
   de -5.0 et de -5.5 (`tests/float_functions.cpp`, lignes 37, 38, 59 et 68) ;
   un libellé faux (`tests/compoundm1.cpp`, ligne 865 : y = [-oo, -1], non [-oo,
   0]) ; une référence de `tanpi` fausse mais jamais atteinte
   (`tests/core_math.cpp`, lignes 1404 à 1407 : `[-oo, +oo]` au lieu du vide
   quand l = r tombe sur un pôle).
5. **Des références calculées par GAOL** (mineur) : `tests/elementary.cpp`
   (lignes 281 à 291), `tests/core_math.cpp` (lignes 1296 à 1313, qui fige en
   plus l'implémentation), `tests/compoundm1.cpp` (lignes 933 à 947) et
   `tests/reverse.cpp` (lignes 545 à 552) ; des références indépendantes
   existent ailleurs : le dire dans leur commentaire.

## PY. Les textes faux de `doc/`, des exemples et du code ; le code mort [M]

Relevés par la relecture du 13 novembre (voir PW). Une pull request par document
ou par groupe ; ce qui va dans le manuel, `doc/differences.md` et le `ChangeLog`
est dans PZ. Aucun point ne change une borne.

1. **Le chemin AVX-512 « mesuré sur l'i7 seul »** (important) :
   `doc/building.md` (lignes 167 et 168 : « the processor halves the AVX-512
   units, and no other was measured ») contre `doc/simd.md` (lignes 108 à 117),
   qui publie un Xeon de machine virtuelle à unités pleines : le rapport des
   temps de SSE2 à ceux du chemin y est de 0,73 et 0,69 sans
   `GAOL_PRESERVE_ROUNDING` (le chemin y perd plus que sur l'i7, 0,83) et de
   3,89 et 4,21 avec ; le manuel a la même phrase (PZ.1).
2. **`doc/math-libraries.md`** : lignes 89 à 93, « this is what GAOL does »
   (déplacer d'un double les valeurs de mathlib), au présent, est faux pour v5
   (important : `doc/accuracy.md`, vers lequel pointe le lien, dit le
   contraire) ; lignes 124 à 127, le `sin` de GAOL 4 ne venait pas de CRlibm
   (c'était `cos(x - [π/2])`) ; lignes 14 et 15, les options de mathlib sont
   dites refusées, alors que configure les ignore avec un avertissement
   (`doc/building.md`, lignes 407 à 410) ; ligne 162, `log` 2,2 fois plus
   rapide, 2,6 dans `doc/compare/performance.md` (ligne 68).
3. **`doc/tests.md`** : lignes 301 à 303, 66 vérifications et treize groupes (67
   et quatorze, d'après la sortie de `rounding_direction`) ; ligne 448, 96
   opérations (97, `flush_operations[]`) ; ligne 1477, 1 969 cas d'`atan2_rel`,
   dont 1 263 avec points (2 071 et 1 338, ce qu'affiche `atan2_rel_values.py`),
   comme `doc/accuracy.md` (ligne 180) ; ligne 593, « 25 of the 45 checks » de
   `static_initialization` (44 aujourd'hui : la mesure date du correctif) ; le
   délai de `numbers` avec autotools (PW.7).
4. **`doc/using.md`** : lignes 539 à 541, `0x8000u >> s` ne compile pas
   (`overlapping_state` est un `enum class` : `static_cast<unsigned>(s)`), et
   `gaol_ieee1788::disjointMask` n'existe que dans des commentaires ; lignes 892
   et 893, `[2980.9579759367944, 2980.9579759367948]` contient bien la valeur
   exacte, mais n'est pas ce qu'écrit GAOL, dont les bornes
   (`0x1.749ea7bd26dc6p+11` et `0x1.749ea7bd26dc7p+11`) sont plus larges :
   écrire `[2980.9579759367943, 2980.9579759367949]` ou les bornes en
   hexadécimal ; lignes 1821 à 1824, `<1e-310, 1e-309>` est refusé parce que
   1e-310 n'est pas un double, non par la comparaison des bits (prendre
   `<0x1p-1070, 0x1p-1071>`) ; lignes 1725 à 1729 et 1866 à 1871, « x86 » pour
   le test par addition, que le x86 32 bits ne fait pas (`fnstcw`, `stmxcsr` ;
   plausible) : « x86-64 ». Piste : une phrase d'exemple pour le piège que
   décrit `gaol/core/gaol_interval_core.h` (lignes 2509 à 2514), `x > +oo`, `x
   >= +oo`, `x < -oo` et `x <= NaN` vrais pour tout x, `interval(±oo)` étant
   vide.
5. **`doc/simd.md` et `doc/accuracy.md`** : `simd.md` (lignes 330 et 331) donne
   6,3 ns pour `sqrt` et 3,75 avec AVX-512, contre 7,35 et 4,36 aux lignes 174
   et 295 ; `accuracy.md` (lignes 62 à 64) dit le chemin AVX-512 identique à
   SSE2 bit à bit, sans la réserve du zéro de `x * d` des lignes 272 à 276 ;
   `accuracy.md` (ligne 226) : `operator<<` prend les chiffres de
   `interval::precision()`, et seulement les drapeaux du flux.
6. **README, INSTALL, `doc/building.md` et `doc/three-builds.md`** : « CMake
   3.14 or later », puis `cmake --install`, qui date de CMake 3.15 (`INSTALL`,
   lignes 9 à 14 ; `README.md`, ligne 66 ; `building.md`, lignes 233 et 236 ; le
   job CMake 3.14.7 installe par `--target install`) ; le tableau des refus de
   `three-builds.md` (lignes 377 à 394) : « the last four » et « the third row
   it alone refuses » ne tombent plus juste depuis la ligne de Cygwin, qui a une
   troisième cellule que GitHub n'affiche pas.
7. **`doc/continuous-integration.md`, `doc/tools.md` et les README des
   outils** : `continuous-integration.md`, lignes 216 à 219 (les conteneurs
   lisent `c++ -dumpmachine`, non `uname -m`, attendent SSE2 sur i386, et
   sautent la vérification sans rien dire pour un autre triplet), lignes 369 à
   373 (sur arm64, `control-register-performance` compare les intervalles FPU
   aux NEON) ; `tools.md`, ligne 76 (« five `*_values.py` » : six), quatre
   scripts non décrits (`doc/compare/code/run_lines_arm64.sh`,
   `tests/tools/pow/negative_power_rows.py`, et `logtiny.sh` et `lognearzero.sh`
   de CORE-MATH), ligne 51 (`check-gcc-windows-fma.sh` exige aussi des
   `vfmadd`), ligne 53, comme `.github/scripts/sde-tests.sh` (ligne 3) : « the
   runners of GitHub for Windows have none » contredit
   `continuous-integration.md` (lignes 88 à 91) et `windows.yml` (lignes 9 à
   11) ; `tests/tools/pow/README.md` (lignes 215 à 217 : le mutant
   `pown-square-bound` n'existe plus ; ligne 170 : 48 mutants, quand la liste en
   a 106) ; `tests/tools/arith/README.md` (ligne 24 : 23 nombres, quand
   `diffarith.cpp` prend 29 bornes) ; `doc/compare/code/README.md`, lignes 171 à
   174 (important : `results.csv` serait du 17 octobre, à `7c1ebb9`, quand
   `machine.txt` dit le 12 novembre, à `2b5e4f9`), ligne 208 (`run_all.sh` lance
   aussi `run_lines.sh`).
8. **`doc/compare/`** : `special_cases.md`, ligne 268 (important : Boost « 27 to
   41 % », les taux de la glibc 2.31, quand le document annonce la 2.43, 20 à 34
   % d'après `enclosure.md`, ligne 43), ligne 29 (« five cases » sans résultat
   d'IEEE 1788 : deux), lignes 172 et 173 (la liste des opérations absentes de
   PROFIL/BIAS en a six qu'il calcule), lignes 179 et 180 (« without a negative
   bound » : sans 0), et le cas 082, `0 * [1, +oo]`, qui vaut aujourd'hui
   `[-0]` ; `performance.md`, ligne 169 (PROFIL/BIAS « the fastest on the square
   root (4.5 ns, against 4.4 for GAOL V5.0.0 with the direction preserved) ») et
   ligne 36 (« GAOL 4.2.3, which it continues » : v5 continue la 4.2.2) ;
   `itf1788.md`, ligne 279 (« `d5a345e`, the last commit of its sources » : 46
   commits depuis ; la suite relancée le 13 novembre donne les mêmes tableaux).
9. **Les exemples** : `examples/examples.md`, lignes 216 et 451, et
   `examples/05_interval_newton.cpp`, ligne 119 (important : « GAOL has no
   `mulRevToPair` », alors que `gaol_ieee1788::mulRevToPair` existe) ; ligne 464
   (« `pow(x, 2L)` and `pow(x, size_t)` are ambiguous » : les deux compilent) ;
   lignes 107 et 108 (autotools et meson construisent les 16 exemples) ; ligne
   171 (un point s'écrit `[a]`) ; lignes 583 (`3rd/mathlib`), 602 à 605, 695 à
   697 et 999 à 1005 (la revue d'origine présente comme actuels des `using`
   globaux, 25 macros et « 0 converts to interval » : dire qu'ils sont
   corrigés) ; `examples/tutorial.md`, lignes 169 à 177 (la boucle de Newton
   coupe X deux fois quand les deux morceaux valent X, et chaque racine sort
   deux fois : huit intervalles pour les trois racines de x³ − x sur [−3, 3]) ;
   les « vraies images » bâties sur les doubles d'`interval(0.99, 1.01)` et
   d'`interval(0.01, 1.0)`, plus étroites que l'image vraie
   (`examples/03_dependency_problem.cpp`, lignes 199 et 205 ;
   `examples/12_affine_arithmetic.cpp`, ligne 199), et les points
   d'`examples/09_parameter_estimation.cpp` (lignes 267 et 375) :
   `textToInterval` (process.md §4, principe 10).
10. **Des commentaires du code** : `gaol/math/gaol_u128.h` (lignes 4 à 6),
    `gaol/math/gaol_core_math_port.h` (lignes 15 à 17 et 107 à 109) et
    `3rd/README.md` (lignes 130 à 133) oublient `log10` parmi les fonctions à
    128 bits (13, non 12) ; `gaol/core/gaol_interval_internal.h`, lignes 500 et
    501 (« from the libm moved three doubles outward » : CORE-MATH, un double),
    lignes 164 à 231 (`quotient_up_sat()` et `quotient_dn_sat()`, annoncés pour
    a ≥ 0, sont faux en a = 0 ; latent, les appelants l'écartent) ;
    `gaol/core/gaol_interval_core.cpp`, lignes 24 à 26 (le carré et la racine de
    NEON sont dans les `*_kernels.h` depuis #308) ;
    `gaol/core/gaol_interval_core.h`, lignes 1946 à 1950 (trois représentations
    du vide, et `is_empty()` n'est plus `islessequal`), ligne 1832 (les seuils
    d'`erfc` sont 26,5433 et 27,2133, non 26,55 et 27,22), lignes 508 et 2702
    (`///!` et `/*!(void)`) ; `gaol/core/gaol_interval_fpu_impl.h`, lignes 307 à
    310 (« bit for bit » est vrai pour `x / y`, non pour le signe d'un zéro de
    `x % y`) ; `gaol/core/gaol_interval_avx512_impl.h`, lignes 379 et 380 (rien
    avant AVX-512F n'a de maximum d'entiers de 64 bits, et la bibliothèque est
    compilée avec `-mfma`, qui implique SSE4.1) ;
    `gaol/core/gaol_interval_trig.cpp`, ligne 554 (`0x1.9153d9443ed0bp+51` est
    la tangente de `0x1.921fb54442d17p+0` ; tan(M_PI_2) vaut
    `0x1.d02967c31cdb5p+53`) ; `gaol/gaol_config.h`, lignes 274 à 276
    (`-fno-honor-infinities` de Clang, seul) ; le Doxygen mal formé de
    `gaol/gaol_common.h` (lignes 69 et 83), de `gaol/parser/gaol_parser.h`
    (lignes 102 et 121) et de `gaol/parser/gaol_eval_stack.h` (lignes 49 et 50).
11. **Du code mort** : `finite_products`, trois blocs `#if 0`, l'ancien
    `inverse()` en commentaire et un namespace vide
    (`gaol/core/gaol_interval_sse_impl.h`, lignes 216 à 220, 470 à 646 et 1115 à
    1172) ; `gaol_core::nearest` (`gaol/math/gaol_double_op.h`, lignes 65 à
    102 : 32 fonctions jamais appelées) ; la branche sans `<fenv.h>`
    (`gaol/gaol_fpu.h`, lignes 492 à 494, et `gaol/gaol_fpu_msvc.h`), qui ne
    compile pas ; `root_of_large_order()`, exportée sans déclaration
    (`gaol/core/gaol_interval_pow.cpp`, ligne 1127 : `static`). *À décider* : la
    macro publique `gaol_ERROR` (`gaol/gaol_common.h`, lignes 203 à 209), le nom
    de GAOL 4, contre la règle des macros en `GAOL_` (process.md §4, principe
    7).
12. **Les avertissements de plus** (mineur ; *à décider* : lesquels valent une
    correction) : relevés en compilant la bibliothèque, les tests et les
    exemples avec GCC 15 (`-Wshadow -Wuseless-cast -Wsuggest-override -Wformat=2
    -Wmissing-declarations` et d'autres) et Clang 21 (`-Wshadow-all
    -Wdocumentation -Wunused-member-function -Wheader-hygiene` et d'autres),
    puis avec clang-tidy 21 (bugprone, clang-analyzer, cert) ; aucun ne cache un
    défaut, mais : `-Wshadow` dans `tests/arithmetic.cpp` (ligne 147),
    `tests/core_math.cpp` (ligne 1714), `tests/elementary.cpp` (ligne 805),
    `tests/rounding_direction.cpp` (lignes 5087, 5092 et 5145) et
    `gaol/parser/gaol_expression.cpp` (ligne 657, le paramètre `f` de
    `pow_itv_node`, sans défaut) ; des conversions inutiles (`-Wuseless-cast`)
    dans `gaol/gaol_fpu_fenv.h` (ligne 120), `gaol/gaol_fpu.h` (ligne 665),
    `gaol/parser/gaol_expression.h` (ligne 877) et 30 lignes de `tests/`
    (`core_math.cpp`, `rounding_direction.cpp`, `u128.cpp`, `unit_tests.h`),
    peut-être voulues pour d'autres cibles : à vérifier avant d'y toucher ; 150
    fonctions virtuelles sans `override` (120 dans
    `gaol/parser/gaol_expression.h`, 30 dans `gaol/parser/gaol_expr_eval.h`) ;
    un `using namespace` au niveau global d'un en-tête (`tests/unit_tests.h`,
    ligne 43) ; un membre inutilisé (`not_an_exception`,
    `tests/expressions.cpp`, ligne 1010) ; des formats non littéraux
    (`gaol/core/gaol_interval_io.cpp`, lignes 250 et 255, sûrs ;
    `tests/numbers.cpp`, ligne 162) ; pour clang-tidy, les valeurs rendues par
    `snprintf()` ignorées (`gaol/core/gaol_interval_io.cpp`, lignes 561, 565 et
    585), trois `using` inutilisés (`gaol/gaol_init_cleanup.cpp`, lignes 26 à
    28) et des compteurs de boucle flottants (`gaol/core/gaol_interval_pow.cpp`,
    lignes 776, 787, 817 et 825). Les 1 140 `-Wfloat-equal` sont voulus
    (l'arithmétique compare des bornes), et les avertissements du lexer et du
    parser générés ne se corrigent pas à la main.

## PZ. Le manuel et les registres [M]

Relevés par la relecture du 13 novembre (voir PW). Le manuel dans une pull
request `manual*`, qui régénère le PDF dans son dernier commit (process.md §6.2,
point 7) et corrige `doc/differences.md` avec lui ; le `ChangeLog` par la pull
request de synthèse (process.md §7, point 5). Après PU à PY, dont les
corrections se reportent ici ; celles de PU et de PV, faites, sont aux points 3,
5, 6 et 7.

1. **Les chiffres du chemin AVX-512 et de FMA** (important) :
   `02_installation.tex` (lignes 872 à 874 : « no other processor was
   measured ») et `04_rounding.tex` (lignes 526 à 528, 1629 et 1630 :
   « expected, not measured ») contre le Xeon du chapitre 12 (lignes 254 à 272),
   où le chemin perd plus que sur l'i7 sans l'option (0,73 et 0,69 contre
   0,83) ; `02_installation.tex` (lignes 843 à 846 et 1927) donne avec FMA
   « `cos` 2.5 times, `sin` 1.3 » et « up to 2.5 times », contre le tableau du
   même chapitre (lignes 1377 et 1378 : `cos` 1,44, `sin` 1,39, `log` 1,56,
   `pow(x, 3)` 3,1).
2. **Le chapitre 14** (important) : lignes 1458 à 1461, 1469 et 1470, cinq jobs
   `-Wall -Wextra -Werror` (sept : six de `linux.yml`, lignes 303 à 312, et un
   de `macos.yml`) ; lignes 1455 et 1456, deux jobs
   `GAOL_SSE2_UNALIGNED_STORAGE` (quatre) ; lignes 1270 à 1279, `release.yml`
   manque parmi les exceptions des déclencheurs (et `manual.yml` dans
   `doc/continuous-integration.md`) ; lignes 87 et 88, 520 et 521 (45
   vérifications de `static_initialization` : 44) ; lignes 440, 478 et 697 (66,
   96 et 1 969, comme PY.3).
3. **Le lecteur** : le chapitre 9 (lignes 285 à 288), comme
   `gaol/parser/gaol_parser.h` (lignes 97 à 99) : les seuils des bornes non
   départagées (au-dessous de 2^-1084 en base 16, et de 10^-331 en décimal,
   d'après `gaol/parser/gaol_interval_lexer.lpp`, lignes 276 à 279) ; et, depuis
   PV.6 (#335), un nombre au-delà des doubles (plus de 10^311, moins de 10^-331
   et leurs équivalents en bases 16, 8 et 2) est lu en temps linéaire, quand les
   passages suivants disent le temps quadratique sans exception :
   `doc/differences.md` (lignes 960 et 975), `03_using.tex` (ligne 1248),
   `09_io.tex` (lignes 192 et 765), `15_migration.tex` (lignes 1280 et 1696),
   `doc/using.md` (ligne 1087) le disant déjà avec l'exception. Les phrases
   « le lecteur connaît toutes les fonctions de GAOL » (chapitre 15, lignes 238,
   1378 et 1379, `doc/differences.md`, lignes 1761 et 1765 à 1770) sont vraies
   depuis PV.1, et le lecteur de la norme lit `cancelMinus`, `cancelPlus`,
   `intersection` et `convexHull` : rien n'y est à corriger pour ce point.
4. **Les autres passages du manuel** : `03_using.tex` (lignes 730 et 731 :
   l'exemple de PY.4) ; `07_arithmetic.tex` (lignes 620 et 621) et
   `11_accuracy.tex` (lignes 749 et 750) : les seuils d'`erfc` ;
   `07_arithmetic.tex` (lignes 103 à 105 : `hull` du vide n'est pas vide) ;
   `06_relations.tex` (lignes 157 à 169 : `set_contains(+oo)` est vrai pour `[1,
   +oo]`, quand `isMember` est faux) ; `05_creation.tex` (lignes 54 à 58 : deux
   constructeurs SIMD publics) ; `11_accuracy.tex` (lignes 1032 à 1038 : les
   chiffres d'`operator<<` ; lignes 1146 à 1149 : une borne finie de K) ;
   `02_installation.tex` (ligne 1653 : de 200 à 7000 bits ; lignes 1616 à 1618
   et 1932 : 1,2 à 1,5 fois) ; `01_interval_arithmetic.tex` (lignes 634 à 637 :
   `fabs` et `copysign`) ; `12_computing.tex` (ligne 1735 : GCC a tourné, sur le
   Xeon ; lignes 754 et 990 : PROFIL/BIAS et la racine carrée ; ligne 853 : la
   4.2.2) ; `abstract.tex` (lignes 27 à 29) et `introduction.tex` (lignes 67 à
   69) : « the double below it » est faux quand f(x) est un double ;
   `conclusion.tex` (lignes 39 à 44 : une phrase cassée, et un coût de 0,6 à 2
   fois, non « a few tens of percent ») ; *à décider* : le financement du
   post-doctorat de Frédéric Goualard, l'ERCIM (`copyright.tex`, lignes 30 à 33)
   ou le FNS suisse (`introduction.tex`, lignes 38 à 41), les deux venant du
   manuel de GAOL 4.
5. **`doc/differences.md` et le `ChangeLog`** : `interval::midrad(m, r)`
   (`differences.md`, lignes 1527 et 1531 ; `ChangeLog`, ligne 872) est
   `centered_form` depuis #298 (important) ; d'après le `ChangeLog` (lignes 1166
   à 1171), la cible `pdf` et `manual.yml` construiraient le manuel de GAOL 4 :
   ils ne construisent que celui de v5 (important ; `differences.md`, lignes
   2742 à 2758, le dit) ; `cos([2^60])` est montré un double plus large que ce
   que GAOL donne (`differences.md`, lignes 1390 et 1391 : `…6947p-1`) ;
   `check/non_arithmetic.cpp` et `check/trigonometric.cpp` cités au présent
   (lignes 1318 et 1319, 1475 à 1478 : `tests/`) ; `stamp-h.in` dit retiré
   (`differences.md`, lignes 2723 et 2724 ; `ChangeLog`, lignes 128 et 129 :
   celui de `gaol/` reste, PW.9) ; dans le `ChangeLog`, `manual/v5/gaol.tex`
   (lignes 211, 721, 1157 et 1161) et des chemins d'avant #308
   (`gaol_interval_{sse,fpu,neon,avx512}.cpp`, les fonctions de
   `gaol_interval_core.cpp`) ; des chiffres différents d'un registre à l'autre
   (NEON préservé : 27 ns contre 39 ou 37 ; sept produits FPU : 35,8 et 23,8
   contre 35,0 et 23,3 ; `max` et `min` « 3 to 4 times ») ; les corrections de
   PU à PY, en puces pour ces deux registres : pour PU, les sections « Pour
   `doc/differences.md` et `ChangeLog` » de #323 à #334, et `differences.md`
   (lignes 2344 à 2353 de `998e61c`), qui dit encore qu'aucune macro ne montre
   les options que #330 refuse avec GCC 12 et après. Pour PV, la section « Pour
   `doc/differences.md` et `ChangeLog` » de #335 : les noms de GAOL et de la
   norme que le lecteur lit, les nœuds d'expression qui ne se copient plus,
   `result()` qui lève l'erreur, `gaol_value()`, le texte de repli d'une borne,
   le nombre très long lu en temps linéaire et le test `bad_snprintf`.
6. **Le PDF** : régénéré dans le dernier commit (l'index de celui de `9d59e0c`
   donne « accuracy, 208 », 207 dans le PDF reconstruit) ; le rapport de
   couverture et les mesures de PJ.2 sont à refaire, PU ayant changé le code de
   `gaol/` (#323, #324 et #326 à #332 ; PJ), et PV aussi (#335 : le lecteur,
   l'écriture d'une borne, l'évaluateur). Le PDF de #335 (391 pages, régénéré
   dans cette pull request à la demande du mainteneur, avec les noms des
   chapitres 9 et 10) a le dernier paragraphe du chapitre 10 seul sur une page
   de trois lignes, suivie d'une page blanche : le mainteneur l'a laissé ; à
   resserrer au besoin, avec le PDF de PZ.
7. **Les passages du manuel que laissent les tâches PU et PV** (important ;
   avant le PDF de PZ.6) : `02_installation.tex`, la section « Interprocedural
   optimization is not offered » (lignes 1612 à 1645 : « With GCC, moreover, it
   breaks the bounds », « which the three builds keep away from CORE-MATH
   alone »), comme
   `doc/three-builds.md` par #325 et #333 (GCC 13.3 et 15.2 passent avec la LTO,
   qu'un job de `linux.yml` construit) ; la liste « No macro of the compiler
   shows the following options » (lignes 243 à 268) et `15_migration.tex`
   (lignes 1602 à 1606), comme `doc/three-builds.md` par #330 ; l'arrêt de
   `init()` quand le sens réglé vers le haut n'arrondit pas vers le haut, sur
   Debian armel et sous valgrind, comme `doc/using.md` par #331 ; le
   constructeur `expression(const expr_node&)` `explicit` (#332), dans le
   chapitre des expressions s'il montre la construction d'un nœud, et dans la
   liste des changements de l'interface du chapitre 15. Pour PV (#335), s'ils
   les montrent : les nœuds d'expression qui ne se copient plus et
   `expr_eval::result()` qui lève l'erreur, dans le chapitre des expressions et
   dans la même liste, et les quatre noms de la norme (`cancelMinus`,
   `cancelPlus`, `intersection`, `convexHull`) que
   `gaol_ieee1788::textToInterval` lit, dans le chapitre 10 (fait) et dans cette
   liste.

## PH. Les signalements à d'autres projets [attend l'accord du mainteneur]

Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à
refaire avant l'envoi et ses décisions ; les issues « À communiquer… » ne sont
pas touchées par ce ménage. **Rien n'est envoyé**, et rien ne l'est sans
l'accord explicite du mainteneur. Ordre : ce qui est prêt et qui a une valeur
pour GAOL, puis ce qui demande une réplique à écrire, puis ce qui ne demande
probablement rien.

Les brouillons sont à remettre à jour avant tout envoi (relecture du 4
novembre, `TODO_DONE.md` 3.25) : des chemins des issues ouvertes ont changé
avec #199 et #283 (`gaol/gaol_ldexp.h` est `gaol/math/gaol_ldexp.h`, dans les
issues #125, #131 et #132 ; `gaol/core_math_port.h` est
`gaol/math/gaol_core_math_port.h`, dans #127 et #131 ;
`gaol/gaol_interval_core.cpp` est `gaol/core/gaol_interval_core.cpp`, dans #131,
#132 et #133 ; `gaol/gaol_interval_neon.cpp` est
`gaol/core/gaol_interval_neon.cpp`, dans l'issue #198), et l'issue #125 donne
encore le correctif 3 « à envoyer » (l'amont l'a fait, `0d8b885a`) et le
correctif 8 « à écrire » (#144 l'a écrit, `3rd/README.md`).

1. **#125 CORE-MATH** : six correctifs de `3rd/README.md` à refaire ou à envoyer
   sur le master du jour, `0b2ae6d1` (importé le 27 octobre). L'amont a fait
   lui-même le 3 (`rsqrt.c`, `0d8b885a`) et le 4 (`sin.c`, `19e85418`) ; le 1
   (les masques de `long`) ne s'applique plus, `cos.c` ayant été réécrit et les
   lignes de `cosh.c` ayant bougé (ceux de `sinh.c` et de `cosh.c` restent),
   donc à refaire ; les 2 et 5 à 8 s'appliquent tels quels (le 8, `exp_1()`, est
   fait dans la copie de GAOL par #144). Des défauts sans correctif :
   `atan2l.c`, la borne de `ss` de `asinpi_acc()`, `return` d'une expression
   `void` dans `erfc.c`, `erfc` et `__builtin_ldexp()` ; qui envoie et comment
   (merge request sur gitlab.inria.fr ou `git format-patch` à
   core-math@inria.fr). Les deux défauts de `cr_pow()` que GAOL contournait
   (*décidé le 24 octobre*, #184), la comparaison `<` d'un NaN (invalidité) et
   le carré `rh * rh` qui déborde, sont corrigés par `c01f08c5` (5 octobre),
   dans l'import : la pull request #213 (fusionnée le 28 octobre) a retiré les
   contournements, sauf la puissance minuscule de |y| ≥ 2^1013, qu'un défaut de
   plus de `cr_pow()` impose encore (`is_exact()` calcule `y * e`, qui déborde à
   partir de |y e| = 2^1024 quand x est une puissance de 2 : `FE_OVERFLOW` pour
   une puissance qui s'annule) ; #213 a gardé le raccourci (*décidé le 28
   octobre*), qui sera retiré si l'amont corrige. Ce défaut est à proposer
   aussi : la réplique et le correctif d'une ligne (`y <= 1074 &&` devant
   `-1074 <= y * e`, car e ≠ 0 dans ce cas) sont écrits et vérifiés le 28
   octobre sur 50 896 000 appels dans les quatre arrondis (mêmes résultats,
   mêmes indicateurs sauf `FE_OVERFLOW`, levé exactement quand 2^(e y) déborde,
   contre 4 551 008 indicateurs en trop avant) ; le brouillon anglais, avec la
   réplique, est dans #125 (point 9, ajouté le 28 octobre à la demande du
   mainteneur), pas encore dans `3rd/README.md` (correctif 9). Un autre, que
   GAOL contourne sans toucher aux sources, est à proposer aussi : `cr_atan()`
   calcule `x + x` pour un NaN avant le test qui le garde, ce qui lève
   `FE_OVERFLOW` à partir de |x| = 2^1023 avec Visual C++ (`/fp:strict`), que
   `atan_lo()` et `atan_hi()` évitent en rendant les bornes de pi/2 à partir de
   |x| = 2^54 ; un import qui le corrigerait permettrait de retirer ce
   contournement. Question de #212 (fusionnée le 28 octobre) sur des arguments
   difficiles à arrondir de `cos.wc` (le fichier des pires cas de l'amont,
   absent de l'arbre) : *décidé le 28 octobre*, non, les 45 arguments de
   `cos_accurate_path()` suffisent ; elle ne voit pas les fautes de moins de
   2^-70 environ en relatif, et les deux mutants qui survivent ne changent aucun
   résultat observable. Depuis #283, `3rd/math-core/src` ne garde que
   `binary64` : ce qui vise `atan2l.c` et `powl.c` de `binary80` ou `expm1q.c`
   de `binary128` se refait contre un clone de l'amont.
2. **#126 GCC** (la comparaison silencieuse rendue signalante par
   l'if-conversion sur ARM 32 bits et POWER9 et par la vectorisation, et le
   miscompilage de GCC 12.2 sur aarch64 dans `tests/numbers.cpp` ; le bogue
   52258 est de la même famille ; le brouillon a aussi, depuis le 5 novembre
   (rien n'est envoyé), le défaut de GCC 14 que #279 a trouvé : `quiet_less(r,
   l) || r == l` fusionné en un seul `r <= l`, qui signale, sur x86-64, AArch64
   et RISC-V, avec sa réplique à écrire), **#127 mingw-w64** (`fma()` et
   `round()` de msvcrt, `fesetexceptflag()` en 32 bits, le `fma()` x87 de
   mingw-w64 11 ; deux triplets où il se trompe, qui ne sont ni dans le
   brouillon ni ailleurs dans le dépôt que `todo-notes/synthese.md`, lignes 1346
   et suivantes, sont à y ajouter),
   **#129 ITF1788** (huit cas dont le résultat attendu n'est pas le plus serré,
   et, depuis le rapport du 4 novembre, deux cas de `textToInterval` où ITF1788
   attend un intervalle pour un littéral `[l, u]` dont l est au-dessus de u, que
   GAOL rend vide selon IEEE 1788-2015, 12.12.7 ; *décidé le 4 novembre* :
   ajoutés au brouillon de #129, en anglais, pour une issue à part ; *à
   décider* depuis #290 : l'ambiguïté de la Table 10.7 que `overlap` relève, les
   spécifications d'ensembles de `meets` et de `metBy` étant vraies aussi pour
   un point d'intervalle sur une borne de l'autre, avec celles de `finishedBy`
   et de `finishes`, par exemple [2] et [1, 2], à ajouter au brouillon de #129
   ou à envoyer au groupe de la norme, si le mainteneur le veut : il n'a pas
   demandé de signalement), **#130 Boost.Interval** (`rounded_transc_opp` donne
   la borne supérieure pour l'inférieure ; issue seule ou avec une pull request,
   selon la politique de Boost) et **#175 Cygwin** (le `fma()` mal arrondi dans
   les quatre sens, le `nextafter()` qui calcule le débordement dans le sens
   d'arrondi du programme, CPack qui ne met pas le lien `README` dans l'archive,
   cause non trouvée ; mesures de CI et brouillon anglais dans l'issue, à
   rejouer avant l'envoi).
3. **#131 Microsoft** (`ldexp()` de l'UCRT, `NAN` et `INFINITY` sous
   `/fp:strict`, `sqrt()` en 32 bits, runtime de Debug, `nextafter()`, le
   `fma()` d'`ucrtbase.dll` jamais vérifié), **#132 Apple** (`ldexp()` et
   `nextafter()` de macOS ; vérifié pendant la session du 20 au 21 octobre, un
   commentaire posté dans l'issue), **#133 LLVM/Clang** (ARM 32 bits, `fcsel`
   sur arm64, `-ffast-math` et `-frounding-math` ; Clang 11 à 23.1 et le trunk
   émettent encore « overriding currently unsupported rounding mode on this
   target » pour ARMv7 à flottants matériels, GCC ARM 10.5 à 16.1 est correct,
   vérifié pendant la même session ; le refus de Clang par GAOL est donc
   intact), **#135 musl** (la conversion décimale de `printf`) et **#136 les
   auteurs du benchmark de Tang et al.** (fuite de mémoire, bornes infinies,
   entrées de FPBench, constantes décimales, README) : chaque point attend une
   réplique d'un fichier, vérifiée sur la plateforme, et un tri entre défaut,
   limite connue et comportement voulu.
4. **#134 gmpy2** (2.2.1 ; probablement rien à envoyer si 2.3.2 les a corrigés),
   **#137 libieeep1788** (probablement rien : les six cas des réciproques sont
   dans #129, comme les deux littéraux de `textToInterval`, qui viennent aussi
   de ses tests) et **#138 la glibc** (probablement rien : relancer avec la
   glibc 2.44) : vérifier, puis fermer.
5. **#128 Frédéric Goualard** : l'annonce de v5.0.0, après l'étiquette (tâche
   PJ, point 71) ; l'issue est fermée depuis le 18 octobre et le brouillon est
   dans Gmail.

## PJ. La publication de v5.0.0 (Y) [S, dans cet ordre]

En dernier : elle suppose les corrections faites et les études de PG faites
(*décidé le 19 octobre*). Les points faits sont coupés d'ici et sont dans
`TODO_DONE.md` : le 32 (la couverture, PJ.1) par #315, le 33 (les temps, PJ.2)
par #316 et #317, le manuel (PJ.5) par #294, les textes qui citent macOS 14
(PJ.6) par #313 et les restes de la synthèse (PJ.7) par #313, #314 et #318 (2.3,
3.23, 3.33, 3.34 et 3.35). À refaire au commit de la version, si le code ou les
textes ont changé depuis : le rapport de couverture (`coverage/`, 5 minutes),
les mesures de PJ.2 (25 minutes, avec le Mac M2 sur secteur et un Xeon à
AVX-512 ; `sqrt` y est plus lent de 10 à 17 % qu'au 17 octobre, cause non
isolée) et le PDF du manuel, avec la date d'édition (dans la pull request qui
change les mesures ou les textes, comme #294). *Vérifié le 12 novembre* à
`0ae1840` : rien n'est à refaire, `gaol/` n'ayant changé depuis #315 et #316 que
d'un commentaire (#318) et les sources du manuel pas depuis son PDF
(`TODO_DONE.md` 3.36) ; à revérifier après PU à PZ, qui changent le code et les
textes : PU a changé celui de `gaol/` (PZ.6). *Décidé le 13 novembre* : pas de
fusion dans `master` tant que PU à PZ ne sont pas faites (« il y a encore des
TODO. Donc ne fusionne pas dans master maintenant ») ; la version passe d'abord
de `dev` à `configure-clean`, notes retirées (process.md §13), puis de
`configure-clean` à `master` (PJ.3).

3. **34** : les recettes FetchContent récupèrent GAOL 4 : `doc/using.md`, le
   manuel et `tests/fetch_content` prennent la branche `master` de
   `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par
   défaut. `master` est GAOL 4.3.2, avec lequel les programmes de la
   documentation ne compilent pas, et il n'y a pas d'étiquette `v5.0.0`. *Décidé
   le 12 novembre* (à la place du chemin du 3 octobre, par `MATH-CORE`, que
   le mainteneur a supprimée sur GitHub le 11 novembre) : une fois
   `configure-clean` fini, fusionner `configure-clean` dans `master` (qui a deux
   commits de plus, `577c850` et `406d850`, ceux de `.claude/CLAUDE.md` que
   `configure-clean` a par `68fe887` et `195e4a2` : une vraie fusion, sans
   différence de contenu), puis étiqueter `v5.0.0`. *Décidé le 12 novembre au
   soir* (`TODO_DONE.md` 3.36) : le mainteneur ouvre et fusionne lui-même cette
   pull request, la session ne l'ouvre pas ; une fois la fusion faite, et sur
   son mot seulement, la session pose sur le commit de fusion l'étiquette
   `v5.0.0`, annotée comme `v4.3.2`, qui lance `release.yml` : la GitHub
   Release, avec les notes que génère GitHub et les trois paquets (`.deb`,
   `.pkg` de macOS, `.exe` de Windows) ; aucune date ne change, et rien n'est
   gardé de l'ancien `master` (pas de branche `gaol-4`, les étiquettes `v4.*`
   suffisent). La fusion d'essai (`git merge-tree`, le 12 novembre à `0ae1840`)
   est sans conflit et donne l'arbre de `configure-clean`. *Décidé le 12
   novembre* : les recettes FetchContent (`doc/using.md`, le manuel,
   `tests/fetch_content`) et le `git clone` de `doc/building.md` visent `master`
   tout le temps, avant comme après l'étiquette, pour avoir la dernière version
   de GAOL ; elles ne citent donc jamais `v5.0.0`, et rien n'est à y changer
   à la publication (elles disent déjà `GIT_TAG master`). *Vérifié le 8
   novembre* par l'API de GitHub (lecture seule) : `master`, la branche par
   défaut, n'a pas de protection classique, mais le jeu
   de règles « master verif » (18 octobre, actif, sans contournement, même pour
   le mainteneur) interdit de la supprimer et d'y pousser autre chose qu'une
   avance rapide, et exige une pull request (0 approbation, fusion, squash et
   rebase permis) ; aucun contrôle d'état n'est exigé, donc aucun job de macOS
   14 ; rien ne vise `configure-clean` ni les étiquettes (les quatre `v4.*`
   existent). La fusion de `configure-clean` dans `master` passe donc par une
   pull request, non par un push direct. Après v5.0.0, le développement
   continue dans `configure-clean`, et `master` ne reçoit que les versions. Les
   commits à ligne de crédit (123) restent (*décidé le 19 octobre*). La licence
   est dite (LGPL pour GAOL, MIT pour CORE-MATH) dans la section « Licences » de
   `README.md` et dans le manuel : fait. Pour #281, les cas de
   `tests/test_ibex_codac.cpp` viennent de tests d'IBEX et de Codac sous LGPL v3
   (leur `COPYING.LESSER`), alors que le `README.md` dit GAOL sous LGPL v2
   (`COPYING.LIB`) ; l'en-tête du fichier donne les copyrights et les deux
   licences sans conclure (*décidé le 2 novembre* : l'en-tête seul). *Décidé le
   5 novembre* : le mainteneur écrit aux auteurs d'IBEX et de Codac (Gilles
   Chabert, Simon Rohou) pour leur accord, et l'en-tête seul reste en attendant.
   *À décider* à leur réponse : une ligne dans la section « Licences » du
   `README.md`, voire un `COPYING.LESSER`.
4. **71 et #128** : annoncer GAOL v5.0.0 à Frédéric Goualard, l'auteur de GAOL,
   une fois l'étiquette posée : pas d'annonce publique (*décidé le 3 octobre*) ;
   la GitHub Release que `release.yml` fait à l'étiquette remplace le « pas de
   GitHub Release » du 3 octobre (*décidé le 12 novembre au soir* :
   `TODO_DONE.md` 3.36). Le message est écrit,
   en brouillon dans Gmail (objet « GAOL v5.0.0 ») ; il suppose que l'étiquette
   existe. À faire avant l'envoi : vérifier l'adresse (celle du brouillon vient
   d'un échange de 2014 et d'une liste de 2020 : elle peut avoir changé) ; le
   second paragraphe (les défauts de GAOL 4) dépasse l'annonce décidée le 3
   octobre, mais *décidé le 24 octobre* : le garder tel quel.
