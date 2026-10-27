# Fait et décidé sur GAOL v5

Tout ce qui est fait sur GAOL v5 et tout ce qui a été décidé, au 27 octobre 2026
(commit `9deb2e1` de `configure-clean`, aucune pull request ouverte). Ce fichier
et l'issue #177 ont le même contenu.

Il réunit l'historique de l'ancienne issue #49 (« Suivi du TODO de GAOL v5 »),
ce qui est fait dans l'ancienne issue #142 (« Reste à faire sur GAOL v5 »), dans
l'ancien `TODO.md` et dans `TODO_mistral.md`, avec les réponses du mainteneur
des 22 et 23 octobre. #49 et #142 sont fermées le 24 octobre, `TODO_mistral.md`
est supprimé. Le reste du travail est dans deux autres registres :

- `TODO.md` (issue #179) : ce qu'il reste à faire, en tâches PF à PJ ;
- `TODO_AFTER.md` (issue #178) : ce qui est reporté, après la version, jusqu'à
  une décision ou jusqu'à du matériel.

## Comment lire ce fichier

- Les points ont des lettres (A à Z, puis AA à AP) et des numéros (1 à 133). Un
  point réunit ce qui touche le même code ou le même fichier, ou ce qu'un ordre
  impose de faire ensemble, et garde en sous-points les numéros de l'ancienne
  liste, auxquels renvoient les pull requests, les issues, les rapports des
  agents et les commentaires du code : « point Q of TODO.md » dans le code
  renvoie au point Q de la section 4 ci-dessous.
- Les numéros viennent : de la revue du 2026-09-27,
  [examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro
  n de sa section 5), pour les anciens 4 à 30 et 34 à 40 ; de la vérification de
  `VERSION.txt` (41 à 44) ; des corrections, des relectures des pull requests,
  du tri de `TODO_mistral.md` et d'une comparaison de `pown` avec le `pow` de
  CORE-MATH (45 à 76) ; de la relecture de `configure-clean` du 14 octobre (77 à 103)
  ; des décisions du point AA et des mesures de la CI de #124 (104 à 112) ;
  d'une question du mainteneur du 18 octobre (113) ; de la relecture des notes
  et des commentaires des pull requests fermées et de #49 (114 à 127, le 18
  octobre) ; des réponses du mainteneur des 22 et 23 octobre (128 à 133).
- Dans les textes écrits avant le 24 octobre, « `TODO.md` », « cette issue »
  (quand le texte vient de #49) et « #142 » désignent les anciens registres :
  l'ancien `TODO.md`, l'ancienne issue #49 et l'ancienne issue #142, que ce
  fichier remplace en partie.
- « Décidé le 3 octobre » (ou une autre date) marque une décision du mainteneur
  : elle s'applique telle quelle.
- Le classement par rang et par tiers (A à I) de l'ancien #142 n'existe plus :
  un « rang N » cité dans un texte d'avant le 24 octobre désigne cet ancien
  classement. Ce qui reste à faire est classé en tâches PF à PJ dans `TODO.md`.
- Ce qui sert encore des rapports des agents est dans
  [todo-notes/synthese.md](todo-notes/synthese.md), leurs originaux au commit
  `16a2f60` (`todo-notes/`, ceux du 30 septembre dans `todo-notes/2026-09-30/`)
  ; la procédure suivie pour chaque point est dans `process.md`.

Sommaire : 1. L'état au 26 octobre ; 2. Les pull requests ; 3. Les décisions,
par date ; 4. Les points faits, par lettre ; 5. Les anciens numéros faits ; 6.
Les questions ouvertes des pull requests, tranchées ; 7. Le travail direct sur
`configure-clean` ; 8. Le ménage fait.

## 1. L'état au 27 octobre 2026

- **`configure-clean`** est au commit `9deb2e1` (fusion de #205). Les pull
  requests des corrections vont vers `configure-clean`, qui reste la branche de
  développement après v5.0.0 ; `MATH-CORE` et `master` (GAOL 4.3.2, plus deux
  commits de `.claude/CLAUDE.md`) ne bougent qu'à la version (point Y).
- **Aucune pull request ouverte.** Le mainteneur a fusionné #165 à #167, #169 à
  #174, #176 et #180 à #184 les 23 et 24 octobre, et #186 à #194, #196 et #197
  le 25 octobre, #199, #200 et #201 le 26 octobre, #202 à #205 le 27 ; #168 et
  #195 sont fermées sans fusion. Les tâches PA, PB, PC, PD et PE sont faites
  (PE : #200 et #204 pour le 95, #201 pour le 92, #205 pour le 3, #202 pour le
  4, #204 pour le 5, #203 pour le 6) ; PG est entamée (la division du chemin
  AVX-512, le 106 a : #191 à #194).
- **Les intervalles de chaque build** (décidé le 25 octobre, #192 à #194) : NEON
  sur tout processeur ARM 64 bits avec tous les compilateurs (GCC, Clang,
  AppleClang, clang-cl, Visual C++, MinGW-w64, MSYS2), SSE2 sur tout processeur
  x86 avec tous les compilateurs (sur Windows 32 bits, GCC et Clang avec
  `-faligned-new=8`, Visual C++ et clang-cl avec des bornes en deux `double`),
  le chemin AVX-512 par-dessus SSE2 avec `GAOL_PRESERVE_ROUNDING`, où le
  compilateur et la machine le permettent, et le FPU seulement où l'on n'a pas
  le choix (ARM 32 bits, POWER, s390x, RISC-V...) ou quand on le demande
  (`GAOL_SIMD=OFF`, qui vaut aussi pour NEON depuis #200 : `GAOL_ARM64` n'est
  plus une option). Un build universel de macOS (CMake, autotools, meson,
  plusieurs architectures) prend NEON pour arm64 et SSE2 pour x86_64, sans FMA
  ni AVX-512.
- **Les options** (décidé le 26 octobre, #200) : les trois builds ont les
  mêmes, `GAOL_PRESERVE_ROUNDING`, `GAOL_SIMD`, `GAOL_COVERAGE`, `WITH_TESTS`,
  `WITH_EXAMPLES`, `BUILD_SHARED_LIBS`, `CMAKE_BUILD_TYPE`, le code indépendant
  de la position et `CMAKE_OSX_ARCHITECTURES`, avec la bibliothèque statique par
  défaut ; les instructions FMA, le chemin AVX-512, l'entier 128 bits et le
  débogage sont décidés par des tests de la configuration, les anciennes
  options restant des réglages des développeurs (`doc/building.md`).
- **Branches** : GitHub a `configure-clean`, `MATH-CORE`, `master` et
  `perf-doc-rerun` (les scripts des mesures reportées, hors des points) ; les
  branches fusionnées sont supprimées en local.
- **Aucune borne fausse n'est connue** : sur environ 1,3 million de puissances
  entières, 550 000 `pow` réels, 600 000 autres opérations et 760 lectures de
  nombres (relecture du 14 octobre, revérifiée le 18 octobre à `4eaf69a`), tout
  résultat contient la valeur exacte. Une exception possible : un programme qui
  met lui-même `FPCR.AH` sur arm64, où `x / y` des intervalles NEON a donné des
  bornes différentes sur un runner de la CI le 25 octobre, dont une borne
  inférieure NaN (issue #198, cause non trouvée).
- **Issues ouvertes** : les issues « À communiquer… » #125 (CORE-MATH), #126
  (GCC), #127 (mingw-w64), #129 (ITF1788), #130 (Boost.Interval), #131
  (Microsoft), #132 (Apple), #133 (LLVM/Clang), #134 (gmpy2), #135 (musl), #136
  (benchmark de Tang et al.), #137 (libieeep1788), #138 (glibc) et #175
  (Cygwin), dont rien n'est envoyé ; #67 (décorations, reportée indéfiniment le
  22 octobre) ; #198 (`x / y` des intervalles NEON avec `FPCR.AH`, remarqué
  pendant la tâche PD) ; et les trois registres #177, #178 et #179. #128
  (l'annonce à Frédéric Goualard) est fermée depuis le 18 octobre ; le brouillon
  est dans Gmail. #107 (`parse_interval(nullptr)`, corrigé par #147) et #159
  (rapport d'état du 21 octobre, périmé) ont été fermées le 24 octobre, et #185
  (le masquage des exceptions autour de CORE-MATH, gardé) le 25 octobre.

## 2. Les pull requests

### 2.1 Fusionnées avant le 30 septembre

Seize pull requests : #31 (7, `atanh([1, x])`), #32 (14, `what()`), #33 (44,
CPack), #34 (Debian 12 arm64), #35 (29, locale à virgule dans la CI), #36 (9,
`tan`), #37 (1, `pow_standard()`), #39 (5, `-ffinite-math-only`), #40 (11,
lecteur et DAZ), #41 (10, `hausdorff()`, `nb_fp_numbers()`), #42 (18, lecture
sous locale à virgule), #43 (13, point écrit sous une forme relisible), #44 (42,
Python du Store), #45 (43, `VERSION.txt` avec BOM), #46 (15, `operator>>` sur
une ligne vide), #47 (24, `is_empty()` et exceptions flottantes). Puis #48 (CI
Windows, MinGW-w64 et `libstdc++-6.dll`).

La PR #29 (`configure-clean` → `MATH-CORE`) a été fermée sans fusion le 2
octobre. Chemin décidé le 3 octobre (point 34) : une fois `configure-clean`
fini, la fusionner dans `MATH-CORE`, puis `MATH-CORE` dans `master`, puis
étiqueter `v5.0.0`.

### 2.2 Le travail du 30 septembre

Chaque branche est faite à partir de `configure-clean`, validée en local (builds
SSE2, FPU et Clang 18, tests de non-régression dont on a montré qu'ils échouent
sans la correction), relue par un relecteur indépendant dont les points
bloquants sont corrigés, puis poussée. Les commits sont au nom de Jordan08, font
une ligne, et n'ont pas de `Co-Authored-By`.

| Branche | Point | Ce qui est fait | État |
| --- | --- | --- | --- |
| `configure-clean` (6 commits directs) | 39, 42, 43 | `examples.md` ne dit plus que tous les exemples du manuel sont justes (Goldstein-Price) ; étape de CI qui lance `meson setup` sans Python dans le `PATH` ; `VERSION.txt` lu avec les mêmes règles par CMake, meson, autoconf et configure (blancs, octet NUL refusé, UTF-16 nommé, CMake 3.14 à 3.16) ; `gaol.pc.in` ; `.claude/CLAUDE.md` | poussé |
| `todo-24b-quiet-empty-operands` | 24 (suite) | plus aucune des 150 opérations de l'interface ne lève FE_INVALID sur un opérande vide ; tests en processus fils sous `feenableexcept(FE_INVALID)` | **#50, fusionnée** |
| `todo-06b-mingw-msvcrt-refused` | 6 (suite) | sur x86-64, refus de tout mingw-w64 avant 12 ou lié à msvcrt (`!defined(_UCRT)`), avec la vraie raison (`fma()` et `round()`) partout ; job de CI MSYS2 MINGW64 refusé ; test de `fma()` et `round()` à l'exécution | **#51, fusionnée** |
| `todo-39-goldstein-price` | 39 | exemple 16 au style du fichier, temps en ms, provenance des valeurs (sympy, mpmath) | **#53, fusionnée** |
| `todo-06-fegetround-mxcsr` | 6 | relu ; **`cbrt.c` corrigé** : sous MinGW-w64 x64, `get_rounding_mode()` rendait 0x800 au lieu de 0 à 3, et `nth_root(x, 3)` ne contenait pas la racine cubique en sept cas, y compris avec les toolchains acceptées ; `configure-clean` (avec #51) fusionné dedans | **#54, fusionnée** |
| `todo-40-small-errors` | 40 | commits de l'agent interrompu vérifiés et complétés ; `GAOL_NODISCARD` prend `[[nodiscard]]` avant C++17 avec GCC 7+ et Visual C++ 16.4+ ; `chi()` du vide écrit `nan` ; commentaire `jail_parser.h` | **#55, fusionnée** |
| `todo-31-upstream-patches` | 31 | `3rd/README.md` : six correctifs contre le master de CORE-MATH, vérifiés ; la glibc n'a aucun de ces défauts ; rien n'est envoyé | **#56, fusionnée** |
| `todo-16-display-formats` | 16 et 13 | formats largeur et centre (milieu et rayon d'IEEE 1788), `[empty]`, `intervalToText` littéral ; un intervalle ponctuel s'écrit `[a]` (un zéro `[0]`) ; sous une locale à virgule, un point garde ses deux bornes ; en Debug, la partie aléatoire de son test est réduite (elle dépassait 300 s sous Visual C++) | **#57, fusionnée** |
| `todo-status` | — | `TODO.md` à jour (avancement, décisions, points 45 à 48, 172 questions ouvertes des PR #31 à #57), rapports dans `todo-notes/2026-09-30/` | poussé ; ramené le 4 octobre dans `todo-notes/` de `configure-clean` (son `TODO.md` est `todo-notes/TODO-todo-status.md` du commit 16a2f60), branche supprimée |

`master` a reçu `.claude/CLAUDE.md` : commits au nom de Jordan08, messages d'une
ou deux lignes, jamais de `Co-Authored-By`.

Le soir du 30 septembre, tous les commits de la journée ont été remis au nom de
Jordan08 (auteur et committer), sans changer leur contenu, et les branches ont
été repoussées en force. Les anciennes et nouvelles empreintes sont dans
[`todo-notes/2026-09-30/hashes.txt`](https://github.com/Jordan08/GAOL/blob/16a2f60/todo-notes/2026-09-30/hashes.txt)
(commit 16a2f60). La PR #52, ouverte par erreur de la branche de la session vers
`master`, est fermée.

### 2.3 Fusionnées depuis le 30 septembre

Une pull request par ligne, dans l'ordre où l'issue #49 les a tenues. Les points
(lettres et numéros) sont ceux de la section 4 et 5.

- #50
- #51 et #53 à #57 (travail du 30 septembre)
- #58 (CI après #50 et #57)
- #59 (clang-cl)
- #60 (armhf)
- #61 (`pow(x, n)` pour n grand)
- #62 (flush-to-zero et denormals-are-zero)
- #63 (`pow(x, y)` exact aux coins)
- #71 (point C : `operator<<` 30 à 45 % plus rapide, et les formats d'affichage
  décidés le 3 octobre)
- #72 et #73 (point K : `GAOL_INFINITY` est
  `std::numeric_limits<double>::infinity()`, clang-cl refusé sans `/fp:strict`,
  clang-cl de Visual Studio 2026 x86, x64 et arm64 dans la CI)
- #74 (point L : la locale à virgule vérifiée dans chaque job, `push` sur les
  branches principales seulement, Ubuntu 24.04 pour 22.04, badges)
- #75 (`make distclean` rend les sources telles que git les a)
- #76 (point J : `gaol::sin(0.5)` compile, en-têtes publics nettoyés, macros
  préfixées `GAOL_`, `-Wall -Wextra -Werror` dans la CI)
- #77 (les constantes `double` de `gaol_port.h` retirées de l'interface)
- #78 (plus de `-lm` explicite : le compilateur C++ lie la bibliothèque
  mathématique que GAOL appelle toujours)
- #79 (`TODO.md`, `process.md` et `todo-notes/` hors de l'archive des sources de
  CPack)
- #81 (les outils qui ont vérifié `pow` au point 1, nettoyés, dans
  `tests/tools/pow/`)
- #83 (point D, anciens 24 et 62 : plus aucune exception flottante sur les 7 746
  cas choisis pour en lever, NaN donnés par le programme, `0 × oo`, `pow` à
  exposant extrême, `div_rel`, boucles vectorisées, `midpoint()` de `[DBL_MAX]`,
  `GAOL_PRESERVE_ROUNDING` ; et leur documentation)
- #84 (point D, ancien 21 : un entier de tout type, sauf `bool`, pris tel quel
  par les constructeurs, `=`, `&=`, `|=`, l'arithmétique, les relations,
  `set_contains()`, `expression(n)`, `numsToInterval()` et `isMember()`, le plus
  petit intervalle qui le contient au-delà de 2^53 ; `gaol::pow(x, n)` et
  `pown(x, n)` pour tout entier ; les fonctions internes dans `gaol_detail`
  plutôt que `gaol_core::detail`)
- #85 (point D, suite du 21 : `nth_root(x, q)`, `rootn(x, q)`, `pownRev(c, x,
  p)` et les puissances d'expressions pour tout entier, les racines d'ordre
  au-delà des `unsigned` encadrées par `pow`)
- #86 (point H, anciens 23, 36 et 48 : `cancel_minus` et `cancel_plus` exacts
  quand GCC à `-O3` déplaçait les termes de leur TwoSum,
  `gaol::restore_rounding()` appelable autant de fois que voulu, la garde
  `gaol_core::rounding_guard` qui protège le lecteur, `operator<<` et
  `intervalToText()` d'un `std::bad_alloc` qui laissait l'arrondi au plus
  proche, et la section « What the upward rounding does to the program » de
  `doc/using.md` achevée : le coût de `GAOL_PRESERVE_ROUNDING` mesuré et les
  mesures TwoSum et TwoProd en arrondi dirigé dans l'exemple 13 ; le 27 par #88
  et la partie manuel, au point I)
- #87 (point D, les en-têtes : `GAOL_PUBLIC`, vide sous Visual C++, au lieu de
  `__GAOL_PUBLIC__` ; `gaol` nomme un par un les noms de `gaol_core` : ceux de
  GAOL 4, dont `NaN_val`, et les nouvelles fonctions sur les intervalles,
  `restore_rounding()` et `exact_string()`, pas les internes ;
  `gaol_allocator.h` supprimé ; plus de cast à la C dans les en-têtes installés)
- #88 (point H, ancien 27 : AVX-512 embedded rounding for +, -, *, /, sqrt)
- #89 (point M : commentaires de `tan`)
- #90 (point S : `what()` donne le nom de la classe sans explication, un
  pointeur nul n'est pas une explication)
- #91 (point G : `-ffinite-math-only` refusé, avec un remède dans les deux
  messages ; tests `refused_*` dans les trois builds ; `-Ofast` sous
  `-fno-fast-math` documenté ; suivi de c7915a7 et ccf694a, entrés sans pull
  request : le remède est de redonner tous les drapeaux de GAOL, Clang coupant
  `-frounding-math` sous `-ffast-math` et `-Ofast`)
- #92 (point R : `hausdorff()` rend +oo avant de vérifier le sens d'arrondi
  quand une borne n'est infinie que d'un côté, 2,5 à 4,4 fois plus vite dans ce
  cas ; `atanh_rel` comparé à ses bornes infinies)
- #94 (point N : `cpack_stale_configure` passe avec ccache dans `CC` et un Ninja
  hors du `PATH`, ce que vérifie un job de `linux.yml`, et se dit ignoré sans
  `sh` ou sans liens symboliques ; `package_source` reconfigure après un
  changement de version de `VERSION.txt`, avec les générateurs Makefile aussi ;
  `CPACK_VERBATIM_VARIABLES`, sans lequel CMake 3 avertissait et CMake 4
  relisait `CPACK_SOURCE_IGNORE_FILES` sans ses barres obliques inverses)
- #93 (point T : le maximum de Goldstein-Price, qu'aucun double ne contient, lu
  avec `textToInterval` dans les exemples 03 et 06, qui vérifient l'image réelle
  ; l'exemple 16 garde la mise en page de GAOL 4 dans `main()`, ses constantes
  suivant le style des autres exemples ; `examples/CMakeLists.txt` dit que les
  trois builds compilent les seize exemples ; les phrases d'`examples.md` qui
  donnaient comme ouverts les 29 formatting slips, `chi([0,0])`, les 400 bits et
  `[[nodiscard]]` en C++17 seulement marquent ce qui est corrigé))
- #96 (point V : `gaol::lexicographic_less`, un foncteur d'ordre total strict
  pour les conteneurs de la bibliothèque standard, l'ensemble vide d'abord puis
  les intervalles par leurs bornes ; `<` est `strictPrecedes` d'IEEE 1788 et
  n'en est pas un : `std::sort` avec `<` lit au-delà de la fin d'un vecteur
  contenant un intervalle vide, vérifié sous AddressSanitizer, et un `std::set`
  avec `<` perd les intervalles qui se chevauchent ; la spécialisation de
  `std::less` n'est pas faite, le mainteneur ayant décidé de ne pas la faire
  (#70, fermée le 19 octobre), et la mise en garde est venue avec le point I,
  par #140)
- #95 (point O : GAOL sous-projet meson, avec `add_project_arguments` et les
  drapeaux de `gaol.pc` dans `gaol_dep`, vérifié par `tests/meson_subproject` ;
  le Release par défaut dans les trois builds, aussi en sous-projet, CORE-MATH
  n'étant pas optimisé par CMake sans type de build ; `enable-debug` et
  `GAOL_DEBUG` ; la macro `GAOL_DEBUG(lvl, cmd)` renommée `GAOL_DEBUG_VERBOSE` ;
  `pipefail` dans les workflows ; le Python du Microsoft Store, la raison de
  C++17 des tests, les `open()` de `meson.build`)
- #97 (point U : les tests de compilation qui doivent échouer, `refused_*` et
  `nodiscard_discard_*`, compilent une copie de leur source refaite et touchée
  avant le build, et ne restent plus en échec après un en-tête ou une source
  rétablis avec leur ancienne date ; `test_input()` ne cache plus l'exception de
  `<-inf,-inf>` et ses six vérifications suivantes tournent ;
  `manual/check_examples.py` vérifie que les 88 exemples du manuel impriment ce
  qu'il montre, dans un job de `linux.yml` ; la sortie hexadécimale de
  `interval(0.1)` du manuel ; le commentaire du format hexadécimal ;
  `doc/tests.md` à 80 colonnes ; et, décidé pendant le travail, les tests et les
  exemples compilent et passent sans exceptions, vérifiés par deux jobs de
  `build-systems.yml`)
- #100 (point Q, anciens 4 et 45 : sous denormals-are-zero, les fonctions qui
  comparent des bornes sans calculer, le constructeur à deux bornes, les
  relations, `&`, `|`, `max`, `min`, `abs`, `sign`, `mig`, `mag`, les arrondis à
  un entier, `is_canonical`, `feven`, `nb_fp_numbers`, `inf` et `sup`, comparent
  les bornes par leurs bits là où elles se comparent égales et arrondissent un
  sous-normal d'après ses bits ; l'écriture, `midpoint()`, `split()` et `chi()`
  effacent les modes ; le chemin AVX-512, dont l'arrondi porté applique FTZ,
  calculait `x - y` après avoir rétabli FTZ avec Clang 18 et
  `GAOL_PRESERVE_ROUNDING` ; une borne sous-normale écrite vers l'extérieur là
  où GAOL ne peut pas effacer les modes, Visual C++ pour ARM64 ; le coût de la
  sonde mesuré sur 68 jobs de la CI, aussi long avec un sous-normal)
- #99 (point P, anciens 25 et 30 : les outils `width_enclosure()`, `inflate()`,
  `interval::midrad()`, `bisect(ratio)`, `is_bisectable()`, `hull()`,
  `intersect()` et `gaol_ieee1788::mulRevToPair` ; le littéral `_iv` de
  `gaol/gaol_literals.h` ; `erf` et `erfc` ; les réciproques d'`atan2`
  (`atan2Rev1`, `atan2Rev2`), de `pow(x, y)` (`powRev1`, `powRev2`), de `max`,
  `min`, `sign` et `floor` ; ITF1788 dans `doc/compare/`, 7 202 des 9 542 cas
  réussis sans aucun bogue, le banc d'essai de Tang et al. (2021),
  Boost.Interval et un test d'encadrement ; et, trouvés par la CI ou la
  relecture : `gaol_ldexp()` pour CORE-MATH, les `ldexp()` de l'UCRT et de macOS
  étant faux dans les arrondis dirigés, les drapeaux de MXCSR sous Windows 32
  bits, `read_in_branch()` contre les opérations calculées avant leur test par
  Clang et AppleClang pour ARM 64 bits, les tables de `tests/reverse.cpp` en
  données `constexpr` pour GCC 12 sur armhf)
- #103 (`.continue/rules` retiré)
- #105 (point E, anciens 12, 40 et 45 ; #98 : les longues sommes ne débordent
  plus la pile, chaque opération calculée dès sa lecture, l'évaluation et la
  destruction des expressions par des boucles, l'imbrication bornée à 10 000
  entrées ; un `std::string` lu en entier, un NUL n'étant le caractère d'aucun
  lexème, et une chaîne de plus de 2^32 caractères plus lue comme ses premiers ;
  `std::bad_alloc`, et tout libéré, quand flex, bison, le lexeur ou une action
  ne peuvent pas allouer, flex appelant `exit(2)` ; sans exceptions, un appel
  faux fait échouer la lecture ; `<a, b>` comparé par les bits sous
  denormals-are-zero ; le Doxygen de `parse_interval()`)
- #108 (le parser régénéré avec bison 3.8.2, celui d'Ubuntu 26.04)
- #109 (`process.md` : l'arbre GAOL_V9, les outils d'Ubuntu 26.04, l'état au 12
  octobre)
- #110 (un job de la CI, `meson-oldest` de `build-systems.yml`, pour meson
  0.53.2, la plus ancienne version que prend le build meson)
- #111 (point A, anciens 31, 6, 56, 3 et 47 : le correctif 6 de CORE-MATH en
  entier dans `cbrt.c`, `rsqrt.c` et `asinpi.c`, qui rend Cygwin juste, et un
  job de la CI qui donne à CORE-MATH les `FE_*` de Cygwin ; les masques de
  `sinpi.c` et `log1p.c` ; `GAOL_RND_MINGW_FENV_ONLY` supprimé ; mingw-w64 pour
  ARM refusé avant 12 ou sans `_UCRT` ; la racine carrée de SSE2 sur tout x86 32
  bits ; les commentaires du sens d'arrondi ; dans `3rd/README.md`, le correctif
  1 étendu à `expm1q.c` et `atan2l.c`, le 6 à `powl.c`, le 4 fait en amont)
- #112 (`process.md` : `configure` régénéré par `autoconf` seul quand seul
  `configure.ac` change ; `check_branch` prend l'arbre courant)
- #113 (chaque ligne changée de CORE-MATH marquée `/* GAOL */`, sans changer un
  octet du code compilé)
- #114 (point F : les noms de fonctions lus quelle que soit la locale, pliés en
  ASCII ; les tests de `numbers` sous les modes qui mettent les sous-normaux à
  zéro aussi sur ARM, vérifiés une fois par #116, jetable ; les textes
  d'`operator>>`, `\option{-Ofast}`, les commentaires du lexeur)
- #115 (`process.md` : `GAOL_V8` et `GAOL_V9` se valent, le mainteneur ouvre les
  pull requests quand le jeton de `gh` ne le peut pas)
- #117 (point B, première des trois pull requests : une seule vérification du
  sens d'arrondi par appel des puissances entières de `pow`, de `nth_root(x, q)`
  pour q < 0, d'`acosh_rel()`, `asinh_rel()`, `atanh_rel()`, `nth_root_rel()` et
  de `modulo_k_pi()` ; |q| sans dépassement dans `nth_root(I, INT_MIN)` ; la
  garde de `[±oo]` de `gaol_pow_hybrid()` gardée, avec un test SSE2)
- #118 (point B, deuxième des trois pull requests : `pow(x, y)` aux coins de
  toute boîte, avec les limites 0 et +∞ là où la base est 0 ou +∞ ou l'exposant
  infini, au lieu d'exp(y log x), les bornes les plus étroites partout et une
  seule vérification du sens d'arrondi ; `gaol_ieee1788::pow` vide pour un
  exposant `[±oo]` ; `nth_root(I, long long)` et `nth_root(I, unsigned long
  long)` ; `GAOL_RND_KEEP()` d'un intervalle par ses deux bornes ;
  `fast_modulo_k_pi()` retirée ; `doc/compare/performance.md` relancé en entier
  sous Ubuntu 26.04.1)
- #119 (suites de l'audit du 13 octobre : les commentaires du chemin AVX-512,
  l'ancien 9 dit mesuré dans le commentaire de `cos_or_sin()` et
  `doc/accuracy.md`, `<a, b>` comparé par ses bits dans `doc/using.md`, le
  manuel et `doc/tests.md`, l'état de `process.md`)
- #120 (point Z : les options de `gaol/gaol_configuration.h` et les macros de la
  cible toujours définies, à 0 ou 1, dans les trois builds, et `-Wundef` dans
  `headers.sh` ; `bisect(ratio)` par les `quiet_*` de `gaol_detail`)
- #121 (point B, troisième des trois pull requests : `pown` aux bornes les plus
  serrées de tout signe dans les `int`, les deux coins à la fois par des
  produits exacts et une borne d'erreur prouvée d'avance, x^-n par l'inverse
  corrigé, x^2 égal à `sqr(x)`, x^3 en 4,9 ns contre 8,8 avec GAOL 4.2.3 ; le
  `pow` de CORE-MATH au-delà des `int` (`gaol_pown_ll()`), et pour les
  expressions au-delà de 2^53 ; les deux espaces de noms, `gaol` et
  `gaol_ieee1788`, dans la documentation ; `doc/compare`, ITF1788 et tang2021
  relancés avec Clang 21 sous `gaol_ieee1788`, avec deux défauts de gmpy2 2.2.1
  contournés ; plus d'exception de division par zéro dans la division et
  l'inverse SSE2 d'un diviseur à borne nulle, ni dans l'inverse FPU sous
  AppleClang)
- #123 (les tests compilés avec `-Wno-psabi` là où le compilateur l'accepte,
  dans les trois builds, les trois projets de test et `.github/scripts/tests.sh`
  : plus de notes de GCC sur le passage des arguments dans les journaux de la CI
  d'armhf, d'aarch64 et de ppc64le ; ni la bibliothèque, ni `gaol.pc`, ni les
  exemples ne reçoivent l'option)
- #122 (point AA : Shekel 5 en 263 ns au lieu de 358 dans le banc de
  `doc/compare` sur le Xeon E3-1220 v6, contre 274 avec GAOL 4.2.3, sans qu'une
  borne change ; `sqr()` des intervalles SSE2 d'un seul produit, les `uipow_*`
  retirés ; `+`, `-`, `/` et les opérateurs avec un double vers le chemin
  AVX-512 en appel terminal ; ce chemin décidé à la configuration
  (`GAOL_AVX512_ARITHMETIC`), jamais en compilation croisée ni avec GCC pour
  Windows, `gaol_core::avx512_arithmetic` retiré ; `gaol::init()` arrête avec un
  message un programme lancé sans AVX-512 ; `enable-prefer-avx512` de meson en
  `combo` `auto`/`true`/`false`)
- #124 (point ZZ, la vitesse de `+ - * /` et `sqr()`, et point AG : Shekel 5 en
  108 ns contre 189 avec GAOL 4.2.3 dans le banc de `doc/compare` sur
  l'i7-1185G7 ; les opérateurs des intervalles SSE2 par des fonctions de la
  bibliothèque qui prennent les bornes dans un registre, `*`, `/` et `sqr()`
  sans `switch` sur les signes ; le chemin AVX-512 pour `sqr()`, les puissances
  entières et `sqrt()` ; `GAOL_RND_ENTER()` qui n'écrit que MXCSR avec
  `GAOL_PRESERVE_ROUNDING` sur x86-64 avec FMA ; dix lignes de `+ - * /` au banc
  et une colonne « rounding direction preserved » ; les avertissements de la CI
  corrigés ; le point AI de `TODO.md`)
- #139 (`atanh([1])` et `atanh([-1])` dans les cas particuliers de
  `doc/compare/special_cases.md`, reste de l'ancien 7 : 290 cas au lieu de 288,
  les anciens n° 126 à 288 devenus 128 à 290 dans la table, le texte et les
  décomptes ; GAOL v5 sous les deux jeux de noms et libieeep1788 rendent ∅,
  filib++ [MAX, +oo] et [-oo, -MAX], Boost.Interval [+oo] et [-oo], PROFIL/BIAS
  abandonne)
- #140 (points I et AF : un premier programme dans `README.md` et
  `examples/tutorial.md` (quatre programmes) ; dans le manuel (« Common errors
  », onze sous-sections, huit exemples de plus, 111 vérifiés par
  `check_examples.py`) et dans `doc/using.md`, les pièges, la table des noms
  d'IBEX, Codac, C-XSC et Boost.Interval avec le sens de `<`, `<=` et `==`, la
  mise en garde contre `<` pour `std::sort`, `std::set` et `std::max` avec
  `gaol::lexicographic_less`, les opérations qui laissent le sens d'arrondi tel
  qu'elles l'ont trouvé, « What the upward rounding does to the program » et
  `restore_rounding()` dans le manuel, le coût de `GAOL_PRESERVE_ROUNDING`
  remesuré (1,3 à 1,7 fois avec le chemin AVX-512, 3,7 à 8 fois sans lui) ; les
  textes qui contredisaient le code (96 à 103) corrigés, le 101 l'étant déjà ;
  les commentaires `inverse()` résolus)
- #141 (la macro `GAOL_USING_ASM`, que plus aucun code ne lisait depuis la
  suppression de `f_negate()` par a6786af, et ses options `GAOL_ASM`,
  `--enable-asm` et `enable-asm` supprimées, avec
  `GAOL_HAVE_ROUNDING_MATH_OPTION`, que rien ne lisait non plus, et les
  commentaires qui citaient `gaol_opposite()` ; les objets de la bibliothèque
  sont les mêmes avant et après, 137 jobs verts)
- #143 (point AO, ancien 127 : la phrase « les macros que lisent les sources de
  GAOL, et rien d'autre » dit maintenant lesquelles des macros de la
  configuration les sources ne lisent pas ; les macros restent, décidé le 19
  octobre)
- #144 (point AB, ancien 77 : `pow` d'une puissance proche du plus grand double,
  y·log(x) de 709,78263 à 709,78267, ne lève plus l'exception d'opération
  invalide que la première phase du `pow` de CORE-MATH levait en calculant ∞ − ∞
  ; `RHO2` de `exp_1()` abaissé au plus grand double tel que rh·`INVLOG2` ≤
  4194303,5 − 2^−31, une ligne marquée `/* GAOL */`, décrite dans
  `3rd/README.md` comme correctif 8 à proposer en amont ; `core_math` et
  `rounding_direction` échouent 28 et 20 fois sans la correction, 137 jobs
  verts)
- #145 (point AE, ancien 90 : les deux vérifications qui refusent Clang pour ARM
  32 bits et un compilateur qui dit de `-frounding-math` « overriding currently
  unsupported rounding mode on this target » portent sur le compilateur C, qui
  compile CORE-MATH, comme sur le compilateur C++, dans les trois builds :
  `configure.ac` (macro `GAOL_REFUSE_COMPILER`) et `configure`, `meson.build`,
  `CMakeLists.txt` ; `configure` garde les `CFLAGS` et `CXXFLAGS` de
  l'utilisateur pour ces tests, comme CMake et meson ; les jobs `clang-arm32` de
  `containers.yml` et `clang-14-arm64` de `linux.yml` vérifient les neuf refus,
  le C seul, le C++ seul et les deux, par les trois builds ; 137 jobs verts)
- #146 (points AI.106 et AI.107 : des intervalles NEON pour arm64, sur le modèle
  des SSE2 (`gaol/core/gaol_interval_neon.h` et `.cpp`), choisis par
  `GAOL_ARM64` (CMake), `--enable-arm64` (configure), `enable-arm64` (meson) et
  la macro `GAOL_USING_NEON_INSTRUCTIONS`, indépendants de `GAOL_SIMD` ; le
  registre FPCR lu par `mrs` et écrit par `msr` seulement s'il change,
  `GAOL_PRESERVE_ROUNDING` de 39 à 27 ns par opération sur un M2, `x - y`, `x *
  y`, `x / y`, `sqr` et `sqrt` de 1,3 à 4 fois plus rapides ;
  `tests/arithmetic.cpp` (environ 1,5 million de vérifications contre une
  référence en doubles du processeur), l'outil `tests/tools/arith`,
  `check-intervals.sh` et des jobs arm64 qui gardent le code FPU couvert ; plus
  de 40 mutations tuées, trois survivantes expliquées)
- #147 (point AD, ancien 114 : `parse_interval(nullptr)` rend `false` au lieu de
  lire le pointeur nul avec `strlen()` et de planter, sans exception et sans
  toucher à l'intervalle, avec les deux jeux de noms ; une note dans le
  commentaire Doxygen de `gaol_parser.h` ; `null_pointer()` de
  `tests/expressions.cpp`, qui meurt sur SIGSEGV sans la correction ; 145
  contrôles verts)
- #148 (point AI, ancien 106 pour Visual C++ et clang-cl x64 : les intervalles
  SSE2 par défaut avec `cl` et clang-cl (`CMakeLists.txt`, `meson.build`,
  `gaol_interval_sse.cpp` qui compile avec Visual C++ : plus de masque
  `m128d_02mask` ni `m128d_20mask`, plus d'`asm` ni d'attribut), les noyaux
  `add_bounds()` et les autres en `__vectorcall` (`GAOL_VECTORCALL`) ; le chemin
  AVX-512 de `GAOL_PREFER_AVX512` avec `cl` (`cpu_runs_avx512f()` par `cpuid` et
  `xgetbv`, l'ordre des opérations tenu par des accès `volatile` et des appels,
  faute d'`asm`) et avec clang-cl ; les défauts propres à `cl` que la CI et
  Compiler Explorer ont montrés, corrigés (`mid()` d'une borne énorme,
  `_mm_storeh_pd()` mal compilé qui vidait `pow`, l'ordre de l'AVX-512 avec
  `GAOL_PRESERVE_ROUNDING`, `/W4`, le calcul de référence d'un test de
  `div_rel`) ; cinq jobs de `windows.yml` exécutent le chemin AVX-512 sous Intel
  SDE, les exécuteurs n'ayant pas AVX-512 (`.github/scripts/sde-tests.sh`, 26
  programmes de test chacun) ; dans la CI, la ligne de 13 opérations de 100 à
  168 ns à 37-49 ns ; 157 contrôles verts)
- #149 (points AD.83 et AD.84 : `[l, u]` est vide quand l est au-dessus de u,
  aussi pour des nombres qui ne sont pas des doubles et qui tombent entre les
  mêmes deux doubles, comparés exactement dans les quatre bases et derrière des
  signes (`[9007199254740993, 9007199254740992]` donnait [2^53],
  `[1.00000000000000001, 1]` donnait [1]) ; pi contre un autre nombre, un nombre
  loin au-delà des doubles et les bornes calculées restent non départagés (le
  littéral est alors l'intervalle des doubles autour) ; `<a, b>` refuse une
  borne qui n'est pas un double (`<0.1, 0x1.9999999999999p-4>` donnait
  [0x1.9999999999999p-4]) ; le second argument de `nth_root`, `pown` et `rootn`
  lu dans un texte peut être un entier de tout double, un `long long` saturé à
  ±(2^63 − 2) ; `gaol_parser.h`, le manuel, `doc/accuracy.md`, `doc/tests.md` et
  `doc/using.md` à jour ; `expressions` échoue 7 201 fois sur 18 187
  vérifications sans la correction ; `compare_numbers()` conforme à
  l'arithmétique rationnelle sur 120 000 couples, et sur environ 500 000 pour
  une seconde relecture ; quatre jobs de Windows ont échoué un temps (un
  `std::string` du contexte de la lecture, que le compteur d'allocations du test
  ne voit pas quand la bibliothèque C++ est dans une DLL), puis 146 contrôles
  verts)
- #150 (point AI.106 pour ARM 32 bits avec GCC, en partie :
  `GAOL_PRESERVE_ROUNDING` lit FPSCR par `vmrs` et l'écrit par `vmsr` seulement
  s'il change, au lieu de `fegetround()` et `fesetround()`
  (`GAOL_RND_FPSCR_DIRECT`, macro interne documentée ; `GAOL_RND_DIRECT`
  généralise le code de #146 à FPCR et à FPSCR) : dans la CI, sur Debian 13
  armhf, `x + y` de 48,0 à 22,5 ns, `x * y` de 57,6 à 33,6 et la ligne de 13 `+ - *
  /` de 668 à 349 ns, sans effet hors de la préservation ; le job
  `control-register-performance` de `containers.yml` et
  `.github/scripts/performance-ab.sh` et `compare-performance.py`, qui mesurent
  deux builds alternés ; les tests de FPCR portés à FPSCR (DN, bits de piège,
  opérations à résultat exact, la séquence où le programme écrit le registre,
  `cancel_minus` et `cancel_plus`) ; et, trouvé à la relecture, un défaut qui
  touchait aussi arm64 depuis #146 : avec GCC (bogue 34678) et avec Clang sous
  Linux, `width()` d'un intervalle construit dans la fonction et `bisect(ratio)`
  calculaient avant l'écriture du registre de contrôle, donc dans le sens du
  programme (la largeur de `[1, 2^53 + 2]` donnait 2^53), corrigés par
  `rnd_reread()`, le test échouant sans la correction sur les jobs arm64 GCC et
  Clang ; trois relecteurs indépendants, et l'assembleur de la bibliothèque
  entière parcouru avec les GCC croisés pour AArch64 et ARM 32 bits ; 147
  contrôles verts)
- #151 (points AB 78 et 80 : `cancel_minus` et `cancel_plus` ne lèvent plus
  l'exception d'opération invalide près du plus grand double. Une différence de
  bornes au-delà de DBL_MAX donnait une somme infinie, dont le TwoSum calculait
  inf − inf (78) ; la relecture a trouvé le même défaut avec une somme finie à
  mi-chemin de deux doubles, arrondie de 2^970 en s'éloignant (`s - b` valait
  DBL_MAX + 2^970, infini), qui donnait NaN et `[-oo, +oo]` pour deux boîtes de
  largeurs égales. `two_sum()` : e = 0 pour une somme infinie, puis le Fast2Sum
  de Dekker avec l'opérande de plus grand module en premier ; table de 9 cas
  dans `tests/rounding_direction.cpp`, qui échoue 19 fois sur 522 361
  vérifications avec les sources de `configure-clean` (GCC 13.3 et Clang 18) ; 2
  millions de tirages comparés à l'arithmétique entière exacte, sans borne
  fausse ni `FE_INVALID`. `1/[0, 1]` retiré de `doc/using.md` et du manuel, qui
  le donnaient en exemple de division par zéro (80) ; 146 jobs verts)
- #152 (point AI, anciens 106 et 108 : les intervalles SSE2 sur Windows 32 bits
  avec GCC et Clang (MinGW-w64, MSYS2), que les trois builds écartaient parce
  qu'un `std::vector` d'intervalles y plantait au premier `movaps` (le runtime C
  de Windows 32 bits aligne `new` sur 8 octets, GCC croit à 16) :
  `-faligned-new=8` donné à la bibliothèque et au code qui l'utilise
  (`gaol::gaol`, `gaol.pc`, `gaol_dep`), et `gaol/gaol_config.h` qui refuse les
  intervalles SSE2 sur Windows 32 bits sans `__cpp_aligned_new` ; un compilateur
  qui ne prend pas l'option, et Clang avec son pilote GNU pour une cible de
  Visual C++, gardent les intervalles FPU ; Visual C++ et clang-cl y gardent les
  intervalles FPU (leur `/Zc:alignedNew` ne suffit pas, seul C++17 allume le
  `new` aligné), décidé le 20 octobre ; `test_heap_alignment` de
  `tests/misc.cpp`, qui échoue au lieu de planter sans le drapeau, et trois
  tests de refus ; les jobs MinGW-w64 x86 (12.2, 13.2, 14.2 et 15.2) passent
  avec le drapeau ; la vitesse n'est pas mesurée)
- #154 (point AN, ancien 126, issue #101 : les pistes SIMD et FMA évaluées dans
  `doc/simd.md`, sans changer un calcul : ce que GAOL calcule avec des vecteurs
  par build, où va le temps d'un programme (sur les six exemples qui résolvent
  quelque chose, l'arithmétique pèse 28 % du temps, les fonctions élémentaires 4
  % et le code hors de GAOL 62 %), le sens d'arrondi porté par l'instruction
  processeur par processeur (seuls x86 avec AVX-512 et RISC-V l'ont), ce que
  donnent les compilateurs (`#pragma STDC FENV_ROUND` est ignoré par GCC et
  Visual C++ et « non supporté » par Clang, sans erreur), un verdict par piste ;
  le `// TODO: Rewrite as many functions as possible to use SIMD instructions`
  de GAOL 4 dans `gaol/core/gaol_interval_core.cpp` remplacé par ce que l'étude
  trouve ; deux relectures indépendantes, leurs erreurs corrigées)
