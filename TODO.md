# À faire

Ce qui reste à faire sur GAOL v5 au commit `c9b5245` de `configure-clean`, le 19
octobre 2026. Ce fichier ne garde que cela : le travail fait, ses pull requests,
ses décisions et ses mesures sont dans l'issue #49 (« Fusionné depuis le 30
septembre » et les sections « Décidé le… »), dans les descriptions des pull
requests et dans l'historique de git (`git log -p TODO.md`, où la version
d'avant le ménage du 18 octobre est celle de `4eaf69a`). L'issue #142 reprend la
liste ci-dessous avec des cases à cocher.

## Priorités

Les tâches ci-dessous sont triées de la première à traiter à la dernière. Les
critères, dans l'ordre : la sûreté des bornes d'abord (une seule configuration
peut en donner de fausses, le 90 ; sinon aucune borne fausse n'est connue) ; ce
qui coûte plus cher en attendant (une décision qui bloque, l'historique avant la
fusion dans `master`) ; ce qui tue un programme ou rompt une promesse écrite
avant ce qui ne fait que ralentir ; le petit et sans dépendance avant le gros ;
la version (Y) en dernier, puis ce qui lui est postérieur. Le détail de chaque
tâche est dans la section de son point plus bas.

### A. Décisions courtes, prises le 19 octobre

Plus rien à décider ici : les trois choix sont faits, et leurs suites sont dans
les rangs 13 et 15 et dans le groupe E.

1. **123 (AL)** : *décidé le 19 octobre* : laisser les commits qui portent une
   ligne de crédit (24 « Co-Authored-By: Claude Opus 5 » du 14 au 21 septembre,
   dont 16 dans `master`, et 6 « Co-authored-by: Jordan08 » de #99), sans
   réécrire `configure-clean`. Reste à retirer les lignes des descriptions et
   des commentaires de pull requests (rang 15).
2. **Les études (AH, AI, AJ, AN)** : *décidé le 19 octobre* : toutes avant
   v5.0.0, donc le groupe E en entier, y compris l'AJ (113), qui peut changer
   ce que la bibliothèque promet (le sens d'arrondi du programme entre
   `gaol::init()` et `gaol::cleanup()`) et les noyaux, donc les temps du 33.
3. **88 et 87 (AD)** : *décidé le 19 octobre* : on garde la décision du 3
   octobre pour la lecture en temps quadratique (4,4 s pour 320 000 chiffres) et
   on ne change pas le lexeur pour le signe devant un nombre incertain : les
   deux se documentent (rang 13).

### B. Corrections du code, décidées, du plus important au moins important

Aucune borne fausse n'est connue : ces défauts tuent un programme (SIGFPE avec
les exceptions activées, plantage), rendent un résultat que la norme dit autre,
ou rompent une promesse de la documentation. Tailles : S, moins d'une
demi-journée ; M, un à deux jours ; L, plusieurs jours ou incertain (estimées
d'après les descriptions).

4. **90 (AE)** : Clang pour ARM 32 bits vérifié sur le seul compilateur C++ par
   autotools et meson. C'est le seul défaut ouvert qui touche à la sûreté des
   bornes (un CORE-MATH compilé par un Clang qui ignore le sens d'arrondi en
   donnerait des fausses), donc le premier, bien qu'il demande des compilateurs
   C et C++ de familles différentes sur armhf et ne soit pas vérifiable ici (la
   CI d'armhf le fera). [S]
5. **114 (AD)** : `parse_interval(nullptr)` plante, le seul plantage connu ; une
   ligne et un test. [S]
6. **77 (AB)** : `pow` lève `FE_INVALID` près du dépassement, et un programme
   qui l'a activée meurt sur SIGFPE ; `pow` est l'opération la plus employée de
   celles d'AB. Décidé : le correctif 8 de CORE-MATH (`/* GAOL */`,
   `3rd/README.md`), qui prépare aussi #125. [M]
7. **78 (AB)** : `cancel_minus` et `cancel_plus` lèvent `FE_INVALID` quand la
   différence déborde ; quelques lignes. [S]
8. **84 (AD)** : `[l, u]` avec l > u lu comme non vide, alors que la norme dit
   vide ; décidé le 14 octobre, le manuel à mettre à jour. [M]
9. **81 (AC)** : `nth_root` d'un sous-normal loin des bornes les plus serrées
   que `doc/accuracy.md` promet (les bornes restent justes). [M]
10. **79 (AB)** : `FE_OVERFLOW` levé pour des résultats finis (`sqrt`, les
    racines, `sin`, `cos`, `tan`, `div_rel`, les `*_rel`) ; décidé, mais il
    touche beaucoup de fonctions et ne se voit qu'avec le piège activé près de
    DBL_MAX. [L]
11. **83 (AD)** : un exposant hors des `int` refusé dans un texte ; rare. [S]

### C. Textes, tests et ménage

Petits, sans risque, à grouper en une ou deux pull requests ; `process.md`
d'abord, puisqu'il guide chaque nouvelle session. [S]

12. **118 (AL)** : `process.md` (la machine d'Ubuntu 24.04, `--no-track`, le §12
    périmé).
13. **80 (AB), 117, 119, 120 (AL), 87, 88, 89 (AD) et les descriptions
    d'options du 112 (AI)** : des textes faux ou qui manquent (l'exemple
    `1/[0, 1]`, la phrase de `pow` d'`accuracy.md`, les compilateurs refusés du
    manuel, le piège de `namespace gaol { }`, le format `agreeing` d'un point, «
    + - * / et sqrt » du chemin AVX-512, le signe collé à un nombre incertain du
    87, la lecture en temps quadratique du 88).
14. **91 et 94 (AE)** : le test `refused_options.sh` qui se dit sauté, et la
    liste de macros de l'audit.
15. **123 (AL)** : retirer les lignes de crédit des descriptions de #15 à #24 et
    #60 à #63 et des commentaires de #58 et #60 à #63 (les commits restent,
    décidé le 19 octobre).
