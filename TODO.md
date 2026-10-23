# À faire

Ce qui reste à faire sur GAOL v5 au commit `a4cf94d` de `configure-clean`, le 23
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

Plus rien à décider ici : les trois choix sont faits, et leurs suites sont
dans le groupe E ; les textes sont faits par #164.

1. **123 (AL)** : *décidé le 19 octobre* : laisser les commits qui portent une
   ligne de crédit (24 « Co-Authored-By: Claude Opus 5 » du 14 au 21 septembre,
   dont 16 dans `master`, et 6 « Co-authored-by: Jordan08 » de #99), sans
   réécrire `configure-clean`. Les lignes des descriptions et des commentaires
   de pull requests sont retirées (fait le 20 octobre, sans pull request).
2. **Les études (AH, AI, AJ ; l'AN est faite par #154)** : *décidé le 19
   octobre* : toutes avant v5.0.0, donc le groupe E en entier, y compris l'AJ
   (113), qui peut changer ce que la bibliothèque promet (le sens d'arrondi du
   programme entre `gaol::init()` et `gaol::cleanup()`) et les noyaux, donc les
   temps du 33.
3. **88 et 87 (AD)** : *décidé le 19 octobre* : on garde la décision du 3
   octobre pour la lecture en temps quadratique (4,4 s pour 320 000 chiffres) et
   on ne change pas le lexeur pour le signe devant un nombre incertain : les
   deux se documentent (fait par #164).

### B. Corrections du code, décidées, du plus important au moins important

Aucune borne fausse n'est connue : ces défauts tuent un programme (SIGFPE avec
les exceptions activées, plantage), rendent un résultat que la norme dit autre,
ou rompent une promesse de la documentation. Tailles : S, moins d'une
demi-journée ; M, un à deux jours ; L, plusieurs jours ou incertain (estimées
d'après les descriptions).

4. **79 (AB)** : `FE_OVERFLOW` levé pour des résultats finis (`sqrt`, les
   racines, `sin`, `cos`, `tan`, `div_rel`, les `*_rel`, et la lecture de
   `1e400`) ; décidé, mais il touche beaucoup de fonctions et ne se voit
   qu'avec le piège activé près de DBL_MAX. [L]

### C. Textes, tests et ménage

Fait par #164 (117, 119, 120, 87, 88, 89, 91, 94, 121, 122, et le `/W4 /WX`
en Debug de #148).

### D. Les trois builds

Principe 6 de `process.md` : les mêmes bibliothèque, en-têtes et options
partout.

5. **95 et 92 (AE)** : des détections qui lient dans un build et compilent dans
   les autres, des drapeaux C différents ; aucun effet connu sur les bornes.
   [M]
6. **85 (AD) et 82 (AC)** : un `long` de 32 bits (non vérifié) et la garde de
   `[±oo]` de `powRev1` et `powRev2` (qu'`interval(__m128d)` seul atteint) ;
   rares. [S]

### E. Études de vitesse

Des mesures d'abord, toutes avant v5.0.0 (décidé le 19 octobre) ; ordre : ce qui
peut changer la suite, puis l'écart mesuré le plus grand sur les builds par
défaut, puis les builds avec `GAOL_PRESERVE_ROUNDING`.

7. **113 (AJ)** : des noyaux qui laissent le sens d'arrondi au plus proche ; un
   prototype de `+ - * / sqrt` suffit à décider. Peut changer le comportement
   documenté et les noyaux (voir le rang 2). [L]
8. **106 (AI), le reste** : Visual C++ arm64 (NEON n'y est pas offert) ;
    POWER, s390x, RISC-V et MinGW-w64 arm64 n'ont pas été mesurés avec les
    noyaux FPU de #157 ; macOS arm64 est fait par #146 (NEON), Visual C++ et
    clang-cl x64 par #148 (SSE2), tous les intervalles FPU par #157 (des noyaux
    scalaires sans branche) ; #150 avait fait le registre de contrôle d'ARM 32
    bits. [M]
9. **108 (AI)** : Visual C++ 32 bits, 80 ns par opération dès le build par
   défaut ; la lecture de x87 et de MXCSR en ligne est faite par #156, et ses
   suites décidées le 22 octobre restent (Q8, en bas). [M]
10. **109 (AI)** : `GAOL_PRESERVE_ROUNDING` à 24 ns par opération avec Visual
    C++ x64 depuis #148 (43-87 avant, 2,5 sans) ; la même condition que les
    macros MXCSR de #124 avec `_M_X64` et `__AVX2__`, décidé le 20 octobre pour
    une pull request qui suit #148 ; le 107, sur macOS arm64, est fait par #146.
    [M]
11. **105 (AH)** : `-mbranches-within-32B-boundaries`, jusqu'à 1 ns par
    opération selon le placement du code. [M]
12. **116 (AI)** : des écarts sans cause trouvée, à refaire d'abord avec Clang
    21. [S]
13. **112 (AI), le reste** : les fonctions élémentaires avec
    `GAOL_PRESERVE_ROUNDING` (jusqu'à 27 %), un test des bits du signe de zéro
    de `x * d` (il demande un x86 avec AVX-512) ; le défaut de CMake et les
    descriptions d'options sont faits par #161. [M]

### F. Signalements à d'autres projets

Chacun attend l'accord explicite du mainteneur ; rien n'est envoyé. Ordre : ce
qui est prêt et qui a une valeur pour GAOL, puis ce qui demande une réplique à
écrire, puis ce qui ne demande probablement rien.

14. **#125 CORE-MATH** : les huit correctifs prêts (le 8, `exp_1()`, est fait
    dans la copie de GAOL par #144), refaits sur le master du jour.
15. **#126 GCC**, **#127 mingw-w64**, **#129 ITF1788**, **#130 Boost.Interval**.
16. **#131 Microsoft**, **#132 Apple**, **#133 LLVM/Clang**, **#135 musl**,
    **#136 le benchmark de Tang et al.** : une réplique d'un fichier à écrire et
    à vérifier sur la plateforme.
17. **#134 gmpy2**, **#137 libieeep1788**, **#138 la glibc** : probablement rien
    à envoyer ; vérifier, puis fermer.

### G. La version, dans cet ordre

En dernier : elle suppose les corrections faites et les études du groupe E
faites (décidé le 19 octobre, rang 2).

18. **La pull request de synthèse** (Y) : `ChangeLog` et `doc/differences.md`,
    d'après `todo-notes/synthese.md` et les 53 descriptions de pull requests.
19. **`examples/examples.md` et le manuel** (Y) : les marques **Fixed**, une
    seule régénération de `manual/v5/gaol.pdf`.
20. **32 (Y)** : le rapport de couverture, et `numbers` en entier sous ASan +
    UBSan, une fois.
21. **33 (Y)** : les temps de `doc/compare/performance.md`, en dernier, sur le
    commit propre de la version, après les études du groupe E.
22. **34 (Y)** : `configure-clean` dans `MATH-CORE` (avance rapide), `MATH-CORE`
    dans `master`, l'étiquette `v5.0.0`, les recettes FetchContent.
23. **71 (Y) et #128** : l'annonce à Frédéric Goualard, après l'étiquette.

### H. Après la version

24. **70 (AK)** : le site GitHub Pages ; sans dépendance, mais l'annonce étant
    privée, rien ne l'exige pour la version.
25. **46 (F, #64)** : `operator>>` ; le comportement actuel (un intervalle par
    ligne) est documenté, le garder est la décision par défaut.
26. **26 (W, #67)** : décorations ou indicateur de sortie de domaine.
27. **28 (X)** : traits pour YalAA, backend pour VNODE-LP.
28. **Le ménage de la fin** : les images Docker de #111, `todo-notes/`, ce qu'il
    reste de `TODO.md`.

## Comment lire ce fichier

Les points sont nommés par des lettres (A à Z, puis AA à AO) : un point réunit
ce qui touche le même code ou le même fichier, ou ce qu'un ordre impose de faire
ensemble. Un point garde en sous-points les numéros de l'ancienne liste,
auxquels renvoient les pull requests, les issues et les rapports des agents ; la
table à la fin donne la lettre de chaque numéro encore à faire, et un numéro qui
n'y est pas est un point fait. Une lettre qui n'a plus de section ici est un
point fait (A à E, G à V, Z, AA, AF, AG, AL, AM, AN, AO, et ZZ, la vitesse
de `+ - * /`, par #124) : ses pull requests sont dans #49, et les commentaires
du code qui citent « point Q of TODO.md » renvoient à ce point fait. « Décidé
le 3 octobre » (ou une autre date) marque une décision du mainteneur :
applique-la telle quelle. Une question reportée renvoie à son issue.

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
programme de reproduction) : 78, 79, 80, 81, 83, 84, 87, 88 et 89 se
reproduisent encore, tels que décrits (les 77, 78, 80, 81, 83 et 84 sont faits,
par #144, #151, #158 et #149 ; les 87, 88 et 89 sont documentés, et les 91
et 94 corrigés, par #164). Les autres (82, 85, 92, 95) n'ont pas été
revérifiés. Le 86 (`_iv` au-delà
de 2^64) est sans objet : `operator""_iv` et `gaol/gaol_literals.h` ont été
retirés à partir du 16 octobre ; le 93 (code mort exporté sous des noms C
génériques) est fait depuis le 15 octobre (`gaol_exact.*`, `sysdeps/` et
`s_nextafter.c` supprimés).

### AB. Des exceptions flottantes levées à tort (79)

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

### AC. Les réciproques de `pow` (82)

- **82.** **`pow_rel` et `pow_exponent_rel` (`powRev1`, `powRev2`) n'ont pas la
  garde de `[±oo]` de `pow`** : `powRev1([+oo], [2], R)` rend [1, 1 + 2^-52],
  `powRev2([+oo], [2], R)` [0, 2^-1074], `powRev1([2], [+oo], R)` [DBL_MAX, +∞]
  et `powRev1([-oo], [0, 1], R)` [1, +∞], là où `pow(x, [±oo])` est vide, et ces
  réciproques avec lui. Ces résultats restent des encadrements, et ces arguments
  ne se construisent qu'avec `interval(__m128d)`, dans les builds SSE2.
  Correction : le test `!(l < +oo && u > -oo)` de `pow_standard_upward()` dans
  `pow_rel_upward()` et `pow_exponent_rel_upward()`
  (`gaol/gaol_interval_core.cpp`).

### AD. La lecture et l'écriture des intervalles (85)

- **85.** **Le plafond de l'exposant déborde un `long` de 32 bits** (Windows,
  i386, armhf) : `k`, `k2`, `kb`, `exponent` et `q` sont des `long`
  (`gaol/parser/gaol_interval_lexer.lpp`), mais le plafond dépasse 2^31 − 1 pour
  un lexème de plus de 536 870 411 caractères, en deçà des 2^29 caractères
  au-delà desquels le lexeur lève `std::bad_alloc` : `"1e"` suivi de 536,8
  millions de 9 serait lu [0, 2^-1074] au lieu de [DBL_MAX, +∞]. Non vérifié,
  faute de chaîne 32 bits sur la machine. Correction : ces variables en `long
  long`.

### AE. Les trois builds (92, 95)

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
- **95.** **Les détections compilent dans un build et lient dans les autres** :
  la sonde de la cible AVX-512 lie avec CMake, mais ne fait que compiler avec
  autotools (`AC_COMPILE_IFELSE`) et meson, alors que `__builtin_cpu_supports`
  demande `__cpu_model` à l'édition de liens ; la sonde de FMA, à l'inverse, ne
  vérifie que le C++ avec CMake, et le C et le C++ avec édition de liens dans
  les deux autres. Non vérifié sur une chaîne où le résultat changerait.
  Correction : lier, et vérifier les deux langages, dans les trois builds.

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
  `process.md` (#155 est fusionnée).
- **Le tableau de `doc/accuracy.md`** cite encore `interval(sl, su)` pour
  `textToInterval(s)`, qui n'existe plus depuis GAOL v5.

### Remarqué par #150, à trier

Les questions ouvertes de la description de #150, non corrigées et sans
numéro :

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
  mémoire, qui cache le défaut. À ajouter à `process.md` (#155 est
  fusionnée).

### Remarqué par #151, à trier

Les questions ouvertes de la description de #151, non corrigées et sans numéro :

- **La phrase d'introduction de la section sur les exceptions flottantes** de
  `doc/using.md` et du manuel, « An infinite bound comes from a division by
  zero or from an overflow », reste vraie pour les deux exemples gardés,
  `log([0, 1])` et `[1e308]*10`, mais n'est pas une règle : `1/[0, 1]` a une
  borne infinie sans aucun indicateur depuis #121.

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

### Remarqué par #155, à trier

Les questions ouvertes de la description de #155, non corrigées et sans numéro :

- **Le paragraphe retiré du §6.1 reste vrai sur une machine** : l'arbre
  `GAOL_V9/GAOL`, les 19 et 20 octobre, avait les outils qu'il décrivait
  (Ubuntu 24.04, GCC 13.3, Clang 18 et 20, CMake 3.28, python 3.12, sans
  autoconf, automake, libtool, meson ni ninja, 4 cœurs) : autoconf 2.72 s'y
  construit depuis le tarball de ftp.gnu.org (sha256 vérifié), meson et ninja
  s'installent dans un environnement virtuel. Une phrase plus courte à la place
  du paragraphe ?
- **Les cœurs** : le §3.3 dit 4 cœurs au plus (« la machine en a 8 ») ; sur
  cette machine, qui en a 4, le mainteneur en a demandé 2 au plus. Une ligne au
  §3.3 pour l'arbre `GAOL_V9/GAOL` ?
- **L'en-tête** de `process.md` renvoie au §12 au lieu de donner une date et un
  commit, qui se périmeraient à la fusion suivante.
- La branche distante `todo-ai-msvc-avx512` n'a pas de pull request.

### Remarqué par #158, à trier

Les questions ouvertes de la description de #158, non corrigées et sans numéro :

- **La phrase « tightest bounds or one double beyond »** reste dans
  `doc/differences.md` (puce « The n-th roots are proved with integer powers »)
  et dans `manual/v5/gaol.tex` (la description de `nth_root`, « The bounds are
  the tightest ones, or one double beyond », et le tableau des précisions) : une
  borne d'une racine peut être à 2 doubles de la plus serrée, à 4 pour
  `rootn(x, q)` avec q < 0, pour tout x et avant #158 (l'erreur relative d'un
  produit arrondi dans un sens dirigé va jusqu'à 2^-52, pas 2^-53). #158 a
  corrigé le commentaire de `newton_root()` et `doc/accuracy.md`. À corriger
  dans la pull request de synthèse ; le manuel se compile avec `pdflatex`.
- **Un x sous-normal, ou de [2^-1022, 2^-1020[, prend 1,3 à 2,2 fois plus de
  temps** (chemin mis à l'échelle, sans pas de Newton pour un sous-normal :
  129 ns avant et 286 après pour `nth_root([1e-310], 5)`, 86 et 153 pour n =
  33) ; les x ordinaires sont inchangés. Acceptable, ou donner à
  `newton_root()` des puissances mises à l'échelle ? Temps pris sur une machine
  non au repos : à refaire au point Y.
- **Le `pow()` de la bibliothèque C de macOS n'est pas exact en arrondi dirigé**
  pour certains résultats sous-normaux (`pow(3·2^-95, 11)` en arrondi vers le
  haut rend `0x1.59fd80000004p-1028` au lieu de `0x1.59fd8p-1028`), alors qu'il
  l'est au plus proche ; un programme lié à GAOL est en arrondi vers le haut
  jusqu'à `gaol::cleanup()`. `tests/arithmetic.cpp` donne donc ses x
  directement ; les autres tests qui calculent un x avec `std::pow` n'ont pas
  été relus pour un résultat sous-normal.

### Remarqué par #157, à trier

Les questions ouvertes de la description de #157, non corrigées et sans numéro
(le mainteneur a décidé de garder tous les noyaux, le 22 octobre) :

- **Une piste non essayée** : un noyau mixte qui garde le chemin à une branche
  quand les deux opérandes ont le même signe, et le noyau à huit produits
  sinon. La variante hybride essayée (l'ancien chemin pour les opérandes
  positifs, mesurée hors de l'arbre) donne sept `*` positifs en 20,5 ns, mais
  `x * y` de signes aléatoires en 8,5 ns et sept `*` aléatoires en 49 ns contre
  35 pour le noyau : une branche sur un signe de nouveau.
- **`gaol_performance` apprend ses opérandes par cœur** : 1 024 opérandes
  répétés, que le prédicteur du M2 apprend (`x * y` de signes aléatoires 5,7 ns
  contre 12,4 sur 16 384). Ses lignes « signes aléatoires » sous-estiment donc
  le coût d'une branche sur les signes : passer ces trois lignes à 16 384
  opérandes, ou en ajouter trois ? Seul un commentaire de
  `tests/performance.cpp` a changé, pour ne pas rompre la comparaison avec les
  tableaux de `doc/three-builds.md`.
- **`fmax()` pour le plus grand de deux doubles** : une instruction sur arm64,
  RISC-V et POWER (2,2 ns contre 2,9 pour `x * y` dans un banc hors de l'arbre),
  mais elle classe +0 au-dessus de −0 (le signe d'un zéro serait celui de NEON)
  et appelle la bibliothèque sur ARM 32 bits et x86 ; utile surtout à Visual C++
  et clang-cl pour arm64, qu'on ne peut pas mesurer.
- **`GAOL_RND_KEEP()` sur la sortie de `*=`, `/=` et `%=` ?** Aujourd'hui non :
  les bornes de `*this` sont de la mémoire de l'appelant, donnée aussi par
  adresse aux fonctions des cas rares, si bien que le compilateur place les
  stores avant un appel ou un `asm` à clobber « memory ». C'est une protection
  de fait, qu'aucun test ne tient ; la remettre en ferait une règle, pour 2,6 ns
  (`x * y`) et 5 ns (`x / y`) avec `GAOL_PRESERVE_ROUNDING` (sans lui, la macro
  est vide).
- **`mid()` et `div_rel()`** ne sont pas touchés (`mid()` coûte 7,7 ns avec NEON
  comme avec la FPU) ; les opérations avec un double (`x * d`, `x / d`) ont
  encore une branche sur le signe de `d`, comme avec SSE2 (#124).
- **Le résultat de `xsmaxcdp` sur un POWER9 réel** n'est lu que dans la source
  de QEMU (`VSX_MAX_MINC`) et dans un commentaire de GCC, ni dans l'ISA ni sur
  un processeur : la phrase de `doc/using.md` (« premier opérande ») est celle
  de QEMU.
- **La seconde relecture de `process.md` §7 n'a pas été faite** (la première,
  sur `b584d78`, n'a pas eu de point bloquant). Non vérifiés : l'exécution sur
  ARM 32 bits, POWER, s390x et RISC-V, le noyau SSE2 (comparé à un modèle,
  jamais exécuté) et la vitesse sur ces processeurs.

### Remarqué par #160, à trier

Les questions ouvertes de la description de #160, non corrigées et sans numéro
(`performance.yml` a réussi sur #160, #162, #163 et #164) :

- **Le sens du rapport** vaut base sur pull request (au-dessus de 1, la pull
  request est plus rapide, comme le « Gain » de #150) ; beaucoup de lecteurs
  attendent l'inverse. C'est une ligne de `compare-performance.py` et de son
  test.
- **Le seuil de la liste** des lignes écartées (un facteur 2 au moins et
  au-delà de la dispersion) : les mesures donnent des rapports jusqu'à 1,17
  pour du code identique ; 1,5 serait possible.
- **Le filtre de chemins** de `performance.yml` n'a ni `Makefile.am`,
  `configure.ac`, `meson.build` et `meson_options.txt` (l'outil ne mesure que le
  build de CMake), ni `doc/compare/code/run_perf.sh`, que lit le test des
  outils.
- **Les jobs** : CMake et Ubuntu 24.04 seulement, ni arm64 Linux, armhf, i386,
  macOS x86_64, clang-cl ni Visual C++ arm64 ; le job armhf/arm64 de #150
  compare toujours deux options d'un arbre, non la base et la pull request. Un
  cache (`actions/cache`, clé : le SHA de la base) du binaire de la base
  éviterait de la reconstruire à chaque poussée, sous Windows surtout ; non
  essayé.
- **`gaol_bench` avec Visual C++** n'avait jamais été compilé avant #160 ; les
  dix lignes ne se comparent qu'avec une base qui a la cible, et un
  avertissement du job dit quand elles sont sautées.
- **L'élargissement de `make perf` à macOS et de la cible `gaol_bench` à tous
  les systèmes** (CMake, meson) vient de l'outil : à garder ou à restreindre.
- **`LINE_LIST` vide** ne lance aucune ligne dans `run_perf.sh` et les dix dans
  `run_lines.sh` ; harmoniser ferait changer `run_lines.sh` (le README dit la
  différence).
- **`process.md` §10** liste les workflows, sans `performance.yml`.
- **Remarqué sans être corrigé** : `actionlint` dit que
  `suppress-deprecation-warnings` n'est pas une entrée de `msys2/setup-msys2@v2`
  (job `msys2-refused` de `windows.yml`), et `shellcheck` fait des remarques sur
  des étapes de `build-systems.yml`, `linux.yml`, `manual.yml` et `windows.yml`.

### Remarqué par #161, à trier

Les questions ouvertes de la description de #161, non corrigées et sans numéro :

- **Une option à trois valeurs `AUTO`, `ON`, `OFF`** pour `GAOL_PREFER_AVX512`
  (comme `auto` de meson) lèverait les limites 2 et 3 du commentaire de
  `CMakeLists.txt` (le texte d'aide de la ligne de commande sert à reconnaître
  une demande ; la variable ordinaire d'un projet englobant), mais change le
  type de l'option et ce que lit un projet englobant : à décider.
- **`configure` a été édité à la main** (par #161 et par #163), sans autoconf
  2.72 : vérifié le 23 octobre à `601fc63`, `autoconf` 2.72 (celui de cette
  machine), lancé sur une copie des fichiers suivis, régénère un `configure`
  identique au commité.
- **Les commentaires de la CI et de `doc/compare/code/`** qui disent encore
  « +, -, *, / et sqrt » (`.github/scripts/sde-tests.sh`, `linux.yml`,
  `windows.yml`, `build_benchmarks.sh`, `run_bench.sh`) : à mettre à jour ou
  non.
- **`doc/compare/code/setup.sh`** garde `-UGAOL_PREFER_AVX512`, utile à un
  dossier configuré avant #161 ; il peut disparaître quand plus aucun ne
  subsiste.
- **La relecture des corrections** que la relecture indépendante a fait faire
  (points A à F) n'a pas été faite.

### Remarqué par #162, à trier

Les questions ouvertes de la description de #162, non corrigées et sans numéro :

- **Le test `e == 2` dans les en-têtes** (`pow` de `gaol/gaol_interval.h` et
  `pown` de `gaol/gaol_ieee1788.h`) : `pow(x, 2)` y coûterait 1,35 ns avec NEON
  au lieu de 1,66, le coût de `sqr`, au prix d'un second endroit à tenir (la
  décision reste dans `gaol_pown`). À décider.
- **Les puissances 3 à 5 avec les intervalles FPU coûtent environ 0,3 ns de
  plus** (+5 %), `gaol_pown` étant maintenant un saut vers `pown_checked`
  (`pow(x, 3)` de 6,41 à 6,93 ns sur un M2, de 6,43 à 6,79 chez le relecteur) ;
  à remesurer sur x86, où un appel de plus coûtait 0,4 à 0,6 ns sur un
  i7-1185G7.
- **L'ensemble vide et le sens d'arrondi** : `pow(vide, 2)`, `2u`, `2LL` et
  `gaol_ieee1788::pown(vide, 2)` laissent le sens du programme, comme
  `sqr(vide)`, mais `gaol_pown_ll(vide, 2)`, `pow(vide, 2.0)`, `pow(vide, [2])`
  et `gaol_ieee1788::pow(vide, [2])` le mettent vers le haut avant de rendre le
  vide. Sans conséquence sur les bornes ; `doc/using.md` ne promet rien.
- **Le texte de #154** (« +0 avec `pow` et −0 avec `sqr` ») était inexact pour
  NEON et FPU (`doc/simd.md` est corrigé) ; les intervalles SSE2, où le signe de
  l'ancien `pow` n'est pas mesuré, peuvent avoir un autre profil.
- **Non vérifiable ici** : toute exécution sur x86 (le signe d'un zéro de
  l'ancien `pow` avec SSE2, le chemin AVX-512), GCC, Windows ; les temps
  viennent d'un seul M2 partagé, et pas avec `GAOL_PRESERVE_ROUNDING`.

### Remarqué par #163, à trier

Les questions ouvertes de la description de #163, non corrigées et sans numéro :

- **Le bogue 54412 de GCC** (la pile de Windows x64 est alignée sur 16 octets,
  non sur 32) : Bugzilla refuse l'accès, et son état vient de lectures
  indirectes (ouvert jusqu'à GCC 16, un correctif sur gcc-patches le 6 juin
  2026) ; le commentaire de `CMakeLists.txt` dit « fixed for GCC 17 only », ce
  qui est plausible et non vérifié.
- **Les `fesetround()` et `fegetround()` de mingw-w64** ont été lus dans les
  sources de `master`, du CRT de MSYS2 de la CI (`4564ee4b5`) et des étiquettes
  v13 et v14, non dans ceux de Chocolatey. Une version qui garderait les
  indicateurs ne gênerait pas la correction, mais n'a pas été cherchée.
- **`GAOL_PRESERVE_ROUNDING` avec autotools et meson sous MSYS2** : les entrées
  de `build-systems.yml` ne l'ont pas, seules celles de `windows.yml` (CMake) le
  font. Le contrôle des indicateurs depuis les six directions n'est lancé que
  pour mingw-w64 : l'étendre à Visual C++ x64, Linux, macOS et ARM est une
  décision de plus, avec le risque de trouver d'autres défauts.
- **`-mavx512f` ou `-march` donnés par l'utilisateur** : avec le Clang croisé,
  `-mavx512f -mfma -mprefer-vector-width=128` laisse 2 lignes `%ymm` et 2 887
  `%zmm`, et `-march=skylake-avx512 -mprefer-vector-width=128` 406 `%ymm` ; le
  relecteur a compté 0 et 0, puis 4 `%ymm`, et l'écart n'est pas élucidé. Rien
  n'est vérifié avec GCC (le relecteur a lu le réglage de GCC 14.2 sans
  l'exécuter). Faut-il le dire dans `doc/using.md` ?
- **Cygwin** reçoit `-mfma` seul, sans `-mprefer-vector-width=128`, comme
  avant : les trois détections ne le prennent pas pour « GCC pour Windows »
  (`WIN32` de CMake, `_WIN32` de configure et de meson), alors que celles
  d'AVX-512 testent aussi `__CYGWIN__`. Lui donner les mêmes drapeaux ? Non
  vérifié (ni Cygwin ici, ni dans la CI).
- **x86 32 bits** (MinGW-w64 12.2 à 15.2) reste sans FMA et n'est pas mesuré ;
  **`-mstackrealign`** n'a pas été essayé.
- **Les corrections qui ont suivi les deux relectures** (`704ca0c`, `e80b6f0`,
  `1de1df0`, `15399b5`) n'ont pas été relues par un relecteur indépendant.

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

### AI. La vitesse sur macOS, Windows et ARM (106, 108, 109, 112, 116)

La CI de #124 (`1a15fc8` et `c6a3d25`, 17 octobre) écrit le tableau de
`gaol_performance` dans l'étape « Performance » de ses jobs : c'est la seule
mesure que nous ayons sur macOS et sur Windows. Les noyaux de #124 (`+ - * /` et
`sqr` sans branche sur les signes, bornes en registre) servent les intervalles
SSE2, c'est-à-dire x86-64 (Visual C++ et clang-cl x64 compris depuis #148),
Windows 32 bits avec GCC et Clang depuis #152, et, depuis #146, les intervalles
NEON d'arm64 ; depuis #157, les intervalles FPU ont eux aussi des noyaux
scalaires sans branche sur les signes pour `x * y`, `x / y`, `x % y` et `sqr`.
Pour la ligne de 13 opérations `+ - * /` du tableau, en ns : 33 sur Linux x86-64
(job d'autotools, Ubuntu 24.04), 36 sur macOS 15 x86-64, 33 à 58 avec MinGW-w64
et MSYS2, 64 à 93 sur macOS arm64, 100 à 168 avec Visual C++ x64 et arm64 et
avec clang-cl, 1 100 à 1 200 avec Visual C++ 32 bits (macOS arm64, avant #146 ;
avec NEON, la ligne passe de 49,8 à 19,2 ns sur un M2 ; avec les intervalles
SSE2 de #148, 49 ns avec Visual C++ x64 2022 et 2026 et 37 avec clang-cl, dans
la CI de `5fa7203`) ; pour les intervalles FPU, ces temps sont d'avant #157.
Les temps de la CI ne se comparent pas d'un passage à l'autre : les runners sont
des machines virtuelles partagées, et entre les deux passages de #124, au code
inchangé, `x * y` de Visual C++ x64 prend 7,9 puis 16,3 ns et `sqrt` de MSYS2
avec GCC 16 6,8 puis 170 ns. Seuls des écarts de plusieurs fois valent ; #160
compare, dans le même job, la base d'une pull request et sa tête
(`.github/workflows/performance.yml`).

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
  Dans la CI de `5fa7203` (un passage), la ligne de 13 opérations
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

  Fait par #157 pour les intervalles FPU (ARM 32 bits, POWER, s390x, RISC-V,
  Visual C++ et clang-cl en x86 32 bits, Visual C++, clang-cl et MinGW-w64 en
  arm64, `GAOL_SIMD=OFF`) : des noyaux scalaires sans branche sur les signes
  pour `x * y` (huit produits), `x / y`, `x % y` et `sqr(x)`
  (`gaol/gaol_interval_fpu.cpp`), et `x - y`, qui lit ses bornes après
  `GAOL_RND_ENTER()` et prend le temps de `x + y` (3,62 puis 1,18 ns sur un M2).
  Sur un M2, `x * y` de signes aléatoires passe de 12,4 à 3,3 ns (16 384
  opérandes), `x / y` de 9,3 à 3,2, `sqr` de 5,9 à 1,8 et Shekel 5 de 298 à 138
  ns ; dans la CI, avec deux builds alternés, `x * y` de signes aléatoires gagne
  de 1,23 à 3,47 fois sur les neuf configurations mesurées (armhf natif, Visual
  C++ et clang-cl x86 et arm64, Ubuntu et macOS en FPU). Pertes acceptées par le
  mainteneur (« tout garder », 22 octobre) : une ligne de sept `*` d'opérandes
  de signes prévisibles prend 50 % de temps de plus sur le M2 (23,8 puis 35,8
  ns) et va à 0,58 de sa vitesse d'avant avec GCC sur Ubuntu, 0,57 avec clang-cl
  arm64 ; le quotient avec Visual C++ arm64 (0,82) ; le carré avec Clang sur
  Ubuntu et clang-cl arm64 (0,85 et 0,84) ; la ligne de treize opérations avec
  le sens préservé sur Visual C++ x86 (0,53) ; et rien de gagné avec
  `GAOL_PRESERVE_ROUNDING`. Le signe d'une borne nulle d'un produit n'est pas
  spécifié avec les intervalles FPU : GCC 13 pour POWER9 compile le maximum en
  `xsmaxcdp`, qui rend le premier de deux zéros de signes opposés (la CI de #157
  l'a montré) ; la valeur de la borne ne change pas.

  À chercher : pour Visual C++ arm64, offrir NEON (`arm64_neon.h`) ; mesurer les
  noyaux de #157 sur POWER, s390x, RISC-V et MinGW-w64 arm64 (sur POWER8, s390x
  et RISC-V, GCC met des branches sur les comparaisons de produits : le noyau y
  est plus long que l'ancien code et y perd peut-être sans rien gagner). Les
  opérations avec un double n'ont ces noyaux sur aucune cible (#124 :
  « `x * 2.0` n'a pas les noyaux de `x * y` »). À rapprocher de l'étude des
  pistes SIMD (`doc/simd.md`, #154).

- **108.** **Visual C++ 32 bits : 80 ns par opération, dès le build par défaut**
  (job « Visual Studio 2026 x86 Release » : `x + y` 80,8 à 83,5 ns, `x * y` 87,8
  à 98, une ligne de 15 `+` 1 211 à 1 307 ns). `gaol/gaol_fpu.h` le sait :
  `fegetround()` et la lecture de MXCSR y coûtent environ 80 ns, et la sonde du
  sens d'arrondi par une addition y est écartée (`GAOL_RND_PROBE` n'est pas
  défini sur x86 32 bits).

  À chercher : lire les deux unités, x87 et SSE, sans appeler `fegetround()`
  (`fnstcw` et `stmxcsr` en ligne, ou `_controlfp()`), et ce que cela coûte.

  Fait par #156 (fusionnée le 20 octobre) : `fnstcw` et `stmxcsr` en ligne sur
  tous les processeurs x86 32 bits, sans `fegetround()`, et seul le registre qui
  n'est pas vers le haut est écrit. Le gain n'a pas été mesuré (la machine de
  travail n'a ni Windows ni x86) : il se lit dans l'étape « Performance » de la
  CI de #156, à comparer à celle de `configure-clean`. Restent, décidés le 22
  octobre (Q8, en bas) : la lecture en ligne dans CORE-MATH, un test de
  `x86_registers::read_control()`, l'outil de mutation et l'émulateur dans
  `tests/tools/`, et le retrait du chemin de repli `fegetround()`.

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

- **112.** **Suites de #124 non faites** (ses questions ouvertes), ce qui reste
  après #161 :
  - avec `GAOL_PRESERVE_ROUNDING`, les fonctions élémentaires paient encore deux
    écritures de MXCSR, jusqu'à 27 % de plus que sans (`exp`) (CORE-MATH calcule
    avec le sens matériel, aucun arrondi embarqué n'y est possible) ; un mode de
    CORE-MATH où le sens serait un paramètre les supprimerait, au prix de
    modifier `3rd/math-core` ;
  - avec `GAOL_PRESERVE_ROUNDING` et AVX-512, `x * d` et `x *= d` (un intervalle
    par un double) donnent le signe opposé pour une borne nulle de 344 résultats
    sur 5,7 millions entre le chemin AVX-512 (`[-0, +0]`) et le chemin SSE2
    (`[+0, -0]`), la valeur étant la même ; relevé par #124, déjà ainsi à
    `7716fec`, et aucun test ne le voit, qui comparent avec `==`. Décidé le 18
    octobre (point AF, #140) : c'est une exception acceptable au « bit for bit
    », dite dans la documentation et les commentaires, sans changer le calcul ;
    il ne reste que, si l'on veut, un test qui compare les bits, qui demande un
    x86 avec AVX-512 pour comparer les deux chemins (#161 ne l'a pas fait).

  Fait par #161 : le défaut de CMake (un second `cmake` dans le même dossier
  faisait passer `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING`,
  `DEFINED` voyant l'entrée de cache du premier passage) : l'entrée garde la
  valeur dérivée, l'entrée interne `GAOL_PREFER_AVX512_DERIVED` la note, et une
  valeur donnée reste jusqu'à `-UGAOL_PREFER_AVX512` ; le test
  `tests/prefer_avx512_reconfigure.cmake` configure plusieurs fois le même
  dossier (il échoue avec le `CMakeLists.txt` de `configure-clean`, et quatre
  mutants de la correction sont tués). Les descriptions de l'option disent
  maintenant « +, -, *, /, sqr, sqrt and, with FMA, the integer powers beyond 1
  and -1 » (`CMakeLists.txt`, `meson_options.txt`, `configure.ac`, les trois
  gabarits de `gaol_configuration.h`, `doc/building.md`, `doc/three-builds.md`,
  le manuel, l'exemple 13).

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

- **Suites de #152 (AI), Windows 32 bits : les intervalles SSE2 avec GCC et
  Clang** (`-faligned-new=8`, donné par `gaol::gaol`, `gaol.pc` et `gaol_dep`),
  relevées par la relecture, non faites :
  - aucun job ne construit autotools ni meson avec MinGW-w64 x86 (le MSYS2 de
    `build-systems.yml` est x64), et l'audit des trois builds ne tourne pas sous
    Windows : leur accord pour Windows 32 bits n'est vérifié que par simulation
    (`CXXFLAGS=-D_WIN32`) ;
  - `-faligned-new=8` ne règle que le tas : l'alignement de la pile 32 bits
    (GCC, bogue 40838) concernerait des intervalles locaux dans des fonctions
    que l'API de Windows appelle avec une pile alignée sur 4 octets (fils
    d'exécution, rappels) ; non testé ;
  - `gaol.pc` et `gaol_dep` de meson donnent `-faligned-new=8` à tout fichier,
    C compris, et GCC avertit alors pour un fichier C (« valid for C++/ObjC++
    but not for C ») ; `gaol::gaol` de CMake le réserve au C++.

  Visual C++ et clang-cl gardent les intervalles FPU sur x86 (décidé le 20
  octobre) : leur `/Zc:alignedNew` ne suffit pas, seul C++17 allume le `new`
  aligné (Compiler Explorer, MSVC 19.51 et clang-cl 22), ce qui ferait compiler
  en C++17 tout le code qui utilise GAOL. Les jobs MinGW-w64 x86 (12.2 à 15.2)
  passent avec le drapeau.

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
  `configure-clean`: no », certains périmés), et, pour le reste, dans les 53
  descriptions de pull requests qui ont une section « Pour `doc/differences.md`
  et `ChangeLog` » : #71, #76, #77, #83 à #97, #99, #100, #105, #108, #111,
  #113, #114, #117 à #124, #139, #140, #141, #144, #145, #146, #147, #148, #149,
  #150, #151, #155, #156, #157, #158 et #160 à #164 (#123, #139, #155 et #160
  n'ont rien à y mettre ; celle de #146 dit que `doc/differences.md` l. 333, «
  the intervals of ARM » sont FPU, n'est plus vrai sur arm64, et celle de #148
  que la même ligne, « of Visual C++ and of 32-bit Windows », ne vaut plus pour
  x64 ; celle de #150 ajoute que `GAOL_PRESERVE_ROUNDING` coûte 22 ns au lieu de
  48 sur ARM 32 bits avec GCC, et le défaut de `width()` et `bisect()` sur arm64
  ; celle de #156 ajoute à `doc/differences.md` (vers les lignes 159 et 272) la
  lecture directe du sens d'arrondi sur x86 32 bits (`fnstcw`, `stmxcsr`) et la
  remise de chaque unité dans son sens avec Visual C++ pour x86 ; celle de #158
  dit que la puce « The n-th roots are proved with integer powers » de
  `doc/differences.md` doit parler de « jusqu'à 2 doubles » et non d'« un double
  », et du chemin des x sous 2^-1020 ; celle de #157 ajoute les noyaux sans
  branche des intervalles FPU (`x * y`, `x / y`, `x % y`, `sqr`, et `x - y` au
  temps de `x + y`), le signe d'une borne nulle d'un produit, non spécifié avec
  la FPU, et les indicateurs `FE_UNDERFLOW` et `FE_OVERFLOW` d'un coin du
  produit ; celle de #161 donne les deux entrées de `ChangeLog` du défaut de
  CMake et des descriptions d'options ; celle de #162 que `pow(x, 2)` est
  `sqr(x)`, au signe d'un zéro et aux indicateurs de `sqr` près ; celle de #163
  que GCC pour Windows x86-64 reçoit `-mfma -mprefer-vector-width=128` et que
  GAOL y lit et écrit lui-même les registres de contrôle avec mingw-w64 13 et
  plus ; celle de #164 dit que le format `agreeing` écrit un intervalle
  ponctuel comme un nombre nu, que la lecture d'un long nombre prend un temps
  quadratique en ses chiffres, qu'une espace est nécessaire entre un signe et
  un nombre incertain, le piège de `namespace gaol { }`, et que
  `gaol_performance` est compilé en C++14). Les points K, L et `make
  distclean` (#72 à #75) n'ont pas cette section
  : à écrire d'après leurs descriptions. À écrire d'après les commits, sans pull
  request : les changements du 4 octobre (`nb_fp_numbers()` rend un `unsigned
  long long`, `gaol_parameters.h` supprimé), et ceux du 15 au 18 octobre, qui
  changent l'interface installée : les en-têtes du parser et des expressions
  dans `include/gaol/parser/` (`gaol_expression.h`…), `gaol_interval.h` scindé
  en `gaol_interval_core.h` et `gaol_interval.h`, les intervalles de floats dans
  `gaol/float/`, `gaol_assert.h`, `gaol_profile.h` (la classe `timepiece`),
  `gaol_exact.h`, `gaol_flags.h`, `gaol/dllmain.cpp` et `operator""_iv`
  (`gaol_literals.h`) supprimés, `f_negate`, `f_negate_simple`, `gaol_opposite`
  et `reset_fpu_cw` supprimés (a6786af), la lecture en bases 16, 8 et 2
  (91dcce2) ; vérifier ce que `doc/differences.md` en dit déjà. Le point G n'a
  pas de texte dans la description de #91 (écrite avant la fin du travail) : à
  écrire d'après c7915a7, ccf694a et `doc/three-builds.md`. Les chiffres de
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

- **Branches** : les branches locales du mainteneur (`MATH-CORE`, `master`) ne
  se touchent pas. Les branches de #124 (`todo-zz-perf`), de #144
  (`todo-ab-pow-invalid`), de #145 (`todo-ae-c-compiler-checks`), de #146
  (`todo-ARM-perf`), de #147 (`todo-ad-parse-interval-null`), de #148
  (`todo-ai-msvc-sse2-avx512`), de #149 (`todo-ad-83-84-reader`), de #150
  (`todo-arm32-fpscr`), de #151 (`todo-ab-cancel-invalid`), de #155
  (`todo-al-process-md`), de #156 (`todo-ai-msvc-x86-csr`), de #157
  (`todo-ai-fpu-kernels`), de #158 (`todo-ac-nth-root-subnormal`), de #160
  (`todo-ai-111-115-ab-bench`), de #161 (`todo-ai-112-cmake-options`), de #162
  (`todo-an-pow2-sqr`), de #163 (`todo-ai-110-gcc-windows-fma`) et de #164
  (`todo-QB-text-doc`) sont supprimées sur GitHub (celles de #145 à #164 à la
  fusion, le 19 octobre pour les premières, du 20 au 23 pour les autres ; celle
  de #156, restée un temps, ne s'y trouve plus le 23) ; en local, celles de
  #146 (`todo-ARM-perf`), de #150 et de #151, le 20 octobre, celles de #155 et
  de #158, le 22, et celle de #164, le 23 ; celle de #157 reste dans l'arbre
  `GAOL_V9/GAOL`, où elle est extraite. Celle de #153
  (`todo-fpcr-gcc-hoisting`), fermée sans fusion, sur GitHub et en local, le 20
  octobre. Restent sur GitHub `todo-ai-msvc-avx512` (sans pull request) et trois
  branches d'expérience : `exp-110-gcc-windows-fma` et
  `exp-110b-stack-alignment` (les mesures du point 110, que la description de
  #163 dit supprimées) et `exp-157-ab` (les mesures en alternance de #157,
  jamais à fusionner).
- **Les lignes de crédit** : retirées le 20 octobre des descriptions de #15 à
  #24 et #60 à #63 et des commentaires de #58 et #60 à #63 ; les commits
  restent (décidé le 19 octobre, rang 1).
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
79 : AB ; 82 : AC ; 85 : AD ; 92 : AE ; 95 : AE ; 105 : AH ; 106 : AI ;
108 : AI ; 109 : AI ; 112 : AI ; 113 : AJ ; 116 : AI.

## Réponses du mainteneur, 22 octobre 2026

Réponses aux seize questions restées sans réponse dans les descriptions des 20
dernières pull requests fusionnées (#122 à #158), dans les sections « Remarqué
par …, à trier » ci-dessus et dans les points 46 (F), 26 (W), 111, 113 (AJ),
115, 121 et 125 (AM). Les rangs et les groupes du haut sont à mettre à jour en
conséquence.

- **Q1 (QI, PR #157, noyaux FPU)** : mesurer d'abord. Ne pas fusionner le
  produit tel quel ; lancer les mesures du produit sur armhf (conteneur Debian,
  vrai ARM 32 bits VFPv4), POWER, s390x et RISC-V avant de décider. La pull
  request reste en brouillon. *Suite* : les mesures en alternance sur la CI
  (armhf natif, Visual C++ et clang-cl, Ubuntu et macOS en FPU) ont été faites,
  le mainteneur a gardé tous les noyaux et #157 est fusionnée ; POWER, s390x et
  RISC-V restent non mesurés (rang 8).
- **Q2 (QB, 121)** : ajouter `.continue/` au `.gitignore`. *Fait par #164.*
- **Q3 (QC, 125 AM)** : clos. `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT`
  ont déjà des valeurs par défaut dans `gaol/gaol_config.h` (4 ou 8 selon
  `UINT_MAX`, 8 selon `ULLONG_MAX`) ; les trois gabarits de configuration ne les
  définissent pas, et l'en-tête installé suffit. Retirer le point 125.
  *Retiré le 23 octobre.*
- **Q4 (QH, 46 F, #64)** : garder un intervalle par ligne (comportement actuel,
  documenté). Pas de changement d'`operator>>`.
- **Q5 (QH, 26 W, #67)** : rien pour l'instant. GAOL reste un intervalle nu ;
  les pièges sont documentés (faits par #140). Reporter indéfiniment.
- **Q6 (QD, 111 et 115 AI)** : mesurer par la CI deux builds alternés dans le
  même job (par exemple `configure-clean` et la branche), comme `doc/compare`
  alterne ses programmes. *Fait par #160.*
- **Q7 (QE, 113 AJ)** : non, après v5.0.0. L'étude des noyaux au plus proche est
  reportée après la version ; le comportement actuel (changement du sens
  d'arrondi) est documenté.
- **Q8 (QJ, #156, x86 32 bits)** : tout oui. (a) Lire MXCSR en ligne dans
  CORE-MATH sur x86 32 bits (`gaol/core_math_port.h` comme sur x86-64). (b)
  Ajouter un test de `x86_registers::read_control()`. (c) Garder le job
  « Visual Studio 2026 x86 Release » avec la direction préservée. (d) Ajouter
  l'outil de mutation et l'émulateur au dépôt (`tests/tools/`). (e) Retirer le
  chemin de repli `fegetround()` (code mort aujourd'hui).
- **Q9 (QJ, #155, process.md)** : (a) Oui, une phrase plus courte à la place du
  paragraphe retiré du §6.1. (b) Non, garder « 4 cœurs au plus » au §3.3 sans
  ligne pour `GAOL_V9/GAOL`. (c) Mettre une date et un commit dans l'en-tête de
  `process.md`, au lieu du renvoi au §12.
- **Q10 (QJ, #154)** : les deux oui. (a) Durcir `sqrt_bounds()` par un masque
  sur le bit de signe (FPCR.AH / FEAT_AFP), en une pull request à part, validé
  sur une copie. (b) Faire `pow(x, 2)` par `sqr` (environ 6 fois plus rapide sur
  cette opération). *(b) fait par #162, 3,3 fois plus rapide avec NEON et 2,5
  avec la FPU : l'« environ 6 fois » était le rapport de l'ancien `gaol_pown` à
  `sqr`.*
- **Q11 (QJ, #151)** : garder la correction élargie. Le Fast2Sum corrige aussi
  la somme finie à mi-chemin de deux doubles, de même cause ; c'est mieux que la
  formule stricte du TODO.
- **Q12 (QJ, #149)** : garder pour tous (actuel). Le contexte garde le texte de
  chaque nombre qui n'est pas un double ; le coût est connu et documenté, et la
  comparaison exacte est un plus.
- **Q13 (QJ, #147)** : lever une erreur de GAOL comme une mauvaise lecture du
  parser. `textToInterval(nullptr)`, `gaol_ieee1788::textToInterval(nullptr)` et
  `exactToInterval(nullptr)` doivent lever `input_format_error` (ou une
  `gaol_error`) pour un pointeur nul, sans passer par la construction
  indéfinie d'un `std::string`.
- **Q14 (QJ, #146, arm64)** : (a) Oui, porter le noyau empaqueté de `pown`
  (`GAOL_POWN_PACKED`) sur arm64, 150 lignes d'intrinsiques, gain ~1,5x. (b)
  Non, pas de test comparant les bits du signe d'un zéro. (c) Non, pas de jobs
  Clang 15 à 17 ni GCC 9 à 10 sur arm64, pas de test `mrs`/`msr` sur Neoverse.
- **Q15 (QJ, #124, noyaux SSE2)** : (a) Mettre les noyaux exportés
  (`add_bounds()`, `sub_bounds()`, `mul_bounds()`, `div_bounds()`,
  `sqr_bounds()`) dans `gaol_detail`. (b) Garder le plus rapide (le signe des
  zéros diffère, c'est documenté). (c) Non, ne pas modifier `3rd/math-core`
  pour passer le sens d'arrondi en paramètre.
- **Q16 (QI, branches)** : garder les deux. `todo-ai-msvc-avx512` (sans pull
  request, semblant remplacée par #148) et `todo-fpcr-gcc-hoisting` (branche de
  #153, fermée sans fusion, correction dans #150) restent sur GitHub.