- #155 (point AL, ancien 118 : `process.md` mis à jour. Au §6.1, le paragraphe
  du 12 octobre sur une machine Ubuntu 24.04 est retiré, « Vérifie donc les
  versions au début du travail » est gardé ; au §7.3, `git switch --no-track -c`
  (des commits de branche étaient arrivés directement dans `configure-clean` le
  6 octobre, c7915a7 et ccf694a) ; au §6.2 et dans `HOUSE_RULES.md`,
  `-DGAOL_ARM64=OFF` avec `-DGAOL_SIMD=OFF` pour la variante FPU, qui prend
  sinon NEON sur arm64 ; l'en-tête devient un renvoi au §12, refait au 20
  octobre avec les commandes qui montrent les pull requests ouvertes et les
  branches distantes. Au-delà du point, les §2 et §9 sont mis en accord avec
  `TODO.md` et les issues #49 et #142 tels qu'ils sont tenus depuis le 18
  octobre (la renumérotation, les sections « Remarqué par #NN, à trier », le
  repli par l'API REST quand `gh pr edit` échoue). Relecture indépendante sans
  point bloquant, ses remarques reprises ; 145 jobs verts)
- #156 (point AI, ancien 108 : x86 32 bits, les registres de contrôle x87 et SSE
  lus en ligne, sans `fegetround()`, qui coûtait environ 80 ns avec Visual C++
  pour x86. `fnstcw` et `stmxcsr` dans une seule instruction ou un seul bloc
  `__asm` (`gaol_read_x86_control()`), sur tous les processeurs x86 32 bits ;
  `round_upward_if_needed()` compare l'état lu et n'écrit que le registre qui
  n'est pas vers le haut, MXCSR une seule fois pour le sens et les modes ; avec
  `GAOL_PRESERVE_ROUNDING`, une opération lit les registres une fois et ne remet
  que ce qui diffère. Les décisions sont des fonctions de `gaol_core` qui ne
  touchent pas au processeur, vérifiées sur des registres simulés
  (`tests/rounding_direction.cpp`, 31 744 vérifications, qui tournent aussi sur
  arm64) : 46 mutations de `gaol/gaol_fpu.h`, toutes tuées. Un job de plus
  (Visual Studio 2026 x86 Release, direction préservée). Aucun temps n'est
  promis, la machine de travail n'ayant ni Windows ni x86 : le gain se lit dans
  la CI. Un relecteur indépendant, sans point bloquant, ses remarques reprises ;
  147 jobs verts)
- #158 (point AC, ancien 81 : `nth_root(x, n)` et `rootn(x, q)` d'un x sous
  2^-1020, sous-normaux compris, ont des bornes à 2 doubles au plus des plus
  serrées, comme ceux des autres x ; `nth_root([2^-1074], 33)` était à 1 983
  doubles de la borne inférieure la plus serrée et à 28 785 de la supérieure
  (3,5·10^8 et 6,9·10^8 pour n = 65). Pour x < 2^-1020 (`tiny_root_argument`),
  les preuves prennent des puissances mises à l'échelle (`ipow_hi_scaled()`,
  `ipow_lo_scaled()`, `compare_scaled()`), jamais sous-normales, avec les mêmes
  produits dans le même ordre ; les recherches sont des gabarits sur le type de
  preuves, choisi une fois, si bien que les x ordinaires gardent le code
  d'avant, bit à bit. `roots_of_tiny_numbers()` de `tests/arithmetic.cpp` échoue
  17 fois sur 3 123 027 vérifications avec les sources de `configure-clean` ; 25
  440 couples (x, n) et (x, q) comparés aux bornes les plus serrées exactes,
  sans borne qui n'encadre pas. La promesse « les plus serrées ou un double
  au-delà » est corrigée dans le commentaire de `newton_root()` et dans
  `doc/accuracy.md` : 2 doubles, 4 pour `rootn(x, q)` avec q < 0, pour tout x.
  Deux relectures indépendantes, leurs remarques reprises ; 158 jobs verts)
- #157 (point AI, ancien 106, intervalles FPU : des noyaux scalaires sans
  branche sur les signes pour `x * y` (huit produits), `x / y`, `x % y` et
  `sqr(x)` (`gaol/core/gaol_interval_fpu.cpp`), et `x - y`, qui lit ses bornes
  après `GAOL_RND_ENTER()` et prend le temps de `x + y` (3,62 à 1,18 ns), pour
  les builds sans intervalles SSE2 ni NEON : ARM 32 bits, POWER, s390x, RISC-V,
  Visual C++ et clang-cl en x86 32 bits et en arm64, MinGW-w64 en arm64,
  `GAOL_SIMD=OFF` et `GAOL_ARM64=OFF`. Sur un M2, `x * y` de signes aléatoires
  de 12,4 à 3,3 ns (16 384 opérandes), `x / y` de 9,3 à 3,2, `sqr` de 5,9 à 1,8,
  Shekel 5 de 298 à 138 ns ; dans la CI, en deux builds alternés (branche
  d'expérience `exp-157-ab`, outils de #160), `x * y` de signes aléatoires de
  1,23 à 3,47 fois plus rapide sur neuf configurations. Pertes dites en clair et
  acceptées : une ligne de sept `*` de signes prévisibles prend 50 % de temps de
  plus sur le M2 (0,58 de la vitesse d'avant avec GCC sur Ubuntu et 0,57 avec
  clang-cl arm64 dans la CI), le quotient avec Visual C++ arm64 (0,82), le carré
  avec Clang sur Ubuntu et clang-cl arm64 (0,85 et 0,84), la ligne de treize
  opérations avec le sens préservé sur Visual C++ x86 (0,53), et rien de gagné
  avec `GAOL_PRESERVE_ROUNDING`. Le signe d'une borne nulle d'un produit n'est
  pas spécifié avec la FPU : la CI a trouvé que GCC 13 pour POWER9 compile le
  maximum en `xsmaxcdp`, qui rend le premier de deux zéros de signes opposés,
  d'où un test réduit à SSE2 et à NEON pour le produit. Sur 2 390 612 lignes de
  `tests/tools/arith` (23 bornes spéciales, quatre sens d'arrondi), aucune ligne
  où les bornes ou les indicateurs `FE_INVALID` et `FE_DIVBYZERO` diffèrent de
  l'ancien code, et 36 332 au signe d'un zéro près. Une relecture indépendante,
  sans point bloquant ; 159 contrôles verts)
- #160 (points AI.111 et AI.115 : le banc A/B de la CI,
  `.github/workflows/performance.yml`, qui construit et mesure dans le même job
  la base d'une pull request (le premier parent de son commit de fusion) et sa
  tête, en trois passages alternés, sur Ubuntu 24.04 (GCC, Clang, GCC avec
  `GAOL_PRESERVE_ROUNDING`), macOS 26 arm64, Visual Studio 2026 (x64, x86, x64
  avec la direction préservée) et MSYS2 (UCRT64, CLANG64), avec
  `.github/scripts/performance-ab.sh` et `compare-performance.py` généralisés de
  #150 (un tableau base / pull request / rapport dans le résumé du job, aucun
  temps ne fait échouer un job) et un test des outils de 19 tests ; `make perf`
  lance les dix lignes de `run_lines.sh` dans les trois builds et sur macOS, et
  l'outil de la CI les compare ; trois affirmations fausses trouvées par la
  relecture indépendante, corrigées ; `performance.yml` a réussi sur #160, #162,
  #163 et #164)
- #161 (point AI, ancien 112 : un second `cmake` dans le même dossier ne fait
  plus passer `GAOL_PREFER_AVX512` de ON à OFF sous `GAOL_PRESERVE_ROUNDING`
  (l'entrée garde la valeur dérivée, l'entrée interne
  `GAOL_PREFER_AVX512_DERIVED` la note, une valeur donnée reste jusqu'à
  `-UGAOL_PREFER_AVX512`) ; `tests/prefer_avx512_reconfigure.cmake` configure
  plusieurs fois le même dossier, échoue avec le `CMakeLists.txt` de
  `configure-clean`, et quatre mutants de la correction sont tués (CMake 3.14.7
  et 4.4.3, et 3.20.5 et 3.28.4 par le relecteur) ; les descriptions de l'option
  disent « +, -, *, /, sqr, sqrt and, with FMA, the integer powers beyond 1 and
  -1 » dans `CMakeLists.txt`, `meson_options.txt`, `configure.ac`, les trois
  gabarits de `gaol_configuration.h`, `doc/building.md`, `doc/three-builds.md`,
  le manuel et l'exemple 13 ; `configure` édité à la main, qu'autoconf 2.72
  régénère à l'identique (vérifié le 23 octobre) ; le test des bits du signe de
  zéro de `x * d` n'est pas fait)
- #162 (suite du point AN, d'après #154 : `pow(x, 2)` d'un intervalle passe par
  `sqr(x)` sous toutes ses formes (`pow(x, 2)`, `2u`, `2.0`, `[2]`, `gaol_pown`,
  `gaol_pown_ll`, `gaol_ieee1788::pown` et `pow`), 1,7 ns au lieu de 5,3 avec
  NEON et 2,2 au lieu de 5,2 avec la FPU sur un M2 (3,3 et 2,5 fois), l'exemple
  Goldstein-Price de 77 à 50 ms avec NEON ; `interval::sqr_upward()`, le corps
  de `sqr` sans son contrôle du sens d'arrondi, dans les trois builds,
  `pown_checked()` pour les autres puissances, et la branche K = 0 de
  `pown_bound()`, devenue inatteignable, supprimée ; 14 010 384 cas par build
  comparés bit à bit à l'ancienne bibliothèque : aucune différence de valeur, le
  signe d'un zéro de la borne basse change (c'est celui de `sqr(x)` de chaque
  build) et les indicateurs sont ceux de `sqr` ; `power_two_is_sqr()` de
  `tests/arithmetic.cpp` (6 695 700 vérifications, 615 832 échecs sans le
  routage) ; le texte de #154 (« +0 avec `pow` et −0 avec `sqr` ») était inexact
  pour NEON et FPU, `doc/simd.md` corrigé ; une relecture indépendante, sans
  point bloquant)
- #163 (point AI, ancien 110 : GCC pour Windows x86-64 reçoit `-mfma
  -mprefer-vector-width=128` dans les trois builds (`-mfma` seul laisse GCC
  vectoriser avec des registres de 256 bits sur une pile que l'ABI de Windows
  x64 n'aligne que sur 16 octets : bogue 54412 de GCC), si bien que CORE-MATH
  n'appelle plus un `fma()` logiciel ; sur MSYS2 UCRT64 (GCC 16.2), `pow(x, 3)`
  de 52,3 à 8,7 ns, `log` de 106,4 à 33,6, `cos` de 260,5 à 55,4.
  `.github/scripts/check-gcc-windows-fma.sh` fait échouer la CI si un objet a
  une instruction `%ymm` ou `%zmm` ou si les `vfmadd` manquent (0 ligne `%ymm`
  sur 79 objets dans la CI de la pull request). Quatre entrées de plus du job
  MSYS2 exécutent le chemin de `GAOL_PRESERVE_ROUNDING` avec mingw-w64 et ont
  montré deux défauts d'avant la pull request, avec mingw-w64 13 et plus sur
  x86-64 : `fesetround()` efface les six indicateurs de MXCSR (`gaol::cleanup()`
  et `restore_rounding()` aussi, même sans la préservation) et `fegetround()`
  joint par OU les champs RC des deux unités ; `gaol/gaol_fpu.h` lit et écrit
  désormais lui-même les registres (`GAOL_RND_MINGW64_X87`), et
  `tests/rounding_direction.cpp` compare l'unité x87, les indicateurs depuis les
  six directions, `cleanup()` et `restore_rounding()` (de 1 à 687 messages
  d'échec selon l'entrée sans la correction). Deux relectures indépendantes,
  dont les corrections qui les ont suivies n'ont pas été relues ; 163 contrôles
  verts)
- #164 (point C, textes, tests et ménage : la phrase « 4 doubles on the special
  cases » de `pow` supprimée de `doc/accuracy.md` (117) ; l'entrée « not tested
  » des compilateurs refusés du manuel reformulée (119) ; le piège de `namespace
  gaol { }` ajouté aux pièges de « Common errors » de `doc/using.md` et du
  manuel (120) ; un signe collé à un nombre incertain documenté, une espace
  étant nécessaire (87) ; la lecture quadratique en le nombre de chiffres
  documentée (88) ; le format `agreeing` d'un intervalle ponctuel documenté, un
  zéro s'écrivant `[0]` dans tous les formats (89) ; `tests/refused_options.sh`
  sonde le compilateur avec `-dM -E -x c++ /dev/null` et échoue au lieu de se
  dire sauté quand les en-têtes manquent (91) ; `GAOL_RND_SSE_REGISTER` ajouté
  aux `MACROS` de `.github/audit/compare.py` (94) ; `.continue/` ajouté au
  `.gitignore`, avec un commentaire (121) ; `gaol_performance` compilé en C++14
  dans les trois builds (122) ; et le C4702 de #148 : les `return`
  inatteignables après `gaol_ERROR()` retirés de `textToInterval()`, un job «
  Visual Studio 2026 x64 Debug, /W4 /WX » ajouté à `windows.yml`, `/wd4702` pour
  le parser bison dans `CMakeLists.txt` ; une relecture indépendante, sans point
  bloquant ; 173 contrôles verts)
- #165 à #167, #169 à #174 : voir le tableau de 2.5 (fusionnées les 23 et 24
  octobre)
- #176 (suite de #173, décidée le 24 octobre : le job Cygwin de `windows.yml`
  n'essaie plus les générateurs de CPack. L'étape avait servi à mesurer : CPack
  échoue avec TGZ, TBZ2 et ZIP sur le lien symbolique `README`, ce qui est écrit
  dans `doc/building.md`, `doc/three-builds.md` et l'issue #175 ; le test
  `cpack_stale_configure` dit déjà, par lui-même, quand CPack ne peut pas faire
  l'archive ; sans relecture indépendante, quinze lignes d'une étape informative
  ; fusionnée le 24 octobre)
- #180 (point AP, ancien 128 : un pointeur nul donné à `gaol::textToInterval()`
  (une chaîne ou deux, à l'une ou l'autre place), à
  `gaol_ieee1788::textToInterval()` et à `exactToInterval()` lève
  `input_format_error` ; ces fonctions prenaient un `const std::string&`,
  construit à l'appel avant GAOL, ce que le standard laisse indéfini
  (`std::logic_error` avec libstdc++). Des surcharges `const char*` (quatre pour
  `gaol`, deux en ligne pour `gaol_ieee1788`), y compris pour
  `gaol_ieee1788::textToInterval()`, qui rend le vide pour un texte mal formé :
  un pointeur nul est un appel faux, non un texte mal formé. `null_text()` de
  `tests/expressions.cpp` (onze appels ; 11 vérifications échouent sur 18 201
  sans les surcharges). Le test `numbers` du job « Debian 12 Bookworm arm64 »
  dépassait 300 s : un défaut de GCC 12.2 sur aarch64 (`precisions[]` de
  `decimal_output()` mal lu) que le choix de la nouvelle surcharge par les
  appels de `tests/numbers.cpp` faisait apparaître ; reproduit sans matériel ARM
  avec le GCC croisé 12.2 et qemu-user (68 s contre plus de 280 s), contourné
  par une vingtaine de `textToInterval(std::string(...))`, gardé sur décision du
  mainteneur. Une relecture indépendante, un point bloquant (le manuel ne
  compilait pas : `std::logic_error` sans `\_`), corrigé ; 192 contrôles verts ;
  fusionnée le 24 octobre)
- #181 (point AP, question de #147 : un `file` nul dans les trois constructeurs
  de `gaol_exception`, dans `gaol_warning()` et dans `gaol_error()` s'écrit «
  (unknown file) », et un message nul de `gaol_warning()` et de `gaol_error()`
  est un texte vide (décidé le 24 octobre, après l'ouverture). Avec libstdc++ un
  pointeur nul écrit sur un flux le met à l'état `bad`, et les diagnostics
  suivants étaient perdus ; `file()` n'est plus jamais nul pour les classes de
  GAOL. Les deux aides, `gaol_detail::known_file()` et `known_text()`, sont
  définies une fois, en ligne, dans `gaol/gaol_common.h` (d'abord dans un espace
  de noms anonyme de chaque source, déplacées à la demande du mainteneur).
  `says<Exception>()` et `null_pointers_in_diagnostics()` de
  `tests/expressions.cpp` : 9 vérifications échouent sur 18 198 sans la
  correction ; une relecture indépendante, aucun point bloquant ; CI verte
  jusqu'au commit 4010b4d, les deux derniers (l'aide dans `gaol_detail`, le
  merge de `configure-clean`) ayant été fusionnés avant que la CI les lance ;
  fusionnée le 24 octobre)
- #182 (points AD.85, AB.79 pour la lecture, et `<a, b>` : le lexeur tient
  l'exposant et ses comptes de chiffres en `long long`, car un `long` de 32 bits
  (Windows, i386, armhf) débordait pour un texte de plus de 536 870 411
  caractères : `"1e"` suivi de 536 870 498 chiffres 9 était lu [0, 2^-1074] au
  lieu de [DBL_MAX, +oo]. Un nombre qui peut dépasser le plus grand double ou
  passer sous le plus petit double normal est comparé exactement à DBL_MAX ou à
  2^-1022 avant d'être donné à `strtod()`, qui n'est plus appelé pour ceux qui
  sont au-delà : la lecture de `1e400`, `1.8e308`, `1e-310` ou `1e-400` ne lève
  plus `FE_OVERFLOW` ni `FE_UNDERFLOW`, bornes inchangées bit à bit (15 536
  textes comparés avec `configure-clean`, dont 11 762 à un calcul exact en
  Python). La lecture près de DBL_MAX garde son temps d'avant (+130 ns pour la
  comparaison) ; celle d'un sous-normal est 3 à 10 fois plus lente (voir
  `TODO.md`, PG). `gaol_literal()` refuse un `<a, b>` dont une borne n'est pas
  un point (`<[0.1], 0x1.9999999999999p-4>`). Tests :
  `readings_above_the_largest_double[]` et
  `readings_below_the_least_normal_double[]` de `tests/rounding_direction.cpp`
  (24 puis 20 vérifications échouent sans la correction, le piège compris sous
  glibc ; avec le piège activé, le processeur signale tout résultat sous-normal,
  même exact, si bien que les produits par 0 ne sont vérifiés que pour le
  drapeau) et `degenerate_bounds()` (21 sur 18 217). Le test de l'exposant de
  512 Mio, qui échouait sans la correction dans un conteneur Debian trixie i386
  (Docker, `long` de 32 bits), est retiré sur décision du mainteneur (1,5 Gio
  sur les jobs Windows x64) : rien ne garde plus le point 85. Fichiers générés
  régénérés après chaque commit (flex 2.6.4, bison 3.8.2). Une relecture
  indépendante, aucun point bloquant (elle a mesuré un temps 8 à 12 fois trop
  long pour `1.7e308` dans la première version, corrigé) ; 192 contrôles verts,
  dont un job MinGW relancé pour une erreur de l'exécuteur ; fusionnée le 24
  octobre)
- #183 (point AC, ancien 82 : `pow_rel` et `pow_exponent_rel` (`powRev1` et
  `powRev2`) rendent l'ensemble vide quand la puissance, l'exposant ou la base
  est `[+oo]` ou `[-oo]`, que seuls les intervalles construits à partir d'un
  registre SSE2 ou NEON peuvent être : `powRev1([+oo], [2])` rendait `[1, 1 +
  2^-52]`, `powRev2([+oo], [2])` `[0, 2^-1074]`, `powRev1([2], [+oo])`
  `[DBL_MAX, +oo]` et `powRev1([-oo], [0, 1])` `[1, +oo]`, là où `pow(x, [±oo])`
  est vide. `holds_no_real()`, qui reprend le test de `pow_standard_upward()`,
  est appelée pour la puissance K et l'exposant J dans `pow_rel_upward()`, pour
  K et la base H dans `pow_exponent_rel_upward()`, après les tests de l'ensemble
  vide ; le x (ou le y) à trouver n'a pas de garde, qui serait redondante (22
  050 appels sans aucune garde, par le relecteur).
  `pow_reverse_infinite_arguments()` de `tests/reverse.cpp` (`[+oo]` et `[-oo]`
  aux trois places, huit valeurs pour chacun des deux autres arguments,
  `pow_rel`, `pow_exponent_rel`, `powRev1` et `powRev2` avec et sans x) : 287
  vérifications échouent sur 1 399 741 sans la correction, et le relecteur a
  rendu chaque garde inactive tour à tour (35, 173, 34 et 65 échecs). Une
  relecture indépendante (GCC 15 et Clang 18, SSE2 et FPU, mutations), aucun
  point bloquant, trois remarques corrigées (la garde de l'inconnue, un sujet de
  commit et un commentaire inexacts) ; la branche NEON du test est compilée pour
  arm64 avec un simulacre de `interval(float64x2_t)`, non exécutée ; 191
  contrôles verts ; fusionnée le 24 octobre)
- #184 (point AB, ancien 79 pour les fonctions, et la phrase d'introduction des
  exceptions flottantes de `doc/using.md` et du manuel : `FE_OVERFLOW` et
  `FE_DIVBYZERO` ne sont plus levés pour un résultat sans borne infinie,
  l'ensemble vide compris, par `sqrt`, `nth_root` (ordres jusqu'à 2^32), `atan`
  (Visual C++), `sqrt_rel`, `nth_root_rel`, `sin`, `cos`, `tan`, `div_rel`,
  `atan_rel`, `atan2_rel_x`, `atan2_rel_y`, `atanh_rel`, `pow_rel` et
  `pow_exponent_rel`. Une sonde, hors dépôt, de 131 formes sur des opérandes
  tirés près de `DBL_MAX`, de 2^k et des sous-normaux a trouvé 20,3 millions
  d'appels fautifs avec `configure-clean` (SSE2, GCC 15), dans 34 formes, dont
  `pow_rel`, `pow_exponent_rel`, `atanh_rel` et `nth_root` d'ordres jusqu'à 2^32
  que le point ne nommait pas ; après, aucun avec les intervalles SSE2, les
  intervalles FPU et le chemin AVX-512, et l'empreinte des bits des bornes de
  chacune des 131 formes est celle de `configure-clean` (au signe d'un zéro
  près). Corrections de `gaol/core/gaol_interval_core.cpp` : des quotients et
  des produits saturants (`quotient_up_sat()`, `quotient_dn_sat()`,
  `product_up_sat()`, `product_dn_sat()`, `quotient_sat()`) pour `div_rel` (le
  corps des intervalles FPU et NEON devient `div_rel_upward()` ; les intervalles
  SSE2 gardent leur code vectoriel, avec un test fait dans les registres),
  `atan2_rel_x`, `atan2_rel_y` et l'amorce de `pow_rel` ; les racines mises à
  l'échelle par des puissances exactes de 2 au-dessus de 2^1000
  (`square_down_below()`, `square_up_equals()`, `cube_up_equals()`,
  `scaled_proofs`), non par le produit fusionné `fma(u, u, -v)` que proposait le
  point, qui aurait dépendu de l'instruction ou de son émulation ; `((p -
  x)/p)/n` dans Newton ; `sqrt()` NEON par le chemin générique au-dessus de
  2^1000 ; la demi-largeur `0,5 Ir + 0,5 Il` pour `sin` et `cos` ;
  `tan([DBL_MAX])` sans calcul ; `atanh_rel()` qui pose `±oo` sans appeler
  `atanh(1)` (`atanh()` garde son drapeau, la borne est infinie) ; `atan_lo()`
  et `atan_hi()` qui rendent les bornes de pi/2 à partir de |x| = 2^54, car
  `cr_atan()` de CORE-MATH calcule `x + x` pour un NaN et déborde avec Visual
  C++ à partir de 2^1023 ; pour `pow_rel` et `pow_exponent_rel`,
  `pow_up_without_overflow_exception()` (le seul endroit de la bibliothèque qui
  masque des exceptions, pour les puissances dont le log2 est à 10^-9 de 1024 :
  MXCSR sur x86, `<cfenv>` ailleurs) et un raccourci de `pow_rounded_up()` pour
  une puissance inférieure à 1 et |y| au moins 2^450 (`cr_pow()` calcule `rh *
  rh`, qui déborde à partir de |rh| = 2^512). `doc/using.md` et le manuel ne
  disent plus qu'une borne infinie vient d'une division par zéro ou d'un
  débordement (`1/[0, 1]` n'a aucun drapeau depuis #121), gardent `log([0, 1])`
  et `[1e308]*10`, et listent les fonctions corrigées ; `doc/accuracy.md` dit le
  signe d'un zéro de `div_rel` avec les intervalles SSE2. Tests de
  `tests/rounding_direction.cpp` :
  `flag_functions_raise_no_exception_for_a_finite_result()` (29 appels sur 20
  000 tirages chacun) et `trap_cases()` (24 cas, aux bornes les plus fines, à un
  ou deux doubles près pour trois, sans drapeau et dans un processus fils où
  `feenableexcept()` active les pièges) : 93 252 vérifications échouent sur 1
  135 741 avec les sources de `configure-clean`. Temps : sans changement
  visible, sauf `pow_rel` (348 puis 383 ns, +10 %) et `pow_exponent_rel` (165
  puis 183 ns). Une relecture indépendante (cinq configurations, 144 fonctions,
  empreintes sur 1,5 million de tirages par fonction), aucun point bloquant,
  quatre remarques corrigées (dont un test scalaire qui coûtait 13 % à
  `div_rel`). La CI a demandé quatre commits de plus : `same_interval()`
  inutilisée (AppleClang `-Werror`), `fesetenv()` de MinGW-w64 12 à 14 qui
  laisse des drapeaux levés et `fesetexceptflag()` qui plante (d'où MXCSR sur
  x86), `atan` avec Visual C++, et le drapeau que MXCSR laissait avec Visual C++
  x86 en Release (d'où `<cfenv>` pour lui) ; une branche jetable,
  `debug-msvc-flags`, a servi au diagnostic ; 192 contrôles verts ; fusionnée le
  24 octobre)

- #186 (point AP, ancien 131 : `add_bounds()`, `sub_bounds()`, `mul_bounds()`,
  `div_bounds()` et `sqr_bounds()`, les noyaux que les opérateurs des
  intervalles SSE2 appellent sur les bornes en registres, étaient déclarés dans
  `gaol_core` avec la visibilité par défaut, dans des en-têtes installés : des
  noms et des symboles de l'espace de noms de l'interface, et un appel ambigu
  pour un programme qui ouvre `gaol_core` et écrit une fonction du même nom et
  du même type. Ils passent dans `gaol_detail`, comme les autres aides internes
  ; ceux des intervalles NEON aussi, la décision Q15a ne nommant que SSE2 («
  très bien », 25 octobre). Les déclarations et les définitions sont entre deux
  `} // namespace gaol_core` et `namespace gaol_core {`, avec un `using
  namespace gaol_core;` dans chaque corps : aucune ligne de calcul ne change,
  chemin AVX-512 compris. `test_kernels_are_not_in_gaol_core` de
  `tests/misc.cpp` déclare des fonctions de même nom et de même type, ouvre
  `gaol_core` et vérifie par `static_assert` que l'appel trouve les siennes (le
  test ne compile pas, appel ambigu, sans le déplacement), puis appelle les cinq
  noyaux sur des bornes exactes en doubles. Une relecture indépendante, aucun
  point bloquant, remarques corrigées (un appel de `sqr_bounds()` non qualifié
  dans la branche AVX-512, une partie du test qui comparait les noyaux à
  eux-mêmes, `std::declval` d'un type vecteur, l'ordre de `doc/tests.md`) ;
  testée en natif sur un Apple M2 (57 tests, `-Werror` sans avertissement) ; 191
  contrôles verts ; fusionnée le 25 octobre)
- #187 (point AP, ancien 129 : `sqrt_bounds()` des intervalles NEON prend la
  partie de x dans [0, +oo] par les bits de signe (`vcltzq_s64` et `vbicq_u64`)
  et non par `vmaxq_f64(…, 0)`, dont un compilateur pouvait permuter les
  opérandes : avec `FPCR.AH` (FEAT_AFP, que GAOL n'efface pas), `FMAX` rend le
  second opérande pour `+0` et `-0`, et `sqrt([-0, r])` aurait eu une borne NaN.
  Identique bit à bit à l'ancien code sur les 9 604 paires de valeurs spéciales
  et 20 millions de paires tirées (programme jetable, sous qemu). Un bloc de
  `tests/rounding_direction.cpp` (aarch64) met `AH` quand le processeur le
  retient et compare les sommes, différences, produits, quotients, carrés et
  racines avec `AH` mis et effacé ; l'Apple M2 et QEMU 7.2 et 8.2 ne retiennent
  pas le bit (le bloc le dit et ne vérifie rien), QEMU 10.0 (`-cpu max`, dans
  une image Debian 13 locale, supprimée ensuite) le retient : 441 paires
  d'opérandes par opération, 0 échec, et 189 vérifications échouent avec les
  opérandes de l'ancien maximum échangés (`sqrt([-0, -0])` rend `[-nan, -0]`).
  `doc/simd.md` et `doc/tests.md` dits. Une relecture, aucun point bloquant,
  plusieurs remarques corrigées ; M2 : 766 654 vérifications de
  `rounding_direction` ; 191 contrôles verts ; fusionnée le 25 octobre)
- #190 (point Q, reste de #146 : `bisect(ratio)` et `is_bisectable()`
  dépendaient du mode FZ de l'appelant pour des bornes sous-normales, que
  flush-to-zero et denormals-are-zero lisent comme des zéros. `is_bisectable()`
  comparait `next_float(left()) < right()` en doubles, sans vérifier la
  direction ni effacer les modes (faux pour tout intervalle aux bornes
  sous-normales, avec ou sans `GAOL_PRESERVE_ROUNDING`) ; `bisect(ratio)`
  refusait un rapport sous-normal comme 0 et, avec `GAOL_PRESERVE_ROUNDING`,
  rétablissait les modes avant de comparer le point de coupe aux bornes (100 099
  coupes fausses sur 200 000 intervalles tirés, sur x86 avec denormals-are-zero
  ; le chiffre de 634 sur ARM n'est pas reproduit tel quel). `is_bisectable()`
  compare les clés entières des bornes (`gaol_detail::bound_key()`, un double
  entre elles si les clés diffèrent d'au moins 2 ; plus de `nextafter()`) ;
  `bisect(ratio)` compare le rapport par `bound_greater()` et `bound_less()`,
  remet le point de coupe dans [l, r] et le décale d'un double
  (`bisect_step_toward()`, par les bits) avant `GAOL_RND_LEAVE()`, et construit
  aussi les deux parties avant (construites après, `[c, r]` de deux sous-normaux
  sortait vide avec GCC 15 ; la cause, probablement une comparaison que le
  compilateur ne sait pas séparée par un changement de mode, n'est pas
  vérifiée). Le test de `tests/rounding_direction.cpp`, dans le bloc des modes
  qui vident les sous-normaux (16 intervalles, 8 rapports de 2^-1074 à 1 −
  10^-9, chaque mode et chaque direction ; référence en multiples de 2^-1074) :
  2 280 vérifications échouent avec les sources de `configure-clean` avec les
  intervalles SSE2, 4 008 avec `GAOL_PRESERVE_ROUNDING`, 808 sur un Apple M2 (un
  état antérieur du test). Une relecture, aucun point bloquant, des sondes
  écrites par elle (350 intervalles, 117 986 vérifications par build), plusieurs
  remarques corrigées ; 191 contrôles verts ; fusionnée le 25 octobre)
- #188 (point AP, ancien 132 : le noyau empaqueté de `pown` (deux voies,
  multiplication-addition fusionnée), qui n'existait que pour SSE2 avec FMA,
  existe pour les intervalles NEON, en `float64x2_t` avec `vmulq_f64` et
  `vfmaq_f64` (`a * b - c` par `vfmaq_f64(-c, a, b)`, la négation étant exacte).
  Bornes identiques bit à bit : le hachage de 4 millions de `pow(x, n)` est
  `494dcfe17924a141` sur x86 et sur aarch64, `diffpow.py` donne 70 500 lignes
  identiques sur un M2, un mutant fait à la main fait échouer `arithmetic`,
  `elementary` et `other_functions`. Le gain est faible, loin du 1,5 attendu :
  `pow(x, 3)` 6,07 / 5,68 ns sur le M2 (1,07), 1,23 avec GCC et 0,90 avec Clang
  sur le Neoverse de la CI (les rangées non touchées y bougent de 5 à 13 %). À
  la demande du mainteneur (25 octobre), les noyaux sont répartis par sorte
  d'intervalles : les opérations des deux voies sont une classe par sorte
  (`pown_scalar_ops` dans `gaol_interval_core.cpp`, qui a aussi le noyau en
  doubles `large()`, `pown_sse_ops` dans `gaol_interval_sse.cpp`,
  `pown_neon_ops` dans `gaol_interval_neon.cpp`, `pown_er_ops` dans
  `gaol_interval_avx512.cpp`), `pown_products<Ops>()` et `pown_powers<Ops>()`
  sont communs et placés avant les inclusions, `pown_large_inline()` est défini
  à la fin du fichier de chaque sorte, et `GAOL_AVX512_TARGET` passe
  d'`avx512.cpp` à `core.cpp`. Après le déplacement : le même hachage pour SSE2
  avec et sans FMA, FPU, AVX-512 avec et sans préservation et NEON ; le banc A/B
  x86 sans changement (6,34 / 6,20 ns) ; Clang 18 avec AVX-512 en `-Werror`.
  `tests/tools/pow/mutate.py` : les mutants disent leur fichier, dix `neon-*`
  sont ajoutés (non lancés : il faut un processeur ARM), trois mutants périmés
  sont remis à jour. Avec `FPCR.AH` mis (QEMU 10.0), le même hachage. Deux
  relectures, aucun point bloquant ; M2 : 57 tests, `-Werror`, `diffpow.py`,
  noyau FPU compris ; 191 contrôles verts ; fusionnée le 25 octobre)
- #189 (point AC, reste du 82 : `interval(const __m128d&)` et `interval(const
  float64x2_t&)` rendent l'ensemble vide pour les voies de `[+oo]` et de `[-oo]`
  (une voie à -oo : `_mm_cmpeq_pd` pour SSE2, les bits pour NEON), qui ne
  contiennent aucun réel et que nul autre constructeur ne fait ; sqr, `pow`,
  `exp`, `div_rel` et `sqrt_rel` les prenaient pour des intervalles. Les 36
  usages internes (les opérateurs, `sqr()`, `rnd_keep()`, les constantes,
  `divide_general()`, `sqrt()`, `pown_large_inline()`, les fonctions de NEON)
  passent par `interval(registre, gaol_detail::unchecked_bounds)`, trouvés en
  marquant provisoirement le constructeur à un argument `[[deprecated]]` et en
  compilant en SSE2, en SSE2 avec AVX-512 et pour NEON (compilation croisée),
  refait après chaque fusion ; les gardes de #183 restent le filet. Coût de la
  garde pour qui construit depuis un registre : 1,086 ns avec et sans (x86), les
  opérateurs à 1,00 sur le M2. `tests/elementary.cpp` : le registre de `[+oo]`
  et de `[-oo]` est vide et six fonctions (`gaol_ieee1788::pow`, `gaol::pow`,
  `sqr`, `exp`, `div_rel`, `sqrt_rel`) le sont aussi, les registres des
  intervalles qui existent sont gardés, une voie à -oo et l'autre finie donne le
  vide ; 13 vérifications échouent sans la garde ; `elementary` et `reverse`
  bâtissent `[+oo]` avec le constructeur sans garde pour continuer d'éprouver
  les gardes de #183. GCC 12.2 sur aarch64 lisait de mauvaises valeurs dans une
  boucle sur un tableau de structures du test (`-O2` et `-O3`) : une fonction
  par cas. Une relecture, aucun point bloquant, plusieurs remarques corrigées ;
  M2 : 57 tests dans trois builds, banc A/B entre 0,98 et 1,03 ; conflits de
  texte avec #186 puis #188 résolus en fusionnant `configure-clean` ; 191
  contrôles verts (un job macOS 14 Rosetta a échoué une fois sur `arithmetic`,
  `gaol_ieee1788::pow(x, [2])` attendu vide, et est passé à la relance) ;
  fusionnée le 25 octobre)
- #191 (point PG.6, la division du chemin AVX-512 : `er_div_low()`,
  `er_div_high()` et `er_div2()` de `gaol/core/gaol_interval_avx512.cpp` font
  une ou deux divisions scalaires à arrondi intégré (`vdivsd {ru-sae}`) là où
  trois endroits divisaient encore en 512 bits : `fast_div_by()` (`x / d` et `x
  /= d`), les 14 divisions de `div_unguarded()` (l'aiguillage d'un diviseur qui
  contient 0) et le `1/h` des puissances négatives (`pown_er_ops::div`) ; plus
  aucune division de 512 bits dans la bibliothèque. La description de PG.6 et
  `doc/simd.md` disaient que l'idée n'avait pas été essayée pour `/` : le `x /
  y` de l'interface, `div_bounds()`, la faisait depuis cf60ef6. Bornes
  identiques bit à bit : le hachage de 4,3 millions de résultats (`x / y`, `x /
  d`, `x /= d`, `pow(x, n)` pour onze exposants négatifs, les quatre sens, FTZ
  et DAZ effacés et posés, MXCSR compris) est `fdfc6b07716f6333` avant et après,
  et celui sans MXCSR est celui du build SSE2 ; cinq mutants du code changé sont
  tués par les tests existants ; dix des 14 divisions de `div_unguarded()` ne
  sont pas atteignables par l'interface, leur égalité repose sur l'algèbre des
  voies (relue). Mesures (i7-1185G7, Clang 21, `GAOL_PRESERVE_ROUNDING=ON`,
  meilleur temps) : `x / d` de 3,62 à 2,76 ns pour d > 0 (0,76), de 3,66 à 3,01
  pour d < 0, `x /= d` de 3,64 à 2,82 ; les diviseurs avec 0, les puissances
  négatives et les témoins au bruit près. Le banc A/B de la CI n'a aucune ligne
  `x / d` ; ses lignes bougeaient de 30 % pour un `add_bounds()` identique octet
  pour octet, dont seule l'adresse changeait (l'effet de placement du 105), et
  `-falign-functions=64` pour les deux builds ramène tout entre 0,98 et 1,03.
  Une relecture indépendante, aucun point bloquant, des phrases de `doc/simd.md`
  corrigées ; CI verte ; fusionnée le 25 octobre)
- #192 (point PG.106 a, puis, à la demande du mainteneur, NEON avec tous les
  compilateurs de l'arm64 : `GAOL_NEON_MSVC` (`cl` pour `_M_ARM64`) ouvre la
  condition de `gaol/gaol_config.h` qui refusait NEON ; `cl` n'a pas d'asm sur
  ARM64, FPCR est lu et écrit par `_ReadStatusReg(ARM64_FPCR)` et
  `_WriteStatusReg()`, avec `_ReadWriteBarrier()` avant et après l'écriture
  (`fpcr_get()` et `fpcr_set()`), et `reread_bounds()` et
  `rnd_keep(float64x2_t)` n'ont pas d'asm (des `double` volatils) ; clang-cl et
  le Clang de MinGW-w64 et de MSYS2 lisent et écrivent FPCR par l'asm GNU comme
  sous Linux (condition `__aarch64__ && __ARM_NEON && (__GNUC__ || __clang__)`,
  le `static_assert` des valeurs `FE_*` retiré sous Windows). Les trois builds :
  une sonde pour `cl` et une pour les autres compilateurs avec CMake et meson,
  la ligne `#error` de configure retirée (`configure` régénéré, 11 lignes). Le
  `mid()` de NEON reprend le contournement des intervalles SSE2 et FPU,
  `rnd_keep()` des moitiés : `cl` (`/O2 /fp:strict`) réécrit `(-.5)*l + .5*r` et
  `mid([2^-1074, DBL_MAX])` rendait le point `DBL_MAX/2` (trouvé par les jobs
  arm64 de la CI). Les opérations NEON de Visual C++ effacent FZ et FIZ, ce que
  ses intervalles FPU ne faisaient pas. Tests : FPCR lu et écrit pour les trois
  cas, le test de FIZ tourne pour `cl`. CI : les jobs arm64 attendent `neon`, et
  six jobs de plus (Visual Studio 2026 arm64 avec la direction préservée, FPU,
  clang-cl arm64 FPU, MSYS2 CLANGARM64 avec et sans la direction préservée,
  meson Visual Studio 2026 arm64). Banc A/B (runners arm64 de GitHub, FPU puis
  NEON, ns) : Visual C++ `x + y` 2,30 à 1,43, `x * y` 10,77 à 5,39, `x / y`
  10,08 à 5,40, `sqrt` 22,99 à 7,24, ligne de 15 `+` 43,13 à 13,26 ; clang-cl `x
  * y` 5,71 à 5,19, `x / y` 4,38 à 5,36 (il perd : voir PG.8 de `TODO.md` et
  #195), `sqrt` 11,02 à 5,65. Un script de modification de `meson.build` y avait
  inséré une copie de 300 lignes (775 à 1097), que meson exécutait en dernier
  (clang-cl et MinGW-w64 seraient restés en FPU sans qu'aucun job le voie) :
  trouvée par le relecteur. Deux relectures indépendantes ; GCC 12.2 croisé sous
  qemu (`rounding_direction`, `misc`, `arithmetic`), un Apple M2 par la session
  MACarm2 (55 tests sur 55 en défaut, avec `GAOL_PRESERVE_ROUNDING=ON` et
  `-Werror`), MSVC 19.44 et 19.51 pour ARM64 par Compiler Explorer (bibliothèque
  et cinq tests compilés, assembleur des noyaux lu : `msr FPCR` précède la
  première opération flottante, qui précède la restauration) ; 198 contrôles
  verts ; fusionnée le 25 octobre)
- #193 (SSE2 sur Windows 32 bits avec Visual C++ et clang-cl, à la demande du
  mainteneur : les bornes d'un intervalle sont rangées dans
  `gaol_detail::unaligned_xmm`, deux `double` d'alignement 8 chargés par
  `_mm_loadu_pd` et rangés par `_mm_storeu_pd`, qui se convertit en `__m128d` et
  depuis lui (les 200 emplois de `xmmbounds` et de `get_xmminterval()` ne
  changent pas) ; la macro `GAOL_SSE2_UNALIGNED_STORAGE` (`gaol/gaol_config.h`)
  le décide d'après la cible (`_WIN32` sans `_WIN64`, `_MSC_VER`, SSE2), et un
  réglage de développeur de CMake la pose pour Linux, où deux jobs de
  `linux.yml` passent toute la suite de tests dans cette forme ; les allocateurs
  de classe (16 octets) n'existent plus dans cette forme et l'`#error` « le
  compilateur doit demander de la mémoire alignée » ne vaut plus pour elle.
  `GAOL_VECTORCALL` vaut `__vectorcall` sur x86 32 bits aussi. Un défaut trouvé
  en chemin par le job x86 avec la direction préservée : `cl` renvoie un
  `double` dans ST0 et le calcule sur l'unité x87 (le `fdiv` de
  `quotient_sat()`), dont la direction restait fausse ; `GAOL_RND_ENTER_SSE()`
  et `GAOL_RND_LEAVE_SSE()` deviennent les macros complètes avec Visual C++ pour
  x86. Le test `test_intervals_aligned_on_8_bytes` de `tests/misc.cpp` a des
  dents (un chargement aligné fait mourir le programme par SIGSEGV) ; coût du
  stockage en deux `double` sur x86-64 : aucune ligne hors du bruit (0,91 à
  1,11). Banc A/B de la CI (Windows 32 bits, FPU puis SSE2, ns) : Visual C++ `x
  * y` 10,65 à 9,38, ligne de 15 `+` 206,0 à 123,4, ligne de 13 `+ - * /` 215,2
  à 112,7 ; clang-cl `x * y` 9,72 à 7,66, ligne de 15 `+` 183,8 à 100,6 ;
  `__vectorcall` sans différence au-delà de 5 %. Les trois builds : CMake et
  meson ne laissent plus SSE2 de côté pour ces compilateurs, configure accepte
  le Clang pour une cible de Visual C++ sans lui donner `-faligned-new=8`, et
  l'audit des trois builds compare la macro. Une relecture indépendante (201
  emplois lus), aucun point bloquant ; MSVC 19.44 et 19.51 pour x86 par Compiler
  Explorer (compilation seulement, l'exécution échoue avec 0xC000007B) ;
  conflits de texte avec #192 résolus en fusionnant `configure-clean` ; CI verte
  sur le dernier commit ; fusionnée le 25 octobre)