16. **121, 122 (AL) et le ménage des branches** : décider de `.continue/` ;
    `gaol_performance` en C++14 ; supprimer `todo-zz-notes` en local.

### D. Les trois builds

Principe 6 de `process.md` : les mêmes bibliothèque, en-têtes et options
partout.

17. **Le défaut de CMake du 112 (AI)** : un second `cmake` dans le même dossier
    fait passer `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING` ;
    à corriger avec un test. [S]
18. **125 (AM)** : décider si `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT`
    restent sans valeur par défaut dans les trois modèles. [S]
19. **95 et 92 (AE)** : des détections qui lient dans un build et compilent dans
    les autres, des drapeaux C différents ; aucun effet connu sur les bornes.
    [M]
20. **85 (AD) et 82 (AC)** : un `long` de 32 bits (non vérifié) et la garde de
    `[±oo]` de `powRev1` et `powRev2` (qu'`interval(__m128d)` seul atteint) ;
    rares. [S]

### E. Études de vitesse

Des mesures d'abord, toutes avant v5.0.0 (décidé le 19 octobre) ; ordre : ce qui
peut changer la suite, puis l'écart mesuré le plus grand sur les builds par
défaut, puis les builds avec `GAOL_PRESERVE_ROUNDING`.

21. **113 (AJ)** : des noyaux qui laissent le sens d'arrondi au plus proche ; un
    prototype de `+ - * / sqrt` suffit à décider. Peut changer le comportement
    documenté et les noyaux (voir le rang 2). [L]
22. **106 (AI)** : les intervalles FPU n'ont pas les noyaux de #124 (macOS
    arm64, Visual C++, clang-cl) : l'écart le plus grand, de 2 à 10 fois, dans
    les builds par défaut. [L]
