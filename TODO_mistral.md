# TODO_mistral.md — Descriptif complet des tâches QA à QJ

État des lieux au **22 octobre 2026**, branche `configure-clean` (commit `ba8d212`), mis à jour le **23 octobre 2026** (puis le soir, après les réponses du mainteneur aux questions des dix dernières pull requests et l'ouverture de #165 à #171, sur `79746c2`) après les fusions de #157, #160, #161, #162 et #163 (commit `601fc63`) : les tâches faites sont dites « FAIT », les rangs de `TODO.md` et de l'issue #142 sont ceux du 23 octobre, et QJ.12 à QJ.16 reprennent les questions ouvertes des cinq pull requests.

**Sources analysées :**
- `TODO.md` (1 139 lignes après ajout des réponses, commit `ba8d212` ; 1 318 lignes après la mise à jour du 23 octobre, faite sur `601fc63`)
- Issue #49 « Suivi du TODO de GAOL v5 » (ouverte, historique, 3 commentaires de décisions du 3, 4 et 18 octobre)
- Issue #142 « Reste à faire sur GAOL v5 » (ouverte, cases à cocher, 0 commentaire)
- 20 dernières PR fusionnées : #122, #123, #124, #139, #140, #141, #143, #144, #145, #146, #147, #148, #149, #150, #151, #152, #154, #155, #156, #158 — sections « Questions ouvertes » extraites le 22 octobre ; puis #157, #160, #161, #162 et #163, fusionnées les 22 et 23 octobre (leurs « Questions ouvertes » sont en QJ.12 à QJ.16)
- #164 « Point C (QB) : textes, tests et ménage » (QB.1 à QB.6, QB.8, QB.9, QB.11 à QB.13), fusionnée le 23 octobre ; puis, le soir du 23 octobre, les pull requests #165 à #171 ouvertes pour les réponses du mainteneur (voir QK)
- Branches distantes le 23 octobre à minuit : `exp-110-gcc-windows-fma`, `exp-110b-stack-alignment`, `exp-157-ab` et `exp-win32-fma-stack` supprimées ce jour-là ; restent `todo-ai-msvc-avx512` (sans PR, gardée) et les branches des pull requests ouvertes #165 à #173 ; `todo-QB-text-doc` (#164, fusionnée), `todo-ai-fpu-kernels` (#157) et `todo-fpcr-gcc-hoisting` (#153) n'y sont plus

**Légende des tailles :** S = moins d'une demi-journée, M = un à deux jours, L = plusieurs jours ou incertain.

**Légende des états :** À faire, En cours, Fait, Clos, Reporté, Décidé.

---

## QA — Corrections du code (sûreté, plantages, bornes)

**Contexte.** Aucune borne fausse n'est connue sur environ 1,3 million de puissances entières, 550 000 `pow` réels, 600 000 autres opérations et 760 lectures de nombres (relecture du 14 octobre, revérifiée le 18 octobre à `4eaf69a`). Les défauts de cette catégorie tuent un programme (SIGFPE avec les exceptions activées, plantage), rendent un résultat que la norme dit autre, ou rompent une promesse écrite de la documentation.

### QA.1 — 79 (AB) : `FE_OVERFLOW` levé pour des résultats finis [L]

**Problème.** `FE_OVERFLOW` (ou `FE_DIVBYZERO`) est levé pour des résultats dont les bornes sont finies :
- `sqrt([1, DBL_MAX])` — le produit de vérification u·u = 2^1024 déborde
- `nth_root([DBL_MAX], 3)`
- `sin([-1, DBL_MAX])`, `tan([DBL_MAX])`, `cos` dans des tirages au hasard
- `div_rel([1], [2^-1074, 1], [1, 2])`, `sqrt_rel([DBL_MAX], [-3, 0])`
- `nth_root_rel`, `atan_rel`, `atan2_rel_x`, `atan2_rel_y` dans des tirages au hasard
- Lecture de `1e400` (ou tout nombre au-delà du plus grand double) : le `strtod()` du lexeur (`gaol_enclose_number()` dans `gaol/parser/gaol_interval_lexer.lpp`) lève `FE_OVERFLOW` ; un programme qui a activé le piège meurt à la lecture

**Promesse rompue.** `doc/using.md` (section sur les exceptions flottantes) et le manuel disent que `FE_OVERFLOW` ou `FE_DIVBYZERO` signalent une borne infinie de l'intervalle.

**Décision.** Décidé le 14 octobre : corriger le code, et non les textes. Mettre à l'échelle les vérifications de `sqrt` et des racines, et chercher les autres débordements intermédiaires (`sin`, `cos`, `tan`, `div_rel`, les fonctions `*_rel`), sans calcul qui puisse déborder pour un résultat fini. Décidé le 20 octobre d'ajouter la lecture de `1e400` au point 79 (relevé par #149).

**Fichiers concernés.** `gaol/gaol_interval_core.cpp` (vérifications de `sqrt`, racines, `div_rel`, `*_rel`), `gaol/parser/gaol_interval_lexer.lpp` (lecture).

**État.** À faire. Revérifié le 18 octobre : les six premiers cas lèvent `FE_OVERFLOW`. Touche beaucoup de fonctions, ne se voit qu'avec le piège activé près de DBL_MAX.

**Suivi.** TODO.md section AB, issue #142 case « 79 », rang 4.

---

### QA.2 — 82 (AC) : `pow_rel` et `pow_exponent_rel` sans la garde de `[±oo]` [S]

**Problème.** `powRev1([+oo], [2], R)` rend `[1, 1 + 2^-52]`, `powRev2([+oo], [2], R)` rend `[0, 2^-1074]`, `powRev1([2], [+oo], R)` rend `[DBL_MAX, +∞]`, `powRev1([-oo], [0, 1], R)` rend `[1, +∞]`, là où `pow(x, [±oo])` est vide. Ces résultats restent des encadrements justes, mais devraient être vides.

**Condition d'atteinte.** Ces arguments ne se construisent qu'avec `interval(__m128d)`, dans les builds SSE2. Rare.

**Correction.** Ajouter le test `!(l < +oo && u > -oo)` de `pow_standard_upward()` dans `pow_rel_upward()` et `pow_exponent_rel_upward()`.

**Fichiers concernés.** `gaol/gaol_interval_core.cpp`.

**État.** À faire. Non revérifié le 18 octobre.

**Suivi.** TODO.md section AC, issue #142 case « 82 », rang 11.

---

### QA.3 — 85 (AD) : plafond de l'exposant déborde un `long` de 32 bits [S]

**Problème.** `k`, `k2`, `kb`, `exponent` et `q` sont des `long` dans le lexeur. Le plafond dépasse 2^31 − 1 pour un lexème de plus de 536 870 411 caractères, en deçà des 2^29 caractères au-delà desquels le lexeur lève `std::bad_alloc`. `"1e"` suivi de 536,8 millions de 9 serait lu `[0, 2^-1074]` au lieu de `[DBL_MAX, +∞]`.

**Plateformes concernées.** Windows, i386, armhf (où `long` fait 32 bits).

**Correction.** Passer ces variables en `long long`.

**Fichiers concernés.** `gaol/parser/gaol_interval_lexer.lpp`.

**État.** À faire. Non vérifié, faute de chaîne 32 bits sur la machine.

**Suivi.** TODO.md section AD, issue #142 case « 85 », rang 11.

---

### QA.4 — 81 (AC) : `nth_root` d'un sous-normal [M] — FAIT par #158

**Problème (résolu).** `nth_root([2^-1074], 33)` rendait des bornes très loin des plus serrées promises par `doc/accuracy.md` : 1 983 doubles sous la borne inférieure, 28 785 au-dessus de la supérieure. Cause : les preuves (`ipow_hi()`, `ipow_lo()`) calculaient une puissance intermédiaire sous-normale.

**Correction (fusionnée).** Preuves par puissances mises à l'échelle, qui ne deviennent jamais sous-normales (méthode de `pown_point()`).

**Fichiers modifiés.** `gaol/gaol_interval_core.cpp` (`newton_root()`), `tests/arithmetic.cpp`, `doc/tests.md`, `doc/accuracy.md`.

**État.** Fait le 22 octobre par #158 (fusionnée, commit `02ccbcb`).

**Suivi.** TODO.md section AC, issue #142 case « 81 » cochée. Questions ouvertes de #158 reprises en QJ.

---

## QB — Textes, documentation et tests

**Contexte.** Petits, sans risque, à grouper en une ou deux pull requests. Des textes faux ou qui manquent, des tests à corriger, du ménage de descriptions de pull requests.

**Au 23 octobre.** La pull request #164 (fusionnée ce jour-là) a traité QB.1 à QB.6, QB.8, QB.9, QB.11, QB.12 et QB.13 ; QB.7 est fait par #161 ; QB.10 était fait le 20 octobre.

### QB.1 — 117 (AL) : phrase fausse de `doc/accuracy.md` sur `pow` [S] — FAIT par #164

**Problème.** La ligne de `pow` contient « 4 doubles on the special cases », phrase héritée de l'ancien `exp(y log x)`. #118 calcule les coins avec le `pow` de CORE-MATH. La ligne cite déjà les 1 347 cas d'ITF1788, les 117 boîtes d'ieee1788 et les 84 070 boîtes aux coins.

**Correction.** Supprimer la phrase (décidé le 18 octobre, question de #99).

**Fichier.** `doc/accuracy.md`.

**État.** Fait par #164 (fusionnée le 23 octobre). Revérifié le 18 octobre : la phrase y est encore.

**Suivi.** TODO.md point 117 (AL), issue #142 rang 5.

---

### QB.2 — 119 (AL) : entrée « not tested » du manuel [S] — FAIT par #164

**Problème.** Dans `manual/v5/gaol.tex`, section « Compilers and options refused », la phrase « Each of the following gave bounds not enclosing the exact results » est suivie de l'entrée de MinGW-w64 pour ARM « not tested » (remarqué par #111, déjà ainsi avant la branche).

**Correction.** Reformuler.

**Fichier.** `manual/v5/gaol.tex`.

**État.** Fait par #164 (fusionnée le 23 octobre).

**Suivi.** TODO.md point 119 (AL), issue #142 rang 5.

---

### QB.3 — 120 (AL) : piège du `namespace gaol { }` [S] — FAIT par #164

**Problème.** Un code écrit à l'intérieur de `namespace gaol { }` voit les using-declarations de `gaol` (#87) : un appel `sqrt(2.0)` y trouve `gaol::sqrt`, qui cache `::sqrt`, et ne compile plus. C'était déjà le cas avec GAOL 4, mais ce n'est écrit nulle part.

**Correction.** Ajouter aux pièges de « Common errors » (`doc/using.md` et le manuel).

**Fichiers.** `doc/using.md`, `manual/v5/gaol.tex`.

**État.** Fait par #164 (fusionnée le 23 octobre).

**Suivi.** TODO.md point 120 (AL), issue #142 rang 5.

---

### QB.4 — 87 (AD) : documenter le signe collé à un nombre incertain [S] — FAIT par #164

**Problème.** `10 - 3.56?1` est lu, mais `10-3.56?1`, `10+3.56?1` et `[1,2]+3?` lèvent `input_format_error` : le lexeur prend `-3.56?1` pour un seul lexème, alors que le manuel dit que ces littéraux se combinent aux opérateurs dans n'importe quel ordre.

**Décision.** Décidé le 19 octobre : documenter sans changer le lexeur. Une espace est nécessaire : `10 - 3.56?1`.

**Fichiers.** `manual/v5/gaol.tex`, `doc/` (probablement `doc/using.md`).

**État.** Fait par #164 (fusionnée le 23 octobre). Revérifié le 18 octobre.

**Suivi.** TODO.md point 87 (AD), issue #142 rang 5 et case « 87 ».

---

### QB.5 — 88 (AD) : documenter la lecture en temps quadratique [S] — FAIT par #164

**Problème.** Lire un long nombre prend un temps quadratique en ses chiffres : `"0."` suivi de n chiffres 3 prend 0,017 s pour n = 20 000, 0,28 s pour 80 000, 4,4 s pour 320 000 (environ 45 s attendues pour un million). La forme incertaine est 2,3 fois plus lente.

**Décision.** Décidé le 19 octobre : garder la décision du 3 octobre (« on ne fait rien »), et documenter dans le manuel et `doc/` que la lecture est quadratique en le nombre de chiffres, avec les temps ci-dessus.

**Fichiers.** `manual/v5/gaol.tex`, `doc/` (probablement `doc/using.md`).

**État.** Fait par #164 (fusionnée le 23 octobre).

**Suivi.** TODO.md point 88 (AD), issue #142 rang 5 et case « 88 ».

---

### QB.6 — 89 (AD) : documenter le format `agreeing` d'un point [S] — FAIT par #164

**Problème.** Le format `agreeing` écrit un intervalle ponctuel comme un nombre nu : `interval(4.0)` s'écrit `4.000000000000000`, quand le format `bounds` écrit `[4]`. Ni le commentaire d'`interval_format` (`gaol/gaol_interval_core.h`) ni le manuel ne le disent.

**Correction.** Documenter.

**Fichiers.** `gaol/gaol_interval_core.h` (commentaire), `manual/v5/gaol.tex`.

**État.** Fait par #164 (fusionnée le 23 octobre). Le tableau de `doc/accuracy.md` le dit déjà.

**Suivi.** TODO.md point 89 (AD), issue #142 rang 5 et case « 89 ».

---

### QB.7 — 112 (AI) : descriptions d'options du chemin AVX-512 [S] — FAIT par #161

**Problème.** Les descriptions des options listent encore « + - * / et sqrt » pour le chemin AVX-512, qui prend aussi `sqr` et les puissances entières.

**Fichiers.** `doc/building.md`, `CMakeLists.txt`, `meson_options.txt`, `configure.ac`, `gaol/gaol_configuration.h.in`, `gaol/gaol_configuration_meson.h.in`.

**État.** Fait par #161 (fusionnée dans la nuit du 22 au 23 octobre) : les descriptions disent « +, -, *, /, sqr, sqrt and, with FMA, the integer powers beyond 1 and -1 » (les puissances entières n'empruntent le chemin que pour n hors de {-1, 0, 1} et avec FMA), dans `CMakeLists.txt`, `meson_options.txt`, `configure.ac`, les trois gabarits de `gaol_configuration.h`, `doc/building.md`, `doc/three-builds.md`, le manuel et l'exemple 13.

**Suivi.** TODO.md point 112 (AI), retiré du rang 5 de `TODO.md` et de #142 le 23 octobre.

---

### QB.8 — 91 (AE) : `tests/refused_options.sh` se dit sauté [S] — FAIT par #164

**Problème.** Le script reconnaît GCC ou Clang en prétraitant le source du test avec `-dM -E`. Si les en-têtes de GAOL manquent, g++ n'écrit rien : le script dit « SKIP: neither GCC nor Clang » (77) au lieu d'échouer. Vérifié avec `GAOL_CXXFLAGS="-std=c++17 -frounding-math"`.

**Correction.** `$GAOL_CXX $GAOL_CXXFLAGS -dM -E -x c++ /dev/null`.

**Fichier.** `tests/refused_options.sh`.

**État.** Fait par #164 (fusionnée le 23 octobre). Relu le 18 octobre : le script prétraite encore le source du test.

**Suivi.** TODO.md point 91 (AE), issue #142 rang 6 et case « 91 ».

---

### QB.9 — 94 (AE) : liste de macros de l'audit incomplète [S] — FAIT par #164

**Problème.** `.github/audit/make_probe.py` sonde `GAOL_RND_SSE_REGISTER`, que `.github/audit/compare.py` ne compare pas (les macros d'AVX-512 que le 94 citait le 14 octobre sont depuis dans les deux listes).

**Correction.** Ajouter `GAOL_RND_SSE_REGISTER` aux `MACROS` de `compare.py`.

**Fichiers.** `.github/audit/compare.py`, `.github/audit/make_probe.py`.

**État.** Fait par #164 (fusionnée le 23 octobre). Relu le 18 octobre.

**Suivi.** TODO.md point 94 (AE), issue #142 rang 6 et case « 94 ».

---

### QB.10 — 123 (AL) : retirer les lignes de crédit des descriptions de PR [S] — FAIT le 20 octobre

**Problème.** Règle : aucune ligne qui crédite Claude ou Claude Code dans un commit, une issue, un commentaire ou une description.
- Les descriptions de #15 à #24 se terminent par « Generated with Claude Code »
- Celles de #60 à #63 et un commentaire de chacune de #58 et #60 à #63 par « Generated by Claude Code »
- 24 commits de `configure-clean` (14-21 septembre, dont 16 dans `master`) portent « Co-Authored-By: Claude Opus 5 », et 6 commits de #99 portent « Co-authored-by: Jordan08 »

**Décision.** Décidé le 19 octobre : laisser les commits sans réécrire l'historique. Les lignes des descriptions et des commentaires de pull requests sont retirées (fait le 20 octobre, sans pull request).

**Correction.** `gh pr edit` et `gh api` pour #15 à #24, #58, #60 à #63.

**État.** Fait le 20 octobre, sans pull request (descriptions de #15 à #24 et #60 à #63, commentaires de #58 et #60 à #63) ; les commits restent.

**Suivi.** TODO.md point 123 (AL), rang 1 de `TODO.md` (décidé, fait).

---

### QB.11 — 121 (AL) : `.continue/` dans `.gitignore` [S] — DÉCIDÉ, FAIT par #164

**Problème.** #103 a retiré `.continue/rules` du dépôt ; si l'extension Continue de l'éditeur le relit en local, le fichier réapparaîtra comme non suivi.

**Décision (Q2, 22 octobre 2026).** Ajouter `.continue/` au `.gitignore`.

**État.** Décidé ; fait par #164 (fusionnée le 23 octobre).

**Suivi.** TODO.md point 121 (AL), issue #142 rang 7, réponse Q2.

---

### QB.12 — 122 (AL) : `gaol_performance` en C++14 [S] — FAIT par #164

**Problème.** `tests/CMakeLists.txt` donne `CXX_STANDARD 17` à `gaol_performance`, alors que `tests/performance.cpp` compile en C++14 (pas de littéral hexadécimal). De même dans `tests/Makefile.am` et `tests/meson.build`.

**Correction.** Passer à C++14 dans les trois build systems.

**Fichiers.** `tests/CMakeLists.txt`, `tests/Makefile.am`, `tests/meson.build`.

**État.** Fait par #164 (fusionnée le 23 octobre). Hors du reste, sans effet.

**Suivi.** TODO.md point 122 (AL), issue #142 rang 7.

---

### QB.13 — `/W4 /WX` en Debug (AI, #148) : C4702 inatteignable [S] — FAIT par #164

**Problème.** `textToInterval()` de `gaol_interval.cpp` a un `return` après `gaol_ERROR()`, que Visual C++ 19.44 et 19.51 disent inatteignable à `/Od` (C4702, vu sur Compiler Explorer, sur le fichier assemblé avec ses en-têtes). Aucun job ne compile en `/W4 /WX` en Debug.

**Correction.** Corriger le code (retirer le `return` inatteignable ou restructurer), et ajouter un job CI qui compile en `/W4 /WX` en Debug.

**Fichiers.** `gaol/gaol_interval.cpp`, `.github/workflows/windows.yml` (nouveau job).

**État.** Fait par #164 (fusionnée le 23 octobre) (un job `/W4 /WX` en Debug dans `windows.yml`, `/wd4702` pour le parser de bison dans `CMakeLists.txt`).

**Suivi.** TODO.md rang 8, issue #142 rang 8, relevé par #148.

---

## QC — Les trois builds (CMake, autotools, meson)

**Contexte.** Principe 6 de `process.md` : les mêmes bibliothèque, en-têtes et options partout.

### QC.1 — 112 (AI) : second `cmake` fait passer `GAOL_PREFER_AVX512` de ON à OFF [S] — FAIT par #161

**Problème.** Un second `cmake` dans le même dossier fait passer `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING` : `DEFINED` voit l'entrée de cache du premier passage et la prend pour une demande de la ligne de commande, alors que le commentaire du `CMakeLists.txt` dit que le défaut se dérive à chaque passage. Contourné par `-UGAOL_PREFER_AVX512` dans `doc/compare/code/setup.sh`.

**Correction.** Corriger le `CMakeLists.txt`, avec un test qui configure deux fois le même dossier.

**Fichiers.** `CMakeLists.txt`, `tests/` (nouveau test).

**État.** Fait par #161 (fusionnée dans la nuit du 22 au 23 octobre) : l'entrée `GAOL_PREFER_AVX512` garde la valeur dérivée, l'entrée interne `GAOL_PREFER_AVX512_DERIVED` la note, et une valeur donnée reste jusqu'à `-UGAOL_PREFER_AVX512` ; `tests/prefer_avx512_reconfigure.cmake` configure plusieurs fois le même dossier, échoue avec le `CMakeLists.txt` de `configure-clean`, et quatre mutants de la correction sont tués. Trouvé par #124, confirmé par la relecture.

**Suivi.** retiré de `TODO.md` et de #142 le 23 octobre ; question ouverte de #124 ; questions ouvertes de #161 : QJ.14.

---

### QC.2 — 125 (AM) : `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT` — CLOS

**Problème (clos).** Ces macros restaient définies par `<limits.h>`, sans valeur par défaut dans les trois gabarits de `gaol_configuration.h`. Un `int` qui n'aurait ni 4 ni 8 octets les laisserait non définis sous `-Wundef`.

**Décision (Q3, 22 octobre 2026).** Clos. `gaol/gaol_config.h` (l'en-tête installé) a déjà des valeurs par défaut conditionnelles :
```c
#ifndef GAOL_SIZEOF_INT
#  if UINT_MAX == 0xFFFFFFFFu
#    define GAOL_SIZEOF_INT 4
#  elif UINT_MAX == 0xFFFFFFFFFFFFFFFFu
#    define GAOL_SIZEOF_INT 8
#  endif
#endif
#ifndef GAOL_SIZEOF_LONG_LONG_INT
#  if ULLONG_MAX == 0xFFFFFFFFFFFFFFFFull
#    define GAOL_SIZEOF_LONG_LONG_INT 8
#  endif
#endif
```
Les trois gabarits de configuration (`gaol/gaol_configuration.h.in`, `cmake/gaol_configuration.h.in`, `configure.ac`, `meson.build`) ne les définissent pas, et l'en-tête installé suffit. Retirer le point 125 de TODO.md et de l'issue #142.

**État.** Clos.

**Suivi.** TODO.md point 125 (AM), issue #142 rang 9, réponse Q3.

---

### QC.3 — 95 (AE) : détections qui compilent dans un build et lient dans les autres [M]

**Problème.** La sonde de la cible AVX-512 lie avec CMake, mais ne fait que compiler avec autotools (`AC_COMPILE_IFELSE`) et meson, alors que `__builtin_cpu_supports` demande `__cpu_model` à l'édition de liens. La sonde de FMA, à l'inverse, ne vérifie que le C++ avec CMake, et le C et le C++ avec édition de liens dans les deux autres. Non vérifié sur une chaîne où le résultat changerait.

**Correction.** Lier, et vérifier les deux langages, dans les trois builds.

**Fichiers.** `CMakeLists.txt`, `configure.ac`, `meson.build`.

**État.** À faire. Non revérifié le 18 octobre.

**Suivi.** TODO.md point 95 (AE), issue #142 rang 10 et case « 95 ».

---

### QC.4 — 92 (AE) : drapeaux C différents dans les trois builds [M]

**Problème.** CMake donne aux sources C `-msse2 -msse3 -fvisibility=hidden`, autotools et meson au C++ seulement. `-ffloat-store` va au C et au C++ avec CMake et meson, au C++ seul avec autotools. Le C est gnu99 avec CMake, c99 ou gnu11 avec meson, celui du compilateur avec autotools. `.github/audit` ne compare que les drapeaux de `gaol_interval.cpp`.

**Correction.** Les mêmes drapeaux C partout, et un source C dans `probe_build.py`.

**Fichiers.** `CMakeLists.txt`, `configure.ac`, `meson.build`, `.github/audit/probe_build.py`.

**État.** À faire. L'effet visible du 14 octobre (symboles `get_inexact` et `clear_inexact` exportés) a disparu avec le code mort supprimé le 15 octobre.

**Suivi.** TODO.md point 92 (AE), issue #142 rang 10 et case « 92 ».

---

## QD — Études de vitesse (groupe E, toutes avant v5.0.0 sauf QE)

**Contexte.** Décidé le 19 octobre : toutes les études de vitesse sont faites avant v5.0.0 (sauf AJ.113, voir QE). Ordre : ce qui peut changer la suite, puis l'écart mesuré le plus grand sur les builds par défaut, puis les builds avec `GAOL_PRESERVE_ROUNDING`.

**Mesures de référence (CI de #124).** Ligne de 13 opérations `+ - * /`, en ns : 33 sur Linux x86-64 (Ubuntu 24.04), 36 sur macOS 15 x86-64, 33 à 58 avec MinGW-w64 et MSYS2, 64 à 93 sur macOS arm64, 100 à 168 avec Visual C++ x64 et arm64 et clang-cl, 1 100 à 1 200 avec Visual C++ 32 bits. Voir QD.6 (le 111, fait par #160) pour la fiabilité de ces temps.

### QD.1 — 106 (AI) : noyaux sans branche pour les intervalles FPU [L] — FAIT par #157 (reste : Visual C++ arm64, mesures POWER, s390x, RISC-V)

**Problème.** Les intervalles FPU (ARM 32 bits, POWER, s390x, RISC-V, Visual C++ arm64, Windows x86 32 bits avec Visual C++/clang-cl, builds avec `GAOL_SIMD=OFF`) gardent le `switch` sur les signes de GAOL 4. `x * y` prend 6,6 à 16,3 ns, `x / y` 4,2 à 8,2, `sqr` 4,1 à 10,4, contre 2,1 à 4,1, 2,1 à 5,2 et 2,1 à 3,1 avec les noyaux SSE2 de #124.

**Travail fait.** PR #157 (fusionnée dans la nuit du 22 au 23 octobre, branche `todo-ai-fpu-kernels` supprimée) : noyaux scalaires sans branche pour `x * y`, `x / y`, `x % y`, `sqr(x)`, et correction de `x - y` (qui prenait 3 fois `x + y`). Mesures sur M2 : `x * y` de signes aléatoires 12,4 → 3,3 ns, Shekel 5 298 → 138 ns, mais sept `*` de signes prévisibles 23,8 → 35,8 ns (+50 %), rien de gagné avec `GAOL_PRESERVE_ROUNDING`.

**Décision (Q1, 22 octobre 2026).** Mesurer d'abord. Ne pas fusionner le produit tel quel ; lancer les mesures du produit sur armhf (conteneur Debian, vrai ARM 32 bits VFPv4 compilé par GCC), POWER, s390x et RISC-V avant de décider. La pull request reste en brouillon.

**Suite (22 octobre au soir).** Les mesures en alternance sur la CI ont été faites (armhf natif, Visual C++ et clang-cl x86 et arm64, Ubuntu et macOS en FPU : `x * y` de signes aléatoires de 1,23 à 3,47 fois plus rapide ; une ligne de sept `*` de signes prévisibles à 0,57-0,58 de la vitesse d'avant avec GCC sur Ubuntu et clang-cl arm64) ; le mainteneur a décidé de garder tous les noyaux, le produit compris, et #157 est fusionnée. Le signe d'une borne nulle d'un produit est non spécifié avec les intervalles FPU (GCC 13 pour POWER9 compile le maximum en `xsmaxcdp`, qui rend le premier de deux zéros de signes opposés). POWER, s390x, RISC-V et MinGW-w64 arm64 ne sont pas mesurés.

**Fichiers.** `gaol/gaol_interval_fpu.cpp`, `tests/arithmetic.cpp`, `tests/performance.cpp`, `doc/using.md`, `doc/accuracy.md`, `doc/three-builds.md`, `doc/tests.md`.

**État.** Fait pour les intervalles FPU par #157. Reste : offrir NEON à Visual C++ arm64, et mesurer les noyaux sur POWER, s390x, RISC-V et MinGW-w64 arm64.

**Suivi.** TODO.md point 106 (AI), rang 13 (le reste), réponse Q1. Questions ouvertes de #157 : QJ.12.

---

### QD.2 — 110 (AI) : GCC pour Windows, `pow(x, 3)`, `log` et `cos` 5 à 10 fois plus lents que Clang [M] — FAIT par #163

**Problème.** MinGW-w64 14.2 et 15.2 et MSYS2 UCRT64 avec GCC 16, contre MSYS2 CLANG64 : `pow(x, 3)` 44 à 57 ns contre 4,1 à 9,0 ; `log` 78 à 106 contre 19 à 35 ; `cos` 221 à 271 contre 32 à 51 ; `exp` 39 à 47 contre 24 à 35. CMake ne donne pas `-mfma` à GCC pour Windows (« Fused multiply-add instructions left out: GCC for Windows does not align the stack for AVX »), de sorte que `pown` n'est pas calculé par paires et CORE-MATH appelle un `fma()` logiciel.

**Piste.** Mesurer un build de GCC pour Windows avec `-mfma` donné à la main, `rounding_direction` en particulier devant passer, puis voir si le défaut d'alignement se contourne (`-mstackrealign`, `-mprefer-vector-width=128`) pour que `GAOL_FMA` y soit pris.

**Travail fait.** Trois exécutions de la CI sur des branches d'expérience ont mesuré `-mfma` et `-mfma -mprefer-vector-width=128` sur MSYS2 UCRT64 (GCC 16.2) : `pow(x, 3)` de 52,3 à 8,7 ns, `log` de 106,4 à 33,6, `cos` de 260,5 à 55,4 ; avec `-mfma` seul, 30 598 lignes `%ymm` dans les objets, avec `-mprefer-vector-width=128` aucune. PR #163 (fusionnée le 23 octobre) : GCC pour Windows x86-64 reçoit `-mfma -mprefer-vector-width=128` dans les trois builds (`GAOL_FMA`, `--enable-fma`, `-Denable-fma`), `.github/scripts/check-gcc-windows-fma.sh` fait échouer la CI si un objet a une instruction `%ymm` ou `%zmm` ou si les `vfmadd` manquent, et deux défauts de `GAOL_PRESERVE_ROUNDING` avec mingw-w64 13 et plus sur x86-64 sont corrigés (`fesetround()` efface les indicateurs de MXCSR, `fegetround()` joint par OU les champs RC des deux unités) : `gaol/gaol_fpu.h` lit et écrit lui-même les registres (`GAOL_RND_MINGW64_X87`), et quatre entrées de plus du job MSYS2 exécutent ce chemin. Les deux branches d'expérimentation (elles restent sur GitHub) :
- `exp-110-gcc-windows-fma` (1 commit `a326b03`) : expérience `-mfma` à la main, et si armhf tourne en natif
- `exp-110b-stack-alignment` (3 commits `b3ca337`, `8e6b01b`, `a326b03`) : `cstdio`/`cstdint` pour `std::printf`, alignement de pile des objets de 256 bits avec GCC pour Windows

**État.** Fait par #163 (fusionnée le 23 octobre).

**Suivi.** retiré de `TODO.md` et de #142 le 23 octobre ; questions ouvertes de #163 : QJ.16.

---

### QD.3 — 109 (AI) : `GAOL_PRESERVE_ROUNDING` à 24 ns par opération avec Visual C++ x64 [M]

**Problème.** Depuis #148, `GAOL_PRESERVE_ROUNDING` coûte 23,8 ns par opération avec Visual C++ x64 (43 à 87 avant, 2,5 sans). Les macros de #124 (`GAOL_RND_ENTER()` n'écrit que MXCSR, une fois à l'entrée et une fois à la sortie) demandent des intervalles SSE2 et `__FMA__`, que `cl` ne définit pas : il y reste `fegetround()`, `fesetround()` et deux écritures séparées. clang-cl, qui définit `__FMA__` avec `/arch:AVX2`, prend 22,6 ns pour `x + y` avec `GAOL_PRESERVE_ROUNDING` et le chemin AVX-512 refusé (518 ns pour la ligne de 15 `+`), sans explication.

**Correction (décidée le 20 octobre).** Dans une pull request qui suit #148 : la condition des macros avec `_M_X64` et `__AVX2__` pour `cl`, qui ne définit pas `__FMA__` ; et chercher ce qui coûte 22,6 ns avec clang-cl.

**Fichiers.** `gaol/gaol_fpu.h`, `gaol/gaol_fpu_fenv.h` (macros `GAOL_RND_ENTER_SSE`).

**État.** À faire (PR à ouvrir après #148, déjà fusionnée).

**Suivi.** TODO.md point 109 (AI), issue #142 rang 15. Question ouverte de #148 : la condition `_M_X64` avec `__AVX2__` du 109 pour `cl` reste à décider (décidé le 20 octobre).

---

### QD.4 — 105 (AH) : `-mbranches-within-32B-boundaries` [M]

**Problème.** L'erratum des sauts des processeurs Skylake à Comet Lake déplace une opération jusqu'à 1 ns selon le seul placement du code (#122 : `x * y`, au code inchangé, passe de 4,96 à 4,69 ns ; `[1] / p`, de 7,0 à 8,1 ns quand seul `sqr()` change).

**À mesurer.** Le gain sur le Xeon E3-1220 v6, le coût sur l'i7-1185G7 (qui n'a pas l'erratum), et ce que les trois builds peuvent donner aux compilateurs qui prennent l'option.

**État.** À faire. Décidé le 14 octobre à la fin du point AA.

**Suivi.** TODO.md point 105 (AH), issue #142 rang 16.

---

### QD.5 — 116 (AI) : écarts de temps sans cause trouvée [S]

**Problème.** `cos(x)` 6 ns plus lent et `sin(x)` 2,4 ns plus rapide dans `gaol_performance` après #100 (Clang 18, i7-1185G7), alors que le code de `cos()` ne diffère que par l'ordre de deux branches. `asinh_rel` et `nth_root_rel` plus lents de 16 % et 13 % avec Clang 18 après #117, avec moins d'instructions. `nth_root(x, -2)` à 22,9 ns au lieu de 18,2 avec `GAOL_PRESERVE_ROUNDING` (205 instructions contre 266). `perf` n'est pas permis sur la machine.

**Correction.** Les mesures d'alors sont faites avec Clang 18 ; les refaire avec Clang 21, le compilateur des mesures depuis le 13 octobre, avant de chercher.

**État.** À faire.

**Suivi.** TODO.md point 116 (AI), issue #142 rang 17.

---

### QD.6 — 111 et 115 (AI) : mesurer par la CI, lancer `run_lines.sh` [M] — DÉCIDÉ, FAIT par #160

**Problème (111).** Les temps de la CI ne se comparent pas d'un passage à l'autre : les runners sont des machines virtuelles partagées (`x * y` de Visual C++ x64 prend 7,9 puis 16,3 ns, `sqrt` de MSYS2 avec GCC 16 6,8 puis 170 ns, entre #124 à `1a15fc8` et à `c6a3d25`, au code inchangé). Seuls des écarts de plusieurs fois valent, et l'étape « Performance » ne fait échouer aucun job.

**Problème (115).** `make perf` et l'étape « Performance » de la CI ne lancent pas les dix lignes de `run_lines.sh` : seul `gaol_performance` en a cinq.

**Décision (Q6, 22 octobre 2026).** Mesurer par la CI deux builds alternés dans le même job (par exemple `configure-clean` et la branche), comme `doc/compare` alterne ses programmes.

**Fichiers.** `.github/workflows/*.yml`, `tests/Makefile.am` (cible `perf`), `tests/run_lines.sh`.

**État.** Fait par #160 (fusionnée dans la nuit du 22 au 23 octobre) : `.github/workflows/performance.yml` (sur `pull_request`, avec un filtre de chemins) construit et mesure la base de la pull request (le premier parent du commit de fusion) et sa tête dans le même job, trois passages alternés, sur Ubuntu 24.04 (GCC, Clang, GCC avec `GAOL_PRESERVE_ROUNDING`), macOS 26 arm64, Visual Studio 2026 (x64, x86, x64 avec la direction préservée) et MSYS2 (UCRT64, CLANG64) ; `.github/scripts/performance-ab.sh` et `compare-performance.py` écrivent le tableau base / pull request / rapport, sans qu'aucun temps ne fasse échouer un job ; un test des outils (19 tests) ; `make perf` lance les dix lignes de `run_lines.sh` dans les trois builds, et sur macOS. `performance.yml` a réussi sur #160, #162, #163 et #164.

**Suivi.** retirés de `TODO.md` et de #142 le 23 octobre, réponse Q6 ; questions ouvertes de #160 : QJ.13.

---

### QD.7 — 112 (AI), le reste : fonctions élémentaires et test des bits [M]

**Problème.** Avec `GAOL_PRESERVE_ROUNDING`, les fonctions élémentaires paient encore deux écritures de MXCSR, jusqu'à 27 % de plus que sans (`exp`). CORE-MATH calcule avec le sens matériel, aucun arrondi embarqué n'y est possible. Un mode de CORE-MATH où le sens serait un paramètre les supprimerait, au prix de modifier `3rd/math-core`.

**Décision (Q15c, 22 octobre 2026).** Non, ne pas modifier `3rd/math-core` pour passer le sens d'arrondi en paramètre.

**Reste.** Un test des bits du signe de zéro de `x * d` (intervalle par un double) : avec `GAOL_PRESERVE_ROUNDING` et AVX-512, `x * d` et `x *= d` donnent le signe opposé pour une borne nulle de 344 résultats sur 5,7 millions entre le chemin AVX-512 (`[-0, +0]`) et le chemin SSE2 (`[+0, -0]`). Décidé le 18 octobre (point AF, #140) : exception acceptable au « bit for bit », dite dans la documentation ; il ne reste que, si l'on veut, un test qui compare les bits.

**Fichiers.** `3rd/math-core` (non modifié), `tests/rounding_direction.cpp` (test bits à ajouter si voulu).

**État.** Le défaut de CMake et les descriptions d'options du 112 sont faits par #161 (QC.1, QB.7). Le test des bits, que #161 n'a pas fait, est décidé le 23 octobre (saut à l'exécution sans AVX-512) et ouvert dans #171 : `x * d` et `x *= d` de `[+0, -0]` pour huit doubles dans chaque direction, `[-0, +0]` avec le chemin AVX-512 et `[+0, -0]` avec SSE2 ; 48 vérifications qui échouent quand on inverse l'attente. Reste, pour ce point, les fonctions élémentaires avec `GAOL_PRESERVE_ROUNDING`.

**Suivi.** TODO.md point 112 (AI), issue #142 rang 18, réponse Q15c.

---

### QD.8 — 108 (AI) : Visual C++ 32 bits, 80 ns par opération [M] — FAIT par #156

**Problème (résolu).** `fegetround()` et la lecture de MXCSR coûtaient environ 80 ns sur x86 32 bits avec Visual C++, dès le build par défaut (`x + y` 80,8 à 83,5 ns, `x * y` 87,8 à 98, une ligne de 15 `+` 1 211 à 1 307 ns). La sonde du sens d'arrondi par une addition y était écartée (`GAOL_RND_PROBE` non défini sur x86 32 bits).

**Correction (fusionnée).** Lecture des deux registres de contrôle (x87 et SSE) en ligne, sans appeler `fegetround()` (`fnstcw` et `stmxcsr` en ligne, `GAOL_RND_X86_DIRECT`). Job CI ajouté « Visual Studio 2026 x86 Release » avec la direction préservée.

**Fichiers modifiés.** `gaol/gaol_fpu.h`, `gaol/gaol_fpu_fenv.h`, `tests/rounding_direction.cpp`, `tests/performance.cpp`, `doc/using.md`, `doc/tests.md`, `.github/workflows/windows.yml`.

**État.** Fait le 20 octobre par #156 (fusionnée, commit `c89c2ac`).

**Suivi.** TODO.md point 108 (AI), issue #142 rang 14. Suites décidées en Q8 (voir QJ.8).

---

## QE — Étude AJ.113 : se passer du changement du sens d'arrondi

**Contexte.** Question du mainteneur, le 18 octobre : avec les algorithmes de base d'aujourd'hui, le changement du sens d'arrondi est-il toujours nécessaire, les calculs ne seraient-ils pas valables sans lui, et dans quels opérateurs ou fonctions élémentaires l'est-il ?

### QE.1 — 113 (AJ) : étudier des noyaux qui laissent le sens d'arrondi au plus proche [L] — REPORTÉE

**Contenu de l'étude.** Les algorithmes actuels supposent l'arrondi vers le haut. Ce qu'il faudrait, c'est un arrondi dirigé, non un changement de sens :
- Chemin AVX-512 (`GAOL_PREFER_AVX512`) : `+ - * / sqrt sqr` et puissances entières n'en changent pas, l'arrondi étant dans l'instruction ; `GAOL_PRESERVE_ROUNDING` y coûte 1,0 à 1,5 fois
- Chemin SSE2 seul : le sens vers le haut est indispensable, `GAOL_PRESERVE_ROUNDING` y coûte deux écritures de MXCSR par opération
- Fonctions élémentaires et `pow` le demandent partout, CORE-MATH ne prenant pas le sens en paramètre

**Formules connues (ni écrites ni testées).**
- `+` et `-` : TwoSum donne l'erreur exacte, d'où les mêmes bornes bit à bit
- `*` : l'erreur `fma(a, b, -p)` est exacte hors sous-dépassement
- `/` et `sqrt` : le reste `fma(-q, b, a)` ou `fma(s, s, -x)` donne de quel côté est le résultat arrondi
- Fonctions élémentaires : seules des bornes valables mais plus larges, `[previous_float(r), next_float(r)]`, 2 ulp au lieu de 1
- `pown` : le dernier `fma(K, h, l)` est un arrondi vers le haut, à remplacer par `next_float`

**À mesurer.** Le gain (les `double` du programme ne sont plus arrondis vers le haut de `gaol::init()` à `gaol::cleanup()`, `GAOL_PRESERVE_ROUNDING` devient inutile) et le coût (ces noyaux font plus d'opérations, donc probablement plus lents). Un prototype de `+ - * / sqrt` au plus proche avec FMA, comparé aux noyaux actuels en résultats (bit à bit, `tests/rounding_direction.cpp`) et en temps (`doc/compare`), suffit à décider.

**Sur arm64.** #146 a ramené `GAOL_PRESERVE_ROUNDING` de 39 à 27 ns par opération sur un M2. Passer sous 5 ns demande de ne pas écrire FPCR.

**Décision (Q7, 22 octobre 2026).** Non, après v5.0.0. L'étude est reportée après la version ; le comportement actuel (changement du sens d'arrondi) est documenté.

**État.** Reportée après v5.0.0.

**Suivi.** TODO.md point 113 (AJ), issue #142 rang 12, réponse Q7.

---

## QF — Signalements à d'autres projets (issues #125 à #138)

**Contexte.** Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à refaire avant l'envoi et ses décisions. **Rien n'est envoyé**, et rien ne l'est sans l'accord explicite du mainteneur. Ordre : ce qui est prêt et qui a une valeur pour GAOL, puis ce qui demande une réplique à écrire, puis ce qui ne demande probablement rien.

### QF.1 — #125 CORE-MATH : huit correctifs prêts

**Contenu.** Huit correctifs de `3rd/README.md` à refaire sur le master du jour (le 1 entre en conflit avec des changements récents de `sinh.c` et `cosh.c` ; le 8, `exp_1()`, fait dans la copie de GAOL par #144, s'applique tel quel au master du 19 octobre). Défauts sans correctif : `atan2l.c`, la borne de `ss` de `asinpi_acc()`, `return` d'une expression `void` dans `erfc.c`, `erfc` et `__builtin_ldexp()`. Questions : qui envoie et comment (merge request sur gitlab.inria.fr ou `git format-patch` à core-math@inria.fr). Après un import d'un CORE-MATH d'après `c01f08c5` : retirer le contournement du `<` de `cr_pow` (`pow_rounded_up()` et `pow_rounded_dn()`).

**État.** À faire. En attente de l'accord du mainteneur.

---

### QF.2 — #126 GCC, #127 mingw-w64, #129 ITF1788, #130 Boost.Interval

- **#126 GCC** : la comparaison silencieuse rendue signalante par l'if-conversion (ARM 32 bits, POWER9) et par la vectorisation, et le miscompilage de GCC 12.2 sur aarch64 (`tests/numbers.cpp`) ; le bogue 52258 est de la même famille.
- **#127 mingw-w64** : `fma()` et `round()` de msvcrt, `fesetexceptflag()` en 32 bits, et le `fma()` x87 de mingw-w64 11.
- **#129 ITF1788** : huit cas dont le résultat attendu n'est pas le plus serré.
- **#130 Boost.Interval** : `rounded_transc_opp` donne la borne supérieure pour l'inférieure ; issue seule ou avec une pull request (politique de Boost sur les contributions).

**État.** À faire. En attente de l'accord du mainteneur.

---

### QF.3 — #131 Microsoft, #132 Apple, #133 LLVM/Clang, #135 musl, #136 benchmark de Tang et al.

Chacun attend une réplique d'un fichier, vérifiée sur la plateforme, et un tri entre défaut, limite connue et comportement voulu.

- **#131 Microsoft** : `ldexp()` de l'UCRT, `NAN` et `INFINITY` sous `/fp:strict`, `sqrt()` en 32 bits, runtime de Debug, `nextafter()`, le `fma()` d'`ucrtbase.dll` jamais vérifié.
- **#132 Apple** : `ldexp()` et `nextafter()` de macOS.
- **#133 LLVM/Clang** : ARM 32 bits, `fcsel` sur arm64, `-ffast-math` et `-frounding-math`.
- **#135 musl** : la conversion décimale de `printf`.
- **#136 benchmark de Tang et al.** : fuite de mémoire, bornes infinies, entrées de FPBench, constantes décimales, README.

**État.** À faire. En attente de l'accord du mainteneur.

---

### QF.4 — #134 gmpy2, #137 libieeep1788, #138 la glibc : probablement rien à envoyer

- **#134 gmpy2** (2.2.1 ; probablement rien à envoyer si 2.3.2 les a corrigés)
- **#137 libieeep1788** (probablement rien : les six cas sont dans #129)
- **#138 la glibc** (probablement rien : relancer avec la glibc 2.44)

**Correction.** Vérifier, puis fermer.

**État.** À vérifier, puis fermer.

---

### QF.5 — #128 Frédéric Goualard : l'annonce de v5.0.0

**Contenu.** L'annonce de v5.0.0 à Frédéric Goualard, après l'étiquette (point Y). L'adresse du brouillon vient d'un échange de 2014 et d'une liste de 2020, à vérifier ; garder ou couper le paragraphe sur les défauts de GAOL 4. Le message est écrit, en brouillon dans Gmail.

**État.** À faire, après l'étiquette `v5.0.0`.

**Suivi.** TODO.md point 71 (Y) et #128, issue #142 rang 28.

---

## QG — La version v5.0.0 (Y), en dernier

**Contexte.** En dernier, dans cet ordre. La version suppose les corrections faites et les études du groupe E faites (décidé le 19 octobre, rang 2). La licence est dite (LGPL pour GAOL, MIT pour CORE-MATH) dans la section « Licences » de `README.md` et dans le manuel : fait.

### QG.1 — La pull request de synthèse (Y) : `ChangeLog` et `doc/differences.md`

**Contenu.** `ChangeLog` (une entrée du 27 septembre, touchée le 4 octobre) et `doc/differences.md` (28 septembre, puis des commits directs des 16 et 18 octobre) ne reprennent pas le travail du 29 septembre au 18 octobre. Les textes proposés sont dans `todo-notes/synthese.md` (section 1) et dans les 52 descriptions de pull requests qui ont une section « Pour `doc/differences.md` et `ChangeLog` » : #71, #76, #77, #83 à #97, #99, #100, #105, #108, #111, #113, #114, #117 à #124, #139, #140, #141, #144, #145, #146, #147, #148, #149, #150, #151, #155, #156, #157, #158 et #160 à #163 (#160 n'a rien à y mettre). À écrire d'après leurs descriptions pour K, L et `make distclean` (#72 à #75) ; d'après les commits pour G (c7915a7, ccf694a), le 4 octobre et les changements d'interface du 15 au 18 octobre sans pull request. Le point G n'a pas de texte dans la description de #91. Les chiffres de `todo-notes/synthese.md` à harmoniser.

**État.** À faire.

**Suivi.** TODO.md rang 23, issue #142 rang 23.

---

### QG.2 — `examples/examples.md` et le manuel (Y)

**Contenu.** `examples/examples.md` : marquer **Fixed** les n° 2, 3, 4, 5, 7, 8, 10, 12, 15, 16, 19, 20, 21 et 22 de la section 5 et de l'annexe B, le n° 10 comme corrigé par la suppression des constructeurs à partir de chaînes (#30), et par #147 pour `parse_interval(nullptr)`. Le manuel : une seule régénération de `manual/v5/gaol.pdf` (la dernière date du 28 septembre ; ne jamais le commiter dans une branche de point), après avoir corrigé dans `gaol.tex` les entrées d'index en conflit de « canonical interval » et l'`Overfull \hbox` connu (lignes 511 à 515 de `gaol.tex` le 14 octobre), et en vérifiant que l'entrée de `what()` ne laisse pas son en-tête seul en bas de page. Décidé le 3 octobre pour les points 7 et 10 : la puce de `atanh` dans `doc/differences.md` reste, en disant que GAOL 4 n'est pas mesuré ; les puces de `hausdorff()` sont fusionnées ; un seul `\newinvfive` dans l'entrée `hausdorff` du manuel.

**État.** À faire.

**Suivi.** TODO.md rang 24, issue #142 rang 24.

---

### QG.3 — 32 (Y) : rapport de couverture, et `numbers` sous ASan + UBSan

**Contenu.** Le rapport de couverture (`coverage/README.md` et `coverage/coverage.html`), refait le 2026-09-27 à `605728e` (87,2 % des lignes de `gaol/`), ne correspond plus aux sources. À refaire au commit de la version (`-DGAOL_COVERAGE=ON`, cible `coverage`, gcovr) et à enregistrer ; décidé le 15 octobre : pas avant. Lancer aussi une fois, sous ASan + UBSan, le test `numbers` en entier (158 s pour `arithmetic` seul à la relecture du 14 octobre) ; `GAOL_NUMBERS_TIMEOUT` n'est pas relevé pour les builds avec sanitizers (décidé le 14 octobre).

**État.** À faire, au commit de la version.

**Suivi.** TODO.md rang 25, issue #142 rang 25.

---

### QG.4 — 33 (Y) : les temps de `doc/compare/performance.md`, en dernier

**Contenu.** Les temps ont été remesurés le 17 octobre (`febd2b1`, `a055160`), la colonne « rounding direction preserved » le 18 (`7fd5c09`), avec Clang 21 sur l'i7-1185G7. Les points AI et AJ peuvent encore les changer. À remesurer en dernier, sur le commit propre de la version, la machine ne faisant rien d'autre (`doc/compare/code/run_bench.sh`, ou `make perf`), puis reprendre le tableau de `doc/compare/README.md`.

**État.** À faire, en dernier.

**Suivi.** TODO.md rang 26, issue #142 rang 26.

---

### QG.5 — 34 (Y) : fusions, étiquette `v5.0.0`, recettes FetchContent

**Contenu.** Les recettes FetchContent récupèrent GAOL 4 : `doc/using.md`, le manuel et `tests/fetch_content` prennent la branche `master` de `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par défaut. `master` est GAOL 4.3.2. Décidé le 3 octobre : une fois `configure-clean` fini, fusionner `configure-clean` dans `MATH-CORE` (avance rapide), puis `MATH-CORE` dans `master` (qui a deux commits de plus, `577c850` et `406d850`, ceux de `.claude/CLAUDE.md` : une vraie fusion, sans différence de contenu), puis étiqueter `v5.0.0` ; les recettes et le test gardent `master` jusqu'à l'étiquette, puis la citent. Après v5.0.0, le développement continue dans `configure-clean`, et `master` ne reçoit que les versions.

**État.** À faire.

**Suivi.** TODO.md rang 27, issue #142 rang 27.

---

### QG.6 — 71 (Y) et #128 : l'annonce à Frédéric Goualard, après l'étiquette

**Contenu.** Annoncer GAOL v5.0.0 à Frédéric Goualard, l'auteur de GAOL, une fois l'étiquette posée : pas d'annonce publique, et pas de GitHub Release, l'étiquette suffisant (décidé le 3 octobre). Le message est écrit, en brouillon dans Gmail ; suivi dans #128.

**État.** À faire, après l'étiquette.

**Suivi.** TODO.md rang 28, issue #142 rang 28, #128.

---

## QH — Après la version

### QH.1 — 70 (AK) : site de documentation sur GitHub Pages

**Contenu.** Décidé le 3 octobre, un site généré depuis `doc/*.md`, avec le PDF du manuel, publié par un workflow, sans la référence HTML de Doxygen (supprimée par #29). Sans dépendance, mais l'annonce étant privée, rien ne l'exige pour la version.

**État.** À faire, après la version.

**Suivi.** TODO.md point 70 (AK), issue #142 rang 29.

---

### QH.2 — 46 (F, #64) : `operator>>` — DÉCIDÉ

**Problème.** `operator>>` lit un intervalle par ligne (`std::getline(is >> std::ws, buffer)`). Un intervalle contient des espaces (`[1, 2]`) et la grammaire admet des littéraux imbriqués et des expressions (`[[1, 2], 3]`, `sqrt(2)+1`, `1 -2`) : la fin d'un intervalle dans un flux est ambiguë, et `os << x << ' ' << y` ne se relit pas ; avec un saut de ligne (`os << x << '\n' << y`) il se relit.

**Décision (Q4, 22 octobre 2026).** Garder un intervalle par ligne (comportement actuel, documenté). Pas de changement d'`operator>>`.

**État.** Décidé. L'issue #64 peut être fermée.

**Suivi.** TODO.md point 46 (F), issue #64, issue #142 rang 30, réponse Q4.

---

### QH.3 — 26 (W, #67) : décorations ou indicateur de sortie de domaine — DÉCIDÉ

**Problème.** GAOL n'a que des intervalles nus (clause 11 d'IEEE 1788-2015) : `sqrt` d'une boîte négative est vide et passe tous les tests d'inclusion, `interval(DBL_MAX*10)`, `x + INFINITY` et `x * NAN` sont vides sans rien dire, et `gaol_ieee1788::textToInterval` rend l'ensemble vide pour un texte mal formé, qu'on ne distingue pas de `[empty]`.

**Décision (Q5, 22 octobre 2026).** Rien pour l'instant. GAOL reste un intervalle nu ; les pièges sont documentés (faits par #140). Reporter indéfiniment.

**État.** Décidé. L'issue #67 peut être fermée (ou laissée ouverte comme suivi).

**Suivi.** TODO.md point 26 (W), issue #67, issue #142 rang 31, réponse Q5.

---

### QH.4 — 28 (X) : traits pour YalAA, backend pour VNODE-LP

**Contenu.** Des en-têtes optionnels au-dessus du cœur scalaire. Décidé le 3 octobre : les traits pour YalAA et le backend pour VNODE-LP, et non la promotion des en-têtes des exemples (`examples/box.h`, `examples/dual.h`, `examples/affine.h`). Le plan détaillé est dans `todo-notes/orchestration/points_w4.py`.

**État.** À faire, après la version.

**Suivi.** TODO.md point 28 (X), issue #142 rang 32.

---

### QH.5 — Le ménage de la fin

**Contenu.** Les images Docker de #111 (`i386/debian:trixie`, `debian:trixie`, `ubuntu:24.04`) restent sur la machine. `todo-notes/` : ramené de la branche `todo-status` (supprimée) ; ses rapports sont retirés de l'arbre (ils restent au commit `16a2f60`), et ce qui en sert encore est dans `todo-notes/synthese.md`. À vider après la version, avec ce qu'il reste de `TODO.md`.

**État.** À faire, après la version.

**Suivi.** TODO.md section « Ménage », issue #142 section « Ménage ».

---

## QI — Travail en cours (branches et PR ouvertes)

### QI.1 — PR #157 (fusionnée) : noyaux FPU sans branche sur les signes

**Branche.** `todo-ai-fpu-kernels`, supprimée sur GitHub à la fusion (merge `ffcce46`, dans la nuit du 22 au 23 octobre) ; elle reste extraite dans l'arbre `GAOL_V9/GAOL`.
**Tête fusionnée.** `250b549` (22 octobre 2026) ; première relecture indépendante sur `b584d78` (20 octobre).

**Contenu.** Point AI de `TODO.md` (ancien 106). Noyaux scalaires sans branche pour `x * y`, `x / y`, `x % y` et `sqr(x)` des intervalles FPU (`gaol/gaol_interval_fpu.cpp`), et correction de `x - y` (qui prenait trois fois le temps de `x + y`). Cibles FPU : ARM 32 bits, POWER, s390x, RISC-V, Visual C++ et clang-cl arm64, Windows x86 32 bits avec Visual C++ et clang-cl, builds avec `GAOL_SIMD=OFF` sur x86 et `GAOL_ARM64=OFF` sur arm64.

**Mesures (M2, Apple M2).** `x * y` de signes aléatoires 12,4 → 3,3 ns (16 384 opérandes), `x / y` 9,3 → 3,2, `sqr(x)` 5,9 → 1,8, `x - y` 3,62 → 1,18. Sur signes prévisibles, sept `*` 23,8 → 35,8 ns (+50 %). Shekel 5 298 → 138 ns. Avec `GAOL_PRESERVE_ROUNDING`, la ligne de sept `*` passe de 279 à 307 ns.

**Décision (Q1, 22 octobre 2026).** Mesurer d'abord. Ne pas fusionner le produit tel quel ; lancer les mesures sur armhf (conteneur Debian), POWER, s390x et RISC-V avant de décider. La PR reste en brouillon.

**Suite.** Les mesures en alternance ont été faites sur la CI (voir QD.1) ; le mainteneur a décidé de garder tous les noyaux, le produit compris, sans changement de code, et #157 est fusionnée.

**Questions ouvertes de #157 (reprises et complétées en QJ.12).**
- Variante hybride (ancien chemin quand les deux opérandes sont positifs) : mesurée hors de l'arbre, non retenue par l'auteur
- `gaol_performance` apprend ses opérandes par cœur (1 024 opérandes répétés) : passer à 16 384 ou ajouter des lignes ?
- `fmax()` pour le plus grand de deux doubles : instruction sur arm64/RISC-V/POWER, mais classe +0 au-dessus de −0 et appelle la bibliothèque sur ARM 32 bits et x86
- `GAOL_RND_KEEP()` sur la sortie de `*=`, `/=`, `%=` : remettre ? (coûte 2,6 ns pour `x * y` et 5 ns pour `x / y` avec `GAOL_PRESERVE_ROUNDING`)
- `xsmaxcdp` sur POWER9 réel : lu seulement dans QEMU et GCC, ni ISA ni processeur
- Non vérifiable ici : ARM 32 bits, POWER, s390x, RISC-V, Visual C++, clang-cl, x86

**État.** Fusionnée (CI verte sur `250b549`, 159 contrôles).

---

### QI.2 — `exp-110-gcc-windows-fma` : expérience pour le point 110

**Branche.** `exp-110-gcc-windows-fma` (1 commit au-delà de `configure-clean`).
**Dernier commit.** `a326b03` (20 octobre 2026) : « Experiment for point 110: GCC for Windows with -mfma by hand, and whether armhf runs native ».

**Contenu.** Expérience pour le point 110 (QD.2) : donner `-mfma` à la main à GCC pour Windows, vérifier que `rounding_direction` passe, et vérifier si les runners arm64 exécutent l'armhf en natif.

**État.** Terminée : l'expérience a servi à #163 (fusionnée le 23 octobre). Branche supprimée sur GitHub le 23 octobre (décidé ce jour-là).

---

### QI.3 — `exp-110b-stack-alignment` : expérience pour le point 110

**Branche.** `exp-110b-stack-alignment` (3 commits au-delà de `configure-clean`).
**Dernier commit.** `b3ca337` (20 octobre 2026) : « Experiment for point 110: cstdio and cstdint for std::printf ».

**Contenu.** Suite de `exp-110-gcc-windows-fma` : `cstdio` et `cstdint` pour `std::printf` (`8e6b01b`), alignement de la pile des objets de 256 bits avec GCC pour Windows (`b3ca337`).

**État.** Terminée : l'expérience a servi à #163 (fusionnée le 23 octobre). Branche supprimée sur GitHub le 23 octobre (décidé ce jour-là).

---

### QI.4 — `todo-ai-msvc-avx512` : branche sans pull request — DÉCIDÉ : garder

**Branche.** `todo-ai-msvc-avx512` (0 commit au-delà de `configure-clean`, dernier commit `fbb962c` du 20 octobre 2026).

**Contenu.** Travail sur le chemin AVX-512 avec Visual C++ et clang-cl sur x64 (cpuid, pas d'attribut), intervalles SSE2 avec Visual C++ et clang-cl sur x64 dans CMake et meson. Semblant remplacée par #148 (fusionnée). Remarqué par #155 : la branche n'a pas de pull request.

**Décision (Q16, 22 octobre 2026).** Garder la branche telle quelle.

**État.** Gardée, sans PR.

---

### QI.5 — `todo-fpcr-gcc-hoisting` : branche de #153, fermée sans fusion — DÉCIDÉ : garder

**Branche.** `todo-fpcr-gcc-hoisting` (2 commits au-delà de `configure-clean`).
**Dernier commit.** `d21ece4` (20 octobre 2026).

**Contenu.** Correction du défaut de `width()` et `bisect()` sur arm64 avec GCC (bogue 34678) : les bornes passent par `rnd_reread()` après la vérification. #153 a été fermée sans fusion le 20 octobre : sa correction est dans #150 (fusionnée).

**Décision (Q16, 22 octobre 2026).** Garder la branche telle quelle.

**État.** Gardée, fermée sans fusion.

**Constat du 23 octobre.** La branche n'existe plus sur GitHub (`git ls-remote --heads origin`), et `TODO.md` (« Ménage ») la dit supprimée, sur GitHub et en local, le 20 octobre : la réponse Q16 de la garder est sans objet, sauf à la restaurer.

---

### QI.6 — PR #164 (fusionnée) : textes, tests et ménage du point C

**Branche.** `todo-QB-text-doc` (PR ouverte le 22 octobre 2026, tête `4b25af5`, 170 contrôles réussis ; GitHub la dit en conflit avec `configure-clean` le 23 octobre après les fusions de #157 à #163).

**Contenu.** Les tâches QB.1 à QB.6, QB.8, QB.9, QB.11, QB.12 et QB.13 (117, 119, 120, 87, 88, 89, 91, 94, 121, 122 et le C4702 de #148) : rangs 5 à 8 de `TODO.md`. QB.7 (112) est fait par #161, QB.10 (123) était fait le 20 octobre.

**État.** Fusionnée le 23 octobre (`a4cf94d`) ; sa branche est supprimée. Ses textes de CI (les commentaires « + - * / et sqrt », `setup.sh`, `process.md` §10) n'avaient pas été repris avant la fusion : ils sont dans #165.

---

### QI.7 — `exp-157-ab` : mesures en alternance de #157

**Branche.** `exp-157-ab` (branche d'expérience, jamais à fusionner), sur GitHub.

**Contenu.** Les mesures en alternance de #157 sur les configurations qui prennent les intervalles FPU (outils de #160 sur la tête et la base de #157), dont les chiffres sont dans la description de #157.

**État.** Supprimée sur GitHub le 23 octobre (décidé ce jour-là).

---

## QJ — Questions reportées des PR récentes (QJ.1 à QJ.11 répondues le 22 octobre 2026, QJ.12 à QJ.16 le 23 octobre)

**Contexte.** Questions ouvertes des descriptions des 20 dernières PR fusionnées, non corrigées et sans numéro, que le mainteneur devait trier. QJ.1 à QJ.11 ont reçu une réponse le 22 octobre 2026 (section « Réponses du mainteneur, 22 octobre 2026 » de `TODO.md`) ; QJ.12 à QJ.16 (#157, #160 à #163, fusionnées ensuite) sont à trier (`TODO.md`, « Remarqué par #NN, à trier »).

### QJ.1 — #146 (arm64 NEON) — RÉPONDU (Q14)

- **`bisect(ratio)` et `is_bisectable()` dépendent du mode FZ de l'appelant** avec `GAOL_PRESERVE_ROUNDING` (`LEAVE` rétablit FZ avant les comparaisons de `c` avec `l` et `r`), pour les sous-normaux : 634 résultats faux sur 200 000 intervalles sous-normaux. L'encadrement n'est pas violé ; les post-conditions documentées le sont (point Q). **À trier.**
- **`pow(x, 3)` à 6,5 ns sur arm64** : le noyau empaqueté de `pown` (`GAOL_POWN_PACKED`, FMA sur deux voies) n'existe que pour SSE2 ; un `vfmaq_f64` ne gagnerait qu'un facteur de l'ordre de 1,5, pour 150 lignes d'intrinsèques SSE2 à porter sans pouvoir les tester avec GCC. **Décision (Q14a) : oui, porter le noyau.**
- **`sqrt([DBL_MAX])` lève `FE_OVERFLOW`** avec NEON comme avec les intervalles FPU : c'est le 79 (QA.1), qu'un produit fusionné `fma(u, u, -v)` réglerait.
- **`doc/compare/itf1788/itf1788.py`, l. 821** : nomme « SSE2 » le build par défaut, qui est NEON sur arm64. **À corriger.**
- **Le signe d'un zéro** diffère de celui des intervalles FPU et SSE2 avec NEON (documenté) : aucun test ne compare les bits d'un zéro. **Décision (Q14b) : non, pas de test.**
- **Pas de job** pour Clang 15 à 17 ni GCC 9 à 10 sur arm64, et aucun test de `mrs` et `msr` sur Neoverse. **Décision (Q14c) : non, pas de jobs ni de test.**
- **Le nom de `float64x2_t` est décoré autrement** par GCC et par Clang sous Linux : une bibliothèque installée construite par l'un et liée à un programme compilé par l'autre ne lierait pas. **Documenté, comme pour `__m128d` avec SSE2.**

---

### QJ.2 — #147 (parse_interval nullptr) — RÉPONDU (Q13)

- **`gaol::textToInterval(nullptr)`**, `textToInterval(nullptr, "1")`, `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)` prennent un `const std::string&` : le `std::string` est construit à l'appel, avant GAOL, ce que le standard laisse indéfini. Avec libstdc++ (GCC 13.3), les quatre lèvent `std::logic_error` ; avec libc++ et Visual C++, probablement un plantage dans `strlen`, non vérifié. **Décision (Q13) : lever une erreur de GAOL comme une mauvaise lecture du parser (`input_format_error`), sans passer par la construction indéfinie d'un `std::string`.**
- **Une `gaol_exception` construite avec un `file` nul** : `operator<<` envoie un `const char*` nul à un flux, qui passe à l'état `bad` avec libstdc++, et les diagnostics suivants sont perdus. Les macros de GAOL passent toujours `__FILE__` ; seuls des appels directs sont en cause. **Peu prioritaire, à trier.**

---

### QJ.3 — #149 (lecture, exposants et [l, u]) — RÉPONDU (Q12)

- **Le coût de la comparaison exacte.** Le contexte de la lecture garde le texte de chaque nombre qui n'est pas un double, qu'un littéral `[l, u]` le compare ou non : une somme de 200 000 termes `0.1` passe de 4,9 Mo et 0,106 s à 6,0 Mo et 0,134 s. **Décision (Q12) : garder pour tous (actuel).**
- **`[l, u]` ne départage pas** pi et un autre nombre de même encadrement, un nombre loin au-delà des doubles (plus de 10^311 ou moins de 10^-331), ni des bornes calculées (`[1/3, 0.3333333333333333]`) : le littéral est alors un sur-ensemble du vide, sûr mais pas le plus serré. **Documenté. À trier.**
- **`<a, b>` accepte encore une borne calculée qui n'est pas un double** (`<[0.1], 0x1.9999999999999p-4>`). **À trier.**
- **Un piège pour `process.md`, section 11** : le test de `tests/expressions.cpp` compte les allocations par le `operator new` qu'il remplace, qui ne voit pas celles d'un `std::string` que fait la bibliothèque C++ quand elle est dans une DLL. **À ajouter à `process.md`.**
- **Le tableau de `doc/accuracy.md`** cite encore `interval(sl, su)` pour `textToInterval(s)`, qui n'existe plus depuis GAOL v5. **À corriger.**

---

### QJ.4 — #150 (ARM 32 bits FPSCR) — À trier

- **Le mode par défaut des intervalles FPU d'ARM 32 bits** n'est pas amélioré par #150 : c'est la suite du 106 (QD.1, PR #157). **Fait par #157** (noyaux scalaires sans branche, mesurés sur armhf natif dans la CI).
- **Les mesures** ne valent que pour les runners arm64 de GitHub, qui exécutent du 32 bits. Le coût de `vmrs` et `vmsr` sur un Cortex-A 32 bits, et le gain, ne sont pas connus. Non vérifiés : bionic et GCC avant 12. Clang reste refusé sur ARM 32 bits.
- **Le même contrôle pour x86** : l'ordre entre l'écriture du registre de contrôle et les opérations sur des registres (bogue 34678 de GCC) a été vérifié sur l'assembleur pour AArch64 et ARM 32 bits, pas pour x86, où `_mm_setcsr` n'est pas mieux ordonné. `width()` en est le cas connu ; à refaire avec un GCC pour x86.
- **Les sujets de deux commits** de la branche de #150 font plus de 100 caractères (117 et 101) : déjà poussés, non récrits.
- **Un piège pour `process.md`, section 11** : une écriture du registre de contrôle par `asm volatile` n'est ordonnée qu'avec les autres `asm`, pas avec les opérations sur des registres. **À ajouter à `process.md`.**

---

### QJ.5 — #151 (cancel_minus/cancel_plus) — RÉPONDU (Q11)

- **La correction va au-delà de la formule du TODO** (« e = 0 quand la somme est infinie ») : le Fast2Sum corrige aussi le cas à somme finie, de même cause. **Décision (Q11) : garder la correction élargie.**
- **La phrase d'introduction de la section sur les exceptions flottantes** de `doc/using.md` et du manuel, « An infinite bound comes from a division by zero or from an overflow », reste vraie pour les deux exemples gardés, `log([0, 1])` et `[1e308]*10`, mais n'est pas une règle : `1/[0, 1]` a une borne infinie sans aucun indicateur depuis #121. **À trier.**

---

### QJ.6 — #154 (étude SIMD, doc/simd.md) — RÉPONDU (Q10)

- **`FPCR.AH` (FEAT_AFP) n'est pas effacé** par les intervalles NEON (`GAOL_RND_FLUSH_BITS` : FZ et FIZ). Avec AH, `FMAX` renvoie le second opérande pour deux zéros de signes différents. Défaut théorique, documenté. **Décision (Q10a) : oui, durcir `sqrt_bounds()` par un masque sur le bit de signe, validé sur une copie, en une pull request à part.**
- **`pow(x, 2)` par `sqr`** : 5 à 6 ns contre 0,8 à 0,9 sur le M2, 40 % de l'exemple Goldstein-Price. **Décision (Q10b) : oui, faire `pow(x, 2)` par `sqr`.** **Fait par #162** (3,3 fois plus rapide avec NEON, 2,5 avec la FPU ; voir QJ.15).
- **Les pistes à mesurer** : le RISC-V scalaire à `rm` statique (il faut une carte), POWER et s390x (du matériel), l'arrondi embarqué scalaire pour `/` sur les processeurs Intel qui divisent les registres de 512 bits. **À trier.**
- **Les runners arm64 de GitHub exécutent l'armhf en natif** : Neoverse N2, « 32-bit EL0 Support » au démarrage.
- **Le chemin générique de `round_upward()`** (`gaol/gaol_fpu_fenv.h`) appelle `fesetround(FE_UPWARD)` sans en lire le retour ; `NI` de POWER et `FS` de MIPS ne sont pas effacés. **À trier.**
- **Trois pièges de compilation** : la pragma `FENV_ROUND`, le `_mm256_add_round_pd` de Clang 20 avec `-mavx10.2`, l'assembleur `vaddpd {ru-sae}`. **Documentés dans `doc/simd.md`.**

---

### QJ.7 — #155 (process.md) — RÉPONDU (Q9)

- **Le paragraphe retiré du §6.1** reste vrai sur une machine (l'arbre `GAOL_V9/GAOL`, 19-20 octobre : Ubuntu 24.04, GCC 13.3, Clang 18 et 20, CMake 3.28, python 3.12, sans autoconf, automake, libtool, meson ni ninja, 4 cœurs). **Décision (Q9a) : oui, une phrase plus courte à la place.**
- **Les cœurs** : le §3.3 dit 4 cœurs au plus (« la machine en a 8 ») ; sur la machine de l'arbre `GAOL_V9/GAOL`, qui en a 4, le mainteneur en a demandé 2 au plus. **Décision (Q9b) : non, garder « 4 cœurs au plus » sans ligne pour `GAOL_V9/GAOL`.**
- **L'en-tête** de `process.md` renvoie au §12 au lieu de donner une date et un commit. **Décision (Q9c) : mettre une date et un commit dans l'en-tête.**
- **La branche distante `todo-ai-msvc-avx512` n'a pas de pull request** (elle semble remplacée par #148). **Décision (Q16) : garder la branche (voir QI.4).**

---

### QJ.8 — #156 (x86 32 bits, registres de contrôle) — RÉPONDU (Q8)

- **(a) Les appels de CORE-MATH à `fegetround()` restent** sur x86 32 bits : `sin`, `cos`, `tan`, `pow`, `cbrt`, `rsqrt` et `asinpi` en appellent un, qui coûte autant qu'avant. **Décision (Q8a) : oui, lire MXCSR en ligne dans CORE-MATH sur x86 32 bits (`gaol/core_math_port.h` comme sur x86-64).**
- **(b) Un test de la lecture elle-même** (`x86_registers::read_control()` contre `read_x87_word()` et `_mm_getcsr()` du test, dans les six états, sur x86 seulement) : non écrit, faute de pouvoir l'exécuter. **Décision (Q8b) : oui, ajouter le test.**
- **(c) Le job ajouté** (Visual Studio 2026 x86 Release, direction préservée) : à garder ? **Décision (Q8c) : oui, garder le job.**
- **(d) Le dossier de travail contient** un outil de mutation (`tools/mutate.py`), l'amalgamation pour Compiler Explorer et l'émulateur (`emu/`) : à ajouter au dépôt (`tests/tools/`) si on veut les rejouer. **Décision (Q8d) : oui, ajouter au dépôt.**
- **(e) Le chemin de repli** de `round_upward_if_needed()` avec `fegetround()` est du code mort aujourd'hui. **Décision (Q8e) : retirer le code mort.**
- **`width()` avec GCC pour x86 32 bits** : GCC 9.4, 12.2 et 15.2 sortent la soustraction de la boucle avant le contrôle ; le premier résultat est faux dans 12 des 16 états. C'est le défaut de #153 pour arm64, que sa correction (`rnd_reread()`) règle aussi sur x86. **Déjà corrigé par #150.**
- **`GAOL_RND_NEAREST_ENTER()`/`LEAVE()` et `GAOL_RND_ENTER_SSE()`/`LEAVE_SSE()`** écrivent sans comparer. Même méthode possible (écrire si ça change) ; non touché : hors du point 108. **À trier.**
- **`GAOL_RND_BARRIER()`** : retirée de l'extrait émulé, elle ne change aucun résultat avec Clang 21 `-O2`. Ce n'est pas un défaut : la barrière protège un cas de GCC 13 à `-O3` (`asinpi`). **Documenté.**

---

### QJ.9 — #158 (nth_root sous-normal) — À trier

- **La phrase fausse « tightest bounds or one double beyond »** reste dans `doc/differences.md` (puce « The n-th roots are proved with integer powers ») et dans `manual/v5/gaol.tex` (deux endroits). **À corriger dans la pull request de synthèse : « jusqu'à 2 doubles (4 pour q < 0) ».**
- **Un x sous-normal prend 1,3 à 2,2 fois plus de temps qu'avant** (chemin mis à l'échelle, pas de pas de Newton pour un sous-normal). **À trier : acceptable, ou donner à `newton_root()` des puissances mises à l'échelle ?**
- **Les temps ont été pris sur une machine non au repos** : à refaire au point Y si l'on veut les citer.
- **Remarqué sans être corrigé** : sur macOS, le `pow()` de la bibliothèque C n'est pas exact en arrondi dirigé pour certains résultats sous-normaux. **À trier.**

---

### QJ.10 — #124 (noyaux SSE2, point ZZ) — RÉPONDU (Q15)

- **(a) Les noyaux dans l'interface de la bibliothèque.** `add_bounds()`, `sub_bounds()`, `mul_bounds()`, `div_bounds()` et `sqr_bounds()` sont exportés (visibilité par défaut, espace de noms `gaol`). **Décision (Q15a) : mettre dans `gaol_detail`.**
- **(b) Le signe des zéros des produits et des quotients.** Garder le plus rapide, ou rétablir le signe que donnait le `switch` ? **Décision (Q15b) : garder le plus rapide (documenté).**
- **(c) Les fonctions élémentaires paient encore deux écritures de MXCSR avec la préservation** (jusqu'à 27 % pour `exp`). Un mode de CORE-MATH où le sens serait un paramètre les supprimerait, au prix de modifier `3rd/math-core`. **Décision (Q15c) : non, ne pas modifier `3rd/math-core`.**
- **CMake : un second `cmake` fait perdre le chemin AVX-512 par défaut.** C'est le QC.1, fait par #161.
- **Les divisions du chemin AVX-512** restent plus lentes que celles du chemin SSE2 dans une chaîne. **Décision : `GAOL_PREFER_AVX512` reste par défaut avec `GAOL_PRESERVE_ROUNDING`.**
- **Un écart déjà présent à `7716fec`** : avec `GAOL_PRESERVE_ROUNDING`, `x *= d` et `x * d` donnent le signe opposé pour une borne nulle de 344 résultats sur 5,7 millions entre AVX-512 et SSE2. **Décidé le 18 octobre (point AF, #140) : exception acceptable, documentée. Test des bits optionnel (voir QD.7).**
- **`make perf` ne lance pas les dix lignes de `run_lines.sh`** : c'est le QD.6 (115), fait par #160.

---

### QJ.11 — #122, #123, #139, #140, #141, #143, #144, #145, #148, #152 — questions diverses

- **#122** : les build directories de meson configurés avant le changement sont à refaire par `meson setup --wipe` (documenté dans `doc/building.md`). Les deux échecs Visual Studio `/W4 /WX` venaient de `configure-clean` (corrigés par `5984cc8`). Décidé le 14 octobre : AA est fait, deux nouveaux points le suivent (relancer le banc de `doc/compare`, étudier `-mbranches-within-32B-boundaries` = QD.4).
- **#123** : un avertissement `-Wconversion` dans `gaol/gaol_profile.cpp:52` (`__time64_t` vers `long`), sans rapport, non corrigé. **À trier.**
- **#139** : après la fusion, `TODO.md` doit perdre la phrase d'`atanh` de l'article 33 et du point R ; l'article 33 dit encore « cinq bibliothèques » (il y en a six) et « remesurés le 2026-09-27 » : à corriger en même temps. **Fait dans le ménage de TODO.md.**
- **#140** : réponses du mainteneur le 18 octobre : pas de job de CI pour les blocs de code des fichiers Markdown ; `f_negate()` et `gaol_opposite()` supprimées ; le sens d'arrondi laissé ou non par une opération à opérande vide est voulu. `examples/examples.md` garde ses anciens chiffres de coût de `GAOL_PRESERVE_ROUNDING`, datés dans le texte. **Décidé.**
- **#141** : faut-il supprimer `GAOL_IX86_LINUX` et `GAOL_AARCH64_LINUX`, et reformuler la phrase pour les autres ? **Décidé de les garder (voir #143).**
- **#143** : supprimer quand même `GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX` et `GAOL_HAVE_AVX512_TARGET` plus tard ? **Le mainteneur a décidé de les garder.** `GAOL_USING_SSE3_INSTRUCTIONS` n'est lue que par les intervalles de floats, désactivés dans la configuration livrée. **Documenté.**
- **#144** : le correctif 8 est à envoyer à CORE-MATH avec les sept autres (#125 = QF.1). `rounding_direction.cpp` ne fixe pas `RHO2` (ses cas sont loin de la frontière) : c'est `core_math.cpp` qui le fait. Commentaire gâché dans `gaol/gaol_roundeven.h` ligne 73 (« a doublegaol_cr_: an integer »). **À trier.**
- **#145** : décidé le 19 octobre : pas de `#error` dans `gaol/core_math_port.h` ; `configure` garde les drapeaux de l'utilisateur ; les drapeaux des sources C restent au point 92 (QC.4). **Rien d'autre ouvert.**
- **#148** : le chemin AVX-512 de `cl` n'est exécuté nulle part où l'on peut le voir (pas d'AVX-512 sur les runners Windows) ; `GAOL_PRESERVE_ROUNDING` sous Windows prend par défaut le chemin AVX-512 quand la machine qui compile l'a ; la condition `_M_X64` avec `__AVX2__` du 109 pour `cl` (voir QD.3) ; le gain de `__vectorcall` non mesuré (voir QD.6) ; pas de changement pour Windows 32 bits ni arm64. **À trier.**
- **#152** : `-faligned-new=8` dans `gaol.pc` et `gaol_dep` de meson donne un avertissement pour un fichier C. L'alignement de la pile 32 bits non testé. Visual C++ x86 garde les intervalles FPU (`fegetround()` y coûte environ 80 ns, point 108 = QD.8). **À trier.**

---

### QJ.12 — #157 (noyaux FPU) — RÉPONDU le 23 octobre (tout gardé)

- **Une piste, décidée le 23 octobre : l'essayer** (pull request brouillon #168). En local (i7-1185G7) le noyau mixte gagne 1,44 fois sur la ligne de 7 `*` avec GCC mais perd 11 à 24 % quand les signes varient, et rien avec Clang. Le banc A/B de la CI (base / PR, médianes de 3 passages) : la ligne de 7 `*` gagne 1,96 fois (GCC x86-64), 1,70 (arm64 GCC), 1,46 (Visual C++ arm64), 1,67 (clang-cl arm64), 1,43 (armhf) ; `x * y` de signes aléatoires perd 5 à 19 % (sauf arm64 GCC, +30 %) ; rien avec Clang x86-64 ni Visual C++ x86. La décision de fermer sans fusion (prise sur les mesures locales) attend d'être confirmée : la pull request reste en brouillon.
- **`gaol_performance` apprend ses opérandes par cœur** (1 024 opérandes répétés : `x * y` de signes aléatoires 5,7 ns contre 12,4 sur 16 384) : passer ses trois lignes « signes aléatoires » à 16 384 opérandes, ou en ajouter trois ? **Décidé le 23 octobre : ne rien changer.**
- **`fmax()` pour le plus grand de deux doubles** : une instruction sur arm64, RISC-V et POWER, mais elle classe +0 au-dessus de −0 et appelle la bibliothèque sur ARM 32 bits et x86 ; utile surtout à Visual C++ et clang-cl pour arm64, qu'on ne peut pas mesurer. **Décidé le 23 octobre : non.**
- **`GAOL_RND_KEEP()` sur la sortie de `*=`, `/=` et `%=` ?** Aujourd'hui non (protection de fait, qu'aucun test ne tient) ; la remettre coûterait 2,6 ns (`x * y`) et 5 ns (`x / y`) avec `GAOL_PRESERVE_ROUNDING`. **Décidé le 23 octobre : ne pas la remettre, tenir la protection par un test** (GCC et le sens préservé) : #170, qui ajoute les jobs FPU x86-64 (GCC, Clang) et arm64 GCC avec le sens préservé et dit dans le commentaire d'`operator*=` quels tests tiennent la protection (mutants : 53/96, 58 et 57 vérifications de la boucle en échec pour `*=`, `/=`, `%=`).
- **`mid()` et `div_rel()`** non touchés ; les opérations avec un double ont encore une branche sur le signe de `d`, comme avec SSE2.
- **Le résultat de `xsmaxcdp` sur un POWER9 réel** n'est lu que dans la source de QEMU et dans un commentaire de GCC.
- **La seconde relecture de `process.md` §7 n'a pas été faite** (décidé le 23 octobre : ne pas la faire) ; l'exécution sur ARM 32 bits, POWER, s390x et RISC-V et le noyau SSE2 (comparé à un modèle) restent non vérifiés.

---

### QJ.13 — #160 (banc A/B, `make perf`) — RÉPONDU le 23 octobre

- **Le sens du rapport** (base sur pull request : au-dessus de 1, la pull request est plus rapide) ; beaucoup de lecteurs attendent l'inverse. **Décidé le 23 octobre : garder.**
- **Le seuil de la liste** des lignes écartées (facteur 2) : des rapports jusqu'à 1,17 pour du code identique ; 1,5 serait possible.
- **Le filtre de chemins** n'a ni les fichiers d'autotools et de meson, ni `doc/compare/code/run_perf.sh`. **Décidé le 23 octobre : ajouter `run_perf.sh` seulement** (#166).
- **Les jobs** : CMake et Ubuntu 24.04 seulement, ni arm64 Linux, armhf, i386, macOS x86_64, clang-cl ni Visual C++ arm64 ; le job armhf/arm64 de #150 compare deux options d'un arbre. Un cache du binaire de la base reste possible. **Décidé le 23 octobre : faire les jobs et le cache** (#166 : dix jobs de plus, cache des programmes de la base sur la clé de la base, du job, de l'image et du processeur).
- **`gaol_bench` avec Visual C++** n'avait jamais été compilé avant #160.
- **L'élargissement de `make perf` à macOS et de `gaol_bench` à tous les systèmes** : à garder ou à restreindre. **Décidé le 23 octobre : garder.**
- **`LINE_LIST` vide** : aucune ligne dans `run_perf.sh`, les dix dans `run_lines.sh`. **Fait (23 octobre)** : aucune ligne dans les deux (#166).
- **`process.md` §10** ne liste pas `performance.yml`. **Fait (23 octobre)** par #165.
- **Remarqué sans corriger** : `actionlint` sur `suppress-deprecation-warnings` de `msys2/setup-msys2@v2` (job `msys2-refused`), et des remarques de `shellcheck` sur d'autres workflows.

---

### QJ.14 — #161 (`GAOL_PREFER_AVX512`, descriptions d'options) — RÉPONDU le 23 octobre

- **Une option à trois valeurs `AUTO`, `ON`, `OFF`** pour `GAOL_PREFER_AVX512` lèverait deux limites du commentaire de `CMakeLists.txt`, mais change le type de l'option : à décider. **Décidé le 23 octobre : garder `ON`/`OFF`.**
- **`configure` édité à la main** (par #161 et #163) : vérifié le 23 octobre à `601fc63`, `autoconf` 2.72 régénère un `configure` identique au commité.
- **Les commentaires de la CI et de `doc/compare/code/`** qui disent encore « +, -, *, / et sqrt » : à mettre à jour ou non. **Décidé le 23 octobre : les mettre à jour** (dans #165, #164 étant déjà fusionnée).
- **`doc/compare/code/setup.sh`** garde `-UGAOL_PREFER_AVX512` : il peut disparaître quand plus aucun dossier configuré avant #161 ne subsiste. **Décidé le 23 octobre : le retirer et nettoyer les dossiers** (#165 ; sept dossiers supprimés, et `setup.sh` supprime lui-même un dossier d'avant #161).
- **La relecture des corrections** de la relecture indépendante (points A à F) n'a pas été faite.
- **Le troisième élément du 112** (test des bits du signe de zéro de `x * d`) n'est pas fait : voir QD.7. **Décidé le 23 octobre : le faire, sauté sans AVX-512** (#171).

---

### QJ.15 — #162 (`pow(x, 2)` par `sqr`) — RÉPONDU le 23 octobre

- **Le test `e == 2` dans les en-têtes** (`pow` de `gaol/gaol_interval.h`, `pown` de `gaol/gaol_ieee1788.h`) : 1,35 ns au lieu de 1,66 avec NEON, au prix d'un second endroit à tenir. **Décidé le 23 octobre : non, la décision reste dans `gaol_pown`.**
- **Les puissances 3 à 5 avec les intervalles FPU** coûtent environ 0,3 ns de plus (+5 %) ; à remesurer sur x86. **Remesuré le 23 octobre** (#169) : +0,8 ns sur x86 (l'appel, le cadre et la garde de pile de `gaol_pown`), laissé et dit ; le remesurage a trouvé un `sqr` FPU 1 ns trop lent depuis #162, corrigé par #169.
- **L'ensemble vide et le sens d'arrondi** : quatre formes de `pow(vide, 2)` laissent le sens du programme, quatre autres le mettent vers le haut ; sans conséquence sur les bornes. **Décidé le 23 octobre : harmoniser vers « le sens du programme »** (#169, quinze formes testées).
- **Le texte de #154** (« +0 avec `pow` et −0 avec `sqr` ») était inexact pour NEON et FPU ; `doc/simd.md` est corrigé.
- **Non vérifiable ici** : toute exécution sur x86, GCC, Windows ; les temps viennent d'un seul M2 partagé.

---

### QJ.16 — #163 (GCC pour Windows, FMA, registres de mingw-w64) — RÉPONDU le 23 octobre

- **Le bogue 54412 de GCC** : état connu par des lectures indirectes seulement (ouvert jusqu'à GCC 16) ; le commentaire de `CMakeLists.txt` dit « fixed for GCC 17 only ».
- **Les `fesetround()` et `fegetround()` de mingw-w64** lus dans les sources de `master`, du CRT de MSYS2 de la CI et de v13 et v14, non dans ceux de Chocolatey.
- **`GAOL_PRESERVE_ROUNDING` avec autotools et meson sous MSYS2** : aucune entrée de `build-systems.yml` ; le contrôle des indicateurs depuis les six directions n'est lancé que pour mingw-w64 : l'étendre aux autres plateformes est une décision de plus. **Décidé le 23 octobre : l'étendre à toutes** (#167).
- **`-mavx512f` ou `-march` donnés par l'utilisateur** : avec le Clang croisé, des lignes `%ymm` et `%zmm` restent ; le relecteur compte autrement ; rien n'est vérifié avec GCC. À dire dans `doc/using.md` ? **Décidé le 23 octobre : le documenter, avec « peut »** (#167).
- **Cygwin** reçoit `-mfma` seul, sans `-mprefer-vector-width=128`, comme avant. Lui donner les mêmes drapeaux ? Non vérifié. **Décidé le 23 octobre : oui** (#167 ; sans job de CI).
- **x86 32 bits** (MinGW-w64 12.2 à 15.2) restait sans FMA et non mesuré ; **`-mstackrealign`** non essayé. **Décidé le 23 octobre : l'essayer par la CI**. **Fait** : MinGW-w64 15.2 et 14.2 mesurés, `-mfma -mprefer-vector-width=128` par défaut aussi à 32 bits (`pow(x, 3)` de 70 à 12 ns), `-mstackrealign` sans effet : #172.
- **Les corrections qui ont suivi les deux relectures** (`704ca0c`, `e80b6f0`, `1de1df0`, `15399b5`) n'ont pas été relues par un relecteur indépendant.
- **Les branches d'expérience** `exp-110-gcc-windows-fma` et `exp-110b-stack-alignment` existaient encore sur GitHub, que la description de #163 dit supprimées : **supprimées le 23 octobre** (avec `exp-157-ab`).

---

## QK — Suites des réponses du 22 et du 23 octobre (128 à 133, point AP), et les pull requests #165 à #173

### QK.1 — 128 (AP) : `textToInterval(nullptr)` lève `input_format_error` [S] — DÉCIDÉ (Q13), à faire

**Problème.** `gaol::textToInterval(nullptr)`, `textToInterval(nullptr, "1")`, `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)` prennent un `const std::string&` : le `std::string` est construit avant GAOL, ce que le standard laisse indéfini (libstdc++ lève `std::logic_error`).
**Décision.** Lever `input_format_error` (ou une `gaol_error`) : une surcharge `const char*` et un test par fonction. **Suivi.** TODO.md rang 4.

---

### QK.2 — 129 (AP) : `sqrt_bounds()` sous `FPCR.AH` [S] — DÉCIDÉ (Q10a), à faire

Un masque sur le bit de signe à la place du maximum de `vmaxq_f64`, validé sur une copie (bit à bit identique), en une pull request à part. **Suivi.** TODO.md rang 6.

---

### QK.3 — 130 (AP) : `process.md` [S] — DÉCIDÉ (Q9a, Q9c), à faire

Une phrase plus courte à la place du paragraphe retiré du §6.1, une date et un commit dans l'en-tête ; le §3.3 reste à « 4 cœurs au plus » (Q9b). **Suivi.** TODO.md rang 7.

---

### QK.4 — 131 (AP) : les noyaux exportés de SSE2 dans `gaol_detail` [S] — DÉCIDÉ (Q15a), à faire

`add_bounds()`, `sub_bounds()`, `mul_bounds()`, `div_bounds()` et `sqr_bounds()`. **Suivi.** TODO.md rang 8.

---

### QK.5 — 132 (AP) : `GAOL_POWN_PACKED` sur arm64 [M] — DÉCIDÉ (Q14a), à faire

Le noyau empaqueté de `pown` (FMA sur deux voies, `vfmaq_f64`), environ 150 lignes d'intrinsèques, gain attendu de l'ordre de 1,5. **Suivi.** TODO.md rang 17.

---

### QK.6 — 133 (AP) : GCC pour Windows, x86 32 bits sans FMA, et un job Cygwin [S] — DÉCIDÉ (23 octobre), en cours (#172, #173)

MinGW-w64 pour x86 n'avait pas de FMA (CORE-MATH et les puissances entières appelaient un `fma()` logiciel). L'essai (une branche d'expérience, supprimée) a construit, testé et mesuré MinGW-w64 15.2 et 14.2 : 38 tests réussis avec chaque jeu de drapeaux ; `-mfma -mprefer-vector-width=128` donne `pow(x, 3)` 70 → 12 ns, `log` 266 → 82, `cos` 490 → 138 (GCC 15.2), `-mfma` seul laisse 4 709 lignes `%ymm` ; `-mstackrealign` ne change rien ; un programme à fils donne les résultats du fil principal dans toutes les variantes. Décidé : ON par défaut (`GAOL_FMA=OFF` reste le remède pour un processeur sans FMA) : #172 (ouverte), avec un job sans FMA qui garde le `fma()` x87 de mingw-w64 11. Et un job Cygwin x86-64 (#173, brouillon), le premier à construire GAOL avec Cygwin. **Suivi.** TODO.md rang 18.

---

### QK.7 — Les pull requests ouvertes le 23 octobre au soir

| PR | Contenu | Décisions qu'elle exécute |
|----|---------|---------------------------|
| #165 | commentaires de la CI et de `doc/compare/code/`, `setup.sh` sans `-U`, `process.md` §10, `NI` et `FS` documentés | #161 textes, #161 `setup.sh`, #160 `process.md`, #154 `NI`/`FS` |
| #166 | dix jobs de plus pour le banc A/B, cache des programmes de la base, `run_perf.sh` dans le filtre, `LINE_LIST` vide | #160 filtre, jobs et cache |
| #167 | Cygwin avec les drapeaux de GCC pour Windows, six directions partout, `-mavx512f`, `-march` et `-faligned-new=8` documentés | #163 |
| #168 (brouillon) | noyau mixte du produit FPU | #157 piste |
| #169 | le vide dans `pow`, le carré FPU | #162 |
| #170 | jobs FPU x86-64 et arm64 GCC avec le sens préservé, commentaire d'`operator*=` | #157 `GAOL_RND_KEEP()` |
| #171 | test des bits du signe de `x * d` | #161 |
| #172 | GCC pour Windows en 32 bits : `-mfma -mprefer-vector-width=128`, un job sans FMA | #163, x86 32 bits |
| #173 (brouillon) | un job Cygwin GCC x64 | #163, Cygwin |

Chacune a eu sa relecture indépendante (sauf #168, expérience, et #173, dont la première exécution tient lieu de relecture) ; la CI de #165, #166 et #167 à #171 est verte (le 23 octobre à minuit).

---

## Synthèse des décisions du 22 octobre 2026

| Question | Catégorie | Décision |
|----------|-----------|----------|
| Q1 | QI.1 | PR #157 : mesurer d'abord (armhf, POWER, s390x, RISC-V), ne pas fusionner le produit tel quel — suite : mesures faites sur la CI, tout gardé, #157 fusionnée |
| Q2 | QB.11 | Ajouter `.continue/` au `.gitignore` |
| Q3 | QC.2 | Point 125 clos : `gaol_config.h` a déjà les valeurs par défaut |
| Q4 | QH.2 | `operator>>` : garder un intervalle par ligne (comportement actuel) |
| Q5 | QH.3 | Décorations : rien pour l'instant, GAOL reste un intervalle nu |
| Q6 | QD.6 | CI : deux builds alternés dans le même job — fait par #160 |
| Q7 | QE.1 | Étude AJ.113 : reportée après v5.0.0 |
| Q8 | QJ.8 | #156 : tout oui (lire MXCSR dans CORE-MATH, test read_control, garder le job, ajouter les outils, retirer le code mort) |
| Q9 | QJ.7 | #155 : phrase courte au §6.1, non pour les cœurs au §3.3, date+commit dans l'en-tête |
| Q10 | QJ.6 | #154 : les deux oui (masque FPCR.AH, `pow(x, 2)` par `sqr`) — `pow(x, 2)` par `sqr` fait par #162 |
| Q11 | QJ.5 | #151 : garder la correction élargie de `cancel_minus`/`cancel_plus` |
| Q12 | QJ.3 | #149 : garder le texte de tous les nombres (actuel) |
| Q13 | QJ.2 | #147 : lever `input_format_error` pour `textToInterval(nullptr)` |
| Q14 | QJ.1 | #146 : (a) oui, porter le noyau `pow(x, 3)` arm64 ; (b) non, pas de test bits zéro ; (c) non, pas de jobs CI |
| Q15 | QJ.10 | #124 : (a) `gaol_detail` ; (b) garder le plus rapide ; (c) non, ne pas modifier `math-core` |
| Q16 | QI.4/QI.5 | Garder les deux branches `todo-ai-msvc-avx512` et `todo-fpcr-gcc-hoisting` |

## Synthèse des décisions du 23 octobre 2026

| PR | Question | Décision |
|----|----------|----------|
| #157 | `GAOL_RND_KEEP()` sur `*=`, `/=`, `%=` | Ne pas le remettre ; un test tient la protection (#170) |
| #157 | noyau mixte | L'essayer ici et par le banc A/B (#168, brouillon) |
| #157 | bancs, `fmax()`, seconde relecture | Rien ; non ; non |
| #162 | `e == 2` dans les en-têtes | Non, la décision reste dans `gaol_pown` |
| #162 | vide et sens d'arrondi | Harmoniser vers « le sens du programme » (#169) |
| #160 | rapport, filtre, `make perf`, `LINE_LIST`, jobs, cache | Base sur PR ; `run_perf.sh` ; garder ; harmoniser ; faire (#166) |
| #161 | `AUTO`, commentaires, bits de `x * d`, `setup.sh` | `ON`/`OFF` ; mettre à jour (#165) ; faire, sauté sans AVX-512 (#171) ; retirer `-U` et nettoyer (#165) |
| #158 | x sous-normal 1,3 à 2,2 fois plus lent | Accepter |
| #163 | Cygwin, `-mavx512f`/`-march`, six directions, pile 32 bits, branches `exp-*` | Mêmes drapeaux ; documenter ; partout (#167) ; essayer `-mstackrealign` (133) ; supprimer les trois |
| #152 | `-faligned-new=8` pour un fichier C | Le dire dans `doc/using.md` (#167) |
| #154 | `NI` de POWER et `FS` de MIPS | Documenter seulement (#165) |