- #194 (le build universel de macOS, à la demande du mainteneur :
  `CMakeLists.txt` le reconnaît (plusieurs architectures dans
  `CMAKE_OSX_ARCHITECTURES`), saute les sondes, qui échouaient pour l'une des
  architectures, et pose `GAOL_UNIVERSAL_SSE2` et `GAOL_UNIVERSAL_NEON` ;
  `cmake/gaol_configuration.h.in` les change en `GAOL_USING_SSE2_INSTRUCTIONS`
  et `GAOL_USING_SSE3_INSTRUCTIONS` (`__x86_64__`) et en
  `GAOL_USING_NEON_INSTRUCTIONS` (`__aarch64__`) : le préprocesseur de chaque
  tranche choisit, sans drapeau. Le fusionné multiplie-addition et le chemin
  AVX-512 restent éteints (leurs sondes échouent pour une architecture) ;
  configure et meson ne font pas de build universel (documenté). Avant, les deux
  tranches calculaient avec le FPU sans que rien le dise. Nouveau job `macOS 26
  universal (arm64 and x86_64)` : les macros de chaque tranche relues par `c++
  -arch ... -dM -E`, 41 tests en natif, 29 programmes de test de la tranche
  x86_64 sous Rosetta, un par un (`arch -x86_64 ctest` échoue avec le `ctest`
  d'Homebrew, arm64 seul), et la première ligne de `gaol_performance` pour
  chaque tranche. Un relecteur indépendant, aucun point bloquant ; un Apple M2
  (MACarm2) : le build universel passe, `lipo` dit `x86_64 arm64`, la tranche
  x86_64 y est compilée et liée seulement (Rosetta n'y est pas installé) ; Linux
  x86-64 et GCC 12.2 croisé sous qemu après la fusion de `configure-clean` ; CI
  verte sur le commit qui précède cette fusion, qui a eu lieu quand la CI du
  commit de fusion n'avait pas commencé ; fusionnée le 25 octobre)

- #196 (tâche PD, première des deux pull requests : le point 130 et des restes
  de textes. `process.md` : l'en-tête porte une date et un commit (décidé le 22
  octobre, Q9c), le §6.1 une phrase pour une machine sous Ubuntu 24.04 (Q9a), le
  §4.11 « Clang 21 (depuis le 13 octobre) » et la colonne avec la direction
  préservée de `doc/compare/performance.md`, le §9 et le §3.4 disent que
  l'en-tête et le §12 se mettent à jour avec les registres, et cinq pièges de
  plus au §11 (le compteur d'allocations de `tests/expressions.cpp` avec une DLL
  de Windows, #149 ; l'`asm volatile` du registre de contrôle et le test qui
  construit son intervalle dans la même fonction, #150 ; le script qui duplique
  300 lignes de `meson.build`, #192 ; Visual C++ pour ARM64, #192 ;
  `arch -x86_64 ctest` et `! grep` sous `bash -e`, #194), dont trois énoncés de
  `TODO.md` corrigés après la relecture (`! grep` ne fait échouer l'étape que
  s'il n'est pas sa dernière commande, 22 caractères dans l'objet pour libc++,
  l'`asm` avec « memory » ordonné avec la mémoire). `doc/accuracy.md` ne cite
  plus `interval(sl, su)`. Le build par défaut de `run.sh` s'appelle « default »
  (SSE2 sur x86, NEON sur arm64) dans `itf1788.py`, `itf1788.md`,
  `doc/compare/README.md` et `tang2021.md`, sauf où une phrase décrit une mesure
  faite sur x86. Une relecture indépendante, un point bloquant (`! grep`),
  corrigé ; les trois derniers commits (§4.11 deux fois, libellés de
  `tang2021.md`) n'ont pas été relus. Le job « Ubuntu 26.04 arm64 Clang » a
  échoué au premier passage, sur le bloc `FPCR.AH` de `rounding_direction`, puis
  est passé à la relance (issue #198) ; fusionnée le 25 octobre, avant la fin de
  sa CI)
- #197 (tâche PD, seconde pull request : le point 7 de la tâche, les mutants de
  `tests/tools/pow/mutate.py`. Les cinq mutants qui ne s'appliquaient plus
  (`std-empty-y-unchecked`, `std-infinite-exponent-unchecked`,
  `corner-lower-not-exact`, `corner-upper-no-limit`, et `er-division-downward`
  depuis #191, que `TODO.md` ne comptait pas) suivent le code de #183
  (`holds_no_real()`), de #184 (`quiet`) et de #191 (`er_div2()`) ;
  `std-empty-y-unchecked` est équivalent parce que ses deux seuls appelants
  rendent le vide avant d'appeler `pow_standard_upward()`, le texte de
  `survives` qui disait que `rounding_direction` le tue est réécrit ; quatre
  mutants de plus pour `er_div_low()`, `er_div_high()` et `er_div2()`, tués par
  `arithmetic` (104 mutants). Les dix `neon-*` ont tourné sur l'Apple M2 par la
  session MACarm2 : 8 tués, 2 équivalents, aucun survivant. Deux mutants ne
  tenaient qu'à 1 et 5 vérifications d'`arithmetic` (`powers-of-c`,
  `inverse-first-order`, leurs jumeaux de SSE2, d'AVX-512, du FPU et de NEON),
  d'où deux changements de `tests/arithmetic.cpp` : le tableau des grandes
  puissances négatives passe de 6 à 46 lignes (bornes de mpmath ; le script
  `tests/tools/pow/negative_power_rows.py` les écrit et le relecteur en a
  recalculé 40 sur 40 sans mpmath, en rationnels exacts), et 12 736 points de
  plus pour `pow([-c], n)` (c à 16 doubles de 1, n de 2 à 200). Avant, après :
  `inverse-first-order` 5 puis 44 vérifications en échec, `powers-of-c` 1 puis 5
  (un point de la grille, c = 1 + 2^-51 et n = 21, était déjà dans la boucle
  existante ; des tirages au hasard et une grille jusqu'à 64 doubles de 1 n'en
  donnaient pas plus). `arithmetic` passe sans mutant en SSE2 avec FMA, AVX-512,
  FPU et NEON, avec GCC 15.2, Clang 18 et Apple clang 21, et en
  `-Wall -Wextra -Werror`. Deux relectures indépendantes, aucun point bloquant ;
  la marge « 2^8 fois 2K » du commentaire était fausse pour n < 0 (au moins 13
  fois 2K avec `pown_bound_inverse()`), corrigée ; fusionnée le 25 octobre,
  avant la fin de sa CI)
- #199 (hors des tâches, à la demande du mainteneur : les neuf fichiers
  `gaol_interval_core.*`, `gaol_interval_neon.*`, `gaol_interval_fpu.*`,
  `gaol_interval_sse.*` et `gaol_interval_avx512.*` passent de `gaol/` à
  `gaol/core/` (`git mv`, renommages reconnus à 98-100 %), sans qu'une ligne de
  calcul change. Les includes (`"gaol/core/gaol_interval_core.h"`...), les trois
  builds (CMake : `GAOL_SOURCES` et une liste `GAOL_CORE_HEADERS` ; autotools :
  `gaol/Makefile.am`, avec `gaol/Makefile.in` régénéré par automake 1.18.1 ;
  meson : `gaol/meson.build`), `.github/scripts/headers.sh` et tous les chemins
  `gaol/gaol_interval_{core,neon,fpu,sse,avx512}` des commentaires, de la
  documentation, des tests, des outils (`tests/tools/pow/mutate.py`), de la CI
  et des registres deviennent `gaol/core/...`. Les quatre en-têtes
  `gaol_interval_{core,fpu,neon,sse}.h` sont installés dans `include/gaol/core/`
  (avant : `include/gaol/`), comme ceux du parser dans `include/gaol/parser/`,
  sans en-tête de renvoi. `gaol_interval_fpu.cpp` rejoint les trois autres
  `.cpp` inclus dans l'`EXTRA_DIST` de `gaol/meson.build`. Le lexeur et le
  parser, régénérés, sont identiques. En local : CMake SSE2 avec tests et
  exemples, 61 sur 61 ; CMake FPU et chemin AVX-512, la bibliothèque ; meson, 29
  réussis et 2 sautés ; autotools, `make check` réussi et `make distclean` sans
  reste ; les mêmes 23 en-têtes installés par les trois builds, `headers.sh`
  sans échec sur chacun. Pas de relecture indépendante (demande du mainteneur) ;
  `check_branch` échoue sur `doc/differences.md`, que le mainteneur a gardé,
  avec les registres, dans la pull request ; 47 lignes passent 80 colonnes,
  laissées ; 201 jobs verts sur le dernier commit de la branche, NEON, Windows
  et macOS compris ; fusionnée le 26 octobre)
- #200 (hors des tâches, à la demande du mainteneur : les trois builds ont les
  mêmes options, celles de l'utilisateur, et le reste est décidé par des tests
  qui compilent et exécutent. Les options : `GAOL_PRESERVE_ROUNDING`,
  `GAOL_SIMD` (SSE2 sur x86, NEON sur ARM 64 bits, le FPU sinon ; `GAOL_ARM64`,
  `--enable-arm64` et `enable-arm64` ne sont plus des options), `GAOL_COVERAGE`
  (`--enable-coverage` et `make coverage` pour configure, `enable-coverage` et
  `ninja gaol-coverage` pour meson, `coverage` étant réservé par meson),
  `WITH_TESTS`, `WITH_EXAMPLES`, `BUILD_SHARED_LIBS` (la bibliothèque statique
  est le défaut des trois builds : configure et meson construisaient les deux),
  `CMAKE_BUILD_TYPE`, le code indépendant de la position, et
  `CMAKE_OSX_ARCHITECTURES` (le build universel de macOS aussi avec autotools et
  meson, par `-arch` plusieurs fois dans les drapeaux). Décidés par la
  configuration : les instructions FMA (prises quand la machine exécute un
  programme compilé avec elles ; en compilation croisée, laissées de côté sauf
  `-DGAOL_MACHINE_RUNS_FMA=1`, `gaol_cv_machine_runs_fma=yes` ou
  `-Dmachine-runs-fma=true`, et de même pour AVX-512), le chemin AVX-512 (il
  suit `GAOL_PRESERVE_ROUNDING` : les quatre builds mesurés dans `doc/simd.md`
  donnent, sur un i7-1185G7 à demi-largeur, une médiane de 2,80 avec la
  direction préservée et de 0,83 sans), l'entier 128 bits (les deux moitiés
  seulement sans type du compilateur) et le débogage (le type `Debug`). Les
  anciennes options restent comme réglages des développeurs : lues avec
  `DEFINED` par CMake, sans aide dans `configure --help`, marquées
  `[developers]` par meson ; `--enable-optimize` est supprimée. Le test
  `automatic_configuration` remplace `prefer_avx512_reconfigure` et échoue sur
  l'ancien `CMakeLists.txt` à six endroits ; le test d'exécution de FMA de
  Visual C++ et de clang-cl exécute l'intrinsèque `_mm_fmadd_sd` au lieu
  d'appeler `std::fma` ; l'audit des trois builds est inchangé. Les workflows
  passent de `GAOL_ARM64=OFF` à `GAOL_SIMD=OFF`, deux jobs « bibliothèque
  partagée » et deux jobs de build universel de macOS s'y ajoutent. Une
  relecture indépendante, aucun point bloquant, tous ses constats corrigés
  (`range()` de meson 0.58, le manuel et des commentaires, CORE-MATH sans les
  compteurs de gcov avec CMake, une entrée d'un ancien répertoire de build,
  `make coverage` sans `--enable-coverage`). En local : CMake (défaut, direction
  préservée, FPU, Clang 18, Debug avec ASan et UBSan, `-Werror` avec GCC et
  Clang) à 100 % ; autotools et meson, avec installation, `headers.sh` et
  `tests.sh` ; `make coverage` 94,1 % et `ninja gaol-coverage` 94,0 %. Le
  premier passage de la CI a trouvé deux défauts de meson (`has_header()` lance
  le préprocesseur, que clang refuse avec plusieurs `-arch`, et le test
  `automatic_cleanup` avec la bibliothèque partagée), corrigés ; 206 jobs verts
  sur le dernier commit, les trois builds universels de macOS compris ;
  fusionnée le 26 octobre à 18 h 45 UTC)
- #201 (hors des tâches ; le point 92 de `TODO.md`, ancien AE : les sources C de
  CORE-MATH reçoivent les mêmes drapeaux avec CMake, autotools et meson,
  `-std=gnu99`, `-msse2 -msse3` là où les intervalles SSE2 sont pris,
  `-fvisibility=hidden`, et `-ffloat-store` du C avec configure ; avant, CMake
  seul donnait les deux premiers, configure laissait la norme au compilateur
  (C23 avec GCC 15) et meson prenait `gnu11` ; l'audit des trois builds compare
  maintenant un source C de CORE-MATH, `sin.c` (`make_probe.py c`, la commande
  de compilation du C lue dans `compile_commands.json` et dans `make -n -C 3rd`,
  drapeaux, norme, optimisation et macros comparés) ; il échoue contre les
  sources de #200 (`C: __STDC_VERSION__`, `C: -msse2`, `C: standard`...) et
  passe avec GCC 15 et Clang 21 ; une bibliothèque partagée exporte les mêmes 38
  symboles avec les trois builds ; sur décision du mainteneur,
  `doc/differences.md` et `ChangeLog` disent aussi ce que #200 a changé aux
  options ; 205 jobs verts sur le dernier commit ; fusionnée le 26 octobre à 21
  h 03 UTC)
- #202 (hors des points numérotés ; le point 4 de la tâche PE, les remarques des
  outils de #160 : les 34 remarques d'actionlint 1.7.12 sur les workflows, dont
  un faux positif, `suppress-deprecation-warnings` de `msys2/setup-msys2`, que
  `.github/actionlint.yaml` écarte avec sa raison, et les 101 de shellcheck 0.11
  sur les scripts, 12 de `.github/scripts/` et 89 de `doc/compare/code/` ; aucun
  comportement ne change : les listes de drapeaux que le shell doit découper
  portent un `# shellcheck disable=` avec la raison, la boucle `for` remplace
  `ls | grep` dans le contrôle des bibliothèques installées, et `.shellcheckrc`
  lit un script avec ceux qu'il charge ; `doc/tools.md` dit comment lancer les
  deux outils ; 185 jobs verts sur le dernier commit ; fusionnée le 27 octobre à
  8 h 39 UTC)
- #203 (hors des points numérotés ; le point 6 de la tâche PE, la relecture
  indépendante des commits `704ca0c`, `e80b6f0`, `1de1df0` et `15399b5`, faite
  par un sous-agent sans le contexte de la session : aucun point bloquant, six
  constats mineurs corrigés : `rounding_direction.cpp` suit les bits de l'x87
  autres que RC (`x87_other_bits()`) après `cleanup()` et `restore_rounding()`
  et règle la direction avant de lever les indicateurs de MXCSR, deux phrases
  que les journaux de la CI contredisaient, « la direction d'aucune »
  (`gaol_fpu.h`, `doc/using.md`), les valeurs de `FE_*` de mingw-w64 13 et
  plus, les onze « point 110 de `TODO.md` » devenus AI.110 ; deux mutants
  tués sur une copie où le chemin de mingw-w64 est forcé sur Linux ; 205 jobs
  verts sur le dernier commit ; fusionnée le 27 octobre à 8 h 41 UTC)
- #204 (hors des points numérotés ; le reste du point 95 de la tâche PE et le
  point 5 : la sonde de la cible AVX-512 lie avec autotools (`AC_LINK_IFELSE`,
  `configure` régénéré par autoconf 2.72) et avec meson (`links()`), comme avec
  CMake, parce que `__builtin_cpu_supports` demande `__cpu_model` à l'édition
  de liens ; CMake essaie aussi le compilateur C avec les drapeaux de FMA
  (`check_c_source_compiles`, qui exige `__FMA__` ou `__ARM_FEATURE_FMA`, et
  `check_c_compiler_flag(/arch:AVX2)` avec Visual C++ et clang-cl) et, s'il
  les refuse, laisse les instructions de côté avec un message ; une étape de
  plus de `tests/automatic_configuration.cmake` donne à CMake un compilateur C
  qui refuse le premier drapeau de FMA pris (le premier essai, qui ne
  refusait que `-mfma`, a fait échouer les quatre jobs armhf, où le drapeau
  est `-mfpu=neon-vfpv4`), et échoue contre l'ancien `CMakeLists.txt` ; la
  sonde liée de la cible AVX-512 n'a pas de dents, aucune chaîne de la machine
  ne compile sans lier ce programme ; le commentaire de `CMakeLists.txt`
  « fixed for GCC 17 only » cite le commit `8aaa5f6e6207` du bogue 54412 de
  GCC ; 205 jobs verts sur le dernier commit ; fusionnée le 27 octobre à 8 h
  44 UTC)
- #205 (hors des points numérotés ; le point 3 de la tâche PE, les jobs qui
  manquent de #152, #173, #174, #193 et #194 : MSYS2 MINGW32 avec autotools,
  meson et l'audit des trois builds, Cygwin avec autotools et meson et un Debug
  complet, Clang pour la cible de Visual C++ avec le pilote de GCC (CMake x64 et
  x86, meson x64 et x86, autotools x64), les trois builds universels de macOS
  sur macOS 15 et 26, le générateur d'Xcode avec la liste `arm64;x86_64` et sans
  liste ; ils ont trouvé quatre défauts, corrigés dans la même pull request : le
  `fma()` de msvcrt dans les tests compilés sans `-mfma` (44 des 100 000
  vérifications de `fma` de `tests/arithmetic.cpp` ; `tests.sh` donne partout
  les Cflags du `gaol.pc` installé), le C++14 que demande la bibliothèque
  standard de Visual C++ (`CMAKE_CXX_STANDARD 14`, `GAOL_CXX_STD` de
  `configure.ac`, `cpp_std=c++14` des seules bibliothèques avec meson), le
  `unistd.h` du lexeur pour un compilateur qui simule Visual C++
  (`cmake/msvc/unistd.h`, `GAOL_MSVC_CPPFLAGS`, `clang_msvc` de `meson.build`,
  bibliothèque statique) et l'architecture du générateur d'Xcode sans liste
  (CMake lui donne celle de la machine) ; sur décision du mainteneur du 27
  octobre, `gaol/gaol_config.h` refuse mingw-w64 12 ou plus pour x86 32 bits lié
  à `msvcrt.dll` (MSYS2 MINGW32) sans `-mfma`, comme msvcrt sur x86-64, et un
  job de CI (`msys2-refused`, paramétré par `msystem`) vérifie le refus, dont
  les dents sont les macros d'un mingw-w64 32 bits simulé sur Linux (accepté
  avant, refusé après) ; la version 11 reste acceptée sans FMA, pour garder le
  job « MinGW-w64 13.2.0 x86 Release, no FMA » (question ouverte de la pull
  request : voir `TODO_AFTER.md`) ; pas de job autotools avec Clang vers MSVC en
  x86 (décidé le 27 octobre) ; relue par un agent indépendant, sans risque pour
  les bornes, ses constats corrigés (la branche en retard sur
  `configure-clean`, le `-std=c++14` de meson en argument de projet, la
  documentation et des commentaires, la bibliothèque partagée de CMake) ; 222
  jobs verts sur le dernier commit ; fusionnée le 27 octobre à 11 h 18 UTC)

### 2.4 Fermées sans fusion

- **#29** (2 octobre) : « The three builds made alike », de `configure-clean`
  vers `MATH-CORE`. Chemin décidé le 3 octobre (point 34) : une fois
  `configure-clean` fini, la fusionner dans `MATH-CORE`, puis `MATH-CORE` dans
  `master`, puis étiqueter `v5.0.0` (voir `TODO.md`, tâche PJ).