23. **110 (AI)** : GCC pour Windows, `pow(x, 3)`, `log` et `cos` 5 à 10 fois
    plus lents que Clang, faute de `-mfma` probablement. [M]
24. **108 (AI)** : Visual C++ 32 bits, 80 ns par opération dès le build par
    défaut. [M]
25. **107 et 109 (AI)** : `GAOL_PRESERVE_ROUNDING` à 45-87 ns par opération sur
    macOS arm64 et avec Visual C++ x64 (FPCR et MXCSR en ligne). [M]
26. **105 (AH)** : `-mbranches-within-32B-boundaries`, jusqu'à 1 ns par
    opération selon le placement du code. [M]
27. **116 (AI)** : des écarts sans cause trouvée, à refaire d'abord avec Clang
    21. [S]
28. **111 et 115 (AI)** : mesurer par la CI deux builds alternés, et lancer
    `run_lines.sh` dans `make perf` ; les temps de la CI ne sont pas stables.
    [M]
29. **112 (AI), le reste** : les fonctions élémentaires avec
    `GAOL_PRESERVE_ROUNDING` (jusqu'à 27 %), un test des bits du signe de zéro
    de `x * d`. [M]
30. **126 (AN, #101)** : les pistes SIMD et FMA (AVX2, NEON, SVE, VSX, RISC-V) ;
    elle dépend des mesures ci-dessus. [L]

### F. Signalements à d'autres projets

Chacun attend l'accord explicite du mainteneur ; rien n'est envoyé. Ordre : ce
qui est prêt et qui a une valeur pour GAOL, puis ce qui demande une réplique à
écrire, puis ce qui ne demande probablement rien.

31. **#125 CORE-MATH** : les sept correctifs prêts, refaits sur le master du
    jour, et le correctif 8 (rang 6).
32. **#126 GCC**, **#127 mingw-w64**, **#129 ITF1788**, **#130 Boost.Interval**.
33. **#131 Microsoft**, **#132 Apple**, **#133 LLVM/Clang**, **#135 musl**,
    **#136 le benchmark de Tang et al.** : une réplique d'un fichier à écrire et
    à vérifier sur la plateforme.
34. **#134 gmpy2**, **#137 libieeep1788**, **#138 la glibc** : probablement rien
    à envoyer ; vérifier, puis fermer.

### G. La version, dans cet ordre

En dernier : elle suppose les corrections faites et les études du groupe E
faites (décidé le 19 octobre, rang 2).

35. **La pull request de synthèse** (Y) : `ChangeLog` et `doc/differences.md`,
    d'après `todo-notes/synthese.md` et les 36 descriptions de pull requests.
36. **`examples/examples.md` et le manuel** (Y) : les marques **Fixed**, une
    seule régénération de `manual/v5/gaol.pdf`.
37. **32 (Y)** : le rapport de couverture, et `numbers` en entier sous ASan +
    UBSan, une fois.
38. **33 (Y)** : les temps de `doc/compare/performance.md`, en dernier, sur le
    commit propre de la version, après les études du groupe E.
39. **34 (Y)** : `configure-clean` dans `MATH-CORE` (avance rapide), `MATH-CORE`
    dans `master`, l'étiquette `v5.0.0`, les recettes FetchContent.
40. **71 (Y) et #128** : l'annonce à Frédéric Goualard, après l'étiquette.

### H. Après la version

41. **70 (AK)** : le site GitHub Pages ; sans dépendance, mais l'annonce étant
    privée, rien ne l'exige pour la version.
42. **46 (F, #64)** : `operator>>` ; le comportement actuel (un intervalle par
    ligne) est documenté, le garder est la décision par défaut.
43. **26 (W, #67)** : décorations ou indicateur de sortie de domaine.
44. **28 (X)** : traits pour YalAA, backend pour VNODE-LP.
45. **Le ménage de la fin** : les images Docker de #111, `todo-notes/`, ce qu'il
    reste de `TODO.md`.

## Comment lire ce fichier

Les points sont nommés par des lettres (A à Z, puis AA à AO) : un point réunit
ce qui touche le même code ou le même fichier, ou ce qu'un ordre impose de faire
ensemble. Un point garde en sous-points les numéros de l'ancienne liste,
auxquels renvoient les pull requests, les issues et les rapports des agents ; la
table à la fin donne la lettre de chaque numéro encore à faire, et un numéro qui
n'y est pas est un point fait. Une lettre qui n'a plus de section ici est un
point fait (A à E, G à V, Z, AA, AF, AG, AO, et ZZ, la vitesse de `+ - * /`, par
#124) : ses pull requests sont dans #49, et les commentaires du code qui citent
« point Q of TODO.md » renvoient à ce point fait. « Décidé le 3 octobre » (ou
une autre date) marque une décision du mainteneur : applique-la telle quelle.
Une question reportée renvoie à son issue.

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

Aucune pull request ouverte.

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
programme de reproduction) : 77, 78, 79, 80, 81, 83, 84, 87, 88 et 89 se
reproduisent encore, tels que décrits ; 91 et 94 ont été relus dans les sources.
Les autres (82, 85, 90, 92, 95) n'ont pas été revérifiés. Le 86 (`_iv` au-delà
de 2^64) est sans objet : `operator""_iv` et `gaol/gaol_literals.h` ont été
retirés à partir du 16 octobre ; le 93 (code mort exporté sous des noms C
génériques) est fait depuis le 15 octobre (`gaol_exact.*`, `sysdeps/` et
`s_nextafter.c` supprimés).

### AB. Des exceptions flottantes levées à tort (77, 78, 79, 80)

- **77.** **`pow` lève l'exception d'opération invalide près du dépassement de
  capacité** : `gaol::pow([2], [1023.9999])` a les bornes les plus serrées, mais
  lève `FE_INVALID`, et un programme qui l'a activée
  (`feenableexcept(FE_INVALID)`) meurt sur SIGFPE. De même `pow(I, d)`, les
  puissances entières au-delà des `int` (`gaol_pown_ll()`, par exemple
  `pow([0x1.000003f82a1fp+0], 3000000001LL)`), `pow_rel` et `pow_exponent_rel`
  (environ 0,5 % de tirages au hasard), et
  `textToInterval("pow([2,2],1023.9999)")`, dans les deux espaces de noms et
  avec tous les builds. La cause est dans CORE-MATH : `exp_1()`
  (`3rd/math-core/src/binary64/pow/pow.c`) écarte rh > RHO2 =
  0x1.62e42e709a95bp+9, mais son k atteint 2^22 dès rh ≥ 709,7826283, et M =
  2047 donne 2^1024 = +∞ : la première phase de `cr_pow` calcule alors ∞ − ∞,
  avant que la seconde rende la bonne valeur. Les puissances entre 1,797541e308
  et 1,797611e308 environ sont touchées. Le `pow.c` de la branche `master` de
  CORE-MATH a encore ce code le 14 octobre. Décidé le 14 octobre : le correctif
  dans CORE-MATH, passer à la seconde phase quand M atteint 0x7ff (ou abaisser
  RHO2), ligne marquée `/* GAOL */` et décrite dans `3rd/README.md` comme
  correctif 8 à proposer en amont (#125), et `pow([2], [1023.9999])` dans
  `extreme_powers[]` de `tests/rounding_direction.cpp`. Revérifié le 18 octobre
  : `pow([2], [1023.9999])` lève encore `FE_INVALID`.
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
  `FE_OVERFLOW`.
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

### AD. La lecture et l'écriture des intervalles (83 à 85, 87 à 89, 114)

- **83.** **Un exposant entier hors des `int` est refusé dans un texte** :
  `gaol::textToInterval("nth_root(8,3000000000)")` lève `invalid_action_error`
  (« nth_root used with non integral 2nd arg. »), et
  `gaol_ieee1788::textToInterval` rend l'ensemble vide pour
  `"rootn(8,3000000000)"` et `"pown([2,3],3000000000)"`, alors que `nth_root(8,
  3000000000LL)` et `pow([2, 3], 3000000000LL)` sont calculés depuis #121
  ([1.000000000693146, 1.000000000693148] et [DBL_MAX, +∞]). Cause : `gaol_call`
  (`gaol/parser/gaol_interval_parser.ypp`) passe par `is_an_int()`
  (`gaol/gaol_interval_core.h`), qui exige un `int`. Correction : les surcharges
  `long long` pour un entier dégénéré hors des `int` (en saturant comme
  `integer_as_long_long()`), ou la limite dite dans `gaol/parser/gaol_parser.h`
  et le manuel. Revérifié le 18 octobre.
- **84.** **`[l, u]` avec l > u est lu comme un intervalle non vide** quand les
  deux nombres s'arrondissent au même double ou se chevauchent :
  `[9007199254740993, 9007199254740992]` donne [2^53], qui ne contient pas 2^53
  + 1 ; `[0x1.999999999999ap-4, 0.1]` donne [0x1.999999999999ap-4], qui ne
  contient pas 0,1 ; `[1.00000000000000001, 1]` donne [1] ; dans les deux
  espaces de noms, alors que `[2, 1]` et `interval(2^53 + 1, 2^53)` sont vides.
  IEEE 1788-2015 (9.7.4 a et 12.12.7) veut l ≤ u, le vide sinon, et
  `doc/accuracy.md` promet le plus serré pour ces littéraux. Correction (la
  fonction du parser qui lit `[l, u]`, dans
  `gaol/parser/gaol_interval_parser.ypp`) : comparer les valeurs exactes des
  deux nombres quand leurs encadrements se touchent, et rendre le vide si l > u.
  Décidé le 14 octobre : le vide dans les deux espaces de noms,
  `gaol::textToInterval` compris, dont le manuel décrit aujourd'hui la lecture
  borne par borne (à mettre à jour). Revérifié le 18 octobre.
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
- **114.** **`parse_interval(nullptr)` plante** (issue #107, relevé par #105) :
  le lecteur prend la longueur de la chaîne par `std::strlen()`, qui lit un
  pointeur nul (`gaol/parser/gaol_parser.cpp`). La revue d'origine
  (`examples/examples.md`, annexe B, n° 10) proposait de rendre `false`.
  Correction : `false` pour un pointeur nul, comme pour toute chaîne qui n'est
  pas une expression, avec un test dans `tests/expressions.cpp` et une note dans
  le commentaire Doxygen de `gaol/parser/gaol_parser.h`.

### AE. Les trois builds (90 à 92, 94, 95)

- **90.** **autotools et meson ne vérifient que le compilateur C++ pour Clang
  sur ARM 32 bits** (`configure.ac`, `meson.build`), quand CMake vérifie aussi
  le compilateur C (`CMakeLists.txt`). Or CORE-MATH est compilé en C, et le
  `#error` de `gaol/gaol_config.h` n'atteint que le C++ : avec `CC=clang
  CXX=g++` sur armhf, configure et meson accepteraient, et CORE-MATH, compilé
  par un Clang qui ignore le sens d'arrondi sur cette cible, donnerait des
  bornes fausses. La vérification de `-frounding-math` ne regarde que le
  compilateur C++ dans les trois builds. Non vérifiable ici, faute de chaîne
  ARM. Correction : les deux vérifications avec le compilateur C aussi.
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
  I, AF faits, AB à AE et le reste à faire d'ici).
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
intervalles SSE2, c'est-à-dire x86 hors Visual C++ et hors Windows 32 bits : les
intervalles FPU ont encore le `switch` sur les signes et les opérandes en
mémoire que #124 a retirés des SSE2. Pour la ligne de 13 opérations `+ - * /` du
tableau, en ns : 33 sur Linux x86-64 (job d'autotools, Ubuntu 24.04), 36 sur
macOS 15 x86-64, 33 à 58 avec MinGW-w64 et MSYS2, 64 à 93 sur macOS arm64, 100 à
168 avec Visual C++ x64 et arm64 et avec clang-cl, 1 100 à 1 200 avec Visual C++
32 bits. Voir le 111 pour ce que valent ces temps.

- **106.** **Les intervalles FPU n'ont pas les noyaux de #124 : macOS arm64,
  Visual C++ et clang-cl.** Sur macOS arm64 (Clang 15 à 21), `x + y` prend 1,3 à
  1,9 ns, mais `x - y` 3,6 à 4,8 (deux à trois fois la somme), `x * y` 3,5 à
  4,2, `x / y` 4,7 à 5,8 et `sqr` 2,3 à 3,0 ; une ligne de 15 `+` prend 51 à 88
  ns (3 à 6 ns par opération, 29 sur Linux x86-64), de 7 `*` 32 à 44 et de 7 `/`
  45 à 60. Avec Visual C++ x64 et arm64 et avec clang-cl, que CMake ne laisse
  pas prendre les intervalles SSE2 (`GAOL_SIMD AND NOT MSVC`, « IBEX and the
  fork of Fabrice Le Bars build GAOL without them »), `x * y` prend 6,6 à 16,3
  ns, `x / y` 4,2 à 8,2 et `sqr` 4,1 à 10,4, contre 2,1 à 4,1, 2,1 à 5,2 et 2,1
  à 3,1 avec MinGW-w64 et MSYS2 ; la ligne de 13 opérations, 100 à 168 ns contre
  33 à 58. Les autres cibles FPU (ARM 32 bits, POWER, s390x, RISC-V) n'ont pas
  été regardées.

  À chercher : les noyaux de #124 pour les intervalles FPU, dont la lenteur a
  probablement la cause que #124 a trouvée pour les SSE2 (le `switch` sur les
  signes, mal prédit, et les bornes relues en mémoire) : un produit par quatre
  produits et un arbre de maxima, un quotient dont les dénominateurs sont
  choisis par des masques, un carré sans branche, la soustraction écrite comme
  l'addition (`x - y` coûte deux à trois fois `x + y` sur arm64). Mesurer sur
  les jobs de la CI, aucune machine ARM n'étant ici. Pour Visual C++ x64 et
  clang-cl, voir d'abord pourquoi CMake leur refuse les intervalles SSE2, et
  s'ils peuvent les prendre. Les opérations avec un double n'ont ces noyaux sur
  aucune cible (#124 : « `x * 2.0` n'a pas les noyaux de `x * y` »). À
  rapprocher de l'étude des pistes SIMD de l'issue #101 (AN, 126), dont NEON
  pour arm64.

- **107.** **Avec `GAOL_PRESERVE_ROUNDING`, chaque opération prend 45 à 50 ns
  sur macOS arm64** (job « macOS 15 arm64, rounding direction preserved » : `x +
  y` 45,6 ns, `x - y` 50,7, `x * y` 48,8, `x / y` 51,6, `sqr` 48,2, `pow(x, 3)`
  83, `exp` 84 ; 756 ns pour une ligne de 15 `+`) : trente fois le temps du
  build par défaut.

  À chercher : ce que coûtent `fegetround()` et `fesetround()` de la
  bibliothèque du système sur arm64, et si la lecture et l'écriture du registre
  FPCR en ligne (`mrs` et `msr`, que GAOL emploie déjà pour FZ et FIZ), une fois
  à l'entrée et une fois à la sortie, comme #124 le fait de MXCSR sur x86-64,
  les ramènent à quelques ns.

- **108.** **Visual C++ 32 bits : 80 ns par opération, dès le build par défaut**
  (job « Visual Studio 2026 x86 Release » : `x + y` 80,8 à 83,5 ns, `x * y` 87,8
  à 98, une ligne de 15 `+` 1 211 à 1 307 ns). `gaol/gaol_fpu.h` le sait :
  `fegetround()` et la lecture de MXCSR y coûtent environ 80 ns, et la sonde du
  sens d'arrondi par une addition y est écartée (`GAOL_RND_PROBE` n'est pas
  défini sur x86 32 bits).

  À chercher : lire les deux unités, x87 et SSE, sans appeler `fegetround()`
  (`fnstcw` et `stmxcsr` en ligne, ou `_controlfp()`), et ce que cela coûte.

- **109.** **Visual C++ x64 avec `GAOL_PRESERVE_ROUNDING` : 43 à 87 ns par
  opération** (job « Visual Studio 2026 x64 Release, rounding direction
  preserved » : `x + y` 43,4 puis 87,2 ns, une ligne de 15 `+` 653 puis 1 294
  ns). Les macros de #124 (`GAOL_RND_ENTER()` n'écrit que MXCSR, une fois à
  l'entrée et une fois à la sortie) demandent des intervalles SSE2 et `__FMA__`,
  que Visual C++ n'a pas : il y reste `fegetround()`, `fesetround()` et deux
  écritures séparées.

  À chercher avec le 106 : si Visual C++ x64 prend les intervalles SSE2, la même
  condition vaut avec `_M_X64` et `__AVX2__` ; sinon, une sauvegarde de MXCSR en
  ligne (`_mm_getcsr()` et `_mm_setcsr()`) pour les intervalles FPU sur x64.

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

### AN. Les pistes SIMD et FMA (126, issue #101)

- **126.** **Évaluer les pistes SIMD et FMA** (issue #101 : les gains dépendent
  du processeur, du compilateur, des options et des opérations mesurées, et
  aucun facteur général ne se déduit de la largeur des registres). Aujourd'hui :
  SSE2 (et SSE3) sur x86 ; la voie AVX-512 (`GAOL_PREFER_AVX512`) pour `+ - * /
  sqrt sqr` et les puissances entières ; `GAOL_FMA`, distinct d'AVX2 et
  désactivé avec GCC pour Windows ; aucune voie dédiée à AVX2, NEON, SVE, VSX ou
  RISC-V. Suite proposée par l'issue : identifier les opérations et les
  applications qui dominent le temps ; mesurer de façon reproductible sur des
  processeurs et des compilateurs représentatifs ; n'ajouter une voie qu'après
  avoir comparé ses temps et vérifié les bornes, les quatre sens d'arrondi, les
  sous-normaux, les NaN et les infinis ; documenter les exigences de processeur
  des binaires spécialisés et garder un repli portable. À rapprocher du 106
  (NEON pour les intervalles FPU sur arm64), du 109 et de l'AJ. `_mm_round_pd`
  (SSE4.1) ne remplace pas l'arrondi dirigé des opérations, et NEON d'ARMv7 n'a
  pas de `double` vectoriel. Le commentaire `// TODO: Rewrite as many functions
  as possible to use SIMD instructions` de `gaol/gaol_interval_core.cpp` (relevé
  CC.m1, vestige de GAOL 4) se retire ou se remplace par le résultat de cette
  étude, au plus tard au point Y.

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
  `configure-clean`: no », certains périmés), et, pour le reste, dans les 36
  descriptions de pull requests qui ont une section « Pour `doc/differences.md`
  et `ChangeLog` » : #71, #76, #77, #83 à #97, #99, #100, #105, #108, #111,
  #113, #114, #117 à #124, #139, #140 et #141 (#123 et #139 n'ont rien à y
  mettre). Les points K, L et `make distclean` (#72 à #75) n'ont pas cette
  section : à écrire d'après leurs descriptions. À écrire d'après les commits,
  sans pull request : les changements du 4 octobre (`nb_fp_numbers()` rend un
  `unsigned long long`, `gaol_parameters.h` supprimé), et ceux du 15 au 18
  octobre, qui changent l'interface installée : les en-têtes du parser et des
  expressions dans `include/gaol/parser/` (`gaol_expression.h`…),
  `gaol_interval.h` scindé en `gaol_interval_core.h` et `gaol_interval.h`, les
  intervalles de floats dans `gaol/float/`, `gaol_assert.h`, `gaol_profile.h`
  (la classe `timepiece`), `gaol_exact.h`, `gaol_flags.h`, `gaol/dllmain.cpp` et
  `operator""_iv` (`gaol_literals.h`) supprimés, `f_negate`, `f_negate_simple`,
  `gaol_opposite` et `reset_fpu_cw` supprimés (a6786af), la lecture en bases 16,
  8 et 2 (91dcce2) ; vérifier ce que `doc/differences.md` en dit déjà. Le point
  G n'a pas de texte dans la description de #91 (écrite avant la fin du travail)
  : à écrire d'après c7915a7, ccf694a et `doc/three-builds.md`. Les chiffres de
  `todo-notes/synthese.md` à harmoniser (l'ancien 18 : « 2.5 s down to 0.03 s »
  contre 0,02 s) et le fait que `doc/differences.md` contredit déjà #71 (la
  locale du flux pour le format `bounds`) sont dans ses notes.
- **`examples/examples.md`** : marquer **Fixed** les n° 2, 3, 4, 5, 7, 8, 10,
  12, 15, 16, 19, 20, 21 et 22 de la section 5 et de l'annexe B, le n° 10 comme
  corrigé par la suppression des constructeurs à partir de chaînes (#30) ;
  reprendre ce qu'il dit des points corrigés (la ligne vide du n° 14 l'est par
  #114 ; les phrases de #114 sur la locale à virgule et l'intervalle ponctuel
  sont déjà corrigées).
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

- **#125 CORE-MATH** : sept correctifs de `3rd/README.md` à refaire sur le
  master du jour (le 1 entre en conflit avec des changements récents de `sinh.c`
  et `cosh.c`), le correctif 8 (`exp_1()`, point AB.77) à écrire dans
  `3rd/README.md` et à faire dans la copie de GAOL, et des défauts sans
  correctif (`atan2l.c`, la borne de `ss` de `asinpi_acc()`, `return` d'une
  expression `void` dans `erfc.c`, `erfc` et `__builtin_ldexp()`) ; qui envoie
  et comment (merge request sur gitlab.inria.fr ou `git format-patch` à
  core-math@inria.fr). Après un import d'un CORE-MATH d'après `c01f08c5` :
  retirer le contournement du `<` de `cr_pow` (`pow_rounded_up()` et
  `pow_rounded_dn()`).
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

- **Branches** : `todo-zz-notes` (contenue dans `origin/configure-clean`) reste
  en local : la supprimer (`git branch -d` vérifie qu'elle est contenue). Les
  autres branches locales du mainteneur (`MATH-CORE`, `master`) ne se touchent
  pas. La branche de #124 (`todo-zz-perf`) est supprimée depuis le 19 octobre.
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
77 : AB ; 78 : AB ; 79 : AB ; 80 : AB ; 81 : AC ; 82 : AC ;
83 : AD ; 84 : AD ; 85 : AD ; 87 : AD ; 88 : AD ; 89 : AD ; 90 : AE ;
91 : AE ; 92 : AE ; 94 : AE ; 95 : AE ; 105 : AH ; 106 : AI ; 107 : AI ;
108 : AI ; 109 : AI ; 110 : AI ; 111 : AI ; 112 : AI ; 113 : AJ ;
114 : AD ; 115 : AI ; 116 : AI ; 117 à 123 : AL ; 125 : AM ; 126 : AN.
