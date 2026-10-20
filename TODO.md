# À faire

Ce qui reste à faire sur GAOL v5 au commit `98413b0` de `configure-clean`, le 20
octobre 2026. Ce fichier ne garde que cela : le travail fait, ses pull requests,
ses décisions et ses mesures sont dans l'issue #49 (« Fusionné depuis le 30
septembre » et les sections « Décidé le… »), dans les descriptions des pull
requests et dans l'historique de git (`git log -p TODO.md`, où la version
d'avant le ménage du 18 octobre est celle de `4eaf69a`). L'issue #142 reprend la
liste ci-dessous avec des cases à cocher.

## Priorités

Les tâches ci-dessous sont triées de la première à traiter à la dernière. Les
critères, dans l'ordre : la sûreté des bornes d'abord (aucune borne fausse n'est
connue) ; ce qui coûte plus cher en attendant (une décision qui bloque,
l'historique avant la fusion dans `master`) ; ce qui tue un programme ou rompt
une promesse écrite avant ce qui ne fait que ralentir ; le petit et sans
dépendance avant le gros ; la version (Y) en dernier, puis ce qui lui est
postérieur. Le détail de chaque tâche est dans la section de son point plus bas.

### A. Décisions courtes, prises le 19 octobre

Plus rien à décider ici : les trois choix sont faits, et leurs suites sont dans
les rangs 8 et 10 et dans le groupe E.

1. **123 (AL)** : *décidé le 19 octobre* : laisser les commits qui portent une
   ligne de crédit (24 « Co-Authored-By: Claude Opus 5 » du 14 au 21 septembre,
   dont 16 dans `master`, et 6 « Co-authored-by: Jordan08 » de #99), sans
   réécrire `configure-clean`. Reste à retirer les lignes des descriptions et
   des commentaires de pull requests (rang 10).
2. **Les études (AH, AI, AJ ; l'AN est faite par #154)** : *décidé le 19
   octobre* : toutes avant v5.0.0, donc le groupe E en entier, y compris l'AJ
   (113), qui peut changer ce que la bibliothèque promet (le sens d'arrondi du
   programme entre `gaol::init()` et `gaol::cleanup()`) et les noyaux, donc les
   temps du 33.
3. **88 et 87 (AD)** : *décidé le 19 octobre* : on garde la décision du 3
   octobre pour la lecture en temps quadratique (4,4 s pour 320 000 chiffres) et
   on ne change pas le lexeur pour le signe devant un nombre incertain : les
   deux se documentent (rang 8).

### B. Corrections du code, décidées, du plus important au moins important

Aucune borne fausse n'est connue : ces défauts tuent un programme (SIGFPE avec
les exceptions activées, plantage), rendent un résultat que la norme dit autre,
ou rompent une promesse de la documentation. Tailles : S, moins d'une
demi-journée ; M, un à deux jours ; L, plusieurs jours ou incertain (estimées
d'après les descriptions).

4. **78 (AB)** : `cancel_minus` et `cancel_plus` lèvent `FE_INVALID` quand la
   différence déborde ; quelques lignes. [S]
5. **81 (AC)** : `nth_root` d'un sous-normal loin des bornes les plus serrées
   que `doc/accuracy.md` promet (les bornes restent justes). [M]
6. **79 (AB)** : `FE_OVERFLOW` levé pour des résultats finis (`sqrt`, les
   racines, `sin`, `cos`, `tan`, `div_rel`, les `*_rel`, et la lecture de
   `1e400`) ; décidé, mais il touche beaucoup de fonctions et ne se voit
   qu'avec le piège activé près de DBL_MAX. [L]

### C. Textes, tests et ménage

Petits, sans risque, à grouper en une ou deux pull requests ; `process.md`
d'abord, puisqu'il guide chaque nouvelle session. [S]

7. **118 (AL)** : `process.md` (la machine d'Ubuntu 24.04, `--no-track`, le §12
   périmé, `-DGAOL_ARM64=OFF` avec la variante FPU).
8. **80 (AB), 117, 119, 120 (AL), 87, 88, 89 (AD) et les descriptions
   d'options du 112 (AI)** : des textes faux ou qui manquent (l'exemple
   `1/[0, 1]`, la phrase de `pow` d'`accuracy.md`, les compilateurs refusés du
   manuel, le piège de `namespace gaol { }`, le format `agreeing` d'un point, «
   + - * / et sqrt » du chemin AVX-512, le signe collé à un nombre incertain du
   87, la lecture en temps quadratique du 88).
9. **91 et 94 (AE)** : le test `refused_options.sh` qui se dit sauté, et la
   liste de macros de l'audit.
10. **123 (AL)** : retirer les lignes de crédit des descriptions de #15 à #24 et
    #60 à #63 et des commentaires de #58 et #60 à #63 (les commits restent,
    décidé le 19 octobre).
11. **121, 122 (AL)** : décider de `.continue/` ; `gaol_performance` en C++14.
12. **`/W4 /WX` en Debug (AI, #148)** : `textToInterval()` de
    `gaol_interval.cpp` a un `return` après `gaol_ERROR()`, que Visual C++ 19.44
    et 19.51 disent inatteignable à `/Od` (C4702, vu sur Compiler Explorer, sur
    le fichier assemblé avec ses en-têtes) ; aucun job ne compile en `/W4 /WX`
    en Debug. À corriger, avec un job. [S]

### D. Les trois builds

Principe 6 de `process.md` : les mêmes bibliothèque, en-têtes et options
partout.

13. **Le défaut de CMake du 112 (AI)** : un second `cmake` dans le même dossier
    fait passer `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING` ;
    à corriger avec un test. [S]
14. **125 (AM)** : décider si `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT`
    restent sans valeur par défaut dans les trois modèles. [S]
15. **95 et 92 (AE)** : des détections qui lient dans un build et compilent dans
    les autres, des drapeaux C différents ; aucun effet connu sur les bornes.
    [M]
16. **85 (AD) et 82 (AC)** : un `long` de 32 bits (non vérifié) et la garde de
    `[±oo]` de `powRev1` et `powRev2` (qu'`interval(__m128d)` seul atteint) ;
    rares. [S]

### E. Études de vitesse

Des mesures d'abord, toutes avant v5.0.0 (décidé le 19 octobre) ; ordre : ce qui
peut changer la suite, puis l'écart mesuré le plus grand sur les builds par
défaut, puis les builds avec `GAOL_PRESERVE_ROUNDING`.

17. **113 (AJ)** : des noyaux qui laissent le sens d'arrondi au plus proche ; un
    prototype de `+ - * / sqrt` suffit à décider. Peut changer le comportement
    documenté et les noyaux (voir le rang 2). [L]
18. **106 (AI)** : Visual C++ arm64 (NEON n'y est pas offert), ARM 32 bits,
    POWER, s390x et RISC-V gardent les intervalles FPU, sans les noyaux de
    #124 ; macOS arm64 est fait par #146 (NEON), Visual C++ et clang-cl x64 par
    #148 (SSE2) ; sur ARM 32 bits, #150 n'a fait que le registre de contrôle de
    `GAOL_PRESERVE_ROUNDING`, pas les noyaux. [L]
19. **110 (AI)** : GCC pour Windows, `pow(x, 3)`, `log` et `cos` 5 à 10 fois
    plus lents que Clang, faute de `-mfma` probablement. [M]
20. **108 (AI)** : Visual C++ 32 bits, 80 ns par opération dès le build par
    défaut. [M]
21. **109 (AI)** : `GAOL_PRESERVE_ROUNDING` à 24 ns par opération avec Visual
    C++ x64 depuis #148 (43-87 avant, 2,5 sans) ; la même condition que les
    macros MXCSR de #124 avec `_M_X64` et `__AVX2__`, décidé le 20 octobre pour
    une pull request qui suit #148 ; le 107, sur macOS arm64, est fait par #146.
    [M]
22. **105 (AH)** : `-mbranches-within-32B-boundaries`, jusqu'à 1 ns par
    opération selon le placement du code. [M]
23. **116 (AI)** : des écarts sans cause trouvée, à refaire d'abord avec Clang
    21. [S]
24. **111 et 115 (AI)** : mesurer par la CI deux builds alternés, et lancer
    `run_lines.sh` dans `make perf` ; les temps de la CI ne sont pas stables.
    [M]
25. **112 (AI), le reste** : les fonctions élémentaires avec
    `GAOL_PRESERVE_ROUNDING` (jusqu'à 27 %), un test des bits du signe de zéro
    de `x * d`. [M]

### F. Signalements à d'autres projets

Chacun attend l'accord explicite du mainteneur ; rien n'est envoyé. Ordre : ce
qui est prêt et qui a une valeur pour GAOL, puis ce qui demande une réplique à
écrire, puis ce qui ne demande probablement rien.

26. **#125 CORE-MATH** : les huit correctifs prêts (le 8, `exp_1()`, est fait
    dans la copie de GAOL par #144), refaits sur le master du jour.
27. **#126 GCC**, **#127 mingw-w64**, **#129 ITF1788**, **#130 Boost.Interval**.
28. **#131 Microsoft**, **#132 Apple**, **#133 LLVM/Clang**, **#135 musl**,
    **#136 le benchmark de Tang et al.** : une réplique d'un fichier à écrire et
    à vérifier sur la plateforme.
29. **#134 gmpy2**, **#137 libieeep1788**, **#138 la glibc** : probablement rien
    à envoyer ; vérifier, puis fermer.

### G. La version, dans cet ordre

En dernier : elle suppose les corrections faites et les études du groupe E
faites (décidé le 19 octobre, rang 2).

30. **La pull request de synthèse** (Y) : `ChangeLog` et `doc/differences.md`,
    d'après `todo-notes/synthese.md` et les 43 descriptions de pull requests.
31. **`examples/examples.md` et le manuel** (Y) : les marques **Fixed**, une
    seule régénération de `manual/v5/gaol.pdf`.
32. **32 (Y)** : le rapport de couverture, et `numbers` en entier sous ASan +
    UBSan, une fois.
33. **33 (Y)** : les temps de `doc/compare/performance.md`, en dernier, sur le
    commit propre de la version, après les études du groupe E.
34. **34 (Y)** : `configure-clean` dans `MATH-CORE` (avance rapide), `MATH-CORE`
    dans `master`, l'étiquette `v5.0.0`, les recettes FetchContent.
35. **71 (Y) et #128** : l'annonce à Frédéric Goualard, après l'étiquette.

### H. Après la version

36. **70 (AK)** : le site GitHub Pages ; sans dépendance, mais l'annonce étant
    privée, rien ne l'exige pour la version.
37. **46 (F, #64)** : `operator>>` ; le comportement actuel (un intervalle par
    ligne) est documenté, le garder est la décision par défaut.
38. **26 (W, #67)** : décorations ou indicateur de sortie de domaine.
39. **28 (X)** : traits pour YalAA, backend pour VNODE-LP.
40. **Le ménage de la fin** : les images Docker de #111, `todo-notes/`, ce qu'il
    reste de `TODO.md`.

## Comment lire ce fichier

Les points sont nommés par des lettres (A à Z, puis AA à AO) : un point réunit
ce qui touche le même code ou le même fichier, ou ce qu'un ordre impose de faire
ensemble. Un point garde en sous-points les numéros de l'ancienne liste,
auxquels renvoient les pull requests, les issues et les rapports des agents ; la
table à la fin donne la lettre de chaque numéro encore à faire, et un numéro qui
n'y est pas est un point fait. Une lettre qui n'a plus de section ici est un
point fait (A à E, G à V, Z, AA, AF, AG, AN, AO, et ZZ, la vitesse de `+ - * /`,
par #124) : ses pull requests sont dans #49, et les commentaires du code qui
citent « point Q of TODO.md » renvoient à ce point fait. « Décidé le 3 octobre »
(ou une autre date) marque une décision du mainteneur : applique-la telle
quelle. Une question reportée renvoie à son issue.

Les anciens numéros 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro n
de sa section 5) ; 41 à 44, de la vérification de `VERSION.txt` ; 45 à 76, des
corrections, des relectures des pull requests, du tri de `TODO_mistral.md` et
d'une comparaison de `pown` avec le `pow` de CORE-MATH ; 77 à 103, de la
relecture de `configure-clean` du 14 octobre ; 104 à 112, des décisions du point
AA et des mesures de la CI de #124 ; 113, d'une question du mainteneur du 18
octobre ; 114 à 127, de la relecture des notes et des commentaires des pull
requests fermées et de l'issue #49 (18 octobre). Ce qui sert encore des rapports
des agents est dans [todo-notes/synthese.md](todo-notes/synthese.md), leurs
originaux au commit `16a2f60`.

## En cours

Quatre pull requests sont ouvertes : #152 (SSE2 sur Windows 32 bits avec GCC et
Clang), #156 (point 108, x86 32 bits), #157 (point AI.106, les noyaux scalaires
des intervalles FPU, brouillon) et #158 (point AC.81, `nth_root` d'un
sous-normal).

## À corriger avant la version

Trouvé par la relecture de `configure-clean` à `533992f`, le 14 octobre au soir,
par six relecteurs indépendants (les puissances entières, le `pow` réel et ses
réciproques, les autres opérations, la lecture et l'écriture, les trois builds,
la documentation) : chaque défaut a été reproduit par un programme, sauf ceux
dits non vérifiés, avec les builds CMake SSE2 et FPU (GCC 15.2), Clang 18 et 21,
et ASan + UBSan, contre des références calculées à part (rationnels exacts,
mpmath, MPFR par gmpy2). Aucune borne fausse : sur environ 1,3 million de
puissances entières, 550 000 `pow` réels, 600 000 autres opérations et 760
lectures de nombres, tout résultat contient la valeur exacte.

Revérifié le 18 octobre à `4eaf69a` (build CMake Release SSE2, GCC 15.2,
programme de reproduction) : 78, 79, 80, 81, 83, 84, 87, 88 et 89 se
reproduisent encore, tels que décrits (les 77, 83 et 84 sont faits, par #144 et
#149) ; 91 et 94 ont
été relus dans les sources.
Les autres (82, 85, 92, 95) n'ont pas été revérifiés. Le 86 (`_iv` au-delà
de 2^64) est sans objet : `operator""_iv` et `gaol/gaol_literals.h` ont été
retirés à partir du 16 octobre ; le 93 (code mort exporté sous des noms C
génériques) est fait depuis le 15 octobre (`gaol_exact.*`, `sysdeps/` et
`s_nextafter.c` supprimés).

### AB. Des exceptions flottantes levées à tort (78, 79, 80)

- **78.** **`cancel_minus` et `cancel_plus` lèvent `FE_INVALID` quand la
  différence des bornes déborde** : `cancel_minus([DBL_MAX], [-DBL_MAX])`,
  l'exemple de la note de 12.12.5 d'IEEE 1788-2015, rend bien [DBL_MAX, +∞],
  mais `two_sum()` (`gaol/gaol_interval_core.cpp`) calcule ∞ − ∞ avant de
  traiter la somme infinie ; de même `cancel_plus([DBL_MAX], [DBL_MAX])`. Le
  test de `tests/rounding_direction.cpp` ne prend que [DBL_MAX] − [1], qui ne
  déborde pas. Correction : e = 0 quand la somme est infinie, avant le calcul de
  ses termes, et [M] − [−M] dans le test. Revérifié le 18 octobre : les deux
  lèvent `FE_INVALID` (et `FE_OVERFLOW`).
- **79.** **`FE_OVERFLOW` levé pour des résultats sans borne infinie** :
  `sqrt([1, DBL_MAX])` (le produit de vérification u·u = 2^1024),
  `nth_root([DBL_MAX], 3)`, `sin([-1, DBL_MAX])`, `tan([DBL_MAX])`,
  `div_rel([1], [2^-1074, 1], [1, 2])`, `sqrt_rel([DBL_MAX], [-3, 0])`, et
  `cos`, `nth_root_rel`, `atan_rel`, `atan2_rel_x` et `atan2_rel_y` dans des
  tirages au hasard. `doc/using.md` (section sur les exceptions flottantes) et
  le manuel disent que `FE_OVERFLOW` ou `FE_DIVBYZERO` signalent une borne
  infinie de l'intervalle. Décidé le 14 octobre : corriger le code, et non les
  textes ; mettre à l'échelle les vérifications de `sqrt` et des racines, et
  chercher les autres débordements intermédiaires (`sin`, `cos`, `tan`,
  `div_rel`, les fonctions `*_rel`), sans calcul qui puisse déborder pour un
  résultat fini. Revérifié le 18 octobre : les six premiers cas lèvent
  `FE_OVERFLOW`. Lire `1e400`, ou tout nombre au-delà du plus grand double,
  lève aussi `FE_OVERFLOW` : le `strtod()` du lexeur (`gaol_enclose_number()`,
  `gaol/parser/gaol_interval_lexer.lpp`) le calcule, et un programme qui a
  activé le piège meurt à la lecture ; déjà ainsi avant #149, qui l'a relevé.
- **80.** **`1/[0, 1]` ne lève plus la division par zéro depuis #121**, mais
  `doc/using.md` (« so does `1/[0, 1]`, which is `[1, +oo]`, with the SSE2
  intervals ») et le manuel (« so does `1/interval(0, 1)` ») le donnent encore
  en exemple, contre `doc/tests.md` (section de `rounding_direction`).
  Correction : garder `log([0, 1])` seul dans l'exemple. Revérifié le 18 octobre
  : `1/[0, 1]` ne lève aucun indicateur, et les deux textes sont inchangés.

### AC. `nth_root` d'un sous-normal et réciproques de `pow` (81, 82)

- **81.** **`nth_root(x, n)` et `rootn` d'un x sous-normal donnent des bornes
  très loin des plus serrées** : `nth_root([2^-1074], 33)` est
  [0x1.5ecf527769956p-33, 0x1.5ecf527771187p-33], et le plus serré
  [0x1.5ecf52776a115p-33, 0x1.5ecf52776a116p-33] (mpmath à 600 bits) : 1 983
  doubles sous la borne inférieure, 28 785 au-dessus de la supérieure. On trouve
  6,9e8 doubles pour n = 65, 5,9e9 pour n = 2^20 + 1, 68 739 pour x = 2^-1060 et
  n = 65, et de même avec `rootn(x, q)` pour q < 0. L'encadrement reste juste ;
  seules les racines d'ordre n ≥ 4 d'un sous-normal sont touchées, et les
  puissances exactes restent exactes. `doc/accuracy.md` (« the tightest or one
  double beyond, whatever the magnitude of x »), le commentaire de
  `newton_root()` et `doc/differences.md` promettent le contraire. Cause : les
  preuves (`ipow_hi()` et `ipow_lo()`, appelées par `is_proved_below_root()` et
  `is_proved_above_root()` dans `gaol/gaol_interval_core.cpp`) calculent par
  exponentiation binaire une puissance intermédiaire sous-normale : pour n = 33
  et x = 2^-1074, l^32 ≈ 2^-1040 n'a plus qu'environ 34 bits. Correction :
  prouver avec une puissance mise à l'échelle, qui ne devient jamais
  sous-normale, par exemple celle de `pown_point()`, qui ramène déjà c à [1, 2)
  en gardant l'exposant à part ; ou corriger les textes. Revérifié le 18 octobre
  : `nth_root([2^-1074], 33)` rend [1.595298509019989e-10,
  1.595298509027944e-10], dont les bornes diffèrent dès le douzième chiffre.
- **82.** **`pow_rel` et `pow_exponent_rel` (`powRev1`, `powRev2`) n'ont pas la
  garde de `[±oo]` de `pow`** : `powRev1([+oo], [2], R)` rend [1, 1 + 2^-52],
  `powRev2([+oo], [2], R)` [0, 2^-1074], `powRev1([2], [+oo], R)` [DBL_MAX, +∞]
  et `powRev1([-oo], [0, 1], R)` [1, +∞], là où `pow(x, [±oo])` est vide, et ces
  réciproques avec lui. Ces résultats restent des encadrements, et ces arguments
  ne se construisent qu'avec `interval(__m128d)`, dans les builds SSE2.
  Correction : le test `!(l < +oo && u > -oo)` de `pow_standard_upward()` dans
  `pow_rel_upward()` et `pow_exponent_rel_upward()`
  (`gaol/gaol_interval_core.cpp`).

### AD. La lecture et l'écriture des intervalles (85, 87 à 89)

- **85.** **Le plafond de l'exposant déborde un `long` de 32 bits** (Windows,
  i386, armhf) : `k`, `k2`, `kb`, `exponent` et `q` sont des `long`
  (`gaol/parser/gaol_interval_lexer.lpp`), mais le plafond dépasse 2^31 − 1 pour
  un lexème de plus de 536 870 411 caractères, en deçà des 2^29 caractères
  au-delà desquels le lexeur lève `std::bad_alloc` : `"1e"` suivi de 536,8
  millions de 9 serait lu [0, 2^-1074] au lieu de [DBL_MAX, +∞]. Non vérifié,
  faute de chaîne 32 bits sur la machine. Correction : ces variables en `long
  long`.
- **87.** **Un nombre incertain juste après un `+` ou un `-` binaire est
  refusé** : `10 - 3.56?1` est lu, mais `10-3.56?1`, `10+3.56?1` et `[1,2]+3?`
  lèvent `input_format_error` (le vide avec `gaol_ieee1788`), le lexeur prenant
  `-3.56?1` pour un seul lexème, alors que le manuel dit que ces littéraux se
  combinent aux opérateurs dans n'importe quel ordre. Décidé le 19 octobre : le
  documenter sans changer le lexeur, donc corriger le manuel et `doc/` (une
  espace est nécessaire : `10 - 3.56?1`) ; l'autre voie était de sortir le signe
  du lexème en le gardant pour les directions `u` et `d` (`-10?u` est [−10,
  −9,5]). Revérifié le 18 octobre.
- **88.** **Lire un long nombre prend un temps quadratique en ses chiffres** :
  `"0."` suivi de n chiffres 3 prend 0,017 s pour n = 20 000 et 0,28 s pour 80
  000 (le 14 octobre : 0,026 s, 0,37 s, et 4,4 s pour 320 000, environ 45 s
  attendues pour un million), et la forme incertaine 2,3 fois plus
  (`gaol_take_apart` et `gaol_natural_mul_pow5` dans le lexeur, un chiffre à la
  fois). Un texte venu d'ailleurs peut ainsi bloquer un programme. Le 3 octobre,
  l'ancien 18 (#42, la lecture sous une locale à virgule) a décidé de laisser
  l'analyse des chiffres quadratique (« on ne fait rien ») : ce défaut en est la
  conséquence. Décidé le 19 octobre : garder cette décision, et documenter dans
  le manuel et `doc/` que la lecture est quadratique en le nombre de chiffres,
  avec les temps ci-dessus. Pour mémoire, `todo-notes/synthese.md` (18.md) note
  que 9 chiffres décimaux par passe l'accéléreraient environ 9 fois, et qu'une
  conversion par dichotomie la rendrait sous-quadratique (garder environ 770
  chiffres significatifs et un chiffre non nul « collant », aucun double ne
  séparant alors les deux valeurs, et convertir 9 chiffres par multiplication).
- **89.** **Le format `agreeing` écrit un intervalle ponctuel comme un nombre
  nu** : `interval(4.0)` s'écrit `4.000000000000000`, quand le format `bounds`
  écrit `[4]` ; ni le commentaire d'`interval_format`
  (`gaol/gaol_interval_core.h`) ni le manuel ne le disent. À documenter. (Le
  tableau de `doc/accuracy.md` le dit déjà.)

### AE. Les trois builds (91, 92, 94, 95)

- **91.** **`tests/refused_options.sh` se dit sauté quand le programme ne
  compile pas** : il reconnaît GCC ou Clang en prétraitant le source du test
  avec `-dM -E`, et si les en-têtes de GAOL manquent, g++ n'écrit rien : le
  script dit « SKIP: neither GCC nor Clang » (77) au lieu d'échouer (vérifié
  avec `GAOL_CXXFLAGS="-std=c++17 -frounding-math"`). Correction : `$GAOL_CXX
  $GAOL_CXXFLAGS -dM -E -x c++ /dev/null`. Relu le 18 octobre : le script
  prétraite encore le source du test.
- **92.** **Les sources C n'ont pas les mêmes drapeaux dans les trois builds** :
  CMake leur donne `-msse2 -msse3 -fvisibility=hidden`, autotools et meson au
  C++ seulement ; `-ffloat-store` va au C et au C++ avec CMake et meson, au C++
  seul avec autotools ; le C est gnu99 avec CMake, c99 ou gnu11 avec meson,
  celui du compilateur avec autotools. `.github/audit` ne compare que les
  drapeaux de `gaol_interval.cpp`. Correction : les mêmes drapeaux C partout, et
  un source C dans `probe_build.py`. (L'effet visible du 14 octobre, les
  symboles `get_inexact` et `clear_inexact` exportés par les bibliothèques
  partagées d'autotools et de meson, a disparu avec le code mort supprimé le 15
  octobre.)
- **94.** **La liste de macros de l'audit ne s'accorde pas avec la sonde** :
  `.github/audit/make_probe.py` sonde `GAOL_RND_SSE_REGISTER`, que
  `.github/audit/compare.py` ne compare pas (les macros d'AVX-512 que le 94
  citait le 14 octobre sont depuis dans les deux listes). Correction : l'ajouter
  aux `MACROS` de `compare.py`.
- **95.** **Les détections compilent dans un build et lient dans les autres** :
  la sonde de la cible AVX-512 lie avec CMake, mais ne fait que compiler avec
  autotools (`AC_COMPILE_IFELSE`) et meson, alors que `__builtin_cpu_supports`
  demande `__cpu_model` à l'édition de liens ; la sonde de FMA, à l'inverse, ne
  vérifie que le C++ avec CMake, et le C et le C++ avec édition de liens dans
  les deux autres. Non vérifié sur une chaîne où le résultat changerait.
  Correction : lier, et vérifier les deux langages, dans les trois builds.

### AL. Les restes des relectures, de `process.md` et de la doc (117 à 123)

Relevés dans les « Questions ouvertes » des pull requests fermées, dans leurs
commentaires et dans les décisions du 18 octobre de l'issue #49 ; chacun est
petit.

- **117.** **`doc/accuracy.md`, la ligne de `pow`** : supprimer la phrase « 4
  doubles on the special cases » (décidé le 18 octobre, question de #99) : elle
  venait de l'ancien exp(y log x), et #118 calcule les coins avec le `pow` de
  CORE-MATH ; la ligne cite déjà les 1 347 cas de `pow` d'ITF1788 (les plus
  serrés), les 117 boîtes de `ieee1788` et les 84 070 boîtes aux coins.
  Revérifié le 18 octobre : la phrase y est encore.
- **118.** **`process.md`** : retirer, au §6.1, le paragraphe du 12 octobre qui
  décrit une machine Ubuntu 24.04 (GCC 13.3, CMake 3.28, sans autotools ni
  meson), en gardant « Vérifie donc les versions au début du travail » (décidé
  le 18 octobre) ; écrire `git switch --no-track -c` au §7.3 (question de #109 :
  avec la synchronisation de VS Code, des commits de branche sont arrivés
  directement dans `configure-clean` le 6 octobre, c7915a7 et ccf694a) ; refaire
  le §12 « Où en est le travail » et l'en-tête « État au 13 octobre » (B, Z, AA,
  I, AF faits, AB à AE et le reste à faire d'ici) ; au §6.2, écrire
  `-DGAOL_ARM64=OFF` avec `-DGAOL_SIMD=OFF` pour la variante FPU, qui sur arm64
  prend sinon NEON (l. 289 de `process.md`, comme l. 107 de
  `todo-notes/orchestration/HOUSE_RULES.md` ; remarqué par #146).
- **119.** **Le manuel, « Compilers and options refused »**
  (`manual/v5/gaol.tex`) : la phrase « Each of the following gave bounds not
  enclosing the exact results » est suivie de l'entrée de MinGW-w64 pour ARM «
  not tested » (remarqué par #111, déjà ainsi avant la branche). Reformuler.
- **120.** **Le code écrit à l'intérieur de `namespace gaol { }`** voit les
  using-declarations de `gaol` (#87) : un appel `sqrt(2.0)` y trouve
  `gaol::sqrt`, qui cache `::sqrt`, et ne compile plus. C'était déjà le cas avec
  GAOL 4, mais ce n'est écrit nulle part (ni `doc/using.md`, ni le manuel). À
  ajouter aux pièges de « Common errors ».
- **121.** **`.continue/` dans `.gitignore` ?** (question de #103, restée sans
  réponse) : #103 a retiré `.continue/rules` du dépôt ; si l'extension Continue
  de l'éditeur le relit en local, le fichier réapparaîtra comme non suivi.
- **122.** **`gaol_performance` compilé en C++17 sans en avoir besoin**
  (remarqué par #95) : `tests/CMakeLists.txt` lui donne `CXX_STANDARD 17`, alors
  que `tests/performance.cpp` compile en C++14 (pas de littéral hexadécimal) ;
  de même dans `tests/Makefile.am` et `tests/meson.build`. Hors du reste, sans
  effet.
- **123.** **Les lignes de crédit qui restent** (règle : aucune ligne qui
  crédite Claude ou Claude Code dans un commit, une issue, un commentaire ou une
  description) :
  - les descriptions de #15 à #24 se terminent par « Generated with Claude Code
    » (elles n'étaient pas dans la liste du ménage du 3 octobre), celles de #60
    à #63 et un commentaire de chacune de #58 et #60 à #63 par « Generated by
    Claude Code » : à retirer par `gh pr edit` et `gh api` ;
  - 24 commits de `configure-clean` (du 14 au 21 septembre, dont 16 sont dans
    `master`) portent « Co-Authored-By: Claude Opus 5 », et 6 commits de #99 (6
    et 7 octobre) une ligne « Co-authored-by: Jordan08 », trois d'entre eux avec
    un corps de 9 à 13 lignes : on ne les corrige pas sans réécrire l'historique
    de branches partagées. Décidé le 19 octobre : les laisser, sans réécrire
    l'historique avant le point Y (qui fusionne `configure-clean` dans
    `MATH-CORE` puis `master`).

### AM. Les suites du point Z (125)

- **125.** **`GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT`** restent définis
  par `<limits.h>`, sans valeur par défaut dans les trois gabarits de
  `gaol_configuration.h` : un `int` qui n'aurait ni 4 ni 8 octets les laisserait
  non définis sous `-Wundef` (question ouverte de #120, case de #49). À décider
  : les laisser, ou leur donner une valeur par défaut. Avec cela, **les tests et
  les `.cpp` de la bibliothèque** ne sont pas sous `-Wundef` (`GAOL_TESTS_SSE`,
  `GAOL_PREFER_AVX512`…) : `headers.sh` ne vérifie que les en-têtes installés,
  et c'est le périmètre décidé le 13 octobre.

## Reportés, en attente d'une décision du mainteneur

### F. `operator>>` : relire `os << x << ' ' << y` (46, #64)

`operator>>` lit un intervalle par ligne (`std::getline(is >> std::ws,
buffer)`). Un intervalle contient des espaces (`[1, 2]`) et la grammaire admet
des littéraux imbriqués et des expressions (`[[1, 2], 3]`, `sqrt(2)+1`, `1 -2`)
: la fin d'un intervalle dans un flux est ambiguë, et `os << x << ' ' << y` ne
se relit pas ; avec un saut de ligne (`os << x << '\n' << y`) il se relit. À
décider (issue #64) : garder un intervalle par ligne (documenté, comportement
actuel) ; n'accepter dans `operator>>` que les littéraux (`[..]`, `<..>`,
`empty`, nombres), lus jusqu'au crochet fermant, et garder la lecture par ligne
pour le reste ; ou une fonction à part qui lit un intervalle dans un flux.
Reporté le 3 octobre.

### W. Des décorations (26, #67)

GAOL n'a que des intervalles nus (clause 11 d'IEEE 1788-2015) : `sqrt` d'une
boîte négative est vide et passe tous les tests d'inclusion,
`interval(DBL_MAX*10)`, `x + INFINITY` et `x * NAN` sont vides sans rien dire,
et `gaol_ieee1788::textToInterval` rend l'ensemble vide pour un texte mal formé,
qu'on ne distingue pas de `[empty]`. À choisir (issue #67) : les décorations
d'IEEE 1788 (plus complet, mais change l'interface et le coût) ; un simple
indicateur local au thread, positionné quand un argument sort du domaine (c'est
le découpage prévu dans `todo-notes/orchestration/points_w4.py`) ; ou seulement
documenter (les pièges de « Common errors », fait par #140). Reporté le 3
octobre ; après la version.

### X. YalAA et VNODE-LP (28)

Des en-têtes optionnels au-dessus du cœur scalaire : promouvoir
`examples/box.h`, `examples/dual.h` et `examples/affine.h` (un type boîte, la
différentiation directe, les formes affines, dont se servent les exemples 03 à
14 et qui ne sont pas installés) en `gaol/box.h`, `gaol/dual.h` et
`gaol/affine.h` ; ou des traits GAOL pour YalAA et un backend intervalle pour
VNODE-LP. Décidé le 3 octobre : les traits pour YalAA et le backend pour
VNODE-LP, et non la promotion des en-têtes des exemples. Le plan détaillé est
dans `todo-notes/orchestration/points_w4.py` (son plan du 28 reprend la
promotion des en-têtes, que la décision du 3 octobre écarte : la décision
l'emporte). Après la version.

### Remarqué par #146, à trier

Les questions ouvertes de la description de #146, non corrigées et sans numéro ;
le mainteneur décide lesquelles deviennent des tâches.

- **`bisect(ratio)` et `is_bisectable()` dépendent du mode FZ de l'appelant**
  avec `GAOL_PRESERVE_ROUNDING` (`LEAVE` rétablit FZ avant les comparaisons de
  `c` avec `l` et `r`), pour les sous-normaux : 634 résultats faux sur 200 000
  intervalles sous-normaux, avec les intervalles NEON comme FPU. L'encadrement
  n'est pas violé ; les post-conditions documentées le sont (point Q).
- **`pow(x, 3)` à 6,5 ns sur arm64** : le noyau empaqueté de `pown`
  (`GAOL_POWN_PACKED`, FMA sur deux voies) n'existe que pour SSE2 ; un
  `vfmaq_f64` ne gagnerait qu'un facteur de l'ordre de 1,5, pour 150 lignes
  d'intrinsèques SSE2 à porter sans pouvoir les tester avec GCC.
- **`sqrt([DBL_MAX])` lève `FE_OVERFLOW`** avec NEON comme avec les intervalles
  FPU (le produit `u*u` du test d'exactitude déborde) : c'est le 79, qu'un
  produit fusionné `fma(u, u, -v)` réglerait.
- **`doc/compare/itf1788/itf1788.py`, l. 821**, nomme « SSE2 » le build par
  défaut, qui est NEON sur arm64.
- **Le signe d'un zéro** diffère de celui des intervalles FPU et SSE2 avec NEON
  (documenté, comme pour AVX-512) : aucun test ne compare les bits d'un zéro.
- **Pas de job** pour Clang 15 à 17 ni GCC 9 à 10 sur arm64, et aucun test de
  `mrs` et `msr` sur Neoverse (le coût et la présence de FIZ ne sont pas
  mesurés ; #150 mesure seulement, sur les runners arm64 de GitHub, les
  intervalles NEON avec FPCR 1,4 à 2,0 fois plus rapides que les intervalles
  FPU avec la bibliothèque C, avec `GAOL_PRESERVE_ROUNDING`).
- **Le nom de `float64x2_t` est décoré autrement** par GCC et par Clang sous
  Linux : une bibliothèque installée construite par l'un et liée à un programme
  compilé par l'autre ne lierait pas, comme pour `__m128d` avec SSE2 (`gaol.pc`
  et `gaol::gaol` donnent le compilateur du programme).

### Remarqué par #147, à trier

Les questions ouvertes de la description de #147, non corrigées et sans numéro :

- **`gaol::textToInterval(nullptr)`**, `textToInterval(nullptr, "1")`,
  `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)`
  prennent un `const std::string&` : le `std::string` est construit à l'appel,
  avant GAOL, ce que le standard laisse indéfini. Avec libstdc++ (GCC 13.3), les
  quatre lèvent `std::logic_error` (« basic_string: construction from null is
  not valid »), ni `input_format_error` ni l'ensemble vide ; avec libc++ et
  Visual C++, probablement un plantage dans `strlen`, non vérifié. Le changer
  demande une surcharge `const char*` : un choix d'interface.
- **Une `gaol_exception` construite avec un `file` nul** (`gaol_exception(f,
  l)`, `gaol_warning`, `gaol_error`) : `operator<<` envoie un `const char*` nul
  à un flux, qui passe à l'état `bad` avec libstdc++, et les diagnostics
  suivants sont perdus. Les macros de GAOL passent toujours `__FILE__` ; seuls
  des appels directs sont en cause. Peu prioritaire.

### Remarqué par #149, à trier

Les questions ouvertes de la description de #149, non corrigées et sans
numéro :

- **`[l, u]` ne départage pas** pi et un autre nombre de même encadrement
  (`[pi, 3.14159265358979323846]`), un nombre loin au-delà des doubles (plus de
  10^311 ou moins de 10^-331 en décimal, environ 2^1026 et 2^-1081 dans les
  autres bases : `[1e400, 1e399]`, `[1.8e308, 1e400]`), ni des bornes calculées
  (`[1/3, 0.3333333333333333]`) : le littéral est alors l'intervalle de la
  borne inférieure de l à la borne supérieure de u, un sur-ensemble du vide,
  sûr mais pas le plus serré (dit dans `gaol_parser.h`, le manuel,
  `doc/accuracy.md` et `doc/tests.md`). Pi demanderait un encadrement intégré à
  plus de chiffres ; les nombres loin au-delà, la comparaison de leurs
  exposants et de leurs premiers chiffres.
- **`<a, b>` accepte encore une borne calculée qui n'est pas un double**
  (`<[0.1], 0x1.9999999999999p-4>`, un littéral comme borne) : seuls les
  nombres écrits et pi sont refusés.
- **Le contexte d'une lecture garde le texte de chaque nombre qui n'est pas un
  double**, qu'un littéral `[l, u]` le compare ou non : une somme de 200 000
  termes `0.1` passe de 4,9 Mo et 0,106 s à 6,0 Mo et 0,134 s. Ne garder que
  les textes des bornes demande de savoir, à la lecture d'un nombre, s'il sera
  une borne.
- **Un piège pour `process.md`, section 11** : le test de
  `tests/expressions.cpp` compte les allocations par le `operator new` qu'il
  remplace, qui ne voit pas celles d'un `std::string` que fait la bibliothèque
  C++ quand elle est dans une DLL (Windows : libstdc++ de MSYS2 et de
  MinGW-w64, libc++ de CLANG64), alors que son destructeur en ligne appelle le
  `operator delete` du programme : un `std::string` de plus de 15 caractères
  dans le code de la lecture fait échouer « a reading frees all it allocated »
  (quatre jobs de #149). Un `std::vector` est sans danger. À ajouter à
  `process.md` une fois #155 fusionnée.
- **Le tableau de `doc/accuracy.md`** cite encore `interval(sl, su)` pour
  `textToInterval(s)`, qui n'existe plus depuis GAOL v5.

### Remarqué par #150, à trier

Les questions ouvertes de la description de #150, non corrigées et sans
numéro :

- **Le mode par défaut des intervalles FPU d'ARM 32 bits** n'est pas amélioré
  par #150 : c'est la suite du 106 (noyaux scalaires sans branche).
- **Les mesures** ne valent que pour les runners arm64 de GitHub, qui exécutent
  du 32 bits ; le coût de `vmrs` et de `vmsr` sur un Cortex-A 32 bits, et le
  gain, ne sont pas connus. Non vérifiés : bionic (le `static_assert` de
  `gaol_fpu_fenv.h` sur les valeurs de `FE_UPWARD`, `FE_DOWNWARD` et
  `FE_TOWARDZERO` le dirait à la compilation, les sources n'en ont pas été
  lues) et GCC avant 12. Clang reste refusé sur ARM 32 bits.
- **Le même contrôle pour x86** : l'ordre entre l'écriture du registre de
  contrôle et les opérations sur des registres (bogue 34678 de GCC) a été
  vérifié sur l'assembleur de toute la bibliothèque pour AArch64 et ARM 32 bits
  (GCC 14.3 croisés : aucune instruction flottante qui dépende du sens entre
  `mrs`/`vmrs` et `msr`/`vmsr`), pas pour x86, où `_mm_setcsr` n'est pas mieux
  ordonné. `width()` en est le cas connu ; à refaire avec un GCC pour x86.
- **Les sujets de deux commits** de la branche de #150 font plus de 100
  caractères (117 et 101) : déjà poussés, non réécrits.
- **Un piège pour `process.md`, section 11** : une écriture du registre de
  contrôle par `asm volatile` n'est ordonnée qu'avec les autres `asm`, pas avec
  les opérations sur des registres ; une fonction en ligne qui lit ses bornes
  avant `GAOL_RND_ENTER()` calcule dans le sens du programme. La règle est dans
  `gaol/gaol_fpu.h` (lire les bornes après la vérification, un double passé par
  valeur à travers `rnd_reread()`) ; un test doit appeler la fonction sur un
  intervalle construit dans la même fonction, pas sur un intervalle relu en
  mémoire, qui cache le défaut. À ajouter à `process.md` une fois #155
  fusionnée.

### Remarqué par #154, à trier

Les questions ouvertes de la description de #154, non corrigées et sans
numéro ; `doc/simd.md` répond aux quatre étapes de l'issue #101, qui peut être
fermée, et garde ce qui reste à essayer :

- **`FPCR.AH` (FEAT_AFP) n'est pas effacé** par les intervalles NEON
  (`GAOL_RND_FLUSH_BITS` : FZ et FIZ). Avec AH, `FMAX` renvoie le second
  opérande pour deux zéros de signes différents ; modélisé en logiciel sur 25,8
  millions de résultats, les bornes sont celles des noyaux, au signe d'un zéro
  près. Une seule dépendance : `sqrt_bounds()` suppose l'ordre des opérandes de
  `vmaxq_f64`, que Clang 21 traite comme commutatif ; un compilateur qui les
  permuterait donnerait l'ensemble vide pour `sqrt([-0, r])` sous AH. Défaut
  théorique, documenté dans la page. À décider : un masque sur le bit de signe
  à la place du maximum, validé sur une copie (bit à bit identique), en une
  pull request à part.
- **`pow(x, 2)` par `sqr`** : 5 à 6 ns contre 0,8 à 0,9 sur le M2, 40 % de
  l'exemple Goldstein-Price, mêmes valeurs sur trois millions d'intervalles
  (le signe d'un zéro peut différer) ; à faire ou non.
- **Les pistes à mesurer** : le RISC-V scalaire à `rm` statique (il faut une
  carte), POWER et s390x (du matériel), l'arrondi embarqué scalaire pour `/` sur
  les processeurs Intel qui divisent les registres de 512 bits. Et les gains
  sans nouveau jeu d'instructions que la page liste : `sqrt` en SSE2, les
  puissances entières en NEON, `mid()`, la réduction d'argument de `sin` et
  `cos`.
- **Les runners arm64 de GitHub exécutent l'armhf en natif** : Neoverse N2,
  « 32-bit EL0 Support » au démarrage, aucune entrée `qemu-arm` (job jetable du
  20 octobre). Les temps de #150 ne sont donc pas ceux d'un émulateur.
- **Le chemin générique de `round_upward()`** (`gaol/gaol_fpu_fenv.h`) appelle
  `fesetround(FE_UPWARD)` sans en lire le retour ; `NI` de POWER et `FS` de
  MIPS, états du même ordre que FTZ et DAZ, ne sont pas effacés par les
  intervalles FPU de ces processeurs.
- **Trois pièges de compilation**, qu'aucun code de GAOL n'emploie : la pragma
  `FENV_ROUND` (ignorée sans erreur par GCC, Visual C++ et Clang), le
  `_mm256_add_round_pd` de Clang 20 avec `-mavx10.2` et l'assembleur
  `vaddpd {ru-sae}, %ymm...` (assemblé par GNU as, `#UD` d'après le manuel
  d'Intel d'aujourd'hui).

## Études de vitesse

### AH. Les sauts et les frontières de 32 octets (105)

- **105.** **Étudier `-mbranches-within-32B-boundaries` pour les builds de
  GAOL**, décidé le 14 octobre à la fin du point AA, sur le Xeon E3-1220 v6 et
  sur l'i7-1185G7. L'erratum des sauts des processeurs Skylake à Comet Lake y
  déplace une opération jusqu'à 1 ns selon le seul placement du code (#122 : `x
  * y`, au code inchangé, passe de 4,96 à 4,69 ns ; `[1] / p`, de 7,0 à 8,1 ns
  quand seul `sqr()` change). À mesurer : le gain sur le Xeon, le coût sur l'i7,
  qui n'a pas l'erratum, et ce que les trois builds peuvent donner aux
  compilateurs qui prennent l'option.

### AI. La vitesse sur macOS, Windows et ARM (106 à 112, 115, 116)

La CI de #124 (`1a15fc8` et `c6a3d25`, 17 octobre) écrit le tableau de
`gaol_performance` dans l'étape « Performance » de ses jobs : c'est la seule
mesure que nous ayons sur macOS et sur Windows. Les noyaux de #124 (`+ - * /` et
`sqr` sans branche sur les signes, bornes en registre) ne servent que les
intervalles SSE2, c'est-à-dire x86-64 hors Windows 32 bits (Visual C++ et
clang-cl x64 compris depuis #148), et, depuis #146, les intervalles NEON
d'arm64 : les intervalles FPU ont encore le `switch` sur les signes et les
opérandes en mémoire que #124 a retirés des SSE2.
Pour la ligne de 13 opérations `+ - * /` du tableau, en ns : 33 sur Linux x86-64
(job d'autotools, Ubuntu 24.04), 36 sur macOS 15 x86-64, 33 à 58 avec MinGW-w64
et MSYS2, 64 à 93 sur macOS arm64, 100 à 168 avec Visual C++ x64 et arm64 et
avec clang-cl, 1 100 à 1 200 avec Visual C++ 32 bits (macOS arm64, avant #146 ;
avec NEON, la ligne passe de 49,8 à 19,2 ns sur un M2 ; avec les intervalles
SSE2 de #148, 49 ns avec Visual C++ x64 2022 et 2026 et 37 avec clang-cl, dans
la CI de `5fa7203`). Voir le 111 pour ce que valent ces temps.

- **106.** **Les intervalles FPU n'ont pas les noyaux de #124 : Visual C++ et
  clang-cl.** Avec Visual C++ x64 et arm64 et avec clang-cl, que CMake ne
  laissait pas prendre les intervalles SSE2 (`GAOL_SIMD AND NOT MSVC`, « IBEX
  and the fork of Fabrice Le Bars build GAOL without them »), `x * y` prend 6,6
  à 16,3 ns, `x / y` 4,2 à 8,2 et `sqr` 4,1 à 10,4, contre 2,1 à 4,1, 2,1 à 5,2
  et 2,1 à 3,1 avec MinGW-w64 et MSYS2 ; la ligne de 13 opérations, 100 à 168 ns
  contre 33 à 58. Les autres cibles FPU (POWER, s390x, RISC-V) n'ont pas été
  regardées ; ARM 32 bits, voir #150 plus bas.

  Fait par #146 pour macOS arm64 (où `x - y` prenait trois fois `x + y`) et
  Linux arm64 : des intervalles NEON, sur le modèle des SSE2 (`GAOL_ARM64`,
  `--enable-arm64`, `enable-arm64`, macro `GAOL_USING_NEON_INSTRUCTIONS`), avec
  GCC, Clang et AppleClang ; sur un M2, `x - y`, `x * y`, `x / y`, `sqr` et
  `sqrt` sont de 1,3 à 4 fois plus rapides. NEON n'est pas pris avec Visual
  C++, clang-cl, MinGW-w64, ARM 32 bits (pas de vecteur de doubles) ni dans un
  build universel de macOS. GCC pour arm64 et les Neoverse ne sont pas mesurés.

  Fait par #148 pour Visual C++ et clang-cl x64 : les intervalles SSE2 (CMake et
  meson ne les refusent plus ; le commentaire de CMake ne donnait aucune raison
  technique), les noyaux `add_bounds()` et les autres en `__vectorcall` (leurs
  `__m128d` passaient par un pointeur), et le chemin AVX-512 de
  `GAOL_PREFER_AVX512` (`cpu_runs_avx512f()` par `cpuid` et `xgetbv`, l'ordre
  des opérations tenu par des accès `volatile` et des appels, faute d'`asm`).
  Dans la CI de `5fa7203` (un passage, voir le 111), la ligne de 13 opérations
  prend 49 ns avec Visual C++ 2022 et 2026 et 37 avec clang-cl (100 à 168
  avant), `x * y` 3,5 à 4,5 ns (6,6 à 16,3 avant) ; l'effet de `__vectorcall`
  n'y est pas séparé. Les exécuteurs n'ont pas AVX-512 : le chemin AVX-512 de
  `cl` ne tourne que dans cinq jobs sous Intel SDE
  (`.github/scripts/sde-tests.sh`, l'action `petarpetrovt/setup-sde` épinglée
  sur un commit, dont la licence d'Intel est acceptée par le projet), jamais sur
  un processeur réel.

  Fait par #150 pour ARM 32 bits avec GCC, en partie : avec
  `GAOL_PRESERVE_ROUNDING`, le registre FPSCR est lu par `vmrs` et écrit par
  `vmsr` seulement s'il change (`GAOL_RND_FPSCR_DIRECT`, une macro interne
  documentée, pas une option des trois builds : décidé), au lieu de
  `fegetround()` et `fesetround()`. Dans la CI (Debian 13 armhf sur les runners
  arm64 de GitHub, médiane de trois passages), `x + y` passe de 48,0 à 22,5 ns,
  `x * y` de 57,6 à 33,6 et la ligne de 13 `+ - * /` de 668 à 349 ; sans la
  préservation rien ne change. La vérification par défaut de chaque opération
  garde la sonde par une addition, `vmrs` y coûtant plus que les deux additions
  (`x + y` 6,1 ns contre 4,1). Le même travail a trouvé et corrigé un défaut
  qui touchait aussi arm64 (#146) : avec GCC (bogue 34678) et avec Clang sous
  Linux, `width()` d'un intervalle construit dans la fonction et `bisect(ratio)`
  calculaient avant l'écriture du registre de contrôle, donc arrondis dans le
  sens du programme (#153, qui avait la même correction, est fermée).

  À chercher : pour Visual C++ arm64, offrir NEON (`arm64_neon.h`) ou les noyaux
  de #124 aux intervalles FPU ; pour ARM 32 bits, des noyaux scalaires sans
  branche (un produit en quatre produits, un quotient par masques, un carré sans
  branche), avec GCC seulement, non commencés ; POWER, s390x et RISC-V ne sont
  pas regardés. Les opérations avec un double n'ont ces noyaux sur aucune cible
  x86 (#124 : « `x * 2.0` n'a pas les noyaux de `x * y` »). À rapprocher de
  l'étude des pistes SIMD (`doc/simd.md`, #154).

- **108.** **Visual C++ 32 bits : 80 ns par opération, dès le build par défaut**
  (job « Visual Studio 2026 x86 Release » : `x + y` 80,8 à 83,5 ns, `x * y` 87,8
  à 98, une ligne de 15 `+` 1 211 à 1 307 ns). `gaol/gaol_fpu.h` le sait :
  `fegetround()` et la lecture de MXCSR y coûtent environ 80 ns, et la sonde du
  sens d'arrondi par une addition y est écartée (`GAOL_RND_PROBE` n'est pas
  défini sur x86 32 bits).

  À chercher : lire les deux unités, x87 et SSE, sans appeler `fegetround()`
  (`fnstcw` et `stmxcsr` en ligne, ou `_controlfp()`), et ce que cela coûte.

- **109.** **Visual C++ x64 avec `GAOL_PRESERVE_ROUNDING` : 24 ns par opération
  depuis #148** (43 à 87 avant : job « Visual Studio 2026 x64 Release, rounding
  direction preserved », `x + y` 43,4 puis 87,2 ns, une ligne de 15 `+` 653 puis
  1 294 ns ; avec les intervalles SSE2, 23,8 ns et 357 ns dans la CI de
  `5fa7203`, contre 2,5 ns et 37 à 54 ns sans `GAOL_PRESERVE_ROUNDING`). Les
  macros de #124 (`GAOL_RND_ENTER()` n'écrit que MXCSR, une fois à l'entrée et
  une fois à la sortie) demandent des intervalles SSE2 et `__FMA__`, que `cl` ne
  définit pas : il y reste `fegetround()`, `fesetround()` et deux écritures
  séparées. clang-cl, qui définit `__FMA__` avec `/arch:AVX2`, prend 22,6 ns
  pour `x + y` avec `GAOL_PRESERVE_ROUNDING` et le chemin AVX-512 refusé (518 ns
  pour la ligne de 15 `+`), sans que je sache pourquoi.

  À faire, décidé le 20 octobre, dans une pull request qui suit #148 : la
  condition des macros avec `_M_X64` et `__AVX2__` pour `cl`, qui ne définit pas
  `__FMA__` ; et ce qui coûte 22,6 ns avec clang-cl.

- **110.** **GCC pour Windows : `pow(x, 3)`, `log` et `cos` prennent 5 à 10 fois
  le temps de Clang** (MinGW-w64 14.2 et 15.2 et MSYS2 UCRT64 avec GCC 16,
  contre MSYS2 CLANG64 : `pow(x, 3)` 44 à 57 ns contre 4,1 à 9,0 ; `log` 78 à
  106 contre 19 à 35 ; `cos` 221 à 271 contre 32 à 51 ; `exp` 39 à 47 contre 24
  à 35). CMake ne donne pas `-mfma` à GCC pour Windows (« Fused multiply-add
  instructions left out: GCC for Windows does not align the stack for AVX »), de
  sorte que `pown` n'est pas calculé par paires et que CORE-MATH appelle un
  `fma()` logiciel : la cause la plus probable, que rien n'a mesurée.

  À chercher : mesurer un build de GCC pour Windows avec `-mfma` donné à la
  main, `rounding_direction` en particulier devant passer, puis voir si le
  défaut d'alignement se contourne (`-mstackrealign`,
  `-mprefer-vector-width=128`) pour que GAOL_FMA y soit pris.

- **111.** **Les temps de la CI ne se comparent pas d'un passage à l'autre.**
  Les runners sont des machines virtuelles partagées : `x * y` de Visual C++ x64
  prend 7,9 puis 16,3 ns, `sqrt` de MSYS2 avec GCC 16 6,8 puis 170 ns, entre
  #124 à `1a15fc8` et à `c6a3d25`, au code inchangé. Seuls des écarts de
  plusieurs fois valent, et l'étape « Performance » ne fait échouer aucun job.

  À décider : faire mesurer par la CI deux builds alternés dans le même job (par
  exemple `configure-clean` et la branche), comme `doc/compare` alterne ses
  programmes, ou n'afficher que le meilleur de plusieurs passages.

- **112.** **Suites de #124 non faites** (ses questions ouvertes) :
  - avec `GAOL_PRESERVE_ROUNDING`, les fonctions élémentaires paient encore deux
    écritures de MXCSR, jusqu'à 27 % de plus que sans (`exp`) (CORE-MATH calcule
    avec le sens matériel, aucun arrondi embarqué n'y est possible) ; un mode de
    CORE-MATH où le sens serait un paramètre les supprimerait, au prix de
    modifier `3rd/math-core` ;
  - **un défaut de CMake** : un second `cmake` dans le même dossier fait passer
    `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING`, `DEFINED`
    voyant l'entrée de cache du premier passage et la prenant pour une demande
    de la ligne de commande, alors que le commentaire du `CMakeLists.txt` dit
    que le défaut se dérive à chaque passage (contourné par
    `-UGAOL_PREFER_AVX512` dans `doc/compare/code/setup.sh`) ; le corriger, avec
    un test qui configure deux fois le même dossier ;
  - les descriptions des options listent encore « + - * / et sqrt » pour le
    chemin AVX-512, qui prend aussi `sqr` et les puissances entières
    (`doc/building.md`, `CMakeLists.txt`, `meson_options.txt`, `configure.ac`,
    `gaol/gaol_configuration.h.in` et `gaol/gaol_configuration_meson.h.in`) ;
  - avec `GAOL_PRESERVE_ROUNDING` et AVX-512, `x * d` et `x *= d` (un intervalle
    par un double) donnent le signe opposé pour une borne nulle de 344 résultats
    sur 5,7 millions entre le chemin AVX-512 (`[-0, +0]`) et le chemin SSE2
    (`[+0, -0]`), la valeur étant la même ; relevé par #124, déjà ainsi à
    `7716fec`, et aucun test ne le voit, qui comparent avec `==`. Décidé le 18
    octobre (point AF, #140) : c'est une exception acceptable au « bit for bit
    », dite dans la documentation et les commentaires, sans changer le calcul ;
    il ne reste que, si l'on veut, un test qui compare les bits.

- **115.** **`make perf` et l'étape « Performance » de la CI ne lancent pas les
  dix lignes de `run_lines.sh`** (question de #124) : seul `gaol_performance` en
  a cinq. À décider : les ajouter à la CI (avec le 111 : les temps n'y sont pas
  stables).

- **116.** **Des écarts de temps relevés par les pull requests, sans cause
  trouvée** : `cos(x)` 6 ns plus lent et `sin(x)` 2,4 ns plus rapide dans
  `gaol_performance` après #100 (Clang 18, i7-1185G7), alors que le code de
  `cos()` ne diffère que par l'ordre de deux branches ; `asinh_rel` et
  `nth_root_rel` plus lents de 16 % et 13 % avec Clang 18 après #117, avec moins
  d'instructions (311 contre 330 et 367 contre 399, comptées par valgrind), et
  `nth_root(x, -2)` à 22,9 ns au lieu de 18,2 avec `GAOL_PRESERVE_ROUNDING` (205
  instructions contre 266). `perf` n'est pas permis sur cette machine. Les
  mesures d'alors sont faites avec Clang 18 ; les refaire avec Clang 21, le
  compilateur des mesures depuis le 13 octobre, avant de chercher.

### AJ. Se passer du changement du sens d'arrondi (113)

Question du mainteneur, le 18 octobre : avec les algorithmes de base
d'aujourd'hui, le changement du sens d'arrondi est-il toujours nécessaire, les
calculs ne seraient-ils pas valables sans lui, et dans quels opérateurs ou
fonctions élémentaires l'est-il ? Ce qui suit vient de la lecture du code et de
la documentation ; rien n'a été écrit ni mesuré.

- **113.** **Étudier des noyaux qui laissent le sens d'arrondi au plus proche.**
  Les algorithmes actuels supposent l'arrondi vers le haut, et ne sont pas
  valables au plus proche : les noyaux SSE2 calculent `<-inf, sup>` par un seul
  arrondi vers le haut (`_mm_add_pd`, `_mm_mul_pd`, `_mm_div_pd` de
  `gaol/gaol_interval_sse.cpp`), et `sup` pourrait alors passer sous la valeur
  exacte ; CORE-MATH lit le sens dans MXCSR, `f_up(x)` y est la borne supérieure
  et `previous_float(f_up(x))` l'inférieure (`gaol/gaol_double_op.h`). Ce qu'il
  faut, c'est un arrondi dirigé, non un changement de sens :
  - sur le chemin AVX-512 (`GAOL_PREFER_AVX512`), `+ - * / sqrt sqr` et les
    puissances entières n'en changent pas, l'arrondi étant dans l'instruction
    (`er_mul2`, `_mm_div_round_sd`, `pown_large_er`) ; `GAOL_PRESERVE_ROUNDING`
    y coûte 1,0 à 1,5 fois (`doc/compare/performance.md`) ;
  - sur le chemin SSE2 seul, le sens vers le haut est indispensable, et
    `GAOL_PRESERVE_ROUNDING` y coûte deux écritures de MXCSR par opération
    (`sqr` : 8,1 ns contre 2,1 avec AVX-512) ;
  - les fonctions élémentaires et `pow` le demandent partout, CORE-MATH ne
    prenant pas le sens en paramètre (voir le 112).

  Ce que donnerait le sens laissé au plus proche, par des formules connues qui
  ne sont ici ni écrites ni testées :
  - `+` et `-` : TwoSum donne l'erreur exacte, hors dépassement, d'où les mêmes
    bornes que maintenant, bit à bit ;
  - `*` : l'erreur `fma(a, b, -p)` est exacte hors sous-dépassement, où il faut
    mettre à l'échelle ;
  - `/` et `sqrt` : le reste `fma(-q, b, a)` ou `fma(s, s, -x)` donne de quel
    côté est le résultat arrondi ;
  - les fonctions élémentaires : seules des bornes valables mais plus larges,
    `[previous_float(r), next_float(r)]`, 2 ulp au lieu de 1 (c'est le
    `namespace nearest` de `gaol/gaol_double_op.h`, qu'aucune opération
    n'emploie) ; le signe de l'erreur de CORE-MATH n'est pas exposé, et le
    modifier irait contre la règle de garder `3rd/math-core` proche de l'amont ;
  - `pown` : le dernier `fma(K, h, l)` est un arrondi vers le haut, à remplacer
    par `next_float`.

  À l'inverse, `midpoint`, `chi` et l'écriture des intervalles passent déjà par
  `round_nearest()`.

  À mesurer : le gain, qui est que les `double` du programme ne sont plus
  arrondis vers le haut de `gaol::init()` à `gaol::cleanup()` (le tableau de
  `doc/using.md`) et que `GAOL_PRESERVE_ROUNDING` devient inutile, mince avec
  AVX-512 et réel sur les machines sans (SSE2 seul, FPU) ; et le coût, ces
  noyaux faisant plus d'opérations que la sonde et l'addition vers le haut du
  build par défaut, donc probablement plus lents. Un prototype de `+ - * / sqrt`
  au plus proche avec FMA, comparé aux noyaux actuels en résultats (bit à bit,
  `tests/rounding_direction.cpp`) et en temps (`doc/compare`), suffit à décider
  ; les fonctions élémentaires restent le point dur.

  Sur arm64, #146 a ramené `GAOL_PRESERVE_ROUNDING` de 39 à 27 ns par
  opération sur un M2 : la vérification du sens est un `mrs` (0,8 ns, sans
  indicateur levé), mais deux `msr` qui changent le sens restent (11 ns chacun,
  le plancher), et `x.midpoint()` (28 ns) en paie deux aussi. Passer sous 5 ns
  demande de ne pas écrire FPCR : les transformations exactes à base de FMA
  ci-dessus, ou un noyau qui calcule vers le plus proche ; à étudier avant de
  s'y engager, les bornes étant en jeu.

## La documentation

### AK. Un site de documentation sur GitHub Pages (70)

Sorti du point I le 18 octobre, à la demande du mainteneur : le site vient après
les textes de I, qu'il publie, et qui sont fusionnés (#140).

- **70.** **Un site de documentation sur GitHub Pages** (conseil de
  `TODO_mistral.md`) : décidé le 3 octobre, un site généré depuis `doc/*.md`,
  avec le PDF du manuel, publié par un workflow, sans la référence HTML de
  Doxygen, supprimée par #29.

## La version

### Y. Préparer et publier v5.0.0 (32, 33, 34, 71)

En dernier, dans cet ordre : la pull request de synthèse, une fois les
corrections ci-dessus faites ; le rapport de couverture (32) et les temps (33)
au commit de la version ; les fusions et l'étiquette (34) ; l'annonce (71). La
licence est dite (LGPL pour GAOL, MIT pour CORE-MATH) dans la section « Licences
» de `README.md` et dans le manuel : fait.

- **La pull request de synthèse** : `ChangeLog` (une entrée du 27 septembre,
  touchée le 4 octobre) et `doc/differences.md` (28 septembre, puis des commits
  directs des 16 et 18 octobre) ne reprennent pas le travail du 29 septembre au
  18 octobre, hors quelques commits directs. Les textes proposés sont, pour les
  points faits du 29 septembre au 2 octobre (#31 à #63), dans
  `todo-notes/synthese.md` (section 1, dont onze sont marqués « In
  `configure-clean`: no », certains périmés), et, pour le reste, dans les 43
  descriptions de pull requests qui ont une section « Pour `doc/differences.md`
  et `ChangeLog` » : #71, #76, #77, #83 à #97, #99, #100, #105, #108, #111,
  #113, #114, #117 à #124, #139, #140, #141, #144, #145, #146, #147, #148, #149
  et #150 (#123 et #139 n'ont rien à y mettre ; celle de #146 dit que
  `doc/differences.md` l. 333, « the intervals of ARM » sont FPU, n'est plus
  vrai sur arm64, et celle de #148 que la même ligne, « of Visual C++ and of
  32-bit Windows », ne vaut plus pour x64 ; celle de #150 ajoute que
  `GAOL_PRESERVE_ROUNDING` coûte 22 ns au lieu de 48 sur ARM 32 bits avec GCC,
  et le défaut de `width()` et `bisect()` sur arm64). Les points K, L et `make
  distclean` (#72 à #75) n'ont pas cette section : à écrire d'après leurs
  descriptions. À écrire d'après les commits, sans pull request : les
  changements du 4 octobre
  (`nb_fp_numbers()` rend un `unsigned long long`, `gaol_parameters.h`
  supprimé), et ceux du 15 au 18 octobre, qui changent l'interface installée :
  les en-têtes du parser et des expressions dans `include/gaol/parser/`
  (`gaol_expression.h`…), `gaol_interval.h` scindé en `gaol_interval_core.h` et
  `gaol_interval.h`, les intervalles de floats dans `gaol/float/`,
  `gaol_assert.h`, `gaol_profile.h` (la classe `timepiece`), `gaol_exact.h`,
  `gaol_flags.h`, `gaol/dllmain.cpp` et `operator""_iv` (`gaol_literals.h`)
  supprimés, `f_negate`, `f_negate_simple`, `gaol_opposite` et `reset_fpu_cw`
  supprimés (a6786af), la lecture en bases 16, 8 et 2 (91dcce2) ; vérifier ce
  que `doc/differences.md` en dit déjà. Le point G n'a pas de texte dans la
  description de #91 (écrite avant la fin du travail) : à écrire d'après
  c7915a7, ccf694a et `doc/three-builds.md`. Les chiffres de
  `todo-notes/synthese.md` à harmoniser (l'ancien 18 : « 2.5 s down to 0.03 s »
  contre 0,02 s) et le fait que `doc/differences.md` contredit déjà #71 (la
  locale du flux pour le format `bounds`) sont dans ses notes.
- **`examples/examples.md`** : marquer **Fixed** les n° 2, 3, 4, 5, 7, 8, 10,
  12, 15, 16, 19, 20, 21 et 22 de la section 5 et de l'annexe B, le n° 10 comme
  corrigé par la suppression des constructeurs à partir de chaînes (#30), et par
  #147 pour `parse_interval(nullptr)` ; reprendre ce qu'il dit des points
  corrigés (la ligne vide du n° 14 l'est par #114 ; les phrases de #114 sur la
  locale à virgule et l'intervalle ponctuel sont déjà corrigées).
- **Le manuel** : une seule régénération de `manual/v5/gaol.pdf` (la dernière
  date du 28 septembre ; ne jamais le commiter dans une branche de point), après
  avoir corrigé dans `gaol.tex` les entrées d'index en conflit de « canonical
  interval » et l'`Overfull \hbox` connu (le seul : les lignes 511 à 515 de
  `gaol.tex` le 14 octobre), et en vérifiant que l'entrée de `what()` ne laisse
  pas son en-tête seul en bas de page (macro `\defmethod`). Décidé le 3 octobre,
  pour les points 7 et 10 : la puce de `atanh` dans `doc/differences.md` reste,
  en disant que GAOL 4 n'est pas mesuré ; les puces de `hausdorff()` sont
  fusionnées ; un seul `\newinvfive` dans l'entrée `hausdorff` du manuel, et
  aucun sur la phrase du domaine de `atanh`.
- **32.** **Le rapport de couverture** ([coverage/README.md](coverage/README.md)
  et `coverage/coverage.html`), refait le 2026-09-27 à `605728e` (87,2 % des
  lignes de `gaol/`), ne correspond plus aux sources : le parser est dans
  `gaol/parser/`, `gaol_interval.cpp` est scindé en `gaol_interval_core.cpp` et
  `gaol_interval.cpp`, et `gaol_exact.c`, `s_nextafter.c`, `gaol_profile.cpp`
  sont supprimés ; `coverage.html` cite encore `sysdeps/gaol_exact_c99.h` et
  `s_nextafter.c`, et `coverage/README.md` a été corrigé à la main (88,0 %). Le
  job « Coverage » de `linux.yml` le refait à chaque push, mais pas la copie du
  dépôt. À refaire au commit de la version (`-DGAOL_COVERAGE=ON`, cible
  `coverage`, gcovr) et à enregistrer ; décidé le 15 octobre : pas avant. Lancer
  aussi une fois, sous ASan + UBSan, le test `numbers` en entier (trop long pour
  la relecture du 14 octobre : 158 s pour `arithmetic` seul) ;
  `GAOL_NUMBERS_TIMEOUT` n'est pas relevé pour les builds avec sanitizers
  (décidé le 14 octobre).
- **33.** **Les temps de
  [doc/compare/performance.md](doc/compare/performance.md)** ont été remesurés
  le 17 octobre (`febd2b1`, `a055160`), sa colonne « rounding direction
  preserved » le 18 (`7fd5c09`), avec Clang 21 sur l'i7-1185G7 ; les points AI
  et AJ peuvent encore les changer. À remesurer en dernier, sur le commit propre
  de la version, la machine ne faisant rien d'autre
  (`doc/compare/code/run_bench.sh`, ou `make perf`), puis reprendre le tableau
  de `doc/compare/README.md`.
- **34.** **Les recettes FetchContent récupèrent GAOL 4** : `doc/using.md`, le
  manuel et `tests/fetch_content` prennent la branche `master` de
  `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par défaut.
  `master` est GAOL 4.3.2, avec lequel les programmes de la documentation ne
  compilent pas, et il n'y a pas d'étiquette `v5.0.0`. Décidé le 3 octobre : une
  fois `configure-clean` fini, fusionner `configure-clean` dans `MATH-CORE`
  (c'est un ancêtre de `configure-clean` : une avance rapide), puis `MATH-CORE`
  dans `master` (qui a deux commits de plus, `577c850` et `406d850`, ceux de
  `.claude/CLAUDE.md` que `configure-clean` a par `68fe887` et `195e4a2` : une
  vraie fusion, sans différence de contenu), puis étiqueter `v5.0.0` ; les
  recettes et le test gardent `master` jusqu'à l'étiquette, puis la citent.
  Après v5.0.0, le développement continue dans `configure-clean`, et `master` ne
  reçoit que les versions. Les commits à ligne de crédit (123) restent, décidé
  le 19 octobre.
- **71.** **Annoncer GAOL v5.0.0 à Frédéric Goualard**, l'auteur de GAOL, une
  fois l'étiquette posée : pas d'annonce publique, et pas de GitHub Release,
  l'étiquette suffisant (décidé le 3 octobre). Le message est écrit, en
  brouillon dans Gmail ; suivi dans #128.

## Les signalements à d'autres projets

Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à
refaire avant l'envoi et ses décisions ; **rien n'est envoyé**, et rien ne l'est
sans l'accord explicite du mainteneur.

- **#125 CORE-MATH** : huit correctifs de `3rd/README.md` à refaire sur le
  master du jour (le 1 entre en conflit avec des changements récents de `sinh.c`
  et `cosh.c` ; le 8, `exp_1()`, fait dans la copie de GAOL par #144, s'applique
  tel quel au master du 19 octobre), et des défauts sans correctif (`atan2l.c`,
  la borne de `ss` de `asinpi_acc()`, `return` d'une expression `void` dans
  `erfc.c`, `erfc` et `__builtin_ldexp()`) ; qui envoie et comment (merge
  request sur gitlab.inria.fr ou `git format-patch` à core-math@inria.fr).
  Après un import d'un CORE-MATH d'après `c01f08c5` : retirer le contournement
  du `<` de `cr_pow` (`pow_rounded_up()` et `pow_rounded_dn()`).
- **#126 GCC** : la comparaison silencieuse rendue signalante par
  l'if-conversion (ARM 32 bits, POWER9) et par la vectorisation, et le
  miscompilage de GCC 12.2 sur aarch64 (`tests/numbers.cpp`) ; le bogue 52258
  est de la même famille.
- **#127 mingw-w64** : `fma()` et `round()` de msvcrt, `fesetexceptflag()` en 32
  bits, et le `fma()` x87 de mingw-w64 11.
- **#128 Frédéric Goualard** : l'annonce de v5.0.0, après l'étiquette (point Y)
  ; l'adresse du brouillon vient d'un échange de 2014 et d'une liste de 2020, à
  vérifier ; garder ou couper le paragraphe sur les défauts de GAOL 4.
- **#129 ITF1788** : huit cas dont le résultat attendu n'est pas le plus serré.
  **#130 Boost.Interval** : `rounded_transc_opp` donne la borne supérieure pour
  l'inférieure ; issue seule ou avec une pull request (politique de Boost sur
  les contributions).
- **#131 Microsoft** (`ldexp()` de l'UCRT, `NAN` et `INFINITY` sous
  `/fp:strict`, `sqrt()` en 32 bits, runtime de Debug, `nextafter()`, le `fma()`
  d'`ucrtbase.dll` jamais vérifié), **#132 Apple** (`ldexp()` et `nextafter()`
  de macOS), **#133 LLVM/Clang** (ARM 32 bits, `fcsel` sur arm64, `-ffast-math`
  et `-frounding-math`) : chaque point attend une réplique d'un fichier,
  vérifiée sur la plateforme, et un tri entre défaut, limite connue et
  comportement voulu.
- **#134 gmpy2** (2.2.1 ; probablement rien à envoyer si 2.3.2 les a corrigés),
  **#135 musl** (la conversion décimale de `printf`), **#136 les auteurs du
  benchmark de Tang et al.** (fuite de mémoire, bornes infinies, entrées de
  FPBench, constantes décimales, README), **#137 libieeep1788** (probablement
  rien : les six cas sont dans #129) et **#138 la glibc** (probablement rien :
  relancer avec la glibc 2.44).

## Ménage

- **Branches** : les branches locales du mainteneur (`MATH-CORE`, `master`)
  ne se touchent pas. Les branches de #124 (`todo-zz-perf`), de #144
  (`todo-ab-pow-invalid`), de #145 (`todo-ae-c-compiler-checks`), de #146
  (`todo-ARM-perf`), de #147 (`todo-ad-parse-interval-null`), de #148
  (`todo-ai-msvc-sse2-avx512`), de #149 (`todo-ad-83-84-reader`) et de #150
  (`todo-arm32-fpscr`) sont supprimées sur GitHub (celles de #145 à #150 à la
  fusion, le 19 octobre pour les premières et le 20 pour #148, #149 et #150) ;
  en local, celles de #146 (`todo-ARM-perf`) et de #150, le 20 octobre. Celle
  de #153 (`todo-fpcr-gcc-hoisting`), fermée sans fusion, sur GitHub et en
  local, le 20 octobre.
- **Les lignes de crédit** : voir le 123.
- **Les images Docker** de #111 (`i386/debian:trixie`, `debian:trixie`,
  `ubuntu:24.04`) restent sur la machine.
- **L'issue #49** reste ouverte comme suivi d'ensemble et garde l'historique ;
  les décisions du 3 octobre et les liens vers les issues #64 à #70 y sont en
  commentaire, comme celles du 4 octobre sur les questions restées ouvertes dans
  les rapports et celles du 18 octobre sur les questions de #99. Le reste à
  faire est suivi dans l'issue #142.
- **`todo-notes/`** : ramené de la branche `todo-status` (supprimée) ; ses
  rapports sont retirés de l'arbre (ils restent au commit `16a2f60`), et ce qui
  en sert encore est dans `todo-notes/synthese.md`. À vider après la version,
  avec ce qu'il reste de `TODO.md`.

## Table des anciens numéros

26 : W ; 28 : X ; 32 : Y ; 33 : Y ; 34 : Y ; 46 : F ; 70 : AK ; 71 : Y ;
78 : AB ; 79 : AB ; 80 : AB ; 81 : AC ; 82 : AC ; 85 : AD ; 87 : AD ; 88 : AD ;
89 : AD ; 91 : AE ; 92 : AE ; 94 : AE ; 95 : AE ; 105 : AH ; 106 : AI ;
108 : AI ; 109 : AI ; 110 : AI ; 111 : AI ; 112 : AI ; 113 : AJ ; 115 : AI ;
116 : AI ; 117 à 123 : AL ; 125 : AM.