- **#38** (29 septembre) : « [WIP] Fix failing GitHub Actions job Visual Studio
  2026 x86 Release ».
- **#52** (30 septembre) : ouverte par erreur de la branche de la session vers
  `master`.
- **#116** (13 octobre) : pull request jetable, qui n'a servi qu'à lancer la CI
  (les tests de `numbers` sous les modes qui mettent les sous-normaux à zéro sur
  ARM).
- **#153** (20 octobre) : `width()` et `bisect()` avec `GAOL_PRESERVE_ROUNDING`
  sur arm64 ; elle avait la même correction que #150, qui l'a fusionnée ; sa
  branche `todo-fpcr-gcc-hoisting` est supprimée.
- **#168** (23 octobre) : le noyau mixte du produit FPU, une expérience pour le
  banc A/B, fermée par le mainteneur après lecture des tableaux (voir
  `TODO_AFTER.md`).
- **#195** (25 octobre) : « [EXPÉRIENCE] Quotient NEON : deux divisions
  scalaires (ne pas fusionner) », une pull request jetable demandée par le
  mainteneur pour chercher pourquoi `x / y` de clang-cl est plus lent en NEON
  qu'avec le FPU (#192) : deux divisions scalaires à la place de la division
  vectorielle sont plus lentes sur le Neoverse N2 (de 5,36 à 6,17 ns avec
  clang-cl, de 5,40 à 6,30 avec Visual C++) et sur l'Apple M2 (de 2,38 à 2,76
  ns), si bien que la perte n'est pas dans la division ; fermée sans fusion, sa
  branche `experiment-neon-scalar-div` est supprimée (voir `TODO.md`, PG.8, et
  `doc/simd.md`).

### 2.5 Les pull requests du 23 octobre au soir et les décisions qu'elles exécutent

Tableau repris de `TODO_mistral.md` (QK.7), complété le 24 octobre. Chacune a eu
sa relecture indépendante (sauf #168, une expérience, et #173, dont la première
exécution tient lieu de relecture) ; la CI de #165, #166 et #167 à #171 était
verte le 23 octobre à minuit.

| PR | Contenu | Décisions qu'elle exécute |
|----|---------|---------------------------|
| #165 (fusionnée) | commentaires de la CI et de `doc/compare/code/`, `setup.sh` sans `-U`, `process.md` §10, `NI` et `FS` documentés | #161 textes, #161 `setup.sh`, #160 `process.md`, #154 `NI`/`FS` |
| #166 (fusionnée) | dix jobs de plus pour le banc A/B, cache des programmes de la base, `run_perf.sh` dans le filtre, `LINE_LIST` vide | #160 filtre, jobs et cache |
| #167 (fusionnée) | Cygwin avec les drapeaux de GCC pour Windows, six directions partout, `-mavx512f`, `-march` et `-faligned-new=8` documentés | #163 |
| #168 (fermée) | noyau mixte du produit FPU | #157 piste |
| #169 (fusionnée) | le vide dans `pow`, le carré FPU | #162 |
| #170 (fusionnée) | jobs FPU x86-64 et arm64 GCC avec le sens préservé, commentaire d'`operator*=` | #157 `GAOL_RND_KEEP()` |
| #171 (fusionnée) | test des bits du signe de `x * d` | #161 |
| #172 (fusionnée) | GCC pour Windows en 32 bits : `-mfma -mprefer-vector-width=128`, un job sans FMA | #163, x86 32 bits |
| #173 (fusionnée) | Cygwin x86-64 pris en charge : `_mm_malloc`, `previous_float()`/`next_float()` aux bords des doubles, `fma_once()` et le test d'exécution de `GAOL_FMA` en `__builtin_fma`, refusé sans FMA, pas de `package_source` (CPack), un job de CI (44 tests, trois sautés) ; décidé de le porter le 24 octobre | #163, Cygwin |
| #174 (fusionnée) | les restes de Cygwin : `is_bisectable()` par `next_float()`, `std::fma` sans optimisation dit dans les docs, le manuel et l'exemple 13 | #173 |
| #176 (fusionnée) | le job Cygwin n'essaie plus les générateurs de CPack | #173, 24 octobre |

## 3. Les décisions, par date

### 3.1 Le 30 septembre

- [x] 1 : `gaol::pow([0], y)` s'écrit `<0, 0>` : c'est bon (ce sera `[0]` avec
  la décision du 13).
- [x] 5 : les tests de compilation n'existent que pour CMake : c'est bon.
- [x] 6 : le refus de MinGW-w64 avant 12 reste, avec sa vraie raison, et s'étend
  aux mingw-w64 liés à msvcrt sur x86-64 (#51). Sur x86 32 bits, les toolchains
  acceptées (mingw-w64 11) ont un `fma()` x87 qui n'est pas correctement
  arrondi, mais passent tous les tests : l'écart est accepté, les tests actuels
  sont gardés tels quels.
- [x] 10 : pas de normalisation des zéros (`-0`) des opérations d'intervalles.
- [x] 11 : l'effet de DAZ hors du lecteur devient le point 45.
- [x] 13 : `operator<<` et `intervalToText` écrivent `[a]` au lieu de `<a, a>`
  (#57).
- [x] 15 : la lecture par mot devient le point 46.
- [x] 18 : l'analyse des chiffres reste quadratique : on ne fait rien.
- [x] 24 : les opérandes vides ne lèvent plus FE_INVALID (#50, fusionnée).
- [x] 31 : les correctifs pour CORE-MATH et la glibc sont écrits dans
  `3rd/README.md` (#56).
- [x] 39, 42, 43 : cas restants corrigés directement dans `configure-clean`.
- [x] 47 : correctif d'un mot (`_WIN32`, marqué `/* GAOL */`) dans `cbrt.c`,
  `rsqrt.c` et `asinpi.c`, et un job clang-cl dans `windows.yml`.
- [x] Commits : au nom de Jordan08, une ou deux lignes, jamais de
  `Co-Authored-By` (écrit dans `.claude/CLAUDE.md`).
- [x] Les pull requests des corrections vont vers `configure-clean`.
- [x] Les questions ouvertes de chaque pull request sont reportées dans le
  `TODO.md` de `todo-status` dès qu'elles sont écrites (ce `TODO.md` est
  `todo-notes/TODO-todo-status.md` du commit 16a2f60).

### 3.2 Le 3 octobre : les questions ouvertes tranchées

Toutes les questions ouvertes du `TODO.md`, de la branche `todo-status` (son
`TODO.md` est `todo-notes/TODO-todo-status.md` du commit 16a2f60) et des
descriptions des pull requests fusionnées (#31 à #63) ont reçu une décision ou
ont été reportées. Chaque point du `TODO.md` de `configure-clean` porte sa
décision (« Décidé le 3 octobre »), et une question reportée renvoie à son issue
(commits de3c467, b293860, e5d1c0b et 1a21755 ; les corrections laissées par les
relectures des pull requests y sont les points 51 à 67, et ce qui restait de
`TODO_mistral.md` les points 68 à 71).

#### Reportées, une issue chacune

- #64 : syntaxe d'`operator>>` pour relire `os << x << ' ' << y` (point 46).
- #65 : qui envoie les correctifs à CORE-MATH, et comment (point 31). Le patch 7
  (`exact_pow()`, #63) part avec eux.
- #66 : joindre ou non l'échec de `pow --rndd` et la borne de `asinpi_acc()`
  (point 31).
- #67 : décorations ou indicateur de sortie de domaine (point 26).
- #68 : denormals-are-zero hors du lecteur, et la question de #58 sur l'écriture
  (point 45).
- #69 : signaler à mingw-w64 son `fma()` et son `round()` pour msvcrt (point 6).
- #70 : spécialiser ou non `std::less` (point 22).

#### Décidées

**Sans point**
- Les choix « à confirmer » de #29 sont acceptés.
- Licence : GAOL v5 reste sous LGPL, CORE-MATH (`3rd/math-core`) sous MIT. À
  dire clairement dans `README.md`, à côté de `COPYING.LIB` et dans le manuel.
- 34 : une fois `configure-clean` fini, fusionner `configure-clean` dans
  `MATH-CORE`, puis `MATH-CORE` dans `master`, puis étiqueter `v5.0.0`. Les
  recettes gardent `master` jusqu'à l'étiquette.

**pow, puissances, racines**
- 1 : le bloc au-delà des int reste dans `pow_standard()`.
- 2 : d'abord les corps sans vérification (`nth_root`, `modulo_k_pi()`), puis
  l'analyse des coins étendue aux bornes infinies pour `pow`.
- 3 (#63) : la racine carrée de SSE2 sur tout x86 32 bits ; pas de recherche de
  cas durs.
- 8 : `ipow_exact_dn(0)` exact ; l'ancien code SSE2 reste sous `#if 0` ; la
  garantie 5 n log2(n) 2^-104 reste.
- 9 : `doc/accuracy.md` dit le cas des largeurs entre `pi_dn` et π théorique ;
  pas de drapeau `narrower_than_pi` ; les tests de `cos_or_sin()` sont dits
  mesurés.
- 50 (nouveau) : sortie anticipée de `hausdorff()` à borne infinie.

**FTZ, DAZ, exceptions flottantes**
- 4 : `gaol.pc` garde `-mno-daz-ftz` ; `-funsafe-math-optimizations` avec GCC
  9.4 est seulement documenté ; le moins unaire reste sans sonde.
- 5 : témoin positif pour `refused_options` ; essayer une vérification à
  l'exécution dans `gaol_init_cleanup.h`.
- 11 : la marque `\newinvfive` du manuel reste ; les tests DAZ tournent aussi
  sur AArch64 et ARM32.
- 24 : `is_empty()` avec `std::isunordered()` sur ARM 32 bits et POWER9 ;
  corriger `0 × oo` et le `pow` à exposant extrême ; première comparaison de
  `interval(l, r)` silencieuse ; `x &= y` avec un seul `std::isunordered()` ;
  relations sans FE_INVALID même vectorisées ; les intervalles de flottants
  gardent leurs comparaisons signalantes, sauf `intervalf::is_empty()` ; les
  trois vérifications de choix de #60 restent.

**Plateformes**
- 6 : pas de MXCSR sur x86 32 bits ; suppression de `GAOL_RND_MINGW_FENV_ONLY` ;
  mingw-w64 ARM64 refusé avant 12 ou sans `_UCRT` ; refus du x86 32 bits avant
  11 gardé ; `#warning` de `cbrt.c`, `rsqrt.c` et `asinpi.c` tu localement.
- 31 : correctif 6 sous sa forme large, après examen de `powl.c` ; `sinpi.c` et
  `log1p.c` embarqués changés dès maintenant.
- 47 : refus de clang-cl sans `/fp:strict` ; correctif 6 en entier pour Cygwin ;
  clang-cl de VS 2026 x64, x86 et arm64 ajoutés à la CI.
- GCC 12.2 sur aarch64 (#34) : contournement laissé tel quel.

**Flux, texte, exceptions**
- 12 : faire taire l'avertissement de Clang 18 sur `gaol_nerrs`.
- 14 : pointeur nul traité comme une absence d'explication ; `what()` sans
  explication, ou avec une explication vide, renvoie le nom de la classe dérivée
  ; la forme courte d'`operator<<` reste, sans réserve sur GAOL 4.
- 15 : `exceptions(eofbit)` accepté tel quel ; `\v` et `\f` laissés.
- 16 : corriger les formats `width` et `agreeing` ; `intervalToText` suit
  `interval::precision()` ; un point s'écrit `[a]`, en `hexa` aussi, et un point
  nul `[0]` ; sous une locale à virgule, garde sur `numpunct::decimal_point()`
  et texte avec un point ; pas de format de plus.
- 18 : la forme incertaine reste quadratique.

**Builds, tests, CI**
- 29 : contrôle `comma-locale.sh check` après les tests là où la locale existe ;
  `TIMEOUT` de 400 s pour `numbers` sous macOS Debug ASan+UBSan, en gardant 30
  tirages en Debug (le commentaire de `d78af72`, qui les attribue à la lenteur
  des flux, est à corriger) ; jobs Ubuntu 22.04 vers `ubuntu-24.04`.
- 41 : corriger le sous-projet meson.
- 42 : adoucir le texte sur l'alias `python.exe` ; ajouter au manuel le cas
  résiduel de `USERPROFILE` ; la documentation plutôt qu'un changement de
  l'ordre `python3`, `python` est confirmée.
- 43 : pas d'étape `pwsh` ; l'étape autoconf de `build-systems.yml` est limitée
  à `matrix.cfg.configure == ''` ; les `?` du message de `configure.ac` et le
  `.strip()` de meson restent.
- 44 : compilateurs passés par `cmake -E env` ; `package_source` dépend d'une
  reconfiguration ; le contrôle reste un test CTest.
- 7 : `atanh([1])` et `atanh([-1])` ajoutés à `special_cases.md` avec le point 33.
- 28 : traits GAOL pour YalAA et backend intervalle pour VNODE-LP, plutôt que de
  promouvoir les en-têtes des exemples.
- 40 : `run_examples.py` va dans `manual/`.

**Ménage**
- CI : `push` seulement sur `master`, `MATH-CORE` et `configure-clean`.
- Branches : les fusionnées, les jetables et `fix-path-core-math` sont
  supprimées, après vérification de chacune ; restent celles des points 12, 17
  et 36, en cours.
- Lignes de crédit : retirées des descriptions de #50, #51, #53 à #57, #59, d'un
  commentaire de #59 et de cette issue ; il en reste dans #58 et #60 à #63.
- Pull request de synthèse : suivre les propositions des relectures pour les
  puces de `doc/differences.md` et les marques `\newinvfive` (`atanh`,
  `hausdorff`).
- Les scripts de `todo-notes/orchestration/` restent dans `todo-status`. Le 4
  octobre, ils ont été ramenés avec les rapports dans `todo-notes/` de
  `configure-clean`, et la branche `todo-status` supprimée.
- Cette issue reste ouverte comme suivi d'ensemble.

#### Tri de `TODO_mistral.md`

Le fichier est supprimé (1a21755). Ses tâches étaient déjà faites (`x op= x`,
refus de `-ffinite-math-only`, FTZ/DAZ, `atanh`, `pow` sous-normal,
`VERSION.txt`, validation par la CI), déjà dans le `TODO.md` (points 22, 25, 34,
35, 36, 38 et la PR de synthèse), écartées (`gaol::Box`, contraire au point 28 ;
une branche de version ; la signature `mid_rad` proposée), ou nouvelles :
- 34 : après v5.0.0, le développement continue dans `configure-clean`, et
  `master` ne reçoit que les versions ; pas de GitHub Release, l'étiquette
  suffit.
- 35 : le tutoriel va dans `examples/tutorial.md`.
- 38 : les pièges vont aussi, en bref, dans `doc/using.md`.
- 68 (nouveau) : un job GCC et un job Clang en `-Wall -Wextra -Werror`.
- 69 (nouveau) : badges de la CI dans `README.md`, sans couverture.
- 70 (nouveau) : site de documentation sur GitHub Pages (`doc/*.md` et le manuel
  en PDF).
- 71 (nouveau) : annoncer v5.0.0 à Frédéric Goualard seulement.
- Pas de vérification des liens de la documentation.

### 3.3 Le 4 octobre

- [x] Dans `configure-clean` : `nb_fp_numbers()` rend un `unsigned long long`,
  sans compatibilité pour `ULONGLONGINT` ni pour les anciens noms des macros ;
  jobs AppleClang (`-Wall -Wextra -Werror`) et Visual C++ x64 (`/W4 /WX`) ;
  `gaol_parameters.h` supprimé ; `gaol_interval2f.h` et
  `manual/v5/relation-cos.tex` en LF
- [x] Les constantes `double` de `gaol_port.h` (`pi`, `half_pi`, `two_pi`,
  `ln2_dn`...) retirées de l'interface : fait par #77 ; il ne reste que `pi_dn`,
  `pi_up`, `half_pi_dn` et `half_pi_up`, dans l'espace de noms `detail`, et le
  manuel renvoie à `interval::pi()`...
- [x] Un job Visual C++ x86 en `/W4 /WX`, `size_t` y ayant 32 bits : fait dans
  `configure-clean` (8ccc515), avec la correction de l'avertissement C4310 qu'il
  a trouvé (7567f87)
- [x] Plus de `-lm` explicite dans les builds autotools et meson ni dans
  `gaol.pc` : fait par #78. GAOL appelle toujours la bibliothèque mathématique
  du système (`<fenv.h>`, `sqrt()`, `floor()`, `round()`, `fma()` sans FMA
  matériel...), même avec CORE-MATH, mais g++ et clang++ la lient eux-mêmes,
  comme le build CMake le supposait déjà

#### Les questions restées ouvertes dans les rapports (commentaire de #49)

Les rapports de `todo-notes/` (29 septembre au 2 octobre) ont été relus. Toutes
leurs questions avaient reçu une réponse le 3 octobre, sauf celles-ci, posées le
4 octobre. Les décisions sont dans `TODO.md` (commit 065191e).

- **Signaler à GCC** la comparaison silencieuse rendue signalante par
  l'if-conversion, sur ARM 32 bits et POWER9 (#60) : reporté, issue #80 (point
  D.24).
- **Les masques `~0ul` et `1ul<<52`** de `binary80/atan2/atan2l.c` et de
  `binary128/expm1/expm1q.c` : à examiner avant d'envoyer le correctif 1 à
  CORE-MATH, et à ajouter au correctif s'ils ont le même défaut (point A.31).
- **Les deux limites du refus de mingw-w64** (un programme qui définit `_UCRT`
  lui-même en se liant à `msvcrt.dll`, un instantané git qui se dit 12 mais date
  d'avant le déplacement de `fma.c` et `round.c`) : acceptées ; la seconde est à
  écrire dans `gaol/gaol_config.h`, à côté de la première (point A.6).
- **`chi([-oo, +oo])`** reste 1, comme dans GAOL 4 et le manuel (point U.40).
- **L'indice « UTF-16 »** que les quatre lecteurs de `VERSION.txt` donnent aussi
  à un fichier UTF-32 : gardé tel quel.
- **Les scripts de vérification du point 1** (`diffpow.cpp`, `gen_table.py`,
  `checkrows.py`, `run_mutations.py`, `mutate2.py`), qui n'existaient plus que
  dans un répertoire temporaire : à ranger, nettoyés, dans `tests/tools/`.
- **Des corrections relevées par les relectures et perdues du TODO.md le 2
  octobre**, ajoutées :
  - G.5 : deux phrases ambiguës sur `-fno-fast-math` (`doc/three-builds.md`,
    `doc/using.md` et le manuel), et le `make test` de CMake, qui lance aussi
    les tests de compilation (`doc/building.md`) ;
  - B.8 : la phrase de `doc/accuracy.md` et du manuel sur les bornes prises aux
    produits arrondis ;
  - S.14 : `explanation()` appelée deux fois par `operator<<` d'une exception ;
  - U.40 : le commentaire du format hexa dans `gaol/gaol_interval.cpp`, faux
    depuis #71.

Corrigé aussi dans `TODO.md` : E.12 (le `#pragma` sur `gaol_nerrs` est déjà là
depuis #76 : le garder à la régénération du parser) et Y (marquer aussi
**Fixed** les n° 2, 7, 16 et 22 de la revue d'`examples/examples.md`).

Les rapports sont retirés de l'arbre et restent au commit 16a2f60. Ce qui en
sert encore est dans `todo-notes/synthese.md` (dcd3b11) : les textes proposés
pour `ChangeLog` et `doc/differences.md`, et les notes utiles aux points
restants.

### 3.4 Du 5 au 24 octobre : ce qui a été décidé pendant chaque point

Chaque titre dit la date, le point (lettre et numéro) et la pull request qui a
fait le travail.

#### Décidé le 5 octobre (fin des anciens 24 et 62 du point D), fait par #83

- [x] Sous Visual C++ x64 et x86, les comparaisons silencieuses de GAOL sont des
  `ucomisd` en ligne (`detail::quiet_less()`... de `gaol_port.h`), plus des
  appels à `_dpcomp()` : `x * y` en x64 passe de 31,5 à 11,0 fois le double (VS
  2026), mieux qu'avant le point (16,3)
- [x] `div_rel` des intervalles SSE2 ne calcule plus la moitié de division qu'il
  jette (+oo/+oo, ou un réel divisé par une borne nulle), aux mêmes bornes bit à
  bit
- [x] Sur ARM 32 bits et POWER9, le constructeur des intervalles FPU teste
  `std::isunordered()` d'abord, comme `is_empty()`
- [x] `GAOL_TESTS_CHOICES` retiré : des jobs armhf et POWER9 compilés pour la
  taille, et un job POWER9 (Ubuntu 24.04, GCC 13.3, sous qemu), vérifient les
  choix sur `is_empty()`
- [x] Le bug de GCC 12.1 à 12.3 et 13.1 à 13.2 (avec `-frounding-math` sur x86,
  `denorm_min()` dans un tableau statique de structures) dit dans `doc/using.md`
  et le manuel
- [x] Le `<` de `cr_pow` sur un NaN commenté dans #65 (maintenant #125), et la
  vectorisation des comparaisons silencieuses par GCC dans #80 (maintenant #126)
- [x] Les coûts hors Visual C++ acceptés ; la correction de `mid()` gardée avec
  celle de `midpoint()`

#### Décidé le 5 octobre (fin du point H), fait par #86

- [x] La formule de Dekker de TwoProd à quatre termes dans l'exemple 13 : celle
  à trois termes est fausse même au plus proche pour des opérandes de magnitudes
  très différentes ; TwoProd mesuré sur des paires de [1, 2), celles d'exposants
  très différents n'y perdant pas leur exactitude ; le nom
  `gaol_core::rounding_guard` ; la partie manuel du point H reportée au point I
- [x] Le « about 7 ns » d'`examples/examples.md` (l. 509) pour un bloc au plus
  proche, contredit par la mesure (11 à 13 ns) : à corriger au point I

#### Décidé le 5 octobre (fin de l'ancien 21 du point D, et les en-têtes)

- [x] Les fonctions internes de GAOL dans `gaol_detail`, qu'un `using namespace
  gaol` n'amène pas : un `namespace detail` du programme était ambigu avec
  `gaol_core::detail` de #77 (fait par #84)
- [x] Une borne `long double` à côté d'un entier reste arrondie, ce que la
  documentation dit ; `x + n` pour un n au-delà de 2^53 reste un encadrement,
  sans être le plus étroit
- [x] `nth_root(x, q)`, `rootn(x, q)`, `pownRev(c, x, p)` et le `pow(e, n)`,
  `pown(e, n)`, `nth_root(e, n)` des expressions pour tout entier, comme `pow` :
  fait par #85, dans une pull request à part
- [x] Le nettoyage des en-têtes, fait par #87 : `GAOL_PUBLIC`, vide sous Visual
  C++ ; `gaol` nomme un par un les noms de GAOL 4 (dont `NaN_val`, mais pas les
  internes que son manuel ne documentait pas : `f_negate()`, `uintdouble`,
  `prec_t`...), les nouvelles fonctions sur les intervalles et leurs nœuds,
  `restore_rounding()`, `exact_string()` et `rnd_keep()` ; `gaol_allocator.h`
  supprimé (ni Codac ni IBEX ne s'en servent) ; `static_cast<>` pour les casts à
  la C, que `headers.sh` vérifie

#### Décidé le 6 octobre (fin du point R)

- [x] La phrase de `doc/using.md` (l. 445) et du manuel selon laquelle chaque
  opération laisse le sens d'arrondi vers le haut, à compléter au point I :
  celles qui rendent avant leur vérification (un opérande vide, `hausdorff()`
  quand une borne n'est infinie que d'un côté) le laissent tel qu'elles l'ont
  trouvé : fait par #140
- [x] La double vérification du sens d'arrondi de `asinh_rel()` et
  `atanh_rel()`, qui appellent `asinh()` et `atanh()` après la leur, ajoutée au
  point B.2 : fait par #117, avec `acosh_rel()` et `nth_root_rel()`
- [x] `atanh([1])` et `atanh([-1])` dans `doc/compare/special_cases.md` (reste
  de l'ancien 7), prévus au moment du point 33 : faits par #139, sans attendre

#### Décidé le 6 octobre (point Q, #68), fait par #100

- [x] Les fonctions qui comparent des bornes sans calculer les comparent par
  leurs bits là où elles se comparent égales, sans écrire dans MXCSR, plutôt que
  par la sonde (3 à 4,5 ns de plus par appel) ; de même les arrondis à un
  entier, `is_an_int`, `is_canonical`, `feven`, `nb_fp_numbers`, `inf` et `sup`
- [x] L'écriture, `midpoint()`, `split()` et `chi()` effacent les modes le temps
  de l'opération (la question de #58)
- [x] Le chemin AVX-512 corrigé dans Q, avec un job Clang et
  `GAOL_PRESERVE_ROUNDING` sur x86_64 dans `linux.yml`
- [x] Là où GAOL ne peut pas effacer les modes (Visual C++ pour ARM64), une
  borne sous-normale écrite vers l'extérieur (2^-1022 ou 0), testée sous
  `_controlfp_s()`
- [x] `gaol_performance` imprime le temps de la sonde avec 2^-1060 et avec 2^-60
  : aussi long sur les 68 jobs de la CI qui tournent sur un processeur
- [x] Le test de `<a, b>` du parser, par les bits : fait au point E par #105
- [x] Vérifier avec `ctest -V -R numbers` que le test de FZ tourne dans les jobs
  Visual C++ arm64, au point F (11) : fait par #114, vérifié une fois par #116

#### Décidé pendant le point P (11 et 12 octobre), fait par #99

- [x] Le littéral `_iv` encadre le nombre écrit (opérateur littéral brut) ;
  Visual C++ refuse les entiers au-delà de 2^64 - 1, ce qui est documenté
- [x] `bisect` lève `invalid_action_error` pour un ratio hors de ]0, 1[
- [x] Des deux versions de l'ancien 25i, `atan2_rel_y` et `atan2_rel_x` sont
  gardés
- [x] `gaol_ldexp()` (`gaol/gaol_ldexp.h`) donné à CORE-MATH avec tous les
  compilateurs, à la place du `ldexp()` de la bibliothèque C
- [x] C4098 désactivé pour `erfc.c` (`/wd4098`), qui reste celui de CORE-MATH
- [x] La relecture de `gaol/core_math_port.h` : sous Windows 32 bits, chaque
  `FE_*` traduit en son bit de MXCSR ; commentaires mis à jour
- [x] `headers.sh` en locale C ; « Jordan NININ » au lieu de « Joran » dans les
  61 fichiers de 68ec12c
- [x] `.continue/rules`, copie de `process.md` ajoutée par 68ec12c, retiré de
  `configure-clean` par #103
- [x] Les questions restées ouvertes dans la description de #99 (l'exception de
  `bisect`, `010_iv`, la politique de Boost, la colonne GAOL v5 de
  `doc/compare/performance.md`, la précision de `pow` de `doc/accuracy.md`, le
  tableau des outils de `process.md`) : quatre réglées le 18 octobre (voir 3.4),
  les deux autres faites par #164 (117, la phrase de `pow`) et #155 (118,
  `process.md`)

#### Décidé à la fin du point E (12 octobre), fait par #105 et #108

- [x] `YYMAXDEPTH` reste à 10 000 : environ 10 000 parenthèses, 5 000 appels ou
  3 300 niveaux de `1+(1+(...))` sont lus, au-delà `input_format_error`
- [x] `eval_stack` et `expr_eval` ne sont plus copiables (la pile de
  l'évaluation grandit dans le tas)
- [x] Un lexème de plus de 2^29 caractères lève `std::bad_alloc` (le tampon de
  flex a une taille `int`)
- [x] Pas de test du temps de lecture d'un long lexème, une limite de temps
  étant fragile ; la mesure est dans #105
- [x] Les vérifications d'échecs d'allocation passées là où la bibliothèque C++
  alloue dans les déplacements `noexcept` de ses conteneurs (Visual C++ Debug),
  ce que le test vérifie à l'exécution
- [x] Le parser régénéré avec bison 3.8.2, celui d'Ubuntu 26.04 : fait par #108
- [x] `headers.sh` sous une locale française : déjà corrigé par #99 (#106,
  fermée)
- [x] `parse_interval(nullptr)` plantait (#107) : fait par #147 (point 114) ;
  l'issue #107 est restée ouverte
- [x] `process.md` (arbre GAOL_V9, outils d'Ubuntu 26.04, état) : fait par #109
  ; le job de CI pour meson 0.53.2 : fait par #110 ; `--no-track` à la section
  7.3 reste ouvert

#### Décidé à la fin du point A (12 octobre), fait par #111, #112 et #113

- [x] `configure` régénéré par `autoconf` seul quand seul `configure.ac` change
  (le libtool de Debian d'Ubuntu 26.04 entrait sinon dans `configure`), et
  `check_branch` qui prend l'arbre courant : fait par #112
- [x] Un job de la CI qui donne à CORE-MATH les `FE_*` de Cygwin, pour que le
  correctif 6 ait des dents : fait par #111
- [x] `atan2l.c` dans le correctif 1, son échec dans les arrondis dirigés étant
  à signaler aux auteurs avec l'envoi : fait par #111 ; #65 mise à jour
- [x] Chaque ligne changée de CORE-MATH marquée `/* GAOL */` : fait par #113

#### Audit du 13 octobre, et décidé pendant le point Z (13 et 14 octobre), fait par #119 et #120

- [x] L'audit de ce qu'annoncent `TODO.md` et les descriptions des pull requests
  pour les points faits (A, C à H, J à V) : le code de chacun est dans
  `configure-clean` (`f469c4e`), avec ses tests ; trois textes faux corrigés par
  #119
- [x] Les options de `gaol_configuration.h` toujours définies, à 0 ou 1, par un
  même bloc `#ifndef` dans les trois gabarits, plutôt que `#cmakedefine01`, qui
  entrait en conflit avec le `-DGAOL_DEBUGGING=1` de CMake en Debug : fait par
  #120
- [x] Un `#ifdef GAOL_PRESERVE_ROUNDING` d'un programme devient toujours vrai :
  l'exemple 13 lit l'option par `#if`, et `doc/differences.md` le dira (texte
  dans #120)
- [x] La portée : les en-têtes installés et `-Wundef` dans `headers.sh`, vérifié
  dans les logs de la CI (GCC et Clang sous Linux, AppleClang arm64) ; ni les
  tests ni les `.cpp` de la bibliothèque
- [x] `bisect(ratio)` par les `quiet_*` de `gaol_detail` : fait par #120
- [x] `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT` laissés à `<limits.h>`,
  sans valeur par défaut (question ouverte de #120) : point AM (125), clos le 22
  octobre (Q3), l'en-tête installé `gaol/gaol_config.h` a déjà les valeurs par
  défaut

#### Décidé le 18 octobre (points I et AF, et deux points nouveaux)

- [x] Point I, ancien 38, fait par #140 : `interval::format()` et
  `interval::precision()` restent non atomiques, l'appelant les synchronise
  (avant de lancer les threads, ou sous un verrou de son programme) ; à
  documenter avec les pièges de « Common errors » et de `doc/using.md`, sans
  changement de code
- [x] Point AF, ancien 103, fait par #140 : +0 et −0 sont la même valeur, et le
  signe d'une borne nulle de `x * d` entre les chemins AVX-512 et SSE2 est une
  exception acceptable au « bit for bit » ; à dire dans la documentation et les
  commentaires, sans changer le calcul
- [x] Le site GitHub Pages (ancien 70) sorti du point I, à la demande du
  mainteneur : point AK, après les textes de I
- [x] Une question du mainteneur sur le sens d'arrondi (ancien 113), notée comme
  étude : point AJ ; rien n'y est écrit ni mesuré

#### Décidé pendant le point AB.77 (19 octobre), fait par #144

- [x] `RHO2` de `exp_1()` abaissé, la seconde des deux solutions du TODO, plutôt
  qu'un test de M à chaque appel : une constante, rien de plus sur le chemin
  rapide, la preuve d'erreur du lemme 7 de CORE-MATH valable sur un
  sous-intervalle ; sa valeur est fixée au double près par `tests/core_math.cpp`
  (128 doubles consécutifs à la frontière, deux variantes d'un et de deux
  doubles trop hautes détectées)
- [x] Les cas dans une table `powers_near_overflow[]` de
  `tests/rounding_direction.cpp` à côté de `extreme_powers[]`, et non dedans :
  décidé le 19 octobre
- [x] Le correctif 8 de `3rd/README.md` écrit contre le master de CORE-MATH du
  19 octobre (`6fc21797`), auquel il s'applique tel quel ; il n'est pas envoyé
  (#125)
- [x] Le commentaire gâché de `gaol/gaol_roundeven.h` (ligne 73, «
  doublegaol_cr_ ») : décidé le 19 octobre, corrigé le 23 octobre par le commit
  direct 79746c2 sur `configure-clean` (hors du groupe C de #142, fait par #164)

#### Décidé pendant le point AE.90 (19 octobre), fait par #145

- [x] Les deux vérifications (Clang pour ARM 32 bits, `-frounding-math` non
  honoré) portent sur le compilateur C++ puis sur le compilateur C, dans les
  trois builds, avec le même message à un mot près (« the C compiler » ou « the
  C++ compiler »)
- [x] `configure` garde les `CFLAGS` et `CXXFLAGS` de l'utilisateur pour les
  tests de `-frounding-math`, comme CMake et meson (décidé le 19 octobre) : un
  `--target=` ou un `-m32` y change ce que le compilateur honore
- [x] Le test a des dents dans la CI, non dans `tests/` : les jobs `clang-arm32`
  (Debian 13 armhf, Clang 19.1.7) et `clang-14-arm64` (Ubuntu 24.04, Clang
  14.0.6) vérifient neuf refus chacun

#### Décidé pendant les points AI.106 et AI.107 (19 octobre), fait par #146

- [x] Les intervalles NEON se choisissent par `GAOL_ARM64`, `--enable-arm64` et
  `enable-arm64`, indépendamment de `GAOL_SIMD`, qui reste pour x86 (décision du
  mainteneur ; remplacée le 26 octobre par #200 : `GAOL_SIMD` les choisit)
- [x] NEON est pris par défaut sur AArch64 avec GCC, Clang et AppleClang sous
  Linux et macOS ; pas avec Visual C++, clang-cl, MinGW-w64, ARM 32 bits (le
  NEON d'ARMv7 n'a pas de vecteur de doubles) ni dans un build universel de
  macOS, où les intervalles FPU servent comme avant
- [x] Le signe d'un zéro en borne peut différer de celui des intervalles FPU et
  SSE2 avec NEON (le maximum d'arm64 classe +0 au-dessus de −0), comme avec
  AVX-512 : documenté, sans changer le calcul

#### Décidé pendant le point AI.106 (20 octobre), fait par #148

- [x] Le commit `fbb962c` (« remove old code », d'une autre session, sur la
  branche de #148) est gardé tel quel ; le chemin AVX-512 reste offert à `cl`
  comme à clang-cl ; les masques `m128d_02mask` et `m128d_20mask` sont supprimés
  ; `__vectorcall` est fait dans #148
- [x] Les exécuteurs de Windows n'ayant pas AVX-512, le chemin AVX-512 de Visual
  C++ et de clang-cl est exécuté par des jobs sous Intel SDE, dans #148
  (l'action `petarpetrovt/setup-sde`, épinglée sur un commit, dont la licence
  d'Intel est acceptée par le projet)
- [x] `GAOL_PRESERVE_ROUNDING` prend par défaut le chemin AVX-512 avec `cl`
  quand la machine qui compile l'a, comme sous Linux (`-DGAOL_PREFER_AVX512=OFF`
  pour un paquet pour tous les processeurs)
- [x] `/volatile:iso` est une erreur de compilation du chemin AVX-512 de `cl` :
  l'ordre des opérations y repose sur `/volatile:ms`, son défaut pour x86 et x64
- [x] Le point 109 (les macros qui n'écrivent que MXCSR, avec `_M_X64` et
  `__AVX2__` pour `cl`) : dans une pull request qui suit #148

#### Décidé pendant les points AD.83 et AD.84 (20 octobre), fait par #149

- [x] Deux nombres qui ne sont pas des doubles et tombent entre les mêmes deux
  doubles sont départagés exactement dans #149 (le lexeur donne au parser le
  rang du texte de chaque nombre, le contexte de la lecture le garde,
  `compare_numbers()` compare), plutôt que laissés à l'intervalle des doubles
  autour
- [x] `<a, b>` refuse une borne qui n'est pas un double (`<0.1,
  0x1.9999999999999p-4>` était lu), dans #149
- [x] Lire `1e400`, ou tout nombre au-delà du plus grand double, lève
  `FE_OVERFLOW` (le `strtod()` du lexeur) : ajouté au point 79, non corrigé dans
  #149
- [x] Le second argument entier de `nth_root`, `pown` et `rootn` lu dans un
  texte : les surcharges `long long`, saturé à ±(2^63 − 2) à partir de 2^63 (les
  puissances sont les mêmes, les racines d'ordre au-delà contiennent 1) ; `[l,
  u]` avec l > u est vide dans les deux espaces de noms, `gaol::textToInterval`
  compris, comme décidé le 14 octobre

#### Décidé pendant le point AI.106 pour ARM 32 bits (20 octobre), fait par #150

- [x] La lecture et l'écriture directes du registre de contrôle d'ARM 32 bits
  restent une macro interne documentée, `GAOL_RND_FPSCR_DIRECT`, et non une
  option des trois builds (décision du mainteneur) ; elle vaut 1 avec GCC et une
  unité de doubles, jamais avec Clang (refusé sur ARM 32 bits),
  `-mfloat-abi=soft` ni une unité à simple précision
- [x] La vérification par défaut de chaque opération reste la sonde par une
  addition sur ARM 32 bits, `vmrs` y coûtant plus que les deux additions (`x +
  y` 6,1 ns contre 4,1) ; sur arm64, c'est la lecture par `mrs`, comme dans #146
- [x] Les tests de FPCR sont tous portés à FPSCR, et la CI mesure deux builds
  alternés (job `control-register-performance`) : décidé par le mainteneur
- [x] Le défaut de `width()` et de `bisect(ratio)` est corrigé dans #150 ; #153,
  qui avait la même correction, est fermée sans fusion le 20 octobre, sa branche
  supprimée

#### Décidé pendant les points AB.78 et AB.80 (20 octobre), fait par #151

- [x] La correction du 78 va au-delà de la formule du TODO (« e = 0 quand la
  somme est infinie ») : le Fast2Sum de Dekker, l'opérande de plus grand module
  en premier, corrige aussi la somme finie à mi-chemin de deux doubles que la
  relecture a trouvée (`cancel_minus([0, DBL_MAX], [0, 3·2^970])` levait
  `FE_INVALID`, `cancel_minus([DBL_MAX − 2^971, DBL_MAX], [2^970, 3·2^970])`
  rendait `[-oo, +oo]`) ; gardée par la fusion
- [x] Le test du 78 est une table de 9 cas, `cancels_next_to_the_largest[]`,
  dans `tests/rounding_direction.cpp`, avec le drapeau, l'exception activée dans
  un processus fils et les bornes les plus serrées ; les bornes attendues sont
  dérivées à la main et ont été recalculées en entiers exacts par la relecture
- [x] Le 80 : `1/[0, 1]` est retiré de `doc/using.md` et du manuel, où il ne
  reste que `log([0, 1])` et `[1e308]*10` ; la phrase d'introduction (« An
  infinite bound comes from a division by zero or from an overflow ») n'est pas
  une règle : voir la section 6 (#151)

#### Décidé pendant le point AL.118 (20 octobre), fait par #155

- [x] Le paragraphe du 12 octobre du §6.1 est retiré, comme décidé le 18 octobre
  ; il restait vrai pour l'arbre `GAOL_V9/GAOL` (Ubuntu 24.04, sans autoconf,
  automake, libtool, meson ni ninja) : voir la section 6 (#155)
- [x] L'en-tête « État au 13 octobre » de `process.md` devient un renvoi au §12,
  plutôt qu'une date et un commit qui se périmeraient à chaque fusion
- [x] Les §2 et §9 de `process.md`, hors de la liste du point 118, sont gardés
  par la fusion

#### Décidé pendant le point AC.81 (20 octobre), fait par #158

- [x] Le TODO offrait de prouver avec une puissance mise à l'échelle ou de
  corriger les textes : la correction du code est gardée par la fusion, la
  promesse de `doc/accuracy.md` étant tenue pour tout x. Les bornes de
  `pown_point()` ne sont pas reprises : `n` y est un `int` et sa borne d'erreur
  K suppose m ≤ 2^31, alors que `nth_root` prend un `unsigned int`
- [x] Le seuil est 2^-1020, quatre fois le plus petit normal, et non 1e-300 :
  au-dessus, les puissances voisines de la racine restent normales et le
  résultat est le même (vérifié par la relecture sur 21,2 millions de candidats
  par plage) ; les x de [2^-1022, 2^-1020[ passent encore par le chemin mis à
  l'échelle
- [x] Les recherches `proved_root_dn()` et `proved_root_up()` sont des gabarits
  sur le type de preuves, choisi une fois d'après x : avec le test de x dans les
  boucles, la racine d'un x ordinaire coûtait 3 ns de plus sur 80 dans un build
  et 28 dans un autre du même code (AppleClang 21, arm64)
- [x] Une borne d'une racine peut être à 2 doubles de la plus serrée (4 pour
  `rootn(x, q)` avec q < 0), pour tout x et avant #158 : l'erreur relative d'un
  produit arrondi dans un sens dirigé va jusqu'à 2^-52. Le commentaire de
  `newton_root()` et la colonne « Tightness » de `doc/accuracy.md` le disent ;
  la même phrase reste dans `doc/differences.md` et le manuel
- [x] Le test donne ses x directement, le `pow()` de macOS n'étant pas exact en
  arrondi dirigé pour certains résultats sous-normaux, et vérifie l'égalité des
  racines exactes (six cas ; la mutation `<` à la place de `<=` échoue)

#### Décidé pendant le point AI.106 (22 octobre), fait par #157

- [x] Garder tous les noyaux de #157 tels quels, `x - y`, le quotient, `%`, le
  carré et le produit, sans changement de code, malgré les pertes mesurées sur
  la CI (une ligne de produits de signes prévisibles, le quotient avec Visual
  C++ arm64, le carré avec Clang, le sens préservé sur Visual C++ x86) : la
  réponse Q1 du 22 octobre (mesurer d'abord sur armhf, POWER, s390x et RISC-V)
  est suivie de mesures en alternance sur la CI (armhf natif, Visual C++ et
  clang-cl, Ubuntu et macOS en FPU) ; POWER, s390x et RISC-V restent non mesurés
- [x] Huit produits pour `x * y`, non quatre : un seul sens d'arrondi étant
  disponible, quatre produits à choix se compilent en branches sur les signes
  (deux avec GCC pour ARM 32 bits, trois avec Visual C++), ce que les huit
  produits n'ont pas
- [x] Le signe d'une borne nulle d'un produit n'est pas spécifié avec les
  intervalles FPU (normaliser le zéro après le noyau, `lb_ += 0.0`, est écarté :
  la ligne de sept `*` passe de +50 % à +63 %, et tous les zéros deviendraient
  +0, une troisième convention) ; dit dans `doc/using.md`, `doc/accuracy.md`,
  `doc/tests.md` et le commentaire de `larger_of()` ; le signe du quotient et
  celui du carré d'un intervalle qui contient 0 strictement sont fixés
- [x] Pas de `GAOL_RND_KEEP()` sur la sortie de `*=`, `/=` et `%=` (protection
  de fait par l'adresse de `*this` donnée aux fonctions des cas rares ; la
  remettre coûterait 2,6 et 5 ns avec `GAOL_PRESERVE_ROUNDING`) ;
  `inverse_upward()`, `mid()` et `div_rel()` gardent leur code
- [x] Les questions ouvertes de #157 (un noyau mixte, `gaol_performance` qui
  apprend ses opérandes, `fmax()`, `GAOL_RND_KEEP()`, `xsmaxcdp` sur un POWER9
  réel, la seconde relecture non faite) : à trier par le mainteneur, reprises en
  section 6 (#157) ; réponses du 23 octobre : « Décidé le 22 et le 23 octobre »
  plus bas et 3.6

#### Décidé pendant les points AI.111 et AI.115 (22 octobre), fait par #160

- [x] Mesurer par la CI deux builds alternés dans le même job (réponse Q6 du 22
  octobre), plutôt que n'afficher que le meilleur de plusieurs passages
- [x] La base est le premier parent du commit de fusion de la pull request, donc
  la tête de `configure-clean` au moment de la fusion calculée par GitHub (non
  `base.sha`, ni la tête de la branche) : la différence est celle de la pull
  request seule
- [x] Aucun temps ne fait échouer un job ; l'outil n'échoue que si les tableaux
  ne se comparent pas ; pas d'étape de plus dans les 81 jobs qui ont déjà une
  étape « Performance » (ils construiraient deux fois), et
  `tests/performance.cpp` n'est pas touché
- [x] Les questions ouvertes de #160 (le sens du rapport, le seuil de la liste,
  le filtre de chemins, les jobs absents, `gaol_bench` avec Visual C++, `perf`
  sur macOS, `LINE_LIST` vide, `process.md` §10) : à trier par le mainteneur,
  reprises en section 6 (#160) ; réponses du 23 octobre : « Décidé le 22 et le
  23 octobre » plus bas et 3.6

#### Décidé pendant le point AI.112 (22 octobre), fait par #161

- [x] Le défaut de CMake et les descriptions d'options du 112 sont faits
  maintenant ; le test des bits du signe de zéro de `x * d` ne l'est pas (il
  demande un x86 avec AVX-512 pour comparer les deux chemins)
- [x] Les descriptions disent « avec FMA » pour les puissances entières : elles
  n'empruntent le chemin AVX-512 que pour n hors de {-1, 0, 1} et avec
  `GAOL_POWN_PACKED`, donc `__FMA__` (vérifié par le prétraitement de
  `gaol_interval_core.cpp`)
- [x] Une valeur donnée pour `GAOL_PREFER_AVX512` reste jusqu'à
  `-UGAOL_PREFER_AVX512`, et le défaut se dérive à chaque `cmake` ; deux limites
  (le texte d'aide de CMake sert à reconnaître une demande, la variable
  ordinaire d'un projet englobant) sont écrites dans le commentaire de
  `CMakeLists.txt`
- [x] Les questions ouvertes de #161 (une option `AUTO`/`ON`/`OFF`, les
  commentaires de la CI qui disent encore « + - * / et sqrt »,
  `-UGAOL_PREFER_AVX512` de `doc/compare/code/setup.sh`, la relecture des
  corrections) : à trier par le mainteneur, reprises en section 6 (#161) ;
  réponses du 23 octobre : « Décidé le 22 et le 23 octobre » plus bas et 3.6

#### Décidé pendant la suite du point AN (22 et 23 octobre), fait par #162

- [x] `pow(x, 2)` par `sqr` avant la version (réponse Q10b du 22 octobre à la
  question de #154) ; seul n = 2 change : n = −2 par `inverse(sqr(x))` est
  écarté
- [x] Le signe d'un zéro de `pow(x, 2)` est celui de `sqr(x)` de chaque build,
  et ses indicateurs ceux de `sqr` (dit dans `doc/using.md` et
  `doc/accuracy.md`) ; le test exige le même résultat que `sqr(x)` bit à bit
  sans exiger un signe (leçon de #157)
- [x] Le test `e == 2` dans les fonctions en ligne des en-têtes est écarté (1,35
  ns au lieu de 1,66 avec NEON, mais un second endroit à tenir) : la décision
  reste dans `gaol_pown`
- [x] Les questions ouvertes de #162 (le test dans les en-têtes, les puissances
  3 à 5 avec la FPU à +0,3 ns, le sens d'arrondi après `pow(vide, 2)`) : à trier
  par le mainteneur, reprises en section 6 (#162) ; réponses du 23 octobre : «
  Décidé le 22 et le 23 octobre » plus bas et 3.6

#### Décidé pendant le point AI.110 (23 octobre), fait par #163

- [x] `-mfma -mprefer-vector-width=128` pour GCC pour Windows x86-64, plutôt que
  `-mfma` seul ou `-mstackrealign` (non essayé) : la propriété « aucune
  instruction de 256 bits » se vérifie par un compte à chaque build
- [x] Les défauts de `GAOL_PRESERVE_ROUNDING` avec mingw-w64 13 et plus sont
  corrigés dans la bibliothèque (GAOL lit et écrit les registres lui-même), et
  non évités en retirant le chemin MXCSR seul pour GCC pour Windows, ce qui
  aurait laissé le premier défaut ; x86 32 bits et Cygwin restent comme avant
- [x] Les questions ouvertes de #163 (le bogue 54412 de GCC,
  `GAOL_PRESERVE_ROUNDING` avec autotools et meson sous MSYS2, `-mavx512f` et
  `-march` donnés par l'utilisateur, Cygwin, x86 32 bits, `-mstackrealign`, les
  corrections non relues) : à trier par le mainteneur, reprises en section 6
  (#163) ; réponses du 23 octobre : « Décidé le 22 et le 23 octobre » plus bas
  et 3.6

#### Décidé le 22 et le 23 octobre (réponses aux questions des pull requests), fait par #165 à #174

- [x] Les seize questions du 22 octobre (Q1 à Q16 de `TODO.md`) et vingt-et-une
  du 23 octobre (les dix dernières pull requests, #152 à #164) sont répondues et
  reportées dans `TODO.md` (« Réponses du mainteneur ») ; les rangs passent à
  32, six tâches nouvelles (128 à 133, point AP)
- [x] `operator>>` : un intervalle par ligne, documenté, pas de changement (Q4, 46)
  ; décorations : rien, reporté indéfiniment (Q5, 26) ; l'étude des noyaux au
  plus proche (113) est reportée après la version (Q7)
- [x] Tâches nouvelles décidées le 22 octobre : `textToInterval(nullptr)` lève
  `input_format_error` (128), `sqrt_bounds()` sous `FPCR.AH` par un masque
  (129), `process.md` une phrase et un en-tête daté (130), les noyaux SSE2
  exportés dans `gaol_detail` (131), `GAOL_POWN_PACKED` sur arm64 (132) ; les
  suites de #156 (Q8 : lecture en ligne dans CORE-MATH, test de
  `read_control()`, outil de mutation, chemin de repli retiré) dans le 108
- [x] `GAOL_RND_KEEP()` n'est pas remis sur `*=`, `/=` et `%=` des intervalles
  FPU : un test tient la protection (#170, jobs FPU x86-64 GCC et Clang avec le
  sens préservé, arm64 GCC) ; le noyau mixte du produit est essayé (#168, fermée
  sans fusion le 23 octobre : le banc A/B de la CI montre 1,4 à 2,1 fois aux
  signes réguliers et 5 à 19 % de perte aux signes aléatoires) ; `fmax()`, la
  seconde relecture de `process.md` §7 et le changement des opérandes de
  `gaol_performance` écartés
- [x] `pow(x, 2)` : la décision reste dans `gaol_pown` ; le vide laisse le sens
  du programme dans toutes les formes de `pow` (#169) ; +0,8 ns pour `pow(x, 3)`
  sur x86 laissé et dit ; le carré des intervalles FPU, 1 ns trop lent depuis
  #162, retrouve sa vitesse (#169)
- [x] `performance.yml` : le rapport reste base sur pull request, `run_perf.sh`
  ajouté au filtre de chemins, `make perf` sur macOS et `gaol_bench` partout
  gardés ; dix jobs de plus, le cache des programmes de la base et `LINE_LIST`
  vide (#166) ; `performance.yml` dans `process.md` §10 (#165)
- [x] `GAOL_PREFER_AVX512` reste `ON`/`OFF` (plus une option depuis #200, le
  26 octobre) ; les commentaires « + - * / et sqrt
  » mis à jour et `-UGAOL_PREFER_AVX512` retiré de `setup.sh`, avec les sept
  dossiers de build d'avant #161 supprimés (#165) ; le test des bits du signe de
  zéro de `x * d`, sauté sans AVX-512 (#171)
- [x] Le ralentissement des racines d'un x sous-normal (#158) accepté ; `NI` de
  POWER et `FS` de MIPS : documentés seulement (#165)
- [x] GCC pour Windows : Cygwin reçoit les mêmes drapeaux que MinGW-w64 (#167,
  non vérifiable, sans job) ; `-mavx512f`, `-march` et `-faligned-new=8` pour un
  fichier C documentés (#167) ; le contrôle des drapeaux d'exception depuis
  chaque direction étendu à toutes les plateformes (#167) ; x86 32 bits :
  `-mstackrealign` essayé par la CI, sans effet, et `-mfma
  -mprefer-vector-width=128` par défaut aussi à 32 bits (133, #172) ; un job
  Cygwin x86-64 (#173, fusionnée)
- [x] Les branches d'expérience `exp-110-gcc-windows-fma`,
  `exp-110b-stack-alignment`, `exp-157-ab` et `exp-win32-fma-stack` supprimées
  sur GitHub (23 octobre) ; les issues #64 (`operator>>`) fermée avec la
  décision, #67 laissée ouverte
- [x] Les pull requests #165 à #167, #169 à #174 et #176 sont fusionnées par le
  mainteneur (les 23 et 24 octobre), #168 est fermée sans fusion ; le
  signalement #175 (défauts de la bibliothèque C de Cygwin) attend l'accord du
  mainteneur, rien n'est envoyé

#### Décidé pendant la tâche PA (22 à 24 octobre), fait par #180 à #182

- [x] Un pointeur nul donné à `textToInterval()` ou à `exactToInterval()` (128)
  lève `input_format_error`, aussi pour `gaol_ieee1788::textToInterval()` : ce
  n'est pas un texte mal formé (le vide), c'est un appel faux que la norme ne
  traite pas. Écartés : un type auxiliaire pour le paramètre (change la
  signature exportée et casse `&gaol::textToInterval`) et un seul jeu `(const
  char*)` (un `nullptr` à côté d'un `std::string` construirait encore un
  `std::string` de lui) ; l'adresse de `gaol_ieee1788::textToInterval` sans cast
  devient ambiguë, ce que `ChangeLog` dira (#180)
- [x] `tests/numbers.cpp` garde son contournement du défaut de GCC 12.2 sur
  aarch64 (les appels de `textToInterval()` construisent leur `std::string`)
  plutôt qu'une option de compilation du seul fichier (`-O1`), qui demanderait
  trois endroits (CMake, autotools, meson) ; il est aussi fragile que celui
  d'avant, un autre changement du fichier ou de ses en-têtes pouvant rendre le
  défaut à un autre tableau (décidé le 24 octobre, #180)
- [x] Le fichier et la ligne de l'exception d'un pointeur nul sont ceux de la
  fonction auxiliaire de `gaol/gaol_interval.cpp`, non ceux de l'appelant : le
  message dit tout ce qu'il faut, cosmétique, non changé (#180)
- [x] Un `file` nul s'écrit « (unknown file) » (question de #147), rangé à la
  construction : `file()` n'est jamais nul, et `operator<<` n'a rien à changer.
  `which_file_` est `protected` : une classe d'un utilisateur qui lui affecte un
  pointeur nul garde le défaut, et `operator<<` n'en est pas défendu. Un message
  nul de `gaol_warning()` et de `gaol_error()` est un texte vide (même défaut,
  étendu le 24 octobre à la demande du mainteneur). Les deux aides sont dans
  `gaol_detail` (`gaol/gaol_common.h`), non dans un espace de noms anonyme de
  chaque source (#181)
- [x] `<a, b>` refuse, par `input_format_error`, toute borne dont la valeur
  n'est pas un point (les deux bornes de sa valeur n'ont pas les mêmes bits, −0
  et +0 étant un) : `0.5`, `(0.25+0.25)`, `2*0.25` et `[0.5]` restent lus
  (décidé le 24 octobre, question de #149, #182)
- [x] Le plafond de l'exposant du lexeur (85) passe en `long long`, sans test :
  celui de 512 Mio (1,5 Gio sur les jobs Windows x64) est retiré, la preuve est
  dans la description de #182, et rien ne garde la correction contre un retour
  du `long` de 32 bits (décidé le 24 octobre, #182)
- [x] La lecture de `1e400` et des nombres au-delà du plus grand double ne lève
  plus `FE_OVERFLOW` (79, #149 l'avait relevé) ; celle d'un nombre au-dessous du
  plus petit double normal ne lève plus `FE_UNDERFLOW` non plus (décidé le 24
  octobre, dans #182). Le temps de lecture d'un sous-normal, 3 à 10 fois plus
  long, est accepté : un meilleur point de départ est une question à `TODO.md`
  (PG.6)

#### Décidé pendant la tâche PB (24 octobre), fait par #183 et #184

- [x] Un `[+oo]` ou un `[-oo]` construit à partir d'un registre (`interval(const
  __m128d&)` des intervalles SSE2 et son équivalent NEON) n'est pas un réel :
  une garde dans le constructeur public, qui rend l'ensemble vide pour `l = +oo`
  ou `u = -oo`, dans une pull request à part, faite par #189 le 25 octobre ; les
  usages internes du constructeur (une vingtaine dans `gaol_interval_sse.cpp`,
  dans les opérations) ne doivent pas payer la comparaison. Les gardes de
  `pow_rel` et de `pow_exponent_rel` de #183 restent, comme filet des chemins
  internes (décidé le 24 octobre, #183)
- [x] Le masquage des exceptions de `pow_up_without_overflow_exception()`, le
  seul de la bibliothèque (les puissances dont le log2 est à 10^-9 de 1024, c
  voisin de `DBL_MAX` dans `pow_rel` et `pow_exponent_rel`), est reporté : issue
  #185 ; #184 le garde en attendant la réponse (décidé le 24 octobre, #184),
  puis gardé le 25 octobre (tâche PD, plus bas)
- [x] Le signe d'un zéro de `div_rel` avec les intervalles SSE2 (`[0, x]` au
  lieu de `[-0, x]` quand `div_rel_upward()` calcule, un tirage sur 300) est
  laissé tel quel et documenté dans `doc/accuracy.md`, commit 9da8118 (décidé le
  24 octobre, #184)
- [x] Les défauts de CORE-MATH que la CI et la sonde ont montrés (`rh * rh` de
  `cr_pow()`, `x + x` de `cr_atan()`) sont contournés dans GAOL
  (`pow_rounded_up()`, `atan_lo()` et `atan_hi()`), les sources reprises restent
  intactes, et les deux correctifs sont à proposer en amont avec ceux de
  `3rd/README.md` (`TODO.md`, PH.1) ; rien n'est écrit dans `3rd/README.md` ni
  envoyé (décidé le 24 octobre, #184)
- [x] La phrase d'introduction de la section sur les exceptions flottantes de
  `doc/using.md` et du manuel, « An infinite bound comes from a division by zero
  or from an overflow », est supprimée, avec les deux exemples `log([0, 1])` et
  `[1e308]*10` gardés (décidé le 24 octobre, fait par #184) ; la phrase du
  manuel et de `doc/using.md` qui disait que `atanh_rel([-1, 2], x)` d'un `x`
  vide lève la division par zéro est retirée, `atanh_rel()` ne la levant plus
- [x] Restés sans réponse à la fusion, les deux points que la description de
  #184 demandait de confirmer : `atanh([0, 1])` et `pow` gardent leur drapeau
  pour une borne infinie, que `atanh_rel()` et les réciproques de `pow` ne
  lèvent plus ; `pow_rel` et `pow_exponent_rel` coûtent 5 à 10 % de plus (le
  test de la puissance d'essai avant chaque appel de CORE-MATH). Le code est tel
  que fusionné

#### Décidé pendant la tâche PC (25 octobre), fait par #186 à #190

- [x] Les noyaux NEON `add_bounds()`, `sub_bounds()`, `mul_bounds()`,
  `div_bounds()` et `sqr_bounds()` passent dans `gaol_detail` avec ceux de SSE2,
  bien que la décision Q15a ne nomme que SSE2 : « très bien » (25 octobre, #186)
- [x] `sqrt_bounds()` de NEON n'a pas de test qui échoue avec les compilateurs
  d'aujourd'hui : le défaut est théorique. Le bloc de test de `FPCR.AH` a des
  dents avec QEMU 10.0, qui retient le bit ; il saute les vérifications sur
  l'Apple M2 et sous QEMU 7.2 et 8.2 (25 octobre, #187)
- [x] Le portage du noyau de `pown` vers NEON est gardé malgré un gain faible
  (1,03 à 1,07 sur un M2, 1,23 avec GCC et 0,90 avec Clang sur le Neoverse de la
  CI, au lieu de 1,5), à condition que les noyaux soient répartis dans
  `gaol_interval_neon.cpp`, `gaol_interval_sse.cpp`, `gaol_interval_fpu.cpp` (et
  `gaol_interval_avx512.cpp`) au lieu de `#if` dans `gaol_interval_core.cpp` (25
  octobre, #188). La cause du gain faible n'est pas cherchée (`TODO.md`, PG.7)
- [x] Les conflits de texte entre #186, #188 et #189 (les mêmes lignes) se
  règlent en fusionnant `configure-clean` dans la seconde pull request à
  fusionner (25 octobre)
- [x] `bisect(ratio)` d'un intervalle non borné, ou à `ratio == 0.5` pour un
  point ou un intervalle symétrique par rapport à 0, ne vérifie pas la direction
  et laisse les modes de l'appelant tels quels sans `GAOL_PRESERVE_ROUNDING`,
  comme `midpoint()` : laissé tel quel (25 octobre, remarqué par #190)
- [x] Pour tester `FPCR.AH`, une image Docker locale (Debian 13, `qemu-user`
  10.0) ; supprimée ensuite, comme celle d'Ubuntu 24.04 (qemu 8.2) qui ne
  retient pas le bit, à la demande du mainteneur (25 octobre)

#### Décidé pendant la tâche PG (25 octobre), fait par #191 à #194

- [x] Les intervalles de chaque build : NEON sur tout processeur ARM 64 bits
  avec tous les compilateurs, SSE2 sur tout processeur x86 avec tous les
  compilateurs, le FPU seulement où l'on n'a pas le choix (« le FPU ne doit être
  utilisé que si on n'a pas le choix », 25 octobre, #192 à #194)
- [x] Visual C++ et clang-cl sur x86 32 bits utilisent SSE2 (#193) ; clang-cl,
  MinGW-w64, MSYS2 et le build universel de macOS sur arm64 utilisent NEON
  (#192, #194) (25 octobre)
- [x] La phrase de PG.6 et de `doc/simd.md` selon laquelle l'arrondi embarqué
  scalaire « n'a pas été essayé pour `/` » était inexacte : `div_bounds()`
  faisait déjà deux divisions scalaires depuis cf60ef6 ; les trois endroits qui
  divisaient encore en 512 bits sont convertis et la phrase est corrigée (25
  octobre, #191)
- [x] `x / y` de clang-cl plus lent en NEON qu'avec le FPU (4,38 contre 5,36 ns)
  : garder NEON et chercher la cause (25 octobre, #192). Fait par #195 (fermée
  sans fusion) : deux divisions scalaires sont plus lentes, la division n'est
  pas en cause ; la cause n'est pas trouvée (`TODO.md`, PG.8)
- [x] C4324 avec `/W4` sur arm64 (les structures qui contiennent un intervalle
  NEON, alignées sur 16 octets) : documenter seulement, sans job ni pragma (25
  octobre, #192, `doc/using.md`)
- [x] `GAOL_VECTORCALL` sur x86 32 bits : à faire dans #193 ; gardé, bien que le
  banc A/B ne montre aucune différence au-delà de 5 % (25 octobre, #193)
- [x] Les mesures de `doc/using.md` et de `doc/three-builds.md` pour Visual C++
  et clang-cl, qui sont celles du FPU : à refaire à la fin, au commit de la
  version (25 octobre, #192 et #193, `TODO.md` PJ.2)
- [x] Le réglage de développeur `-DGAOL_SSE2_UNALIGNED_STORAGE=ON` de CMake (la
  forme à deux `double` des intervalles SSE2 de Visual C++, que deux jobs Linux
  éprouvent) reste sans option documentée, comme `GAOL_FLOAT_INTERVALS` (25
  octobre, #193)
- [x] La tranche x86_64 d'un build universel de macOS n'a pas le fusionné
  multiplie-addition (`-mfma` échoue pour la tranche arm64 ; CORE-MATH appelle
  le `fma()` de la bibliothèque de macOS et les puissances entières n'ont pas le
  noyau empaqueté) : le documenter seulement (`doc/building.md`, ligne
  `GAOL_FMA`), sans le mesurer ni donner de drapeaux propres à cette tranche (25
  octobre, #194)
- [x] Les conflits de texte entre #192, #193 et #194 (les mêmes lignes de
  documentation et de workflows) se règlent en fusionnant `configure-clean` dans
  la pull request suivante après chaque fusion ; les registres ne sont mis à
  jour qu'après la fusion de #194, quand le mainteneur le dit (25 octobre)

#### Décidé pendant la tâche PD (25 octobre), fait par #196 et #197

- [x] Le masquage des exceptions de `pow_up_without_overflow_exception()` (issue
  #185) est gardé, sans changement de code : le mainteneur a choisi de le
  garder plutôt que de le retirer (le drapeau resterait levé pour ces 2^12
  valeurs de c, la phrase de `doc/using.md` et du manuel serait à changer) ou de
  ne le garder que sur x86-64 avec glibc et UCRT ; #185 est fermée
- [x] `process.md` : l'en-tête porte une date et un commit, avec le renvoi au
  §12 gardé ; le §6.1 une phrase pour Ubuntu 24.04 ; rien pour `GAOL_V9/GAOL` au
  §3.3 (22 octobre, Q9a à Q9c, fait par #196) ; le §4.11 dit « Clang 21 (depuis
  le 13 octobre) » et la colonne avec la direction préservée (corrigé dans #196
  sur la demande du mainteneur)
- [x] Les libellés « SSE2 » du build par défaut sont alignés dans les rapports
  de `doc/compare/` dans #196 (réponse du mainteneur : dans cette pull request),
  sauf ceux qui décrivent une mesure faite sur x86 (`tang2021.md` lignes 180 et
  254, `special_cases.md` ligne 34), où « SSE2 » est exact
- [x] `doc/differences.md` (ligne 790) cite encore `interval(sl, sr)` : pour la
  pull request de synthèse (tâche PI) ; celle d'`examples/examples.md` (ligne
  1110) est le texte de la revue d'origine
- [x] Les tests de `std::pow` : il n'en reste qu'un dans `tests/`
  (`arithmetic.cpp`, racines n-ièmes), à résultats normaux et gardé par
  `compare()` : rien à changer
- [x] Les mutants qui ne tenaient qu'à 1 et 5 vérifications : tests renforcés
  dans #197 (réponse du mainteneur : dans cette pull request, plutôt qu'à part)
  ; les dix `neon-*` lancés sur l'Apple M2 par la session MACarm2 (sur un Apple
  M2 plutôt qu'un job de CI ou une compilation croisée)
- [x] Quatre mutants des divisions de #191 ajoutés dans #197 (réponse du
  mainteneur)
- [x] Le défaut possible de `x / y` des intervalles NEON avec `FPCR.AH` (52 des
  441 paires du bloc de #187, sur un runner arm64 de la CI, vert à la relance) :
  une issue (#198), sans tâche dans `TODO.md` (25 octobre) ; après la relance
  du job, le mainteneur a gardé « reporter » plutôt que de chercher la cause
  tout de suite
- [x] L'échec de CI de macOS 14 x86_64 (Rosetta), non reproduit (ancien PD.8) :
  passe dans `TODO_AFTER.md` (4.8), à reprendre s'il revient
- [x] Les registres se mettent à jour après les fusions du mainteneur, et le
  commit des registres est fait au-dessus de son commit local « update
  README.md. », que le même push emporte (25 octobre)

#### Décidé pendant le déplacement dans gaol/core/ (25 octobre), fait par #199

- [x] Le dossier est `gaol/core/` ; les en-têtes des quatre sortes d'intervalles
  et de leur noyau y sont installés dans `include/gaol/core/`, sur le modèle de
  `gaol/parser/` (25 octobre, #199)
- [x] Aucun en-tête de renvoi aux anciens chemins
  `include/gaol/gaol_interval_{core,fpu,neon,sse}.h` : comme pour le parser,
  tant pis pour la rétrocompatibilité ; l'installation dans `include/gaol/core/`
  est à écrire dans `ChangeLog` et `doc/differences.md` (tâche PI) (25 octobre,
  #199)
- [x] Le second commit de #199 (`TODO_DONE.md`, `TODO_AFTER.md`,
  `doc/differences.md`, `todo-notes/`) reste dans la pull request, bien que
  `process.md` §7.5 veuille les registres hors des branches et que
  `check_branch` refuse `doc/differences.md` : le mainteneur avait demandé «
  tous les fichiers » (25 octobre, #199)
- [x] `gaol_interval_fpu.cpp` ajouté à l'`EXTRA_DIST` de `gaol/meson.build` dans
  #199, plutôt qu'une pull request à part ou une issue (25 octobre)
- [x] Les 47 lignes de commentaires, de documentation et de tests que les
  chemins plus longs font passer au-dessus de 80 colonnes sont laissées, pour un
  diff mécanique ; celles des registres sont refaites (25 octobre, #199)
- [x] Pas de relecture indépendante pour #199, une substitution de chemins
  vérifiée par les trois builds et la CI (demande du mainteneur, 25 octobre)

#### Décidé pendant la tâche des options (26 octobre), fait par #200

- [x] Les réglages que la CI et les développeurs donnent (FPU sur x86, sans FMA,
  sans AVX-512, émulation de l'entier 128 bits, `GAOL_SSE2_UNALIGNED_STORAGE`,
  `GAOL_DEBUG` sur un sous-projet) restent acceptés mais cachés : lus avec
  `DEFINED` par CMake, sans texte d'aide dans configure, marqués `[developers]`
  par meson, qui ne peut pas les masquer ; section « The settings of the
  developers » de `doc/building.md` (26 octobre)
- [x] En compilation croisée, où aucune sonde ne s'exécute, la configuration
  prend les chemins sûrs : ni FMA ni AVX-512, avec un message, sauf
  `-DGAOL_MACHINE_RUNS_FMA=1` et `-DGAOL_MACHINE_RUNS_AVX512=1` (CMake),
  `gaol_cv_machine_runs_*=yes` (configure) ou `-Dmachine-runs-*=true` (meson)
  (26 octobre ; avant : les instructions FMA étaient prises sur la parole du
  compilateur)
- [x] Le chemin AVX-512 se décide par une mesure sur place, quatre builds de la
  même source, `performance-ab.sh --runs 9 --lines`, seuil d'un gain médian de 5
  % : il suit `GAOL_PRESERVE_ROUNDING` (médiane de sept opérations 2,80 avec,
  0,83 sans ; dix lignes 3,39 et 0,68) ; une seule machine, à demi-largeur
  AVX-512, dans `doc/simd.md` (26 octobre)
- [x] La bibliothèque statique est le défaut des trois builds, et le code est
  indépendant de la position ; `--enable-shared` et `-Ddefault_library=shared`
  demandent la partagée (26 octobre)
- [x] Le build universel de macOS pour autotools et meson aussi (choix du
  mainteneur, contre la recommandation d'en rester à CMake) : `-arch` plusieurs
  fois dans les drapeaux, la dépendance de dépôt d'automake coupée ; les trois
  jobs universels sont verts (26 octobre)
- [x] `GAOL_VERBOSE_MODE` et la désactivation des exceptions sont des réglages
  des développeurs dans les trois builds, celle des exceptions ajoutée à CMake
  (26 octobre)
- [x] Une seule pull request, un commit par étape, l'audit des trois builds
  n'étant vert qu'à la fin (26 octobre)
- [x] Le test d'exécution de FMA de Visual C++ et de clang-cl exécute
  l'instruction (`_mm_fmadd_sd`), dans cette pull request (réponse du mainteneur
  le 26 octobre)
- [x] Le type `Debug` fixe `GAOL_DEBUG`, donc `-O0 -g` pour GAOL et CORE-MATH,
  un `-O1` de `CMAKE_CXX_FLAGS_DEBUG` écarté pour la bibliothèque : gardé tel
  quel (26 octobre)
- [x] Le point 92 (PE.2, les drapeaux C de CORE-MATH) sur une branche à part,
  `todo-ae-c-flags`, partie de celle de #200, et sa pull request #201 ouverte
  après la fusion de #200 (26 octobre)
- [x] Remplace deux décisions : le 23 octobre, `GAOL_PREFER_AVX512` reste
  `ON`/`OFF` (pas d'option `AUTO`), qui n'est plus une option ; les 19 et 20
  octobre, les intervalles NEON indépendants de `GAOL_SIMD` (#146), que
  `GAOL_SIMD` choisit maintenant
- [x] La suite de la tâche PE (le reste du 95 et les points 3 à 6) à la demande
  du mainteneur, après la fusion de #200 ; le reste du 95 (lier la sonde de la
  cible AVX-512 avec autotools et meson, vérifier le C des drapeaux de FMA avec
  CMake) dans une pull request à part, partie de `configure-clean` une fois #201
  fusionnée (26 octobre)
- [x] La règle du chemin AVX-512, mesurée sur un seul processeur, est gardée
  avec sa limite dite dans `doc/simd.md` ; la mesure sur une machine à AVX-512
  pleine largeur va dans PJ, avec les mesures de la version (26 octobre)
- [x] Les passages de `doc/differences.md` et de `ChangeLog` qui citent les
  anciennes options, et le texte des drapeaux C de CORE-MATH, sont corrigés dans
  la pull request #201 et non dans la pull request de synthèse (26 octobre)
- [x] Les dossiers de build des sessions précédentes dans `/tmp/claude-1000` (11
  Go, sur un quota presque atteint) supprimés à la demande du mainteneur (26
  octobre)

#### Décidé pendant la tâche PE (26 et 27 octobre), fait par #201 et les pull requests qui suivent

- [x] Les quatre cas qui restaient du point 3 (#152, #173, #174, #193, #194)
  sont tous traités, aucun laissé de côté : autotools et meson sous Cygwin,
  Clang pour la cible de Visual C++ avec le pilote de GCC (sans clang-cl), le
  build universel de macOS sur chaque version, et le générateur d'Xcode
- [x] Le point 3 en une seule pull request ; `tests.sh` donne partout les Cflags
  du `gaol.pc` installé ; pour autotools avec Clang vers MSVC, deux essais de CI
  au plus (le premier a réussi, une fois le `unistd.h` réglé) ; si un job échoue
  à cause de GAOL, la correction va dans la même pull request (elle a trouvé le
  C++14 de la STL de Visual C++, le `unistd.h` du lexeur et l'architecture
  d'Xcode)
- [x] Le générateur d'Xcode sans `CMAKE_OSX_ARCHITECTURES` : CMake lui donne
  l'architecture de la machine, comme les autres générateurs, configure et meson
  ; la liste `arm64;x86_64` donne le build universel
- [x] `shellcheck` installé par le mainteneur ; `actionlint` dans l'image Docker
  officielle `rhysd/actionlint` ; toutes les remarques de `shellcheck` des
  scripts (101, `.github/scripts` et `doc/compare/code`) sont traitées dans
  #202, avec des `# shellcheck disable=` là où une liste de drapeaux doit être
  découpée
- [x] Le commentaire de `CMakeLists.txt` sur le bogue 54412 de GCC se corrige
  avec le reste du 95, dans une pull request à part ; la relecture des
  corrections A à F de #161 n'est pas refaite (#200 a supprimé le test et
  l'option ; le point A est vérifié dans le code) ; les 350 références « point X
  of TODO.md » des sources restent
- [x] Les registres sont mis à jour à la fusion de chaque pull request, non
  avant
- [x] Vérifié le 26 octobre (point 5) : le correctif du bogue 54412 de GCC est
  le commit `8aaa5f6e6207` (« function: honour over-alignment in
  assign_stack_local [PR54412] », validé le 25 septembre 2026 sur la branche de
  développement, GCC 17, non publié) ; « fixed for GCC 17 only » est donc exact,
  sans rétroportage vu pour GCC 16 ; les paquets `mingw` de Chocolatey ont les
  bibliothèques mingw-w64 v10 (12.2.0), v11 (13.2.0), v12 (14.2.0) et v13
  (15.2.0), dont les sources de `fesetround()` et `fegetround()` sont celles
  lues
- [x] MSYS2 MINGW32 lie le `fma()` de msvcrt : `gaol_config.h` le refuse sans
  `-mfma` (`__FMA__` non défini), comme msvcrt sur x86-64, pour mingw-w64 12 ou
  plus seulement, la version 11 restant acceptée sans FMA pour garder le job
  « MinGW-w64 13.2.0 x86 Release, no FMA » (réponse du 27 octobre ; l'écart de
  la 11 est une question ouverte de #205, voir `TODO_AFTER.md`) ; avec `-mfma`,
  le défaut, MSYS2 MINGW32 est construit et testé
- [x] Pas de job autotools avec Clang vers MSVC en x86 : CMake et meson couvrent
  l'x86, autotools l'x64 (27 octobre)
- [x] Après #205, s'arrêter et attendre : aucune autre tâche (PF, PG...) sans la
  demande du mainteneur (27 octobre)

#### Le 18 octobre : les questions restées ouvertes dans la description de #99 (commentaire de #49)

Les six questions que la case « Les questions restées ouvertes dans la
description de #99 » de cette issue gardait ouvertes ont reçu une réponse. Elles
viennent de la section « Questions ouvertes » de la description de
[#99](https://github.com/Jordan08/GAOL/pull/99). Les liens pointent vers
`configure-clean` au commit
[496a3f2](https://github.com/Jordan08/GAOL/commit/496a3f2).

##### Sans objet

- **`010_iv`** : le littéral `_iv` n'existe plus. `operator""_iv` a été retiré
  le 16 octobre ([faf71a9](https://github.com/Jordan08/GAOL/commit/faf71a9) :
  `textToInterval` dans la documentation et l'exemple 02), puis
  `gaol/gaol_literals.h`
  ([4e42879](https://github.com/Jordan08/GAOL/commit/4e42879) et
  [302a035](https://github.com/Jordan08/GAOL/commit/302a035)). La question de
  `010_iv` valant 8 ne se pose plus. Le point 86 de
  [`TODO.md`](https://github.com/Jordan08/GAOL/blob/496a3f2/TODO.md#L692) (`_iv`
  hexadécimal, octal ou binaire au-delà de 2^64) parle du même littéral : il est
  lui aussi sans objet.
- **La colonne GAOL v5 de `doc/compare/performance.md`** : le fichier a été
  relancé en entier le 13 octobre, sous Ubuntu 26.04.1 puis avec Clang 21
  ([#118](https://github.com/Jordan08/GAOL/pull/118),
  [#121](https://github.com/Jordan08/GAOL/pull/121)), puis le 17 octobre avec
  les longues lignes et la colonne « rounding direction preserved »
  ([#124](https://github.com/Jordan08/GAOL/pull/124)). Le compilateur du
  [tableau des
  bibliothèques](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/compare/performance.md#L26)
  et les chiffres du texte (`log` 20,9 ns contre 46,8, `exp` 18,3 contre
  35,1...) sont ceux des tableaux de résultats. Les temps de Boost, marqués «
  indicatifs » à cause d'une machine chargée, sont de la même mesure que ceux
  des autres bibliothèques.

##### Décidées

- [x] **L'exception de `bisect`** : un ratio hors de ]0, 1[, ou NaN, lève
  `invalid_action_error`, comme `nth_root` avec un second argument non entier.
  Rien ne change : le comportement est dans
  [`interval::bisect`](https://github.com/Jordan08/GAOL/blob/496a3f2/gaol/core/gaol_interval_core.cpp#L5671),
  [`doc/using.md`](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/using.md#L495),
  le
  [manuel](https://github.com/Jordan08/GAOL/blob/496a3f2/manual/v5/gaol.tex#L4051)
  et le
  [test](https://github.com/Jordan08/GAOL/blob/496a3f2/tests/other_functions.cpp#L289).
  Écartés : l'exception seulement hors de [0, 1], le retour silencieux de (∅, x)
  ou (x, ∅) de la première version, et le ratio ramené dans ]0, 1[.
- [x] **La politique de Boost** : `rounded_transc_opp` reste celle des colonnes
  de `performance.md` et de `special_cases.md`
  ([`boost_policies.h`](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/compare/code/boost_policies.h#L31)).
  C'est la politique de l'exemple de Boost (`findroot_demo.cpp`) et la plus
  rapide ;
  [`enclosure.md`](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/compare/enclosure.md#the-policies-of-boostinterval)
  mesure déjà les deux (34 % de résultats qui manquent f(x) avec
  `rounded_transc_opp`, 20 % avec `rounded_transc_std`). Pas de colonne
  `rounded_transc_std` dans `performance.md`.
- [x] **La précision de `pow` de `doc/accuracy.md`** : supprimer la phrase « 4
  doubles on the special cases » de la [ligne de
  `pow`](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/accuracy.md#L96).
  Elle venait de l'ancien `exp(y log x)` ;
  [#118](https://github.com/Jordan08/GAOL/pull/118) calcule les coins avec le
  `pow` de CORE-MATH, et la ligne cite déjà les 117 boîtes de `ieee1788`, les 1
  347 cas de `pow`
  d'[ITF1788](https://github.com/Jordan08/GAOL/blob/496a3f2/doc/compare/itf1788.md)
  (les plus serrées) et les 84 070 boîtes aux coins. *Fait par #164 (117).*
- [x] **Le tableau des outils de `process.md`** : le [tableau du
  §6.1](https://github.com/Jordan08/GAOL/blob/496a3f2/process.md#L248) est
  refait par [#109](https://github.com/Jordan08/GAOL/pull/109) et correspond à
  la machine d'aujourd'hui (Ubuntu 26.04.1 : GCC 15.2, Clang 18, 20 et 21, CMake
  4.2.3, autoconf 2.72, automake 1.18.1, libtool 2.5.4, bison 3.8.2, flex 2.6.4,
  meson 1.10.1, ninja 1.13.2, python 3.14.4 avec mpmath 1.3.0, TeX Live 2025,
  gcovr 7.2). Retirer le [paragraphe du 12
  octobre](https://github.com/Jordan08/GAOL/blob/496a3f2/process.md#L261) qui
  décrit une machine Ubuntu 24.04 (GCC 13.3, CMake 3.28, sans autotools ni
  meson) ; garder « Vérifie donc les versions au début du travail ». *Fait par
  #155 (118).*

### 3.5 Le 22 octobre : réponses du mainteneur aux seize questions

Réponses aux seize questions restées sans réponse dans les descriptions des 20
dernières pull requests fusionnées (#122 à #158), dans les sections « Remarqué
par …, à trier » ci-dessus et dans les points 46 (F), 26 (W), 111, 113 (AJ),
115, 121 et 125 (AM). Les nouveaux numéros 128 à 133 sont ceux du point AP.

- **Q1 (QI, PR #157, noyaux FPU)** : mesurer d'abord. Ne pas fusionner le
  produit tel quel ; lancer les mesures du produit sur armhf (conteneur Debian,
  vrai ARM 32 bits VFPv4), POWER, s390x et RISC-V avant de décider. La pull
  request reste en brouillon. *Suite* : les mesures en alternance sur la CI
  (armhf natif, Visual C++ et clang-cl, Ubuntu et macOS en FPU) ont été faites,
  le mainteneur a gardé tous les noyaux et #157 est fusionnée ; POWER, s390x et
  RISC-V restent non mesurés (voir `TODO_AFTER.md`).
- **Q2 (QB, 121)** : ajouter `.continue/` au `.gitignore`. *Fait par #164.*
- **Q3 (QC, 125 AM)** : clos. `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT`
  ont déjà des valeurs par défaut dans `gaol/gaol_config.h` (4 ou 8 selon
  `UINT_MAX`, 8 selon `ULLONG_MAX`) ; les trois gabarits de configuration ne les
  définissent pas, et l'en-tête installé suffit. Retirer le point 125. *Retiré
  le 23 octobre.*
- **Q4 (QH, 46 F, #64)** : garder un intervalle par ligne (comportement actuel,
  documenté). Pas de changement d'`operator>>`. *Le 46 est sans suite ; l'issue
  #64 est fermée le 23 octobre.*
- **Q5 (QH, 26 W, #67)** : rien pour l'instant. GAOL reste un intervalle nu ;
  les pièges sont documentés (faits par #140). Reporter indéfiniment. *Le 26 est
  reporté indéfiniment (voir `TODO_AFTER.md`).*
- **Q6 (QD, 111 et 115 AI)** : mesurer par la CI deux builds alternés dans le
  même job (par exemple `configure-clean` et la branche), comme `doc/compare`
  alterne ses programmes. *Fait par #160.*
- **Q7 (QE, 113 AJ)** : non, après v5.0.0. L'étude des noyaux au plus proche est
  reportée après la version ; le comportement actuel (changement du sens
  d'arrondi) est documenté. *Le 113 est dans `TODO_AFTER.md`.*
- **Q8 (QJ, #156, x86 32 bits)** : tout oui. (a) Lire MXCSR en ligne dans
  CORE-MATH sur x86 32 bits (`gaol/core_math_port.h` comme sur x86-64). (b)
  Ajouter un test de `x86_registers::read_control()`. (c) Garder le job « Visual
  Studio 2026 x86 Release » avec la direction préservée. (d) Ajouter l'outil de
  mutation et l'émulateur au dépôt (`tests/tools/`). (e) Retirer le chemin de
  repli `fegetround()` (code mort aujourd'hui). *Le 108 est dans la tâche PF de
  `TODO.md`.*
- **Q9 (QJ, #155, process.md)** *((a) et (c) sont le 130)* : (a) Oui, une phrase
  plus courte à la place du paragraphe retiré du §6.1. (b) Non, garder « 4 cœurs
  au plus » au §3.3 sans ligne pour `GAOL_V9/GAOL`. (c) Mettre une date et un
  commit dans l'en-tête de `process.md`, au lieu du renvoi au §12.
- **Q10 (QJ, #154)** : les deux oui. *(a) est le 129.* (a) Durcir
  `sqrt_bounds()` par un masque sur le bit de signe (FPCR.AH / FEAT_AFP), en une
  pull request à part, validé sur une copie. (b) Faire `pow(x, 2)` par `sqr`
  (environ 6 fois plus rapide sur cette opération). *(b) fait par #162, 3,3 fois
  plus rapide avec NEON et 2,5 avec la FPU : l'« environ 6 fois » était le
  rapport de l'ancien `gaol_pown` à `sqr`.*
- **Q11 (QJ, #151)** : garder la correction élargie. Le Fast2Sum corrige aussi
  la somme finie à mi-chemin de deux doubles, de même cause ; c'est mieux que la
  formule stricte du TODO.
- **Q12 (QJ, #149)** : garder pour tous (actuel). Le contexte garde le texte de
  chaque nombre qui n'est pas un double ; le coût est connu et documenté, et la
  comparaison exacte est un plus.
- **Q13 (QJ, #147)** *(c'est le 128)* : lever une erreur de GAOL comme une
  mauvaise lecture du parser. `textToInterval(nullptr)`,
  `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)` doivent
  lever `input_format_error` (ou une `gaol_error`) pour un pointeur nul, sans
  passer par la construction indéfinie d'un `std::string`.
- **Q14 (QJ, #146, arm64)** *((a) est le 132)* : (a) Oui, porter le noyau
  empaqueté de `pown` (`GAOL_POWN_PACKED`) sur arm64, 150 lignes d'intrinsiques,
  gain ~1,5x. (b) Non, pas de test comparant les bits du signe d'un zéro. (c)
  Non, pas de jobs Clang 15 à 17 ni GCC 9 à 10 sur arm64, pas de test
  `mrs`/`msr` sur Neoverse.
- **Q15 (QJ, #124, noyaux SSE2)** *((a) est le 131)* : (a) Mettre les noyaux
  exportés (`add_bounds()`, `sub_bounds()`, `mul_bounds()`, `div_bounds()`,
  `sqr_bounds()`) dans `gaol_detail`. (b) Garder le plus rapide (le signe des
  zéros diffère, c'est documenté). (c) Non, ne pas modifier `3rd/math-core` pour
  passer le sens d'arrondi en paramètre.
- **Q16 (QI, branches)** : garder les deux. `todo-ai-msvc-avx512` (sans pull
  request, semblant remplacée par #148) et `todo-fpcr-gcc-hoisting` (branche de
  #153, fermée sans fusion, correction dans #150) restent sur GitHub. *Le 23
  octobre, la seconde n'est plus sur GitHub (supprimée le 20 octobre, d'après le
  « Ménage » ci-dessus) : rien à restaurer. Le 24 octobre, le mainteneur a fait
  supprimer aussi `todo-ai-msvc-avx512` (un ancêtre de `configure-clean`).*

### 3.6 Le 23 octobre : réponses du mainteneur aux questions des dix dernières pull requests

Réponses aux questions restées sans réponse dans les descriptions des dix
dernières pull requests (#152 à #164), posées le 23 octobre en quatre séries ;
le travail qu'elles décident est fait le même soir et ouvert en pull requests
(#165 à #173). Les puces nomment la pull request et le point.

- **#157, `GAOL_RND_KEEP()` sur `*=`, `/=` et `%=`** : ne pas le remettre ;
  tenir la protection par un test, avec GCC et la préservation du sens, ou LTO.
  *Fait par #170* : le test existait (`tight_cases` de
  `rounding_direction.cpp`), des mutants le montrent (53/96, 58, 57
  vérifications de la boucle en échec pour `*=`, `/=`, `%=`), et les jobs
  manquaient (FPU x86-64 GCC et Clang avec la préservation, arm64 GCC). Pas de
  `-flto` : GAOL n'offre pas d'option qui puisse casser un encadrement.
- **#157, le noyau mixte** : l'essayer ici et par le banc A/B ; *fermer sans
  fusion* si les tableaux ne montrent rien. *Fait par #168 (brouillon)* : les
  tableaux montrent des gains (1,4 à 2,1 fois aux signes réguliers) et des
  pertes (5 à 19 % aux signes aléatoires) ; le mainteneur a ensuite confirmé la
  fermeture sans fusion, les tableaux lus (voir `TODO_AFTER.md`).
- **#157, les bancs et la relecture** : `gaol_performance` inchangé, pas de
  `fmax()`, pas de seconde relecture de `process.md` §7.
- **#162** : garder la décision dans `gaol_pown` (pas de `e == 2` dans les
  en-têtes) ; harmoniser le vide vers « le sens du programme ». *Fait par #169*.
  Les puissances 3 à 5 avec la FPU : +0,8 ns sur x86, laissé et dit.
- **#160** : le rapport reste base sur pull request ; le filtre de chemins gagne
  `run_perf.sh` seulement ; `make perf` pour macOS et `gaol_bench` partout
  gardés ; `LINE_LIST` vide, les jobs absents et le cache du binaire de la base.
  *Faits par #166* ; `process.md` §10 par #165.
- **#161** : `GAOL_PREFER_AVX512` reste `ON`/`OFF` (pas de `AUTO`) ; les
  commentaires « + - * / et sqrt » mis à jour (par #165, #164 étant déjà
  fusionnée) ; le test des bits du signe de `x * d`, sauté sans AVX-512 (*fait
  par #171*) ; `-UGAOL_PREFER_AVX512` retiré de `setup.sh` et les dossiers
  nettoyés (*#165*, sept dossiers supprimés).
- **#158** : le ralentissement pour un x sous-normal est accepté.
- **#163** : Cygwin reçoit les mêmes drapeaux que GCC pour Windows (non
  vérifiable, sans job) ; `-mavx512f` et `-march` documentés ; le contrôle des
  drapeaux depuis chaque direction étendu à toutes les plateformes. *Faits par
  #167.* Les trois branches d'expérience supprimées (*fait*). `-mstackrealign`
  pour le x86 32 bits : essayé par la CI, sans effet ; le x86 32 bits reçoit
  `-mfma -mprefer-vector-width=128` par défaut (*#172*, le 133) ; un job Cygwin
  x86-64 (*#173*, brouillon).
- **Issues** : #64 (`operator>>`) fermée le 23 octobre avec la décision de Q4 ;
  #67 (décorations) laissée ouverte, reportée indéfiniment (Q5).
- **#152** : l'avertissement de `-faligned-new=8` pour un fichier C est dit dans
  `doc/using.md` (*#167*).
- **#154** : `NI` de POWER et `FS` de MIPS laissés par le chemin générique de
  `round_upward()` : documenter seulement (*#165*).
- **Cygwin et l'empilement des PR (24 octobre, après les constats de #173)** :
  porter Cygwin maintenant (le correctif de `gaol_port.h`, la cause de `sinh`,
  CPack), le correctif du test d'exécution de `GAOL_FMA` reste dans #173,
  empiler les PR ouvertes (les fusions du mainteneur l'ont rendu presque sans
  objet : #171 et #172 ont reçu `configure-clean`) ; pas de tâche nouvelle pour
  le moment, finir celles commencées. *Fait par #173.*
- **Constat** : #164 avait été fusionnée avant que ses textes de CI soient
  repris ; ils sont dans #165.

### 3.7 Le 24 octobre

- **Cygwin** : voir la dernière puce de 3.6 (porter Cygwin maintenant, finir les
  tâches commencées, pas de tâche nouvelle). Les pull requests #173 et #174 sont
  fusionnées le matin, puis #176 : l'étape d'essai de CPack est retirée du job
  Cygwin (décidé le 24 octobre).
- **Branches** : `todo-ai-msvc-avx512` supprimée sur GitHub (un ancêtre de
  `configure-clean`, sans pull request ; le mainteneur avait répondu de la
  garder le 22 octobre, Q16), `exp-cygwin-diag` et `exp-cygwin-diag2` supprimées
  le jour même ; il ne reste que `configure-clean`, `MATH-CORE` et `master`.
- **Le signalement #175** (la bibliothèque C de Cygwin : `fma()` faux dans les
  quatre sens, `nextafter()` qui calcule le débordement dans le sens d'arrondi
  du programme, CPack qui ne met pas le lien `README` dans l'archive) est un
  brouillon : rien n'est envoyé.
- **Sept questions de `TODO.md`, tranchées par le mainteneur** (24 octobre) :
  PA.4 `<a, b>` avec une borne calculée qui n'est pas un double : la refuser
  aussi ; PA.5 une `gaol_exception` avec un `file` nul : écrire « (unknown file)
  » ; PB.3 la phrase d'introduction des exceptions flottantes de `doc/using.md`
  et du manuel : la supprimer, quand le 79 est fait ; PC.4 `bisect(ratio)` et
  `is_bisectable()` sous FZ : revérifier, puis corriger ; PF.3
  `GAOL_RND_NEAREST_ENTER()` et `GAOL_RND_ENTER_SSE()` : n'écrire que si ça
  change, aux deux familles, mesuré par le banc A/B ; PG.4 le seuil de la liste
  de `compare-performance.py` : 1,5 ; PJ.4 le second paragraphe de l'annonce à
  Frédéric Goualard : le garder tel quel. La première tâche à traiter est PA.
- **Quatre décisions sur les pull requests #180 à #182** (24 octobre, après leur
  ouverture) : `tests/numbers.cpp` garde son contournement du défaut de GCC 12.2
  sur aarch64 (les appels de `textToInterval()` construisent leur `std::string`,
  #180) ; le test de l'exposant de 512 Mio est retiré, la correction `long long`
  reste (#182) ; `FE_UNDERFLOW` de la lecture d'un nombre sous le plus petit
  double normal est corrigé dans #182, et un message nul de `gaol_warning()` et
  de `gaol_error()` dans #181 ; on attend les fusions du mainteneur pour mettre
  à jour les registres.
- **Quatre décisions sur les pull requests #183 et #184** (24 octobre, après
  leur ouverture) : le masquage des exceptions de
  `pow_up_without_overflow_exception()` est reporté, issue #185, et #184 le
  garde en attendant ; le signe d'un zéro de `div_rel` avec les intervalles SSE2
  est laissé tel quel et documenté (`doc/accuracy.md`) ; un `[+oo]` ou un
  `[-oo]` construit à partir d'un registre aura une garde dans le constructeur
  public, dans une pull request à part (`TODO.md`, PC.5) ; les défauts de
  CORE-MATH (`rh * rh` de `cr_pow()`, `x + x` de `cr_atan()`) sont contournés
  dans GAOL et à proposer en amont (PH.1). #183 et #184 sont fusionnées le jour
  même, et les registres mis à jour.
- **Les registres** : ménage des TODO. #49 et #142 sont fermées,
  `TODO_mistral.md` est supprimé, `TODO.md` est vidé puis réécrit avec le reste
  à faire en tâches PA à PJ ; les trois nouveaux registres sont #177 (ce
  fichier, `TODO_DONE.md`), #178 (`TODO_AFTER.md`) et #179 (`TODO.md`).
- **Les issues restées ouvertes** : #107 (corrigé par #147) et #159 (rapport
  d'état périmé) fermées ; #67 reste ouverte (reportée indéfiniment), les issues
  « À communiquer… » ne sont pas touchées.
- **`process.md`** : le titre garde « un point du TODO.md » ; les §2 (sources de
  vérité), §9 (après la fusion : les trois registres et leurs issues, avec le
  script qui joint les lignes pour les issues) et §12 (état au 24 octobre) sont
  réécrits pour #177 à #179, ainsi que les renvois des §3, §7 et §6.

## 4. Les points faits, par lettre

Les points qui ne sont pas (ou pas entièrement) faits ont leur reste dans
`TODO.md` (tâches PF à PJ) ou dans `TODO_AFTER.md` ; leur partie faite est dite
ici ou dans la section 5.

### Même changement, ou même code

- [x] **A.** Le correctif 6 de CORE-MATH, MinGW-w64 et Cygwin (anciens 31, 6,
  56, 3, 47) : fait par #111 et #113 ; envoi reporté (#125), mingw-w64 (#127)
- [x] **B.** pow et pown (1, 2, 8, 51, 72, 73, 74) : fait par #117, #118 et #121
  ; décidé pendant le travail (13 et 14 octobre) : la méthode du prototype P1y
  dans les `int`, `cr_pow` au-delà seulement, un double de trop accepté où x^n
  est à moins de 2K d'un double, `gaol_pown_ll`, x^(2^31 − 1) à 1,64 fois GAOL
  4.2.3, le produit `pow(e, [h])·e^l` pour les expressions au-delà de 2^53, le
  test `__int128` limité aux modes GNU, les mesures avec Clang 21
- [x] **C.** Formats d'affichage et vitesse d'`operator<<` (16, 17, 67) : fait
  par #71.
- [x] **D.** Le constructeur, les exceptions flottantes, les entiers et les
  en-têtes (24, 62, 21) : fait par #83, #84, #85 et #87 ; le signalement à GCC
  de la comparaison rendue signalante reste dans #126
- [x] **E.** Le parser et les longues sommes (12, 40, 45 ; #98) : fait par #105
  ; décisions ci-dessus

### Même fichier de test ou de documentation

- [x] **F.** La lecture des nombres et `operator>>` (11, 15, 18, 46, 58, 59, 60, 61)
  : fait par #114 (vérification unique par #116) ; syntaxe de la lecture par mot
  reportée (#64) ; `-Wundef` devient le point Z
- [x] **G.** Les options refusées (5, 53, 54, 55, 40), fait par #91 (et c7915a7,
  ccf694a) ; la vérification à l'exécution, essayée, n'est pas gardée (décidé le
  6 octobre).
- [x] **H.** Le sens d'arrondi (23, 36, 48, 27) : les anciens 23, 36 et 48 faits
  par #86 (`restore_rounding()`, la garde `rounding_guard`, `rnd_keep()` qui
  corrige `cancel_minus` sous GCC -O3, la documentation de ce que l'arrondi vers
  le haut fait au programme) ; le 27 par #88 (AVX-512 embedded rounding for +,
  -, *, /, sqrt) ; la partie manuel par #140
- [x] **I.** La documentation pour l'utilisateur (35, 37, 38) : fait par #140 :
  un programme dans `README.md`, `examples/tutorial.md`, table des noms, pièges
  dans le manuel et dans `doc/using.md` (dont les réglages de format non
  atomiques, décidé le 18 octobre) ; le site GitHub Pages (70) est au point AK

### Ordre imposé

- [x] **J.** Les en-têtes, puis `-Werror` (19, 20, 68, 16) : fait par #76
  (`gaol::sin(0.5)`, en-têtes internes non installés, macros préfixées `GAOL_`,
  un job GCC et un job Clang en `-Wall -Wextra -Werror`), puis dans
  `configure-clean` les jobs AppleClang et Visual C++ ; suites ci-dessus
- [x] **K.** clang-cl (49, puis 47) : fait par #72 (`GAOL_INFINITY` =
  `std::numeric_limits<double>::infinity()`) et #73 (refus sans `/fp:strict`,
  `-msse2` pour clang-cl x86, clang-cl de VS 2026 x86, x64 et arm64 dans la CI)
  ; la partie Cygwin du 47 reste au point A.
- [x] **L.** Les workflows de la CI (29, 43, 69) : locale à virgule hors de
  Linux, `TIMEOUT` de 400 s sous macOS Debug ASan+UBSan, Ubuntu 24.04 avant le
  17 avril 2027, `push` seulement sur `master`, `MATH-CORE` et
  `configure-clean`, puis les badges : fait par #74

### Autres points

- [x] **M.** `tan` (9, 57), fait par #89.
- [x] **N.** `cpack_stale_configure` (44, 63), fait par #94, avec
  `CPACK_VERBATIM_VARIABLES` (CMP0010) ajouté à la demande.
- [x] **O.** meson et les fichiers de build (41, 42, 52, 65) : fait par #95 ;
  décidé pendant le travail : le Release par défaut dans les trois builds, aussi
  en sous-projet, `enable-debug` et `GAOL_DEBUG` pour le débogage, la macro
  `GAOL_DEBUG_VERBOSE`, `pipefail` ; Visual C++ avant meson 0.57 et le backend
  Visual Studio de meson gardent le type de build du projet (accepté,
  documenté).
- [x] **P.** Les outils et ITF1788 (25, 30) : fait par #99 ; décisions ci-dessus
- [x] **Q.** Le flush-to-zero et DAZ hors du lecteur (4, 45 ; #68) : fait par
  #100 ; le test de `<a, b>` du parser va au point E
- [x] **R.** `atanh` et `hausdorff()` (7, 50), fait par #92 et #139
  (`atanh([1])` et `atanh([-1])` dans `doc/compare/special_cases.md`).
- [x] **S.** Les exceptions de GAOL (14), fait par #90.
- [x] **T.** Les exemples (39, 66) : fait par #93
- [x] **U.** Petites erreurs (40, 64) : fait par #97 ; décidé pendant le travail
  : les tests sans exceptions et deux jobs de `build-systems.yml`, l'exemple 15
  adapté, `gaol_ieee1788::textToInterval` sans exceptions reporté au point E
  (#98)
- [x] **V.** Un ordre total pour les conteneurs (22) : fait par #96 ; pas de
  spécialisation de `std::less`, décidé par le mainteneur (#70 fermée le 19
  octobre), la mise en garde par #140
- [x] **Z.** `-Wundef` (75) : les macros testées par `#if` dans les en-têtes
  installés toujours définies, à 0 ou 1, puis `-Wundef` dans `headers.sh` : fait
  par #120 ; décisions ci-dessus
- [x] **AA.** Shekel 5, plus lent qu'avec GAOL 4 (76) : fait par #122 ; décidé
  pendant le travail : le chemin AVX-512 à la configuration et le défaut gardé
  (15 octobre), le `combo` de meson, l'arrêt dans `init()` et GCC pour Windows
  écarté (16 octobre) ; décidé le 14 octobre : les points AG et AH le suivent,
  et la phrase « sqr et pown passent par uipow » de la décision du point H est
  retirée
- [x] **AB.** Des exceptions flottantes levées à tort (77 à 80) : le 77 (`pow`
  près du dépassement) par #144, le 78 et le 80 (`cancel_minus`, `cancel_plus`,
  `1/[0, 1]`) par #151, le 79 par #182 (la lecture de `1e400` et de `1e-310`) et
  #184 (les fonctions, et la phrase d'introduction de `doc/using.md` et du
  manuel)
- [x] **AC.** Les réciproques de `pow` et `nth_root` (81, 82) : le 81
  (`nth_root` d'un sous-normal) par #158, le 82 (`[+oo]` et `[-oo]` dans
  `pow_rel` et `pow_exponent_rel`) par #183
- [x] **AD.** La lecture et l'écriture des intervalles (83 à 89, 114) : les 83
  et 84 par #149, le 114 (`parse_interval(nullptr)`) par #147, les 87, 88 et 89
  (documentés) par #164, le 85 (l'exposant du lexeur en `long long`) par #182 ;
  le 86 (`_iv` au-delà de 2^64) est sans objet depuis le retrait du littéral
  `_iv`
- [x] **AG.** Le banc de `doc/compare` sur l'i7-1185G7 (104) :
  `doc/compare/performance.md` et les temps AVX-512 de `doc/using.md`, mesurés
  avant #122 : fait par #124
- [x] **AL.** Les restes des relectures de pull requests, de `process.md` et de
  la documentation (117 à 124) : fait par #164 (la phrase de `pow`
  d'`accuracy.md`, le manuel « not tested », le piège de `namespace gaol { }`,
  `.continue/` dans `.gitignore`, `gaol_performance` en C++14) ; le 118,
  `process.md`, fait par #155 ; le 123, les lignes de crédit, fait le 20 octobre
  (décidé le 19 octobre : les laisser, sans réécrire l'historique) ; fermer #70
  fait le 19 octobre
- [x] **AM.** Les suites du point Z (125) : clos le 22 octobre (Q3) :
  `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT` ont déjà des valeurs par
  défaut dans `gaol/gaol_config.h` (4 ou 8 selon `UINT_MAX`, 8 selon
  `ULLONG_MAX`), l'en-tête installé suffit
- [x] **AN.** Les pistes SIMD et FMA (126, issue #101) : fait par #154
  (`doc/simd.md`, qui répond aux quatre étapes de l'issue ; une voie AVX2, SVE,
  VSX ou RISC-V seulement après des mesures, NEON étant fait par #146), et
  `pow(x, 2)` par `sqr` par #162 ; ce qui reste est dans `TODO.md` (« Remarqué
  par #154, à trier »)
- [x] **AO.** Des macros de configuration que rien ne lit (127) :
  `GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX` et `GAOL_HAVE_AVX512_TARGET`, comme
  #141 l'a fait pour `GAOL_USING_ASM` et `GAOL_HAVE_ROUNDING_MATH_OPTION` :
  gardées (décidé le 19 octobre), la phrase qui disait que les sources les
  lisent corrigée par #143 (celle de `doc/differences.md` reste pour la pull
  request de synthèse)

- [x] **Hors des points (26 octobre).** Les options des trois builds : fait par
  #200 ; décidé pendant le travail : voir 3.4.

### Trouvé par la relecture de `configure-clean` du 14 octobre (62dad3a) : ce qui est fait

Six relecteurs indépendants (puissances entières, `pow` réel et réciproques,
autres opérations, lecture et écriture, trois builds, documentation), chaque
défaut reproduit par un programme sauf ceux dits non vérifiés dans `TODO.md`,
contre des références calculées à part (rationnels exacts, mpmath, MPFR). Aucune
borne fausse : environ 1,3 million de puissances entières, 550 000 `pow` réels,
600 000 autres opérations et 760 lectures de nombres encadrent tous la valeur
exacte. Le détail est dans `TODO.md`, anciens 77 à 103.
- [x] **AF.** Des textes que le code contredit (96 à 103), fait par #140 avec le
  point I : « tightest » sans la limite acceptée de `pown`, `mulRevToPair` dit
  absent, le signe d'un exposant, des comptes de tests, `GAOL_USING_ASM`, les
  temps de `pow(x, 3)` d'avant le point B, « keeps its names and its results »,
  le signe d'une borne nulle de `x * d` entre les chemins AVX-512 et SSE2
  (décidé le 18 octobre : une exception acceptable au « bit for bit », à dire
  dans la documentation et les commentaires, sans changer le calcul)

### Les points partiellement faits

Les points suivants gardent un reste (voir `TODO.md` et `TODO_AFTER.md`) ; voici
leur partie faite.

- **AE.** Les trois builds : le 90 (le compilateur C vérifié comme le C++) par
  #145, les 91 et 94 par #164, le 93 (code mort exporté) le 15 octobre, le 95
  en partie par #200 (les sondes d'exécution de FMA et d'AVX-512 dans les trois
  builds). Le 92 est fait par #201 (fusionnée le 26 octobre). Le reste du
  95 (les sondes liées, le C des drapeaux de FMA avec CMake) est fait par #204
  (fusionnée le 27 octobre) : AE est faite en entier.
- **AI.** La vitesse sur macOS, Windows et ARM : le 106 pour arm64 (intervalles
  NEON) par #146, pour Visual C++ et clang-cl x64 (SSE2) par #148, pour Windows
  32 bits avec GCC et Clang (SSE2 avec `-faligned-new=8`) par #152, pour ARM 32
  bits avec GCC (lecture directe de FPSCR) par #150, pour tous les intervalles
  FPU (noyaux sans branche) par #157, pour Visual C++, clang-cl, MinGW-w64 et
  MSYS2 sur arm64 (NEON, le 106 a) par #192, pour Visual C++ et clang-cl sur
  Windows 32 bits (SSE2, bornes en deux `double`) par #193 et pour le build
  universel de macOS (NEON et SSE2 par tranche) par #194 ; le 107 (FPCR en ligne
  sur arm64) par #146 ; le 108 (x87 et MXCSR en ligne sur x86 32 bits) par
  #156 ; le 110 (GCC pour Windows, `-mfma -mprefer-vector-width=128`) par #163,
  puis le 133 (x86 32 bits et Cygwin) par #172, #173 et #174, et les jobs qui
  manquent de #152, #173, #174, #193 et #194 (MSYS2 MINGW32, Cygwin avec
  autotools et meson, Clang vers Visual C++, macOS 15, Xcode) par #205 ; les 111
  et 115 (banc A/B, dix lignes de `run_lines.sh`) par #160 et #166 ; le 112, le
  défaut de CMake et les descriptions d'options par #161, le test des bits du
  signe de `x * d` par #171. Restent les 105 (AH), 106 (b, c, d), 108, 109, 112
  et 116.
- **AJ, AK, W, X, Y.** Rien n'est fait ; voir `TODO_AFTER.md` (AJ, AK, W, X) et
  `TODO.md` (Y, tâches PI et PJ).

## 5. Les anciens numéros faits

Les cases cochées de l'ancienne issue #142, avec leur compte rendu, par point.

### AB. Des exceptions flottantes levées à tort

- **77.** `pow([2], [1023.9999])` (et `pow(I, d)`, `gaol_pown_ll()`, `pow_rel`,
  `pow_exponent_rel`, `textToInterval`) lève `FE_INVALID` près du dépassement :
  `exp_1()` de CORE-MATH calcule ∞ − ∞. Décidé le 14 octobre : le correctif dans
  `3rd/math-core/src/binary64/pow/pow.c`, ligne marquée `/* GAOL */`, correctif
  8 de `3rd/README.md` (à proposer en amont, #125), et un cas dans
  `extreme_powers[]` de `tests/rounding_direction.cpp`. Fait le 19 octobre par
  #144 : `RHO2` abaissé à `0x1.62e42d29db3f0p+9` (le plus grand double tel que
  rh·`INVLOG2` ≤ 4194303,5 − 2^−31), cas dans `powers_near_overflow[]` de
  `rounding_direction.cpp` et dans `core_math.cpp`.
- **78.** `cancel_minus([DBL_MAX], [-DBL_MAX])` et `cancel_plus([DBL_MAX],
  [DBL_MAX])` lèvent `FE_INVALID` (`two_sum()` calcule ∞ − ∞) : e = 0 quand la
  somme est infinie, et [M] − [−M] dans le test. Fait le 20 octobre par #151 :
  `two_sum()` donne e = 0 pour une somme infinie, puis le Fast2Sum de Dekker
  (l'opérande de plus grand module en premier), qui corrige aussi une somme
  finie à mi-chemin de deux doubles que la relecture a trouvée ;
  `cancels_next_to_the_largest[]` dans `tests/rounding_direction.cpp`, 9 cas.
- **79 (la lecture).** Lire `1e400`, ou tout nombre au-delà du plus grand
  double, lève `FE_OVERFLOW` : le `strtod()` du lexeur (`gaol_enclose_number()`)
  le calcule, et un programme qui a activé le piège meurt à la lecture ; relevé
  par #149, ajouté au 79 le 20 octobre. Fait le 24 octobre par #182 : un nombre
  qui peut dépasser le plus grand double est comparé à DBL_MAX avant `strtod()`,
  qui n'est plus appelé s'il est au-dessus ; `FE_UNDERFLOW` de `strtod()` pour
  les nombres sous 2^-1022 est réglé de la même façon (décidé le 24 octobre) ;
  `readings_above_the_largest_double[]` et
  `readings_below_the_least_normal_double[]` de `tests/rounding_direction.cpp`.
- **79 (les fonctions).** `sqrt([1, DBL_MAX])`, `nth_root([DBL_MAX], 3)`,
  `sin([-1, DBL_MAX])`, `tan([DBL_MAX])`, `div_rel([1], [2^-1074, 1], [1, 2])`,
  `sqrt_rel([DBL_MAX], [-3, 0])` et d'autres levaient `FE_OVERFLOW` pour un
  résultat fini, alors que `doc/using.md` et le manuel disent que `FE_OVERFLOW`
  ou `FE_DIVBYZERO` signalent une borne infinie. Décidé le 14 octobre : corriger
  le code, non les textes ; mettre à l'échelle les vérifications de `sqrt` et
  des racines, et chercher les autres débordements intermédiaires. Fait le 24
  octobre par #184 (voir 2.3) : la sonde a trouvé 34 formes fautives sur 131,
  dont `pow_rel`, `pow_exponent_rel`, `atanh_rel` (division par zéro) et
  `nth_root` jusqu'à l'ordre 2^32, que le point ne nommait pas ; quotients et
  produits saturants, racines mises à l'échelle (et non le produit fusionné
  `fma(u, u, -v)`), `pow_up_without_overflow_exception()` ; la phrase « An
  infinite bound comes from a division by zero or from an overflow » est
  supprimée de `doc/using.md` et du manuel (décidé le 24 octobre) ;
  `flag_functions_raise_no_exception_for_a_finite_result()` et `trap_cases()` de
  `tests/rounding_direction.cpp`.
- **80.** `1/[0, 1]` ne lève plus la division par zéro depuis #121, mais
  `doc/using.md` et le manuel le donnent encore en exemple : garder `log([0,
  1])` seul. Fait le 20 octobre par #151 : `1/[0, 1]` retiré de `doc/using.md`
  et du manuel, où il reste `log([0, 1])` et `[1e308]*10`.

### AC. `nth_root` d'un sous-normal et réciproques de `pow`

- **81.** `nth_root([2^-1074], 33)` et `rootn` d'un sous-normal (n ≥ 4) sont à
  des milliers, voire des milliards de doubles des bornes les plus serrées que
  `doc/accuracy.md` promet : les preuves `ipow_hi()` et `ipow_lo()` passent par
  une puissance sous-normale. Prouver avec une puissance mise à l'échelle, ou
  corriger les textes. Fait le 20 octobre par #158 : pour x < 2^-1020, les
  preuves prennent des puissances mises à l'échelle (`ipow_hi_scaled()`,
  `ipow_lo_scaled()`, `compare_scaled()`), choisies une fois par des gabarits
  (`plain_proofs`, `scaled_proofs`) pour que les autres x gardent le code
  d'avant ; `roots_of_tiny_numbers()` dans `tests/arithmetic.cpp`, qui échoue 17
  fois avec les sources de `configure-clean` ; une borne peut être à 2 doubles
  de la plus serrée (4 pour q < 0), pour tout x, ce que disent le commentaire de
  `newton_root()` et `doc/accuracy.md`.
- **82.** `pow_rel` et `pow_exponent_rel` (`powRev1`, `powRev2`) n'avaient pas
  la garde de `[±oo]` de `pow` : `powRev1([+oo], [2], R)` rendait [1, 1 +
  2^-52], `powRev2([+oo], [2], R)` [0, 2^-1074], `powRev1([2], [+oo], R)`
  [DBL_MAX, +oo] et `powRev1([-oo], [0, 1], R)` [1, +oo], là où `pow(x, [±oo])`
  est vide ; ces arguments ne se construisent qu'avec `interval(__m128d)` ou son
  équivalent NEON. Fait le 24 octobre par #183 : `holds_no_real()` dans
  `pow_rel_upward()` et `pow_exponent_rel_upward()`, et
  `pow_reverse_infinite_arguments()` de `tests/reverse.cpp` (287 vérifications
  échouent sans la correction). Le reste, décidé le 24 octobre, une garde dans
  le constructeur public, est fait le 25 octobre par #189.

### AD. La lecture et l'écriture des intervalles

- **83.** Un exposant entier hors des `int` est refusé dans un texte
  (`nth_root(8,3000000000)`, `rootn`, `pown`), alors que `nth_root(8,
  3000000000LL)` est calculé. Fait le 20 octobre par #149 : le second argument
  est un `long long` (`gaol_integer_exponent()`), saturé à ±(2^63 − 2) à partir
  de 2^63 ; `integer_exponents()` dans `tests/expressions.cpp`.
- **84.** `[l, u]` avec l > u est lu comme un intervalle non vide quand les deux
  nombres s'arrondissent au même double (`[9007199254740993, 9007199254740992]`
  donnait [2^53]). Fait le 20 octobre par #149 : le vide dans les deux espaces
  de noms ; les nombres qui ne sont pas des doubles sont des `inside_node`,
  strictement dans leur encadrement, et deux d'entre eux entre les mêmes deux
  doubles sont comparés exactement (`compare_numbers()` du lexeur) ; pi contre
  un autre nombre, les nombres loin au-delà des doubles et les bornes calculées
  restent non départagés (sur-ensemble sûr) ; `<a, b>` refuse une borne qui
  n'est pas un double ; `reversed_bounds()` et `degenerate_bounds()` dans
  `tests/expressions.cpp`.
- **85.** Le plafond de l'exposant déborde un `long` de 32 bits (Windows, i386,
  armhf) : `"1e"` suivi de 536,8 millions de 9 était lu [0, 2^-1074] au lieu de
  [DBL_MAX, +oo]. Fait le 24 octobre par #182 : `k`, `k2`, `kb`, `exponent`,
  `q`, `places` et les comptes de chiffres du lexeur en `long long` ; le test de
  512 Mio qui la gardait est retiré (1,5 Gio sur les jobs Windows x64), la
  preuve est dans la description de #182 (conteneur Debian i386).
- **87.** Un nombre incertain juste après un `+` ou un `-` binaire est refusé
  (`10-3.56?1`, `[1,2]+3?`), alors que le manuel dit qu'ils se combinent dans
  n'importe quel ordre : décidé le 19 octobre : le documenter sans changer le
  lexeur (une espace est nécessaire : `10 - 3.56?1`). Fait le 23 octobre par
  #164 : documenté dans le manuel (paragraphe « The uncertain form ») et dans
  `doc/using.md` (section « Common errors »).
- **88.** Lire un long nombre prend un temps quadratique en ses chiffres (0,28 s
  pour 80 000 chiffres, 4,4 s pour 320 000), donc un texte venu d'ailleurs peut
  bloquer un programme. Décidé le 19 octobre : garder la décision du 3 octobre
  de l'ancien 18 (« on ne fait rien ») et documenter dans le manuel et `doc/`
  que la lecture est quadratique en le nombre de chiffres. Fait le 23 octobre
  par #164 : documenté dans le manuel (paragraphe « Numbers ») et dans
  `doc/using.md`, avec les temps (0,017 s pour 20 000 chiffres, 0,28 s pour 80
  000, 4,4 s pour 320 000).
- **89.** Le format `agreeing` écrit `interval(4.0)` `4.000000000000000`, quand
  `bounds` écrit `[4]` : à documenter dans le commentaire d'`interval_format` et
  dans le manuel. Fait le 23 octobre par #164 : documenté dans le commentaire
  d'`interval_format` (`gaol/core/gaol_interval_core.h`) et dans le manuel
  (section « Output format ») ; un zéro s'écrit `[0]` dans tous les formats.
- **114.** (nouveau, issue #107) `parse_interval(nullptr)` plante (`strlen` d'un
  pointeur nul) : rendre `false`, avec un test dans `tests/expressions.cpp` et
  une note Doxygen dans `gaol/parser/gaol_parser.h`. Fait le 20 octobre par #147
  : `parse_interval()` rend `false` pour un pointeur nul, `null_pointer()` dans
  `tests/expressions.cpp`.
- **`<a, b>` et une borne calculée** (question de #149). `<a, b>` acceptait une
  borne calculée qui n'est pas un double (`<[0.1], 0x1.9999999999999p-4>`) ;
  seuls les nombres écrits et pi étaient refusés. Fait le 24 octobre par #182 :
  toute borne dont la valeur n'est pas un point est refusée par
  `input_format_error` ; huit entrées de plus à `degenerate_bounds()` de
  `tests/expressions.cpp`.

### AE. Les trois builds

- **90.** autotools et meson ne vérifient que le compilateur C++ pour Clang sur
  ARM 32 bits, et `-frounding-math` que le C++ dans les trois builds, alors que
  CORE-MATH est compilé en C. Non vérifiable ici. Fait le 19 octobre par #145 :
  le compilateur C vérifié comme le C++ dans les trois builds, les jobs
  `clang-arm32` et `clang-14-arm64` vérifient les neuf refus.
- **91.** `tests/refused_options.sh` se dit sauté (77) au lieu d'échouer quand
  le programme ne compile pas : prétraiter `/dev/null`. Fait le 23 octobre par
  #164 : le script sonde le compilateur avec `$GAOL_CXX $GAOL_CXXFLAGS -dM -E -x
  c++ /dev/null` ; sans les en-têtes de GAOL, il échoue (FAIL) au lieu de se
  dire sauté.
- **94.** `make_probe.py` sonde `GAOL_RND_SSE_REGISTER`, que `compare.py` ne
  compare pas. Fait le 23 octobre par #164 : `GAOL_RND_SSE_REGISTER` ajouté aux
  `MACROS` de `compare.py`.

### AP. Les suites des réponses du 22 et du 23 octobre

- **128.** `gaol::textToInterval(nullptr)`, `textToInterval(nullptr, "1")`,
  `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)`
  construisent un `std::string` d'un pointeur nul avant GAOL (indéfini ;
  `std::logic_error` avec libstdc++). Décidé le 22 octobre (Q13) : lever
  `input_format_error`. Fait le 24 octobre par #180 : des surcharges `const
  char*` ; `null_text()` de `tests/expressions.cpp`.
- **Un `file` nul dans une `gaol_exception`** (question de #147). Écrit sur un
  flux, il le met à l'état `bad` avec libstdc++. Décidé le 24 octobre : «
  (unknown file) ». Fait le 24 octobre par #181, avec le message nul de
  `gaol_warning()` et de `gaol_error()` (texte vide).
- **133.** GCC pour Windows : x86 32 bits sans FMA et Cygwin. Fait par #172
  (`-mfma -mprefer-vector-width=128` aussi à 32 bits, `pow(x, 3)` de 70 à 12 ns,
  un job sans FMA qui garde le `fma()` x87 de mingw-w64 11) puis #173 et #174
  (Cygwin x86-64 pris en charge : `_mm_malloc`,
  `previous_float()`/`next_float()` aux bords des doubles, `fma_once()` et le
  test d'exécution de `GAOL_FMA` en `__builtin_fma`, refusé sans FMA, pas de
  `package_source` sous Cygwin, un job de CI ; `is_bisectable()` par
  `next_float()`). Fusionnées les 23 et 24 octobre. Restent le Debug complet de
  Cygwin et autotools et meson sous Cygwin, non vérifiés.
- **131.** Les noyaux de SSE2 (`add_bounds()`, `sub_bounds()`, `mul_bounds()`,
  `div_bounds()`, `sqr_bounds()`) exportés dans `gaol_core` : dans
  `gaol_detail`, comme les autres aides internes. Décidé le 22 octobre (Q15a,
  #124) ; Q15b (garder le plus rapide) et Q15c (ne pas passer le sens d'arrondi
  à `3rd/math-core`) sont des « non ». Fait le 25 octobre par #186, avec les
  noyaux NEON (voir 2.3).
- **129.** Durcir `sqrt_bounds()` des intervalles NEON par un masque sur le bit
  de signe, à la place du maximum, contre `FPCR.AH`. Décidé le 22 octobre (Q10a,
  #154) : validé sur une copie, bit à bit identique, dans une pull request à
  part. Fait le 25 octobre par #187.
- **130.** `process.md` : une phrase pour Ubuntu 24.04 au §6.1 à la place du
  paragraphe que #155 avait retiré, une date et un commit dans l'en-tête, rien
  pour `GAOL_V9/GAOL` au §3.3. Décidé le 22 octobre (Q9a à Q9c, #155). Fait le
  25 octobre par #196.
- **132.** Porter le noyau empaqueté de `pown` (`GAOL_POWN_PACKED`) sur arm64.
  Décidé le 22 octobre (Q14a, #146) ; Q14b (un test des bits du signe d'un zéro)
  et Q14c (jobs Clang 15 à 17 et GCC 9 à 10, test `mrs` et `msr` sur Neoverse)
  sont des « non ». Fait le 25 octobre par #188, avec la répartition des noyaux
  par sorte d'intervalles (voir 2.3) : gain de 1,03 à 1,07 sur un M2.
- **`bisect(ratio)` et `is_bisectable()` sous FZ** (reste du point Q, #146).
  Décidé le 24 octobre : revérifier le défaut, puis le corriger par les bits,
  avec un test qui échoue sans la correction. Fait le 25 octobre par #190.
- **Les restes de la tâche PD** (25 octobre) : les cinq pièges de #149, #150,
  #192 et #194 dans `process.md` §11 ; `doc/accuracy.md` sans `interval(sl, su)`
  (#149) ; le nom du build par défaut dans `itf1788.py` (#146) ; les tests de
  `std::pow` (#158), rien à changer ; le masquage des exceptions de #184 (#185),
  gardé ; les mutants de `mutate.py` (#188), dont les dix `neon-*` lancés sur un
  Apple M2 ; l'échec de CI de #189, passé dans `TODO_AFTER.md`. Faits par #196
  et #197 (voir 2.3).

### Restes des relectures de pull requests, de `process.md` et de la documentation (AL, nouveau)

- **117.** `doc/accuracy.md`, ligne de `pow` : supprimer la phrase « 4 doubles
  on the special cases » (décidé le 18 octobre, question de #99). Fait le 23
  octobre par #164 : la phrase est supprimée.
- **118.** `process.md` : retirer au §6.1 le paragraphe du 12 octobre sur une
  machine Ubuntu 24.04 (décidé le 18 octobre) ; `git switch --no-track -c` au
  §7.3 (question de #109) ; refaire le §12 « Où en est le travail » et l'en-tête
  « État au 13 octobre » ; au §6.2, `-DGAOL_ARM64=OFF` avec `-DGAOL_SIMD=OFF`
  pour la variante FPU (l. 289, comme l. 107 de
  `todo-notes/orchestration/HOUSE_RULES.md`). Fait le 20 octobre par #155 : le
  paragraphe d'Ubuntu 24.04 retiré du §6.1, `git switch --no-track -c` au §7.3,
  `-DGAOL_ARM64=OFF` avec la variante FPU au §6.2 et dans `HOUSE_RULES.md`,
  l'en-tête et le §12 refaits ; et, au-delà du point, les §2 et §9.
- **119.** Manuel, « Compilers and options refused » : « Each of the following
  gave bounds not enclosing the exact results » est suivi de l'entrée MinGW-w64
  pour ARM « not tested » (remarqué par #111). Fait le 23 octobre par #164 :
  reformulé en « Each of the following gave bounds not enclosing the exact
  results or was not tested ».
- **120.** Documenter que le code écrit dans `namespace gaol { }` voit les
  using-declarations de `gaol` (`sqrt(2.0)` n'y compile plus) : pièges de «
  Common errors » (remarqué par #87). Fait le 23 octobre par #164 : ajouté aux
  pièges de « Common errors » de `doc/using.md` et du manuel.
- **(AI, #148)** `/W4 /WX` en Debug : `textToInterval()` de `gaol_interval.cpp`
  a un `return` après `gaol_ERROR()`, inatteignable à `/Od` pour Visual C++
  19.44 et 19.51 (C4702, vu sur Compiler Explorer) ; aucun job ne compile en
  `/W4 /WX` en Debug. Fait le 23 octobre par #164 : les `return` retirés de
  `textToInterval()`, un job « Visual Studio 2026 x64 Debug, /W4 /WX » dans
  `windows.yml`, `/wd4702` pour le parser bison dans `CMakeLists.txt`.
- **121.** `.continue/` dans `.gitignore` (question de #103 ; décidé le 22
  octobre, Q2). Fait le 23 octobre par #164 : `.continue/` ajouté au
  `.gitignore`, avec un commentaire.
- **122.** `gaol_performance` est compilé en C++17 dans les trois builds sans en
  avoir besoin (remarqué par #95). Fait le 23 octobre par #164 : C++14 dans les
  trois builds (CMake `CXX_STANDARD 14`, autotools `-std=c++14`, meson
  `cpp_std=c++14`).
- **123.** Lignes de crédit qui restent : les descriptions de #15 à #24 («
  Generated with Claude Code »), de #60 à #63 et un commentaire de chacune de
  #58 et #60 à #63 à nettoyer ; 24 commits (14 au 21 septembre, dont 16 dans
  `master`) portent « Co-Authored-By: Claude Opus 5 » et 6 commits de #99 «
  Co-authored-by: Jordan08 » : on ne les corrige pas sans réécrire l'historique
  de branches partagées. Décidé le 19 octobre : les laisser, sans réécrire
  l'historique avant la fusion dans `MATH-CORE` et `master`. Fait le 20 octobre
  pour les descriptions et les commentaires de pull requests ; les commits
  restent (décidé le 19 octobre).
- **124.** Fermer #70 (le mainteneur y a répondu : pas de `std::less` pour
  `gaol::interval`) et retirer de #49 « la spécialisation de `std::less` reste
  reportée » : fait le 19 octobre.
- **127.** (AO, question de #141) `GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX` et
  `GAOL_HAVE_AVX512_TARGET` sont définies par les trois builds et lues par aucun
  code (celle-ci par l'audit seulement) : décidé le 19 octobre, on les garde, et
  #143 corrige les phrases « les macros que lisent les sources de GAOL, et rien
  d'autre » (celle de `doc/differences.md` reste pour la pull request de
  synthèse) : fait le 19 octobre.
- **125.** (AM) `GAOL_SIZEOF_INT` et `GAOL_SIZEOF_LONG_LONG_INT` laissés à
  `<limits.h>`, sans valeur par défaut (question de #120, case de #49) ; les
  tests et les `.cpp` de la bibliothèque ne sont pas sous `-Wundef`. Clos le 22
  octobre (Q3) : `gaol/gaol_config.h` a déjà les valeurs par défaut (4 ou 8
  selon `UINT_MAX`, 8 selon `ULLONG_MAX`) ; point retiré de `TODO.md` le 23
  octobre.

### Études de vitesse, toutes avant v5.0.0 (décidé le 19 octobre), sauf le 113 (reporté après la version le 22 octobre)

- **107.** `GAOL_PRESERVE_ROUNDING` : 45 à 50 ns par opération sur macOS arm64
  (trente fois le build par défaut) ; lire et écrire FPCR en ligne (`mrs`,
  `msr`). Fait le 19 octobre par #146 : FPCR lu par `mrs` et écrit seulement
  s'il change, `GAOL_PRESERVE_ROUNDING` de 39 à 27 ns par opération sur un M2 ;
  les 27 ns qui restent (deux `msr`) sont dans le 113.
- **110.** GCC pour Windows : `pow(x, 3)`, `log` et `cos` 5 à 10 fois plus lents
  que Clang, probablement faute de `-mfma` (`fma()` logiciel) ; mesurer avec
  `-mfma` donné à la main. Fait le 23 octobre par #163 : GCC pour Windows x86-64
  reçoit `-mfma -mprefer-vector-width=128` (aucune instruction `%ymm` dans les
  objets, ce que `.github/scripts/check-gcc-windows-fma.sh` vérifie à chaque
  build) ; sur le runner (MSYS2 UCRT64, GCC 16.2), `pow(x, 3)` de 52,3 à 8,7 ns,
  `log` de 106,4 à 33,6 et `cos` de 260,5 à 55,4. Deux défauts de
  `GAOL_PRESERVE_ROUNDING` avec mingw-w64 13 et plus sur x86-64, que les
  nouvelles entrées du job MSYS2 ont montrés, sont corrigés : `fesetround()`
  efface les indicateurs de MXCSR (`gaol::cleanup()` et `restore_rounding()`
  aussi, sans la préservation) et `fegetround()` joint par OU les champs RC des
  deux unités ; GAOL lit et écrit maintenant lui-même les registres.
- **111.** Les temps de la CI ne se comparent pas d'un passage à l'autre :
  mesurer deux builds alternés dans le même job, ou garder le meilleur de
  plusieurs passages (à décider). Décidé le 22 octobre (deux builds alternés) et
  fait par #160 (fusionnée dans la nuit du 22 au 23 octobre) :
  `.github/workflows/performance.yml` construit et mesure, dans le même job, la
  base d'une pull request (le premier parent de son commit de fusion) et sa
  tête, en trois passages alternés (Ubuntu 24.04 avec GCC, Clang et
  `GAOL_PRESERVE_ROUNDING`, macOS 26 arm64, Visual Studio 2026 x64 et x86,
  MSYS2), et écrit dans le résumé du job un tableau base / pull request /
  rapport, sans qu'aucun temps ne fasse échouer un job ;
  `.github/scripts/performance-ab.sh`, `compare-performance.py` et un test des
  outils.
- **115.** (nouveau) `make perf` et l'étape « Performance » de la CI ne lancent
  pas les dix lignes de `run_lines.sh` (question de #124). Fait par #160 : `make
  perf` lance les dix lignes dans les trois builds (aussi sur macOS), et l'outil
  de la CI les compare entre la base et la pull request.
- **126 (AN).** Évaluer les pistes SIMD et FMA : fait par #154 (`doc/simd.md`,
  qui répond aux quatre étapes de l'issue #101 ; le `// TODO: Rewrite as many
  functions as possible to use SIMD instructions` de
  `gaol/core/gaol_interval_core.cpp` est remplacé). Ce qui reste (durcir
  `sqrt_bounds()` contre `FPCR.AH`, fait par #187 le 25 octobre ; l'arrondi
  embarqué scalaire de `/` sur Intel, fait par #191 le 25 octobre ; RISC-V
  scalaire, POWER, s390x) est dans `TODO_AFTER.md` ; `pow(x, 2)` par `sqr` est
  fait par #162 (3,3 fois plus rapide avec NEON, 2,5 avec la FPU, au signe d'un
  zéro et aux indicateurs de `sqr` près).
- **106 (a).** NEON pour Visual C++ arm64 (`arm64_neon.h`), qui n'y était pas
  pris, ni avec clang-cl ni avec MinGW-w64. Fait le 25 octobre par #192, puis, à
  la demande du mainteneur (NEON avec tous les compilateurs de l'arm64, SSE2
  avec tous ceux de l'x86, le FPU là où l'on n'a pas le choix), par #193 (SSE2
  pour Visual C++ et clang-cl sur x86 32 bits) et #194 (le build universel de
  macOS) : voir 2.3 pour les mesures et la section 1 pour la règle.
- **PG.6** (tâche PG, nouveau). La division du chemin AVX-512 : essayer deux
  `vdivsd {er}` scalaires pour `/` (`doc/simd.md`, tableau « Verdicts »). Fait
  le 25 octobre par #191 : `x / d` de 3,62 à 2,76 ns sur l'i7-1185G7 ; le `x /
  y` de l'interface le faisait déjà depuis cf60ef6, ce que la description du
  point ignorait ; les divisions du dispatch et du `1/h` sont converties sans
  gain (`TODO.md`, PG.9, pour les questions restées ouvertes).

### Décisions du mainteneur attendues (reportées)

- **F (46, #64).** `operator>>` : garder un intervalle par ligne, n'accepter que
  les littéraux lus jusqu'au crochet fermant, ou une fonction à part pour lire
  un intervalle dans un flux. Décidé le 22 octobre (Q4) : garder un intervalle
  par ligne (comportement actuel, documenté), pas de changement ; l'issue #64
  est fermée le 23 octobre.
- **W (26, #67).** Décorations d'IEEE 1788, simple indicateur de sortie de
  domaine local au thread, ou seulement documenter. Décidé le 22 octobre (Q5) :
  rien pour l'instant, GAOL reste un intervalle nu et les pièges sont documentés
  (#140) ; reporté indéfiniment.
- Les décisions à prendre des sections 1 à 3 : 121 (fait par #164, décidé le 22
  octobre, Q2) et 125 (clos le 22 octobre, Q3).

## 6. Les questions ouvertes des pull requests, tranchées

Les « Questions ouvertes » des descriptions de #146 à #163, que l'ancien
`TODO.md` gardait sous « Remarqué par #NN, à trier », avec leur sort. Les
questions qui restent sans réponse sont dans `TODO.md` (tâches PF à PJ) et
`TODO_AFTER.md` ; elles ne sont pas répétées ici.

### #146

- **`pow(x, 3)` à 6,5 ns sur arm64** : le noyau empaqueté de `pown`
  (`GAOL_POWN_PACKED`, FMA sur deux voies) n'existe que pour SSE2 ; un
  `vfmaq_f64` ne gagnerait qu'un facteur de l'ordre de 1,5, pour 150 lignes
  d'intrinsèques SSE2 à porter sans pouvoir les tester avec GCC. *Décidé le 22
  octobre (Q14a)* : le porter, c'est le 132.
- **Le signe d'un zéro** diffère de celui des intervalles FPU et SSE2 avec NEON
  (documenté, comme pour AVX-512) : aucun test ne compare les bits d'un zéro.
- **Pas de job** pour Clang 15 à 17 ni GCC 9 à 10 sur arm64, et aucun test de
  `mrs` et `msr` sur Neoverse (le coût et la présence de FIZ ne sont pas mesurés
  ; #150 mesure seulement, sur les runners arm64 de GitHub, les intervalles NEON
  avec FPCR 1,4 à 2,0 fois plus rapides que les intervalles FPU avec la
  bibliothèque C, avec `GAOL_PRESERVE_ROUNDING`). *Décidé le 22 octobre (Q14b,
  Q14c)* : ni test des bits du signe d'un zéro, ni ces jobs.
- **Le nom de `float64x2_t` est décoré autrement** par GCC et par Clang sous
  Linux : une bibliothèque installée construite par l'un et liée à un programme
  compilé par l'autre ne lierait pas, comme pour `__m128d` avec SSE2 (`gaol.pc`
  et `gaol::gaol` donnent le compilateur du programme).

### #147

- **`gaol::textToInterval(nullptr)`**, `textToInterval(nullptr, "1")`,
  `gaol_ieee1788::textToInterval(nullptr)` et `exactToInterval(nullptr)`
  prennent un `const std::string&` : le `std::string` est construit à l'appel,
  avant GAOL, ce que le standard laisse indéfini. Avec libstdc++ (GCC 13.3), les
  quatre lèvent `std::logic_error` (« basic_string: construction from null is
  not valid »), ni `input_format_error` ni l'ensemble vide ; avec libc++ et
  Visual C++, probablement un plantage dans `strlen`, non vérifié. Le changer
  demande une surcharge `const char*` : un choix d'interface. *Décidé le 22
  octobre (Q13)* : lever `input_format_error`, c'est le 128, fait par #180.

### #149

- **Le contexte d'une lecture garde le texte de chaque nombre qui n'est pas un
  double**, qu'un littéral `[l, u]` le compare ou non : une somme de 200 000
  termes `0.1` passe de 4,9 Mo et 0,106 s à 6,0 Mo et 0,134 s. Ne garder que les
  textes des bornes demande de savoir, à la lecture d'un nombre, s'il sera une
  borne.

### #150

- **Les sujets de deux commits** de la branche de #150 font plus de 100
  caractères (117 et 101) : déjà poussés, non réécrits.

### #154

- **`FPCR.AH` (FEAT_AFP) n'est pas effacé** par les intervalles NEON
  (`GAOL_RND_FLUSH_BITS` : FZ et FIZ). Avec AH, `FMAX` renvoie le second
  opérande pour deux zéros de signes différents ; modélisé en logiciel sur 25,8
  millions de résultats, les bornes sont celles des noyaux, au signe d'un zéro
  près. Une seule dépendance : `sqrt_bounds()` suppose l'ordre des opérandes de
  `vmaxq_f64`, que Clang 21 traite comme commutatif ; un compilateur qui les
  permuterait donnerait l'ensemble vide pour `sqrt([-0, r])` sous AH. Défaut
  théorique, documenté dans la page. Un masque sur le bit de signe à la place du
  maximum, validé sur une copie (bit à bit identique), en une pull request à
  part. *Décidé le 22 octobre (Q10a)* : c'est le 129.
- **Les runners arm64 de GitHub exécutent l'armhf en natif** : Neoverse N2, «
  32-bit EL0 Support » au démarrage, aucune entrée `qemu-arm` (job jetable du 20
  octobre). Les temps de #150 ne sont donc pas ceux d'un émulateur.
- **Le chemin générique de `round_upward()`** (`gaol/gaol_fpu_fenv.h`) appelle
  `fesetround(FE_UPWARD)` sans en lire le retour ; `NI` de POWER et `FS` de
  MIPS, états du même ordre que FTZ et DAZ, ne sont pas effacés par les
  intervalles FPU de ces processeurs. *Décidé le 23 octobre* : le dire sans rien
  changer ; fait par #165 (fusionnée).
- **Trois pièges de compilation**, qu'aucun code de GAOL n'emploie : la pragma
  `FENV_ROUND` (ignorée sans erreur par GCC, Visual C++ et Clang), le
  `_mm256_add_round_pd` de Clang 20 avec `-mavx10.2` et l'assembleur `vaddpd
  {ru-sae}, %ymm...` (assemblé par GNU as, `#UD` d'après le manuel d'Intel
  d'aujourd'hui).

### #155

- **Le paragraphe retiré du §6.1 reste vrai sur une machine** : l'arbre
  `GAOL_V9/GAOL`, les 19 et 20 octobre, avait les outils qu'il décrivait (Ubuntu
  24.04, GCC 13.3, Clang 18 et 20, CMake 3.28, python 3.12, sans autoconf,
  automake, libtool, meson ni ninja, 4 cœurs) : autoconf 2.72 s'y construit
  depuis le tarball de ftp.gnu.org (sha256 vérifié), meson et ninja s'installent
  dans un environnement virtuel. Une phrase plus courte à la place du paragraphe
  ? *Décidé le 22 octobre (Q9a)* : oui, c'est le 130.
- **Les cœurs** : le §3.3 dit 4 cœurs au plus (« la machine en a 8 ») ; sur
  cette machine, qui en a 4, le mainteneur en a demandé 2 au plus. Une ligne au
  §3.3 pour l'arbre `GAOL_V9/GAOL` ? *Décidé le 22 octobre (Q9b)* : non.
- **L'en-tête** de `process.md` renvoie au §12 au lieu de donner une date et un
  commit, qui se périmeraient à la fusion suivante. *Décidé le 22 octobre (Q9c)*
  : une date et un commit, c'est le 130.
- La branche distante `todo-ai-msvc-avx512` n'avait pas de pull request : son
  dernier commit (`fbb962c`) est un ancêtre de `configure-clean`, elle ne
  portait rien de plus ; supprimée sur GitHub le 24 octobre, sur décision du
  mainteneur (qui avait répondu de la garder le 22, Q16).

### #158

- **Un x sous-normal, ou de [2^-1022, 2^-1020[, prend 1,3 à 2,2 fois plus de
  temps** (chemin mis à l'échelle, sans pas de Newton pour un sous-normal : 129
  ns avant et 286 après pour `nth_root([1e-310], 5)`, 86 et 153 pour n =
  33) ; les x ordinaires sont inchangés. Acceptable, ou donner à `newton_root()`
      des puissances mises à l'échelle ? Temps pris sur une machine non au repos
      : à refaire au point Y. *Décidé le 23 octobre* : accepter le
      ralentissement.

### #157

- **Une piste non essayée** : un noyau mixte qui garde le chemin à une branche
  quand les deux opérandes ont le même signe, et le noyau à huit produits sinon.
  La variante hybride essayée (l'ancien chemin pour les opérandes positifs,
  mesurée hors de l'arbre) donne sept `*` positifs en 20,5 ns, mais `x * y` de
  signes aléatoires en 8,5 ns et sept `*` aléatoires en 49 ns contre 35 pour le
  noyau : une branche sur un signe de nouveau. *Décidé le 23 octobre* :
  l'essayer, sur la machine du mainteneur et par le banc A/B de la CI. Fait : la
  pull request brouillon #168 ; en local (i7-1185G7, médianes de 5 passes
  alternées) la ligne de 7 `*` gagne 1,44 fois avec GCC 15.2 mais `x * y` de
  signes aléatoires, `mul8` et `mix_tree` perdent 11 à 24 %, et avec Clang 21
  rien ne gagne (`x * y` 0,94, la ligne de 7 `*` 0,83). Le banc A/B de la CI
  (base / PR, médianes de 3 passages) dit autre chose : la ligne de 7 `*` gagne
  1,96 fois (GCC x86-64), 1,70 (arm64 GCC), 1,46 (Visual C++ arm64), 1,67
  (clang-cl arm64) et 1,43 (armhf), et `x * y` de signes aléatoires perd 5 à 19
  % (0,89 GCC x86-64, 0,93 Visual C++ arm64, 0,81 clang-cl arm64, 0,91 armhf),
  sauf arm64 GCC (1,30) ; rien avec Clang x86-64 (0,90 la ligne de 7 `*`) ni
  Visual C++ x86 (0,95). Le mainteneur l'a fermée sans fusion le 23 octobre, ces
  tableaux lus : l'essai est un changement de `mul_finite()` dans
  `gaol/core/gaol_interval_fpu.cpp` (commit `f86ada7`, lisible dans #168), qu'on
  reprendrait si les signes réguliers deviennent le cas qu'on veut servir (rang
  11).
- **`gaol_performance` apprend ses opérandes par cœur** : 1 024 opérandes
  répétés, que le prédicteur du M2 apprend (`x * y` de signes aléatoires 5,7 ns
  contre 12,4 sur 16 384). Ses lignes « signes aléatoires » sous-estiment donc
  le coût d'une branche sur les signes : passer ces trois lignes à 16 384
  opérandes, ou en ajouter trois ? Seul un commentaire de
  `tests/performance.cpp` a changé, pour ne pas rompre la comparaison avec les
  tableaux de `doc/three-builds.md`. *Décidé le 23 octobre* : ne rien changer.
- **`fmax()` pour le plus grand de deux doubles** : une instruction sur arm64,
  RISC-V et POWER (2,2 ns contre 2,9 pour `x * y` dans un banc hors de l'arbre),
  mais elle classe +0 au-dessus de −0 (le signe d'un zéro serait celui de NEON)
  et appelle la bibliothèque sur ARM 32 bits et x86 ; utile surtout à Visual C++
  et clang-cl pour arm64, qu'on ne peut pas mesurer. *Décidé le 23 octobre* :
  non.
- **`GAOL_RND_KEEP()` sur la sortie de `*=`, `/=` et `%=` ?** Aujourd'hui non :
  les bornes de `*this` sont de la mémoire de l'appelant, donnée aussi par
  adresse aux fonctions des cas rares, si bien que le compilateur place les
  stores avant un appel ou un `asm` à clobber « memory ». C'est une protection
  de fait, qu'aucun test ne tient ; la remettre en ferait une règle, pour 2,6 ns
  (`x * y`) et 5 ns (`x / y`) avec `GAOL_PRESERVE_ROUNDING` (sans lui, la macro
  est vide). *Décidé le 23 octobre* : ne pas la remettre, tenir la protection
  par un test, avec GCC et la préservation du sens ; fait par #170 (fusionnée) :
  les `tight_cases` de `rounding_direction.cpp` la tiennent déjà (deux mutants
  de `operator*=`, `/=` et `%=` les font échouer, 20 035 vérifications avec GCC
  et Clang pour le noyau calculé après `GAOL_RND_LEAVE()`), il manquait des jobs
  qui les lancent avec les intervalles FPU et la préservation sur x86-64 et
  arm64 GCC ; pas de job avec `-flto`.
- **`mid()` et `div_rel()`** ne sont pas touchés (`mid()` coûte 7,7 ns avec NEON
  comme avec la FPU) ; les opérations avec un double (`x * d`, `x / d`) ont
  encore une branche sur le signe de `d`, comme avec SSE2 (#124).
- **La seconde relecture de `process.md` §7 n'a pas été faite** (la première,
  sur `b584d78`, n'a pas eu de point bloquant ; *décidé le 23 octobre* : ne pas
  la faire). Non vérifiés : l'exécution sur ARM 32 bits, POWER, s390x et RISC-V,
  le noyau SSE2 (comparé à un modèle, jamais exécuté) et la vitesse sur ces
  processeurs.

### #160

- **Le sens du rapport** vaut base sur pull request (au-dessus de 1, la pull
  request est plus rapide, comme le « Gain » de #150) ; beaucoup de lecteurs
  attendent l'inverse. C'est une ligne de `compare-performance.py` et de son
  test. *Décidé le 23 octobre* : garder base sur pull request.
- **Le filtre de chemins** de `performance.yml` n'a ni `Makefile.am`,
  `configure.ac`, `meson.build` et `meson_options.txt` (l'outil ne mesure que le
  build de CMake), ni `doc/compare/code/run_perf.sh`, que lit le test des
  outils. *Décidé le 23 octobre* : ajouter `run_perf.sh` seulement ; fait par
  #166 (fusionnée).
- **Les jobs** : CMake et Ubuntu 24.04 seulement, ni arm64 Linux, armhf, i386,
  macOS x86_64, clang-cl ni Visual C++ arm64 ; le job armhf/arm64 de #150
  compare toujours deux options d'un arbre, non la base et la pull request. Un
  cache (`actions/cache`, clé : le SHA de la base) du binaire de la base
  éviterait de la reconstruire à chaque poussée, sous Windows surtout ; non
  essayé. *Décidé le 23 octobre* : faire les jobs absents et le cache ; fait par
  #166 (fusionnée) : dix jobs de plus (FPU x86-64 avec GCC et Clang, arm64,
  macOS Intel, Visual Studio arm64, clang-cl) et deux conteneurs (i386, armhf)
  sans cache, le cache des programmes de la base étant sur la clé de la base, du
  job, de l'image du runner et du processeur.
- **`gaol_bench` avec Visual C++** n'avait jamais été compilé avant #160 ; les
  dix lignes ne se comparent qu'avec une base qui a la cible, et un
  avertissement du job dit quand elles sont sautées.
- **L'élargissement de `make perf` à macOS et de la cible `gaol_bench` à tous
  les systèmes** (CMake, meson) vient de l'outil : à garder ou à restreindre.
  *Décidé le 23 octobre* : garder.
- **`LINE_LIST` vide** ne lance aucune ligne dans `run_perf.sh` et les dix dans
  `run_lines.sh` ; harmoniser ferait changer `run_lines.sh` (le README dit la
  différence). *Fait le 23 octobre* par #166 (fusionnée) : une `LINE_LIST` vide
  ne lance aucune ligne dans les deux.
- **`process.md` §10** liste les workflows, sans `performance.yml`. *Fait le 23
  octobre* par #165 (fusionnée).

### #161

- **Une option à trois valeurs `AUTO`, `ON`, `OFF`** pour `GAOL_PREFER_AVX512`
  (comme `auto` de meson) lèverait les limites 2 et 3 du commentaire de
  `CMakeLists.txt` (le texte d'aide de la ligne de commande sert à reconnaître
  une demande ; la variable ordinaire d'un projet englobant), mais change le
  type de l'option et ce que lit un projet englobant. *Décidé le 23 octobre* :
  garder `ON`/`OFF` (remplacé le 26 octobre : plus une option, #200).
- **`configure` a été édité à la main** (par #161 et par #163), sans autoconf
  2.72 : vérifié le 23 octobre à `601fc63`, `autoconf` 2.72 (celui de cette
  machine), lancé sur une copie des fichiers suivis, régénère un `configure`
  identique au commité.
- **Les commentaires de la CI et de `doc/compare/code/`** qui disent encore « +,
  -, *, / et sqrt » (`.github/scripts/sde-tests.sh`, `linux.yml`, `windows.yml`,
  `build_benchmarks.sh`, `run_bench.sh`) : à mettre à jour ou non. *Décidé le 23
  octobre* : les mettre à jour ; #164 étant déjà fusionnée, c'est dans #165
  (fusionnée).
- **`doc/compare/code/setup.sh`** garde `-UGAOL_PREFER_AVX512`, utile à un
  dossier configuré avant #161 ; il peut disparaître quand plus aucun ne
  subsiste. *Décidé le 23 octobre* : le retirer et nettoyer les dossiers ; les
  sept dossiers de build sont supprimés, et #165 (fusionnée) fait supprimer par
  `setup.sh` lui-même un `gaol-preserve-build` d'avant #161 (sans cela,
  `FORCE_GAOL=1` sur un ancien dossier aurait refusé le chemin AVX-512 sans le
  dire).
- **Le test des bits du signe de zéro de `x * d`** (112) : *décidé le 23
  octobre*, saut à l'exécution sans AVX-512 ; fait par #171 (fusionnée).

### #162

- **Le test `e == 2` dans les en-têtes** (`pow` de `gaol/gaol_interval.h` et
  `pown` de `gaol/gaol_ieee1788.h`) : `pow(x, 2)` y coûterait 1,35 ns avec NEON
  au lieu de 1,66, le coût de `sqr`, au prix d'un second endroit à tenir (la
  décision reste dans `gaol_pown`). *Décidé le 23 octobre* : non, la décision
  reste dans `gaol_pown`.
- **Les puissances 3 à 5 avec les intervalles FPU coûtent environ 0,3 ns de
  plus** (+5 %), `gaol_pown` étant maintenant un saut vers `pown_checked`
  (`pow(x, 3)` de 6,41 à 6,93 ns sur un M2, de 6,43 à 6,79 chez le relecteur) ;
  à remesurer sur x86, où un appel de plus coûtait 0,4 à 0,6 ns sur un
  i7-1185G7. *Remesuré le 23 octobre* (#169, ouverte) : +0,8 ns sur x86 (FPU et
  SSE2, GCC et Clang), la cause étant l'appel, le cadre et la garde de
  `-fstack-protector-strong` de `gaol_pown` ; un essai d'appel terminal ne
  marche pas (GCC ne fait pas d'appel terminal d'une fonction qui rend un
  `interval` par pointeur) ; laissé tel quel et dit. Le remesurage a trouvé un
  `sqr` FPU 1 ns trop lent depuis #162 (GCC n'inlinait pas `sqr_upward()`),
  corrigé par #169.
- **L'ensemble vide et le sens d'arrondi** : `pow(vide, 2)`, `2u`, `2LL` et
  `gaol_ieee1788::pown(vide, 2)` laissent le sens du programme, comme
  `sqr(vide)`, mais `gaol_pown_ll(vide, 2)`, `pow(vide, 2.0)`, `pow(vide, [2])`
  et `gaol_ieee1788::pow(vide, [2])` le mettent vers le haut avant de rendre le
  vide. Sans conséquence sur les bornes ; `doc/using.md` ne promet rien. *Décidé
  le 23 octobre* : harmoniser vers « le sens du programme » ; fait par #169
  (fusionnée), un test de quinze formes qui échoue sans la correction.
- **Le texte de #154** (« +0 avec `pow` et −0 avec `sqr` ») était inexact pour
  NEON et FPU (`doc/simd.md` est corrigé) ; les intervalles SSE2, où le signe de
  l'ancien `pow` n'est pas mesuré, peuvent avoir un autre profil.

### #163

- **`GAOL_PRESERVE_ROUNDING` avec autotools et meson sous MSYS2** : les entrées
  de `build-systems.yml` ne l'ont pas, seules celles de `windows.yml` (CMake) le
  font. Le contrôle des indicateurs depuis les six directions n'est lancé que
  pour mingw-w64 : l'étendre à Visual C++ x64, Linux, macOS et ARM est une
  décision de plus, avec le risque de trouver d'autres défauts. *Décidé le 23
  octobre* : l'étendre à toutes les plateformes (six directions avec SSE, quatre
  ailleurs) ; fait par #167 (fusionnée), dont la CI est le test ; pas d'entrée
  `GAOL_PRESERVE_ROUNDING` pour autotools et meson sous MSYS2.
- **`-mavx512f` ou `-march` donnés par l'utilisateur** : avec le Clang croisé,
  `-mavx512f -mfma -mprefer-vector-width=128` laisse 2 lignes `%ymm` et 2 887
  `%zmm`, et `-march=skylake-avx512 -mprefer-vector-width=128` 406 `%ymm` ; le
  relecteur a compté 0 et 0, puis 4 `%ymm`, et l'écart n'est pas élucidé. Rien
  n'est vérifié avec GCC (le relecteur a lu le réglage de GCC 14.2 sans
  l'exécuter ; GCC 15.2 sur une petite boucle : aucun `%ymm` ni `%zmm` avec
  `-mavx512f -mfma`, `-march=skylake-avx512` ou `-march=native`). Faut-il le
  dire dans `doc/using.md` ? *Décidé le 23 octobre* : le documenter, avec « peut
  » ; fait par #167 (fusionnée).
- **Cygwin** reçoit `-mfma` seul, sans `-mprefer-vector-width=128`, comme avant
  : les trois détections ne le prennent pas pour « GCC pour Windows » (`WIN32`
  de CMake, `_WIN32` de configure et de meson), alors que celles d'AVX-512
  testent aussi `__CYGWIN__`. Lui donner les mêmes drapeaux ? Non vérifié (ni
  Cygwin ici, ni dans la CI). *Décidé le 23 octobre* : oui ; fait par #167
  (fusionnée), sans job de CI (Cygwin et l'environnement MSYS de MSYS2 reçoivent
  les mêmes drapeaux).
- **x86 32 bits** (MinGW-w64 12.2 à 15.2) reste sans FMA et n'est pas mesuré ;
  **`-mstackrealign`** n'a pas été essayé. *Décidé le 23 octobre* : l'essayer
  par la CI ; fait par #172 et #173.
- **Le `-faligned-new=8` pour un fichier C** (#152) : GCC avertit (« valid for
  C++/ObjC++ but not for C »), et avec `-Werror` c'est une erreur ; `gaol.pc` et
  `gaol_dep` de meson le donnent à tous les langages. *Décidé le 23 octobre* :
  le dire dans `doc/using.md` ; fait par #167 (fusionnée).

## 7. Le travail direct sur `configure-clean`

### 7.1 Le 15 octobre (commits directs)

- **Déplacement du parser et des expressions dans `gaol/parser/`** (35866d6) :
  13 fichiers, les trois builds, la CI et la documentation ; les 5 en-têtes
  publics (`gaol_expression.h`, `gaol_expr_eval.h`, `gaol_expr_visitor.h`,
  `gaol_eval_stack.h`, `gaol_parser.h`) sont installés dans
  `include/gaol/parser/` — décision du mainteneur : on garde ce chemin, tant pis
  pour la rétrocompatibilité. Validé : matrice complète de `process.md` 6.2, les
  trois builds, `headers.sh`, `tests.sh`, le manuel.
- **Ménage** : `gaol_exact.h`, `gaol_exact.c` et `sysdeps/` (756e29c),
  `gaol_flags.h` (`prec_t` déplacé dans `gaol/parser/gaol_expression.h`) et
  `s_nextafter.c` (10a3cad), `gaol_assert.h` (5d106b3 : `GAOL_ASSERT` remplacé
  par `assert`, `GAOL_DEBUGGING` ne gouverne plus que `GAOL_DEBUG_VERBOSE`). Le
  point 93 de AE est fait.
- **Textes corrigés** (1f9d294) : ceux qui disaient que les builds Debug
  vérifient les assertions de GAOL avec `GAOL_DEBUGGING` disent maintenant que
  l'absence de `NDEBUG` garde `assert()` actif ; `doc/using.md` liste
  `gaol_assert.h` parmi les en-têtes de GAOL 4 retirés.
- **`gaol/float/`** (3e869cb) : `gaol_intervalf.*`, `gaol_interval2f.*` et
  l'outil de l'auteur `gaol_generator_mul_i2f.py` déplacés ; cast explicite dans
  `gaol_profile.cpp:52` (avertissement `-Wconversion` sur armhf). Validé avec
  `-DGAOL_FLOAT_INTERVALS=1` : `intervalf` et `interval2f` ne sont plus sautés
  et passent.
- **TODO.md** à jour (7c353ae) : relecture du 15 octobre, trouvailles CC.m4 à
  CC.m6, questions CC.q5 à CC.q7 répondues.
- **Pull requests** : #123 (`-Wno-psabi`) fusionnée le 16 octobre, #122 (point
  AA) le 17 ; #124 (point ZZ) et #139 (`atanh([1])` et `atanh([-1])` dans les
  cas particuliers) le 18 octobre.
- **CI** : verte sur les derniers commits de `configure-clean`.

### 7.2 Du 18 au 23 octobre

- **Pull requests** : #139, #140 et #141 fusionnées. En commits directs sur
  `configure-clean` : `f_negate`, `f_negate_simple`, `gaol_opposite` et
  `reset_fpu_cw` supprimés (a6786af), puis `get_fpu_cw` (4fb9f6a), et
  `GAOL_HAVE_ROUNDING_MATH_OPTION` avec #141, et le commentaire gâché de
  `gaol_roundeven.h` corrigé (79746c2, le 23 octobre).
- **Relecture de cette issue, de `TODO.md`, de `todo-notes/` et des notes et
  commentaires des pull requests fermées (#15 à #140)** : `TODO.md` nettoyé (1
  669 lignes à 976 : il ne garde que ce qui reste à faire) et trié par priorité,
  quatorze tâches nouvelles (114 à 127) ; le reste à faire est dans #142.
- **Le 19 octobre** : #70 fermée (pas de `std::less`), la branche `todo-zz-perf`
  supprimée, #143, #144, #145, #146 et #147 fusionnées (points AO.127, AB.77,
  AE.90, AI.106, AI.107 et AD.114), cette issue mise à jour.
- **Le 20 octobre** : #148 fusionnée (point AI.106 pour Visual C++ et clang-cl
  x64), la branche `todo-ai-msvc-sse2-avx512` supprimée, `TODO.md` renuméroté
  (43 rangs, un rang nouveau en C pour le `/W4` en Debug), cette issue mise à
  jour. #149 fusionnée (points AD.83 et AD.84), la branche
  `todo-ad-83-84-reader` supprimée, `TODO.md` renuméroté (41 rangs), cette issue
  mise à jour. #150 fusionnée (point AI.106 pour ARM 32 bits, en partie), les
  branches `todo-arm32-fpscr` et `todo-ARM-perf` supprimées en local, `TODO.md`
  mis à jour (les rangs ne changent pas), cette issue mise à jour ; #153, qui
  avait la même correction, fermée sans fusion, sa branche supprimée. #151
  fusionnée (points AB.78 et AB.80), la branche `todo-ab-cancel-invalid`
  supprimée, `TODO.md` renuméroté (40 rangs), cette issue mise à jour. #155
  fusionnée (point AL.118), la branche `todo-al-process-md` supprimée, `TODO.md`
  renuméroté (38 rangs), cette issue mise à jour.
- **Le 22 octobre** : #158 fusionnée le 20 (point AC.81), la branche
  `todo-ac-nth-root-subnormal` supprimée, `TODO.md` renuméroté (37 rangs), cette
  issue et #142 mises à jour.
- **Le 22 octobre, suite** : #156 fusionnée le 20 (point AI.108, la lecture en
  ligne de x87 et de MXCSR ; ses suites, décidées le 22 octobre, restent), les
  lignes de crédit du 123 retirées des descriptions et des commentaires de pull
  requests (le 20 octobre), `TODO.md` renuméroté (36 rangs), cette issue et #142
  mises à jour.
- **Le 23 octobre, soir** : réponses aux questions des dix dernières pull
  requests (vingt-et-une) reportées dans `TODO.md` (rangs 32, tâches 128 à 133),
  les branches `exp-110-gcc-windows-fma`, `exp-110b-stack-alignment` et
  `exp-157-ab` supprimées sur GitHub, sept dossiers de build de
  `doc/compare/code/` supprimés, et huit pull requests ouvertes pour les
  décisions : #165 (textes de la CI, `setup.sh`, `process.md` §10, `NI` et
  `FS`), #166 (banc A/B : dix jobs de plus, cache de la base), #167 (GCC pour
  Windows : Cygwin, six directions partout, documentation), #168 (fermée sans
  fusion : noyau mixte du produit FPU), #169 (le vide dans `pow`, le carré FPU),
  #170 (la protection des stores de `*=`, `/=`, `%=`), #171 (bits du signe de `x *
  d`), #172 (GCC pour Windows en 32 bits avec `-mfma`), #173 (brouillon : un job
  Cygwin). #164 avait été fusionnée avant la reprise de ses textes de CI, d'où
  #165.
- **Le 23 octobre** : #152 et #154 (fusionnées le 20 octobre, omises jusqu'ici
  de cette issue) ajoutées ; #157 (point AI.106), #160 (points AI.111 et AI.115)
  et #161 (point AI.112) fusionnées dans la nuit du 22 au 23 octobre, #162
  (suite du point AN) et #163 (point AI.110) le 23 ; les branches
  `todo-ai-fpu-kernels`, `todo-ai-111-115-ab-bench`,
  `todo-ai-112-cmake-options`, `todo-an-pow2-sqr` et
  `todo-ai-110-gcc-windows-fma` supprimées sur GitHub à la fusion, et celle de
  #156 n'y est plus ; `TODO.md` renuméroté (33 rangs), cette issue et #142 mises
  à jour. #164 (point C : textes, tests et ménage) fusionnée, la branche
  `todo-QB-text-doc` supprimée sur GitHub à la fusion et en local le 23 octobre,
  `TODO.md` renuméroté (28 rangs), cette issue et #142 mises à jour. Le
  commentaire gâché de `gaol/gaol_roundeven.h` (ligne 73, « doublegaol_cr_ »)
  corrigé par le commit direct 79746c2 sur `configure-clean`, cette issue mise à
  jour.

### 7.3 Le 24 octobre

- #173, #174 et #176 fusionnées (Cygwin x86-64 pris en charge, ses restes,
  l'étape de CPack retirée du job) ; la branche `todo-ai-msvc-avx512` supprimée
  sur GitHub ; `TODO.md` et `TODO_mistral.md` mis à jour pour #173 et #174
  (c35298d, 88b003c), puis #176 notée en cours (f204b7b).
- Ménage des registres : #177, #178 et #179 créées, #49, #142, #107 et #159
  fermées, `TODO_mistral.md` supprimé, `process.md` mis à jour pour les trois
  registres.
- #180, #181 et #182 (la tâche PA) fusionnées par le mainteneur ; `TODO.md`,
  `TODO_DONE.md` et `process.md` (§12) mis à jour pour elles, les branches
  fusionnées supprimées en local.
- #183 et #184 (la tâche PB) fusionnées par le mainteneur ; `TODO.md`,
  `TODO_DONE.md` et `process.md` (§12) mis à jour pour elles, l'issue #185 créée
  pour le masquage des exceptions de `pow`, les branches fusionnées supprimées
  en local.

### 7.4 Le 25 octobre

- #186, #187, #188, #189 et #190 (la tâche PC) fusionnées par le mainteneur le
  25 octobre, #186, #187 et #190 le matin, #188 et #189 ensuite après la fusion
  de `configure-clean` dans leurs branches ; `TODO.md`, `TODO_DONE.md`,
  `TODO_AFTER.md` et `process.md` (§12) mis à jour pour elles, les branches
  fusionnées supprimées en local.
- #191, #192, #193 et #194 (la tâche PG en partie : la division du chemin
  AVX-512, NEON et SSE2 avec tous les compilateurs) fusionnées par le mainteneur
  le 25 octobre, #191 le matin, #192 en fin d'après-midi, puis #193 et #194
  après la fusion de `configure-clean` dans leurs branches (la CI de #194
  n'avait pas commencé sur ce dernier commit quand il l'a fusionnée) ; #195
  (l'expérience du quotient NEON) fermée sans fusion ; `TODO.md`,
  `TODO_DONE.md`, `TODO_AFTER.md` et `process.md` (§12) mis à jour pour elles,
  les branches fusionnées supprimées en local.
- #196 et #197 (la tâche PD) fusionnées par le mainteneur le 25 octobre, dans la
  soirée, avant la fin de leur CI (aucun échec parmi les jobs finis, après la
  relance du job « Ubuntu 26.04 arm64 Clang » de #196) ; l'issue #198 créée
  pour le défaut de `x / y` sous `FPCR.AH`, #185 fermée ; `TODO.md`,
  `TODO_DONE.md`, `TODO_AFTER.md` et `process.md` (§12) mis à jour pour elles,
  au-dessus du commit local du mainteneur « update README.md. », poussé avec ;
  les branches fusionnées supprimées en local.

### 7.5 Le 26 octobre

- #199 (le déplacement des sources des intervalles dans `gaol/core/`) fusionnée
  par le mainteneur le 26 octobre au matin, la CI de son dernier commit verte
  (201 jobs) : la CI du commit de fusion, redondante (même arbre que le dernier
  commit de la branche), n'est pas attendue (§10) ; `TODO.md`, `TODO_DONE.md`,
  `TODO_AFTER.md` et `process.md` (§12) mis à jour pour elle, l'issue #198 reste
  ouverte, la branche supprimée sur GitHub à la fusion et en local le 26
  octobre.
- #200 (les options des trois builds, hors des tâches) fusionnée par le
  mainteneur le 26 octobre à 18 h 45 UTC, ses 206 jobs verts sur le dernier
  commit ; la CI du commit de fusion, redondante, n'est pas attendue (§10) ; la
  branche supprimée sur GitHub à la fusion et en local le 26 octobre ;
  `TODO.md`, `TODO_DONE.md` et `process.md` (§12) mis à jour pour elle. La pull
  request #201 (le point 92, PE.2) ouverte juste après, sur `todo-ae-c-flags` où
  `configure-clean` est fusionnée. #201 fusionnée par le mainteneur le 26
  octobre à 21 h 03 UTC, ses 205 jobs verts sur le dernier commit ; la CI du
  commit de fusion, redondante, n'est pas attendue (§10) ; la branche supprimée
  sur GitHub à la fusion et en local le 27 octobre ; `TODO.md`, `TODO_DONE.md`
  et `process.md` (§12) mis à jour pour elle. #202 (le point 4 de PE) fusionnée
  par le mainteneur le 27 octobre à 8 h 39 UTC, ses 185 jobs verts sur le
  dernier commit ; la CI du commit de fusion, redondante, n'est pas attendue
  (§10) ; la branche supprimée sur GitHub à la fusion et en local le 27
  octobre ; `TODO.md`, `TODO_DONE.md` et `process.md` (§12) mis à jour pour
  elle. #203 (le point 6 de PE) fusionnée par le mainteneur le 27 octobre à 8 h
  41 UTC, ses 205 jobs verts sur le dernier commit ; la CI du commit de fusion,
  redondante, n'est pas attendue (§10) ; la branche supprimée sur GitHub à la
  fusion et en local le 27 octobre ; mêmes registres mis à jour pour elle, avec
  #202, dans le même commit. #204 (le reste du 95 et le 5 de PE) fusionnée par
  le mainteneur le 27 octobre à 8 h 44 UTC, ses 205 jobs verts sur le dernier
  commit ; la CI du commit de fusion, redondante, n'est pas attendue (§10) ; la
  branche supprimée sur GitHub à la fusion et en local le 27 octobre ; mêmes
  registres mis à jour pour elle. #205 (le point 3 de PE) fusionnée par le
  mainteneur le 27 octobre à 11 h 18 UTC, ses 222 jobs verts sur le dernier
  commit ; la CI du commit de fusion, redondante, n'est pas attendue (§10) ; la
  branche supprimée sur GitHub à la fusion et en local le 27 octobre ; mêmes
  registres mis à jour pour elle ; la tâche PE est faite

## 8. Le ménage fait

### 8.1 Branches, lignes de crédit, outillage

- [x] Branches fusionnées, jetables et abandonnée supprimées le 3 octobre, puis
  celles de C, K, L, J, de `make distclean`, de #77, #78, #79, #81, #83, #84,
  #85, #86, #87, #88, #89, #90, #91, #92, #93, #94, #95, #96, #97, #99, #100,
  #103, #105, #108, #109, #110, #111, #112, #113, #114, #115, #117, #118, #119,
  #120 et #121 après leur fusion, et la jetable de #116 (celle de #84, restée
  sur GitHub, le 6 octobre). Celles de #122 et #123, sur GitHub et en local,
  après leur fusion. Les worktrees d'une ancienne session (E et H) retirés le 6
  octobre. Celle de #139, sur GitHub et en local, le 18 octobre, après sa
  fusion. Celles de #140 (`todo-i-documentation`) et de #141 (`no-using-asm`) ne
  sont plus sur GitHub. Celle de #124 (`todo-zz-perf`), sur GitHub et en local,
  le 19 octobre. Celles de #144 (`todo-ab-pow-invalid`), de #145
  (`todo-ae-c-compiler-checks`), de #146 (`todo-ARM-perf`) et de #147
  (`todo-ad-parse-interval-null`), sur GitHub à la fusion, et celle de #144 en
  local, le 19 octobre. Celle de #148 (`todo-ai-msvc-sse2-avx512`), sur GitHub à
  la fusion et en local, le 20 octobre. Celle de #149 (`todo-ad-83-84-reader`),
  sur GitHub à la fusion et en local, le 20 octobre. Celle de #150
  (`todo-arm32-fpscr`), sur GitHub à la fusion et en local, le 20 octobre, avec
  celle de #146 (`todo-ARM-perf`) en local. Celle de #151
  (`todo-ab-cancel-invalid`), sur GitHub à la fusion et en local, le 20 octobre.
  Celle de #155 (`todo-al-process-md`), sur GitHub à la fusion, le 20 octobre,
  et en local, le 22. Celle de #153 (`todo-fpcr-gcc-hoisting`), fermée sans
  fusion, sur GitHub et en local, le 20 octobre. `todo-zz-notes`, contenue dans
  `configure-clean`, n'existe plus en local (vérifié le 20 octobre). Celle de
  #158 (`todo-ac-nth-root-subnormal`), sur GitHub à la fusion et en local, le 22
  octobre. Celles de #156 (`todo-ai-msvc-x86-csr`), de #157
  (`todo-ai-fpu-kernels`), de #160 (`todo-ai-111-115-ab-bench`), de #161
  (`todo-ai-112-cmake-options`), de #162 (`todo-an-pow2-sqr`), de #163
  (`todo-ai-110-gcc-windows-fma`) et de #164 (`todo-QB-text-doc`), sur GitHub (à
  la fusion, et constaté le 23 octobre pour celle de #156) ; celle de #157 reste
  extraite dans l'arbre `GAOL_V9/GAOL`, et celle de #164 (`todo-QB-text-doc`)
  supprimée en local le 23 octobre. Les branches d'expérience
  `exp-110-gcc-windows-fma`, `exp-110b-stack-alignment`, `exp-157-ab`,
  `exp-win32-fma-stack`, `exp-cygwin-diag` et `exp-cygwin-diag2` sont supprimées
  sur GitHub (les 23 et 24 octobre), ainsi que `todo-ai-msvc-avx512` (sans pull
  request, un ancêtre de `configure-clean`, le 24 octobre). Celles de #176
  (`todo-ai-cygwin-cpack-step`), de #180 (`todo-ap-text-nullptr`), de #181
  (`todo-ap-exception-file`), de #182 (`todo-ad-lexer-numbers`), de #183
  (`todo-ac-powrev-infinite`) et de #184 (`todo-ab-overflow-flag`), sur GitHub à
  la fusion et en local le 24 octobre, avec la branche jetable
  `debug-msvc-flags` du diagnostic de la CI de #184 ; celles de #186
  (`todo-ap-kernels-detail`), #187 (`todo-ap-neon-sqrt-mask`), #188
  (`todo-ap-neon-pown-packed`), #189 (`todo-ac-register-guard`) et #190
  (`todo-q-bisect-flush`), sur GitHub à la fusion et en local le 25 octobre ;
  celles de #191 (`todo-pg-avx512-div`), #192 (`todo-pg-msvc-arm64-neon`), #193
  (`todo-pg-sse2-win32-msvc`) et #194 (`todo-pg-universal-macos`), et la jetable
  de l'expérience #195 (`experiment-neon-scalar-div`), sur GitHub à la fusion et
  en local le 25 octobre ; celles de #196 (`todo-pd-texts`) et de #197
  (`todo-pd-pow-mutants`), sur GitHub à la fusion et en local le 25 octobre ;
  celle de #199 (`interval-sources-in-core`), sur GitHub à la fusion et en local
  le 26 octobre ; celles de #201 (`todo-ae-c-flags`), de #202
  (`todo-pe-tool-remarks`), de #203 (`todo-pe-review-fixes`), de #204
  (`todo-pe-probes`) et de #205 (`todo-pe-mingw32`), sur GitHub à la fusion et
  en local le 27 octobre ; celle de #200 (`automatic-configuration`), sur GitHub
  à la fusion et en local le 26 octobre : sur GitHub, il ne reste que
  `configure-clean`, `MATH-CORE`, `master` et `perf-doc-rerun`.
- [x] Lignes de crédit retirées des descriptions de #50, #51, #53 à #57 et #59,
  d'un commentaire de #59 et de cette issue. Il en reste dans les messages des
  commits a0d3871, 790ca12 et f1f4239 de #99, fusionnée sans squash. Retirées
  aussi, le 20 octobre (point 123), des descriptions de #15 à #24 et #60 à #63
  et des commentaires de #58 et #60 à #63 ; les commits restent (décidé le 19
  octobre).
- [x] `TODO_mistral.md` trié et supprimé (anciens points 68 à 71).
- [x] Points du `TODO.md` regroupés et nommés par des lettres (4abc35b).
- [x] La procédure suivie pour chaque point écrite dans `process.md` (946ec6e) ;
  les rapports et l'outillage de la branche `todo-status` ramenés dans
  `todo-notes/` de `configure-clean` (16a2f60), et la branche `todo-status`
  supprimée ; puis les rapports relus, leurs dernières questions posées
  (décisions en commentaire), et les rapports retirés de l'arbre, ce qui en sert
  encore étant dans `todo-notes/synthese.md` (dcd3b11, 065191e), le 4 octobre.

### 8.2 Les branches inachevées

- [x] E, `todo-12-long-sums` : fini, et fusionné par #105
- [x] H, `todo-36-upward-rounding-effects` : l'arrondi vers le haut dans la
  documentation, fait autrement par #86 et #88 (la branche locale reste)

### 8.3 Ce que disait `TODO_mistral.md`

`TODO_mistral.md` (« Descriptif complet des tâches QA à QJ », écrit le 22
octobre par un autre modèle, tenu à jour jusqu'au 24 octobre) décrivait, tâche
par tâche (QA à QK), ce que `TODO.md` et #142 listaient : problème, correction,
fichiers, état, renvoi aux rangs. Il est supprimé le 24 octobre ; ses faits sont
dans les sections 2 à 6 ci-dessus, dans `TODO.md` et dans `TODO_AFTER.md`. Un
premier fichier du même nom avait déjà été trié et supprimé le 3 octobre (commit
1a21755) : voir 3.2.

Le détail de ses tâches faites : QA.4 (81) par #158 ; QB.1 à QB.6, QB.8, QB.9,
QB.11 à QB.13 par #164 ; QB.7 et QC.1 par #161 ; QB.10 le 20 octobre ; QC.2 clos
(Q3) ; QD.1 (106) par #157 ; QD.2 (110) par #163 ; QD.6 (111, 115) par #160 ;
QD.8 (108) par #156 ; QK.6 (133) par #172, #173 et #174 ; QI (branches et pull
requests ouvertes) : toutes fusionnées ou supprimées ; QJ (les questions des
pull requests) : section 6 et 3.5, 3.6.
