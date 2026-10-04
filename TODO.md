# À faire

Ce qui reste à faire sur GAOL v5 au commit `338f2b3` de `configure-clean`.
Depuis le 3 octobre, les points sont regroupés et nommés par des lettres : un
point réunit ce qui touche le même code ou le même fichier, ou ce qu'un ordre
impose de faire ensemble. Chacun garde, en sous-points, les numéros de
l'ancienne liste, auxquels renvoient les pull requests, les issues (#49, #64 à
#70) et les rapports de `todo-notes/` (branche `todo-status`) ; la table à la
fin donne la lettre de chaque ancien numéro, et un numéro qui n'y est pas est un
point fait.

Les anciens points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro n
de sa section 5) ; 41 à 44, de la vérification de `VERSION.txt` (2026-09-28) ;
45 à 49, des corrections et des relectures ; 50, d'une décision du 3 octobre ;
51 à 67, des relectures des pull requests ; 68 à 71, du tri de
`TODO_mistral.md`. Les décisions du 3 octobre sont écrites dans chaque point
(« Décidé le 3 octobre ») ; une question reportée renvoie à son issue. Fait
depuis : le point C (anciens 16, 17 et 67), par #71 ; le point K (anciens 49 et
47, hors Cygwin, resté au point A), par #72 et #73 ; le point L (anciens 29, 43
et 69, et le `push` restreint du ménage), par #74 ; `make distclean`, qui rend
les sources telles que git les a, par #75 ; le point J (anciens 19, 20, 68 et
16), par #76. Décidé à la fin du point J, le 4 octobre, et fait dans
`configure-clean` : `nb_fp_numbers()` rend un `unsigned long long`, sans
compatibilité pour `ULONGLONGINT` ni pour les anciens noms des macros ;
`headers.sh` dans les seuls jobs d'avertissements ; des jobs AppleClang
(`-Wall -Wextra -Werror`) et Visual C++ x64 et x86 (`/W4 /WX`, `size_t` ayant
32 bits en x86) ; `gaol_parameters.h` supprimé ; `gaol_interval2f.h` et
`manual/v5/relation-cos.tex` en LF. Décidé aussi le 4 octobre : les constantes
`double` de `gaol/gaol_port.h` (`pi`, `half_pi`, `two_pi`, `ln2_dn`...), que le
manuel documentait depuis GAOL 4, retirées de l'interface par #77 (avec `using
namespace gaol`, un `pi` du programme était ambigu ; il ne reste que `pi_dn`,
`pi_up`, `half_pi_dn` et `half_pi_up`, dans l'espace de noms `detail`, et le
manuel renvoie à `interval::pi()`...) ; plus de `-lm` explicite dans les builds
autotools et meson ni dans `gaol.pc`, par #78 : GAOL appelle toujours la
bibliothèque mathématique du système (`<fenv.h>`, `sqrt()`, `floor()`, `fma()`
sans FMA matériel...), que le compilateur C++ lie lui-même, comme le build
CMake le supposait déjà.

## En cours

Ces branches sont poussées, mais pas fusionnées dans `configure-clean`.

- **E** (ancien 12), `todo-12-long-sums` (553e649) : inachevé (commits
  « WIP ») ; restent la fin du travail, son rapport, la relecture, la CI et la
  pull request.
- **H** (ancien 36), `todo-36-upward-rounding-effects` (1559af5) : inachevé
  (« WIP » fait sur `a2ca992`, conflit avec `configure-clean` dans
  `doc/using.md`) ; voir le point H ; restent aussi la fusion de
  `configure-clean`, la relecture et la pull request.

## Même changement, ou même code

### A. Le correctif 6 de CORE-MATH, MinGW-w64 et Cygwin (31, 6, 56, 3, 47)

Le correctif 6 en entier (la table) est à la fois la forme décidée pour
CORE-MATH (31) et la correction de Cygwin (47) ; le `#warning` à taire (6) est
dans les mêmes fichiers, `cbrt.c`, `rsqrt.c` et `asinpi.c`, et les commentaires
(56) et la racine carrée de SSE2 (3) dans `gaol/core_math_port.h` et
`gaol/gaol_fpu.h`. Corriger d'abord `3rd/README.md` ; l'envoi est reporté (#65,
#66), le signalement à mingw-w64 aussi (#69).

- **31.** **Envoyer à CORE-MATH les correctifs écrits dans
  [3rd/README.md](3rd/README.md)**, dont la section « Changes to propose
  upstream » donne six correctifs contre son master `b1a4badf`, vérifiés avec
  `./check.sh --worst` et `--special` ; la glibc n'a aucun de ces défauts. Rien
  n'est envoyé. Décidé le 3 octobre : le correctif 6 sous sa forme large (la
  table), après examen de `binary80/pow/powl.c` ; changer dès maintenant les
  `sinpi.c` et `log1p.c` embarqués (`/* GAOL */`, comme sinh, cosh et tanh) ;
  envoyer aussi le patch 7 (`exact_pow()`, #63). Reportés : qui envoie et
  comment (merge request sur gitlab.inria.fr, ou `git format-patch` à
  core-math@inria.fr), #65 ; y joindre ou non l'échec du master à `./check.sh
  --worst --rndd pow` et la borne de `ss` dans `asinpi_acc()`, #66. Corriger
  d'abord les phrases inexactes de `3rd/README.md` relevées par les relectures
  de #54, #56 et #59.
- **6.** **Suites de `fegetround()` lu dans MXCSR et du refus de MinGW-w64**
  (#54, #51). Décidé le 3 octobre : ne pas lire MXCSR sur x86 32 bits avec
  SSE2 ; supprimer `GAOL_RND_MINGW_FENV_ONLY` (mingw-w64 11 i686 passe ctest
  sans lui sous wine) ; refuser mingw-w64 ARM64 avant 12 ou sans `_UCRT`, comme
  sur x86-64 (le refus actuel de mingw-w64 ARM avant 11 y est inclus) ; garder
  le refus du x86 32 bits avant 11, par version ; taire le `#warning` de
  `cbrt.c`, `rsqrt.c` et `asinpi.c` (changement `/* GAOL */`) en attendant le
  point 31. Reporté : signaler à mingw-w64 son `fma()` et son `round()` pour
  msvcrt (#69). Le `fma()` logiciel de `ucrtbase.dll` n'a jamais été vérifié.
- **56.** **Les commentaires du sens d'arrondi avec mingw-w64** (suite du
  point 6, #54) : le commentaire de `round_upward_if_needed()`
  (`gaol/gaol_fpu.h`, l. 334) dit que l'unité x87 « computes none of GAOL's
  doubles », ce qui est faux avec mingw-w64 x64, dont `ldexp()` est le `fscale`
  x87 : écrire « none of GAOL's doubles but exact ones » ; « as the
  get_rounding_mode() of cbrt.c, rsqrt.c and asinpi.c does »
  (`gaol/core_math_port.h`, l. 232) ne vaut que pour GCC et Clang, pas pour
  Visual C++ x64.
- **3.** **Suites de `pow(x, y)` exact aux coins** (#63). Restent, décidés le
  3 octobre : la racine carrée de SSE2 sur tout x86 32 bits, et non plus sur
  Windows 32 bits seulement ; pas de recherche de cas durs pour MinGW-w64 x86
  avant #63.
- **47** (une partie). Cygwin x64, hors CI, reste faux (`FE_*` de newlib, pas de
  `_WIN32`) : décidé le 3 octobre, prendre le correctif 6 de `3rd/README.md` en
  entier plutôt que refuser Cygwin.

### B. pow (1, 2, 8, 51)

Tous dans `pow_standard()`, `gaol_pow_hybrid()` et `ipow_exact_dn()`
(`gaol/gaol_interval.cpp`), avec leurs commentaires et les tests de
`tests/ieee1788.cpp` et `tests/arithmetic.cpp`.

- **1.** **Suites du pow de la norme écrit une fois** (#37). `pow_standard()`
  (`gaol/gaol_interval.cpp`) est la seule copie du pow de la table 9.1. Deux
  vérifications sont inutiles : la garde de l'exposant `[±oo]` de
  `gaol_pow_hybrid()`, inatteignable, et le cas `at_upper == 1.0` du bloc
  au-delà des int de `pow_standard()`. Les retirer reprendrait une part des 2 à
  3 % perdus par `gaol::pow` à exposant non entier (mesurés avec GCC 9.4
  seulement) ; le `is_empty()` de `gaol_pow_hybrid()` est à garder. Décidé le
  3 octobre : le bloc au-delà des int et le pown dans les int sur x coupé à [0,
  +oo] restent dans `pow_standard()`, où `gaol_pow_hybrid()`, qui prend `[n]`
  avant, ne les atteint jamais, et non dans `gaol_ieee1788::pow`.
- **2.** **Le sens d'arrondi est encore vérifié plus d'une fois** : trois fois
  par `pow(x, y)` quand `pow_standard()` passe par exp(y log x) (une borne
  infinie, ou une base partant de 0 avec un exposant qui n'est pas au-dessus de
  0), deux fois par `nth_root(x, q)` pour q < 0 et par `modulo_k_pi()`.
  Correction : appeler les corps de ces opérations après une seule vérification,
  comme `tan()` et les puissances négatives ; les corps de `log()`, `exp()` et
  `*` sans la vérification restent à écrire. `nth_root(I, q)` calcule `|q|`
  par `-static_cast<long>(q)` pour inclure `INT_MIN`, mais `long` fait aussi
  32 bits sous MSVC et MinGW : `nth_root(I, INT_MIN)` déclenche alors un
  dépassement signé indéfini ; calculer la magnitude sans négation signée et
  couvrir ce cas dans les tests. Décidé le 3 octobre : les deux,
  dans cet ordre ; pour `pow`, après le point 3, étendre ensuite l'analyse des
  coins aux bornes infinies et à une base partant de 0 : plus d'exp(y log x), et
  des boîtes serrées (aujourd'hui, `pow([4, +oo], 0.5)` vaut [2 − 2^-52, +oo],
  et d'autres ont des centaines de doubles de trop), avec des tests aux limites.
- **8.** **`pow(x, n)` pour n grand** (revue n° 7) : dans `ipow_exact_dn()`, la
  borne inférieure perd le carré du reste (1962 doubles sous la plus serrée pour
  n = 2^32 − 1), et les builds SSE2 et FPU multiplient les produits arrondis
  dans des ordres différents, contre « les mêmes bornes sur toutes les
  machines » de `doc/accuracy.md`. Fait par #61. Une borne nulle, ou une seule
  borne hors de la plage, envoie les deux bornes aux produits arrondis (jusqu'à
  10 doubles de trop). Décidé le 3 octobre : rendre `ipow_exact_dn(0)` exact,
  sans traiter chaque borne à part ; garder l'ancien code SSE2 sous `#if 0` ;
  garder la garantie 5 n log2(n) 2^-104. `pown([-2, 3], 100)` différait entre
  SSE2 et FPU avec GCC 9.4 (#37) : à revérifier.
- **51.** **Les commentaires et les textes de pow** (suite du point 1, #37) : le
  commentaire d'en-tête de `pow_standard()` (`gaol/gaol_interval.cpp`) raconte
  l'histoire (« This was the second half… ») au lieu de dire ce que fait la
  fonction ; le Doxygen de `gaol_pow_hybrid()` (`gaol/gaol_interval.h`, l. 835)
  dit la partie standard exp(J log(I)), alors que les bornes finies viennent des
  coins du `pow` de CORE-MATH ; `doc/tests.md` (l. 187 et 327) et
  `tests/ieee1788.cpp` (l. 133) disent les bornes vérifiées « bit for bit »,
  alors que `==` ne distingue pas −0 de +0, et racontent l'histoire : dire
  plutôt que les littéraux ont été relevés sur l'ancien code et vérifiés avec
  mpmath à 500 bits, ce que `tests/gaol_tests.h` doit citer aussi.

### D. Le constructeur et les exceptions flottantes (24, 21, 62)

La première comparaison silencieuse (24), les constructeurs pour les entiers
(21) et, si #68 la décide, la comparaison par les bits sous DAZ (point Q)
touchent le même constructeur (`gaol_interval_sse.h`, `gaol_interval_fpu.h`,
`gaol_interval.h`) ; la documentation des exceptions flottantes (62) va avec le
24. `gaol_interval.h` contient aussi `using namespace gaol_core;` à corriger.
Plusieurs headers utilisent des macros avec noms réservés comme `__GAOL_PUBLIC__`
(à renommer en `GAOL_PUBLIC`) et des casts C-style à remplacer par `static_cast<>`.
`gaol_allocator.h` utilise `= 0` au lieu de `= nullptr` ; son
`aligned_allocator<T>` ne garantit que 16 octets d'alignement, donc ne convient
pas aux types suralignés (`alignof(T) > 16`). Aligner selon `alignof(T)` ou
refuser ces types explicitement. Son `construct(pointer, const T&)` bloque aussi
la construction avec d'autres arguments via `allocator_traits`, notamment pour
les types déplaçables mais non copiables : fournir un `construct` variadique ou
laisser `allocator_traits` utiliser son placement-new par défaut.

- **24.** **Exceptions flottantes : ce qui reste après #47 et #50.** Après #60,
  `is_empty()` reste compilé en `vcmpe` sur armhf dans le code du programme :
  l'écrire avec `std::isunordered()` sur ARM 32 bits (décidé le 3 octobre).
  Avec `GAOL_PRESERVE_ROUNDING`, les opérations SSE2 masquent de nouveau les
  exceptions du programme et effacent ses indicateurs : à corriger. `0 × oo`
  dans l'`operator*=` SSE2 et le `pow` de CORE-MATH pour un exposant extrême
  lèvent FE_INVALID, sans test : les corriger, avec un test et le coût de `*`
  mesuré (décidé le 3 octobre). Décidé le 3 octobre : la première comparaison de
  `interval(l, r)` devient silencieuse, ce qui rend gratuits `floor`, `ceil` et
  `integer` (`interval(NAN)`, `x += NAN` et `set_contains(NAN)` ne lèvent plus
  FE_INVALID) ; `x &= y` s'écrit avec un seul `std::isunordered()` (à vérifier
  avec Clang 18, et son coût sous Visual C++ à mesurer) ; les intervalles de
  flottants (`gaol_intervalf.h`, `gaol_interval2f.h`) gardent leurs comparaisons
  signalantes, mais `intervalf::is_empty()` reste silencieux (#47). Décidé aussi
  (#60) : `is_empty()` écrit avec `std::isunordered()` sous `_ARCH_PWR9`, comme
  sur ARM 32 bits ; les relations en ligne (`set_le`, `set_strictly_contains`,
  `less`…) corrigées pour ne plus lever FE_INVALID, même dans une boucle
  vectorisée (AArch64, intervalles FPU x86-64) ; les trois vérifications de
  choix de #60, fragiles par nature, gardées.
- **21.** **Les entiers au-delà de 2^53** : `interval(0)`, `interval(0, 0)`, `x
  = 0`, `x < 0` et `max(x, 0)` compilent, les constructeurs `interval(double)`
  et `interval(double, double)` n'étant pas `explicit` (essayé le 28 septembre
  puis retiré) : l'entier devient un double, et un entier au-delà de 2^53 un
  double qui ne le contient pas. Correction : des constructeurs templates sur
  les types entiers, contraints, dans les en-têtes, sans changement d'ABI ; pas
  un simple `interval(int)`, qui rend `interval(5L)` ambigu. De plus, les
  constructeurs existants devraient être marqués `explicit` pour éviter les
  conversions implicites dangereuses.
- **62.** **La documentation des exceptions flottantes** (suite du point 24,
  #47, #50) : la liste des opérations qui, sous `GAOL_PRESERVE_ROUNDING`,
  masquent de nouveau les exceptions du programme (`doc/using.md` l. 527, manuel
  l. 1235) oublie `%`, `div_rel` et `+= d`, `-= d`, `*= d`, `/= d`, `%= d`, et
  ne dit pas qu'elles effacent les indicateurs ; « `fetestexcept(FE_INEXACT)` is
  raised whatever the result » (`doc/using.md` l. 513, manuel l. 1219) est trop
  fort (`-X`, `abs`, `&`, `|`, `floor`, `max(X, Y)` ne le lèvent pas) : « after
  most operations » ; la ligne courte de `doc/accuracy.md` (l. 60) ;
  `\newinvfive` sur les derniers paragraphes de la section 3.3 du manuel ; la
  structure `EmptySet` de `tests/rounding_direction.cpp` (l. 252), qui porte
  aussi `nonempty_sets` : un nom neutre.

### E. Le parser et les longues sommes (12, 40)

La branche `todo-12-long-sums` régénère le parser : une seule régénération
suffit si la partie parser du point Q (#68) est décidée avant.

- **12.** **Les longues sommes débordent la pile** (revue n° 17) :
  `textToInterval()` d'une somme de 100 000 termes plante encore, comme les
  expressions aussi longues construites par l'API (une récursion par nœud dans
  l'évaluation et la destruction). La correction est dans la branche
  `todo-12-long-sums`, inachevée : chaque opération calculée dès sa lecture par
  le parser régénéré, l'évaluation et la destruction des expressions par des
  boucles, l'imbrication bornée par `YYMAXDEPTH` (au-delà, `input_format_error`)
  et `operator<<` d'une expression encore récursif (limite documentée). Faire
  aussi remonter comme `std::bad_alloc` l'échec d'allocation du buffer Flex :
  `gaol__scan_string()` appelle actuellement `yy_fatal_error()`, qui termine
  tout le processus par `exit(2)`. Capturer également les `std::bad_alloc` de
  l'analyse lexicale ou des actions du parser et les faire passer par son abort
  contrôlé : si elles traversent `yyparse()`, `gaol_parse_string()` détruit le
  scanner mais les `%destructor` de Bison ne libèrent pas les nœuds encore sur
  sa pile. Traiter aussi les octets NUL dans les entrées `std::string` et
  `operator>>` : ils sont tronqués par `yy_scan_string()` (`strlen`), donc une
  entrée telle que `1\0texte` est acceptée comme `1` au lieu d'être rejetée ;
  rejeter ces entrées ou transmettre une longueur explicite au scanner. Dans
  le même parcours, `gaol_ieee1788::textToInterval()` attrape tout
  `std::exception`, dont `std::bad_alloc`, et transforme ainsi un échec
  d'allocation en ensemble vide : laisser remonter au moins les erreurs
  d'allocation. À la régénération, faire taire l'avertissement de Clang 18 sur
  `gaol_nerrs` (décidé le 3 octobre).
- **40** (une partie). Le commentaire Doxygen « Parse a string to create an
  interval » de `gaol/gaol_parser.h` est placé avant l'énumération et non avant
  `parse_interval()`, et ne liste que les formats de GAOL 4 (la branche ajoute
  une note au même commentaire).

## Même fichier de test ou de documentation

### F. La lecture des nombres et `operator>>` (11, 15, 18, 46, 58, 59, 60, 61)

Tous touchent `tests/numbers.cpp`, le lexeur ou `operator>>` ; la syntaxe d'une
lecture par mot (46) est reportée (#64).

- **11.** **Suites du lecteur sous denormals-are-zero** (#40). Le lecteur est
  juste sous FTZ et DAZ (comparaisons sur les bits) ; le reste est au point 45.
  Restent : vérifier une fois, avec `ctest -V -R numbers`, si les tests DAZ de
  `numbers` s'exécutent ou se sautent sur macOS x86_64 sous Rosetta et avec
  l'UCRT en Release ; dans le manuel, la phrase sur ces modes garde sa marque
  `\newinvfive` (décidé le 3 octobre). Décidé aussi (#40) : faire tourner les
  tests DAZ de `numbers` sur AArch64 et ARM32 (FPCR, FPSCR), avec l'outil de
  `tests/gaol_tests.h` venu de #62.
- **58.** **Deux restes du lecteur sous DAZ** (suite du point 11, #40) : le
  manuel (`gaol.tex`, l. 3579) écrit `\code{-Ofast}` au lieu de
  `\option{-Ofast}` ; `tests/numbers.cpp` teste `#if GAOL_TESTS_HAVE_MXCSR`
  (l. 176, 238…), macro indéfinie hors x86, qui avertit sous `-Wundef` : écrire
  `#ifdef`.
- **15.** **Suites de la lecture des lignes vides par `operator>>`** (#46), qui
  lit par `std::getline(is >> std::ws, buffer)` : une ligne vide est sautée, et
  `while (in >> x)` finit sans exception. Sous `exceptions(eofbit)`, un
  intervalle lu en fin d'entrée lève avec `eofbit` seul (un double reçoit
  `eofbit | failbit`), et une dernière ligne sans fin de ligne est perdue ;
  `std::ws` saute `\v` et `\f`, que le lecteur ne prend pas pour des blancs.
  Décidé le 3 octobre : on garde les deux comportements (le premier est
  documenté, et le lecteur ne change pas).
- **59.** **Les textes d'`operator>>`** (suite du point 15, #46) :
  `doc/differences.md` (l. 402-403) et `examples/examples.md` (l. 62, 459 et
  634) disent encore qu'une ligne vide fait lever `input_format_error` ; le
  commentaire d'`operator>>` (`gaol/gaol_interval.cpp`, l. 668, « std::ws is no
  extraction: it constructs no sentry ») et `doc/tests.md` (l. 259) donnent pour
  général ce qui est vrai du `std::ws` de libstdc++ 9 et 10 : écrire « le
  `std::ws` de libstdc++ » ; `doc/tests.md` (l. 250, « with or without
  `std::noskipws` ») se lit comme si un nombre sautait les blancs sous
  `noskipws` ; l'exemple du manuel (`gaol.tex`, l. 3501) lit et écrit `x` dans
  la même expression : en faire deux instructions, et couper en deux la phrase
  voisine (ligne vide « avec ou sans `std::noskipws` », intervalle sur plusieurs
  lignes) ; le Doxygen d'`operator>>` (`gaol/gaol_interval.h`) ne dit pas, comme
  le manuel, qu'avec les exceptions désactivées une ligne refusée écrit un
  message sur `cerr` et arrête le programme.
- **60.** **`stream_without_buffer()` en dernier** (#46) : `tests/numbers.cpp`
  l'appelle en dernier (l. 1891) ; en cas de régression, le programme meurt par
  SIGSEGV sans afficher les échecs précédents, stdout étant tamponné :
  `std::fflush(stdout)` avant l'appel.
- **18.** **Suites de la lecture des longs nombres** (#42). Le texte d'un nombre
  est analysé une fois : 20 000 caractères se lisent en 0,02 s sous toutes les
  locales, et l'analyse des chiffres reste quadratique (décision du
  30 septembre). Décidé le 3 octobre : cette décision vaut aussi pour la forme
  incertaine, dont les sommes passent par `gaol_decimal_add_sub()`, qui insère
  en tête d'une `std::string` (100 000 chiffres : 1,1 s, contre 0,46 s) ; on n'y
  touche pas.
- **61.** **Les commentaires de la lecture des longs nombres** (suite du
  point 18, #42) : le commentaire du membre `beyond` de `gaol_number`
  (`gaol/gaol_interval_lexer.lpp`, l. 216, une ligne de 119 colonnes) laisse
  croire qu'il est positionné dès que le nombre est sous le plus petit double
  positif ou au-dessus du plus grand, alors qu'il ne l'est que loin au-delà
  (10^311 et plus, ou sous 10^-330) : le corriger, au-dessus du membre ; le
  commentaire de `gaol_enclose_number()` (l. 368-372) dit qu'une lecture sous
  une locale à virgule prend le temps de la locale C : écrire « à peu près »
  (rapport mesuré de 1,0 à 1,2) ; dans `tests/numbers.cpp`, le message d'échec
  du test de temps est construit par `std::to_string` sous la locale à virgule
  (« 2,112549 s ») : le construire sous la locale C.
- **46.** **`operator>>` ne relit pas `os << x << ' ' << y`** (suite du
  point 15) : il lit un intervalle par ligne, et la fin d'un intervalle dans un
  flux est ambiguë, un intervalle contenant des espaces et la grammaire
  admettant des littéraux imbriqués et des expressions (`[[1, 2], 3]`, `1 -2`).
  Il faut d'abord décider de la syntaxe acceptée : un intervalle par ligne (ce
  qui est documenté), les seuls littéraux lus jusqu'au crochet fermant, ou une
  fonction à part. Reporté le 3 octobre : issue #64.

### G. Les options refusées (5, 53, 54, 55, 40)

`tests/refused_options.cpp`, `gaol/gaol_config.h`, `tests/CMakeLists.txt` et les
textes qui décrivent le refus.

- **5.** **Suites du refus de `-ffinite-math-only`** (#39), fait avec ses tests
  CMake. Restent : avec GCC, `-Ofast` n'est pas refusé dès que `-fno-fast-math`
  est sur la ligne de commande, avant ou après, alors que `doc/three-builds.md`
  et `doc/using.md` le disent refusé ; le `#error` de `__FAST_MATH__` ne donne
  pas de remède, contrairement à celui de `__FINITE_MATH_ONLY__` ;
  `tests/refused_options.cpp` n'a pas de témoin positif (un test sans option,
  environ 1 s), et ses bornes constantes, et non `volatile`, laissent Clang
  calculer l'intersection à la compilation ; ce qu'aucune macro ne révèle
  (`-funsafe-math-optimizations`, `-fno-honor-nans`, les pragmas) n'est que
  documenté, et une vérification à l'exécution dans `gaol/gaol_init_cleanup.h`
  reste à essayer. Décidé le 3 octobre : ajouter le témoin positif, et essayer
  cette vérification (prototype, ce qu'elle détecte et son coût, puis la garder
  ou non).
- **53.** **Le commentaire du refus de `-ffinite-math-only`** (suite du point 5,
  #39, #47) : `gaol/gaol_config.h` (l. 208) dit encore que `is_empty()` lit
  l'ensemble vide comme `!(left() <= right())`, alors que c'est
  `!std::islessequal(left(), right())` depuis #47. La phrase qui dit `([1, 2] &
  [3, 4]).is_empty()` faux (« GCC 9, Clang 18 » dans `gaol_config.h`, « GCC 9.4,
  Clang 18 » dans `doc/three-builds.md`, « with GCC 9 and Clang 18 » dans
  `doc/tests.md`) est à préciser : avec `-ffinite-math-only`, GCC 13 le rend
  faux à -O0, -O2 et -O3, Clang 18 à -O0, et à -O2 et -O3 seulement avec des
  bornes `volatile`.
- **54.** **L'annexe B n° 4 de `examples/examples.md`** (#39, l. 1072) dit qu'un
  test de compilation vérifie le message avec `PASS_REGULAR_EXPRESSION`, « as
  `tests/fp_strict` does » : c'est faux, `tests/fp_strict` fait un
  `try_compile()` à la configuration, pour Visual C++ seulement.
- **55.** **`.github/audit`** (#39) : `compare.py` (l. 25) et `make_probe.py`
  (l. 15) comparent `__FAST_MATH__` mais pas `__FINITE_MATH_ONLY__` ; sans effet
  tant que `-fno-fast-math` lui-même est comparé.
- **40** (une partie). Les tests `refused_*` de `tests/CMakeLists.txt` sont faux
  quand GAOL est un sous-projet (ils prennent `CMAKE_BINARY_DIR` et
  `CMAKE_SOURCE_DIR`).

### H. Le sens d'arrondi (23, 36, 48, 27)

`gaol_core::rnd_keep()`, que le 23 doit présenter comme une barrière, corrige le
48 ; la branche du 36 documente déjà `rnd_keep()` et écrit une garde ; le 27 est
l'étude qui rendrait ces précautions inutiles. La partie du 36 pour le manuel
s'écrit avec le point I.

- **36.** **Ce que l'arrondi vers le haut fait au programme**, dans
  `doc/using.md` et dans « Common errors » du manuel, avec la table de la
  section 2.8 de `examples/examples.md` (`printf`, `strtod`, `lrint`, les
  allers-retours par le texte, TwoSum, un double calculé avant `cleanup()` et
  réutilisé après), et `GAOL_PRESERVE_ROUNDING` avec son coût. Commencé dans la
  branche `todo-36-upward-rounding-effects` (une section de `doc/using.md` et
  des ajouts à `examples/13_rounding_environment.cpp`). Restent : le coût de
  `GAOL_PRESERVE_ROUNDING` (la section finit sur `XXCOSTXX`), le manuel, et les
  mesures de TwoSum et TwoProd en arrondi dirigé (exacts avec `fma`).
- **23.** **Gérer le sens d'arrondi** : `gaol::cleanup()` ne le restaure qu'à
  son premier appel, il n'y a pas de garde à portée, et `rnd_keep()` n'est pas
  présenté comme une barrière. Correction : un `gaol::restore_rounding()` qu'on
  peut appeler autant de fois qu'on veut, une garde qui calcule un bloc au plus
  proche (environ 7 ns) et `rnd_keep()` documenté ; à faire avec le point 36,
  dont la branche documente déjà `rnd_keep()` et écrit une telle garde dans
  `examples/13_rounding_environment.cpp`. Les sauvegardes manuelles sautent
  aussi si une allocation lève `std::bad_alloc` : après `round_nearest()`,
  `gaol_enclose_number()` compare le nombre avec des `std::vector` sans garde ;
  la construction du texte dans `operator<<` ou `intervalToText()` peut
  également échouer avant `GAOL_RND_RESTORE()` ou `GAOL_RND_LEAVE()`. Le lecteur
  peut alors laisser l'arrondi au plus proche même sans
  `GAOL_PRESERVE_ROUNDING` ; protéger ces chemins par une garde RAII.
- **48.** **`cancel_minus` et `cancel_plus` sont faux avec la bibliothèque
  compilée par GCC à `-O3`** (le build Release par défaut) : dans
  `difference_at_least()` (`gaol/gaol_interval.cpp`), GCC déplace les termes
  d'erreur de `two_sum` après `GAOL_RND_NEAREST_LEAVE()`, où TwoSum n'est plus
  exact. `cancel_minus([0.5], [4.9e-324, 1e-300])` donne [0x1.fffffffffffffp-2,
  0.5] au lieu de [-oo, +oo], et `cancel_minus([DBL_MAX], [0.5])` [-oo, +oo].
  Correction (essayée) : faire passer `s1`, `e1`, `s2` et `e2` par
  `gaol_core::rnd_keep()` avant `GAOL_RND_NEAREST_LEAVE()` (`GAOL_RND_KEEP` ne
  corrige rien), un test avec ces cas et `cancel_minus([DBL_MAX], [1])`, qui
  lève aussi FE_INVALID, et le commentaire de `GAOL_RND_NEAREST_ENTER`
  (`gaol/gaol_fpu.h`) à corriger.
- **27.** **Un sens d'arrondi qui ne fuit pas** : l'arrondi porté par chaque
  instruction (AVX-512, le FPCR d'AArch64 en assembleur), comme le fait inari,
  trois fois plus rapide sur les additions ; dans la direction de P2746.
  Aujourd'hui GAOL laisse l'arrondi vers le haut au programme (point 36) ou,
  avec `GAOL_PRESERVE_ROUNDING`, le change et le rend à chaque opération,
  plusieurs fois plus lentement.

### I. La documentation pour l'utilisateur (35, 37, 38, 70)

`README.md`, `doc/using.md` et « Common errors » du manuel : les écrire
ensemble, avec la mise en garde du point V et la partie manuel du point H, pour
ne régénérer le PDF qu'une fois (point Y) ; le site (70) vient après.

- **35.** **Un premier programme avant les détails** : `README.md` n'a ni code
  C++ ni renvoi à `examples/`, le premier programme est à la ligne 96 de
  `doc/using.md`, et aucun document ne montre un algorithme sur les intervalles.
  Correction : un programme de dix lignes dans `README.md` qui renvoie à
  `examples/` et à `examples/examples.md`, et un court tutoriel (encadrement
  d'image et subdivision, Newton avec `%` ou `div_rel`, un contracteur avec les
  fonctions `*_rel`, séparation et évaluation), tiré des exemples 03, 05, 06 et
  07, dans `examples/tutorial.md` (décidé le 3 octobre).
- **38.** **Les pièges dans les erreurs courantes du manuel** (« Common errors »
  de `manual/v5/gaol.tex`, section 5.3 de `examples/examples.md`) : `sqrt(2)`
  sur un nombre ; `interval(m - r, m + r)` avec des doubles ; l'ensemble vide
  qui passe tous les tests ; le domaine sans décorations ; `split()` d'un
  intervalle canonique ; `/` contre `%` dans la méthode de Newton ; `pow(x, 2)`
  qui prend le pow de GAOL quand les deux espaces de noms sont ouverts ; un
  ordre non entier de `nth_root` dans un texte, qui lève
  `invalid_action_error` ; `gaol_ieee1788::textToInterval`, qui rend l'ensemble
  vide pour un texte mal formé ; une borne nulle écrite `-0`, dont le signe
  diffère entre les builds SSE2 et FPU ; les réglages globaux
  `interval::format()` et `interval::precision()`, dont la modification
  concurrente avec l'affichage (`operator<<` ou `intervalToText()`) crée une
  data race : décider si l'API doit être rendue sûre entre threads ou si les
  changements doivent être synchronisés par l'appelant, puis le documenter.
  Décidé le 3 octobre : les pièges retenus sont aussi décrits dans une section
  courte de `doc/using.md`, qui renvoie au manuel pour le détail. Les méthodes
  `inverse()` de `interval`, `interval2f` et `intervalf` ont des commentaires
  `// TODO: document inverse()` à résoudre.
- **37.** **Une table des noms** pour les utilisateurs d'IBEX, de Codac, de
  C-XSC, de Boost et d'IEEE 1788, dans `doc/using.md` ou le manuel, avec ce que
  `<`, `<=` et `==` signifient dans chacun : dans C-XSC et PROFIL/BIAS, `<=` est
  l'inclusion, et le code porté depuis eux compile et change de sens. La section
  2.3 de `examples/examples.md` en donne une pour GAOL v5, `gaol_ieee1788`, IBEX
  et Codac seulement.
- **70.** **Un site de documentation sur GitHub Pages** (conseil de
  `TODO_mistral.md`) : décidé le 3 octobre, un site généré depuis `doc/*.md`,
  avec le PDF du manuel, publié par un workflow, sans la référence HTML de
  Doxygen, supprimée par #29.

## Autres points

### M. `tan` (9, 57)

- **9.** **Suites de `tan([-M_PI_2, M_PI_2])`** (#36). `tan()` donne maintenant
  ±1,63·10^16. Un intervalle sans pôle dont la largeur exacte est entre `pi_dn`
  et π donne encore [-oo, +oo] ; on n'en connaît aucun, et le rendre serré
  demanderait de comparer `r - l` à π en double-double. Décidé le 3 octobre :
  `doc/accuracy.md` dit le cas théorique, et `tan()` garde `!(w <= pi_dn)` sans
  le drapeau `narrower_than_pi`. Que les tests `w < pi_dn` de `cos_or_sin()`
  donnent le plus serré est mesuré, pas démontré : décidé, le dire mesuré (29
  400 intervalles) dans le commentaire et dans `doc/accuracy.md`.
- **57.** **Le commentaire de `tan()`** (suite du point 9, #36) : au-dessus de
  `const double w` (`gaol/gaol_interval.cpp`), « the test was w < pi_dn » est
  inexact (l'ancien code testait `!(w < pi_up)`, puis `w < pi_dn` pour le
  drapeau), et « though none holds a pole » se lit comme si aucun intervalle de
  cette largeur n'avait de pôle, alors que `[0, pi_dn]` contient π/2. L'entrée
  `tan` du manuel ne parle pas des largeurs entre `pi_dn` et π : une phrase.

### N. `cpack_stale_configure` (44, 63)

- **44.** **Les suites du test `cpack_stale_configure`** (#33), qui vérifie que
  CMake avertit d'un `configure` généré pour une autre version. Il échoue à tort
  quand le compilateur du parent porte un argument (`CC="ccache gcc"`) ou que
  Ninja est hors du `PATH` : décidé le 3 octobre, passer les compilateurs par
  `cmake -E env`. Sans `sh` ou sans liens symboliques, il s'arrête sur
  `FATAL_ERROR` au lieu de se dire ignoré. Avec les générateurs Makefile,
  `package_source` ne relance pas CMake après un changement de `VERSION.txt`
  (documenté ; le vérifier à l'archive demanderait CMake 3.19) : décidé, faire
  dépendre `package_source` d'une reconfiguration ; le contrôle reste un test
  CTest (jusqu'à 52 s sous QEMU).
- **63.** **Le commentaire de `cpack_stale_configure`** (suite du point 44,
  #33) : « The script removes what it made in this directory, and nothing else »
  (`tests/cpack_stale_configure.cmake`, l. 46) est inexact,
  `file(REMOVE_RECURSE)` retirant tout le répertoire (la suppression est sûre,
  c'est le commentaire qui est faux) ; `doc/tests.md` (l. 595) dit « CMake
  build » sans dire pourquoi autotools et meson n'ont pas ce test (seul CMake
  fait l'archive, et le job autotools compare déjà `configure --version` à
  `VERSION.txt`).

### O. meson et les fichiers de build (41, 42, 52, 65)

- **41.** **GAOL ne peut pas être un sous-projet meson** : `meson.build` appelle
  `add_global_arguments`, que meson refuse dans un sous-projet (de 0.53.2 à
  1.11.2), alors que le commentaire de `gaol_dep` (`gaol/meson.build`) promet
  cet usage. Correction : `add_project_arguments` (essayé avec meson 1.4.1), et
  mettre dans les `compile_args` de `gaol_dep` les options dont le code
  utilisant GAOL a besoin (celles de `pc_cflags`, que `gaol.pc` donne déjà :
  `-frounding-math`, `-ffp-contract=off`…), les arguments du projet ne passant
  pas au projet parent (décidé le 3 octobre, plutôt que retirer la promesse).
- **42.** **Suites du faux Python du Microsoft Store** (#44) : que `WindowsApps`
  ait un alias `python.exe` en plus de `python3.exe`, comme le disent
  `meson.build`, `doc/building.md` et le manuel, n'a pas été vérifié sous
  Windows : décidé le 3 octobre, adoucir le texte. Ajouter aussi au manuel le
  cas résiduel que donne `doc/building.md` (l. 263-268) : un profil dont le
  répertoire diffère de `USERPROFILE`. La documentation plutôt qu'un changement
  de l'ordre `python3`, `python` (#44) est confirmée.
- **65.** **Les `open()` de `meson.build`** (#45) : `open(sys.argv[-1],
  …).read(…)` (l. 32, 53 et 60) ne ferme pas le fichier ; sans effet sous
  CPython, un `with` le ferait.
- **52.** **La raison de C++17 dans les tests** (#37) : `tests/CMakeLists.txt`,
  `tests/Makefile.am` et `tests/meson.build` la donnent par les littéraux de
  `elementary_values.h` seulement, alors que `ieee1788.cpp`, `arithmetic.cpp`,
  `core_math.cpp` et d'autres en ont aussi : parler des tests en général.

### P. Les outils et ITF1788 (25, 30)

ITF1788 a des tests pour les fonctions réciproques à ajouter (`mulRevToPair`,
`powRev1`, `powRev2`, `atan2Rev1`, `atan2Rev2`).

- **25.** **Les outils que chaque algorithme réécrit** : `mulRevToPair` (un
  `div_rel` en deux morceaux), `inflate`, `bisect(ratio)` et `is_bisectable()`
  (`split()` coupe au milieu, ±DBL_MAX pour une demi-droite), un constructeur
  milieu-rayon, `hull()` et `intersect()` comme fonctions, un encadrement de la
  largeur (`width()` est un majorant), un littéral `_iv` dans un espace de noms
  `gaol::literals`, `erf` et `erfc` (leurs sources sont dans `3rd/math-core`,
  mais aucun build ne les compile), et les fonctions réciproques de `atan2`,
  `pow(x, y)`, `max`, `min`, `sign` et `floor`, qu'IBEX écrit lui-même.
- **30.** **Des suites de tests et des bancs d'essai où GAOL est absent** :
  passer ITF1788 (toutes les opérations d'IEEE 1788 ; seuls les cas des
  fonctions réciproques sont repris, dans `tests/reverse_values.py`) et le banc
  d'essai de Tang et al. (2021) sur GAOL v5, et ajouter à `doc/compare/`
  Boost.Interval, la bibliothèque que les utilisateurs prennent d'abord, dont
  les fonctions élémentaires ne sont pas sûres.

### Q. Le flush-to-zero et DAZ hors du lecteur (4, 45)

Le 45 est reporté (#68) ; sa partie constructeur se fait avec le point D, sa
partie parser avec le point E.

- **4.** **Le flush-to-zero et le denormals-are-zero rendent les bornes
  fausses** (revue n° 2) : `[1e-300] * [1e-20]` vaut [0, 0] dans un programme
  lié avec `-Ofast` (par `crtfastmath.o`) ou qui charge un plug-in compilé
  ainsi. Fait par #62 : une sonde sous-normale qui efface FTZ et DAZ (FZ sur
  ARM), `-mno-daz-ftz` à l'édition de liens, un test et la documentation. Décidé
  le 3 octobre : `gaol.pc` garde `-mno-daz-ftz`, qui arrête l'édition de liens
  avec Clang 18 ou un GCC antérieur à 11.4 (c'est documenté) ; GCC 9.4 sous
  `-funsafe-math-optimizations` réduit la sonde, même sous-normale, à `tiny ==
  0.0` : c'est seulement documenté, sans double `volatile` ; le moins unaire
  reste sans sonde (#62). Restent : mesurer sur les processeurs de la CI le coût
  de la sonde (+0,5 ns sur `x * y` sur un Xeon) ; ARM avec Visual C++ ou
  clang-cl, et FIZ, ne sont pas couverts ; `-fno-signed-zeros` fait sauter le `+
  0.0` de la sonde dans le code du programme (seulement documenté). La suite est
  au point 45.
- **45.** **Le denormals-are-zero fausse encore des bornes hors du lecteur**
  (suite du point 11) : des fonctions comparent des sous-normaux, que DAZ lit
  comme 0, avant toute sonde. Le parser accepte `<1e-310, 1e-309>`,
  `interval(0x1p-1073, 0x1p-1074)` n'est pas vide, `bound_to_text()` écrit une
  borne sous-normale au plus proche (à un chiffre, [22·2^-1074] s'écrit
  `[1e-322]`, relu plus petit), `sign(interval(-denorm_min))` donne `[0, 0]`
  au lieu de `[-1, -1]`, et `log`, `sqrt`, `abs`, `div_rel`, `mid()` et les
  relations se trompent : depuis #62 (point 4), avant la première opération
  qui sonde, et à chaque appel avec `GAOL_PRESERVE_ROUNDING`. Correction :
  comparer les bits, ou sonder avant de comparer, et régénérer le parser.
  Question de #58 : retirer DAZ et FTZ le temps de l'écriture, gdtoa et le
  runtime Debug de Visual C++ écrivant 0 un sous-normal sous DAZ. Les tests DAZ
  ne tournent pas sur ARM (FPCR.FZ). Reporté le 3 octobre : la correction et la
  question de #58 sont dans l'issue #68. Sous DAZ, `pow([0.5], [2^-1074])` vaut
  [1, 1] depuis le point 1 (#63) : vérifier s'il le vaut encore après #62 ; s'il
  le vaut, il relève de ce point. Depuis #71, `operator<<` écrit une borne qui
  vaut 0 à la comparaison sans en avoir les bits, un sous-normal sous DAZ, par
  un flux et au plus proche, comme avant, le `snprintf` qu'il appelle pour les
  autres bornes l'ayant écrite 0 avec MSYS2 CLANG64 : c'est elle qu'il faut
  arrondir vers l'extérieur.

### R. `atanh` et `hausdorff()` (7, 50)

- **7.** **Suites de `atanh([1, x])`** (#31) : un `TEST_EQ` de `atanh_rel` à
  borne infinie (`atanh_rel([0.5, 1], [0, +oo])`), possible depuis le point 10 ;
  `atanh([1])` et `atanh([-1])` manquent à `doc/compare/special_cases.md`
  (relancer les cinq bibliothèques) : décidé le 3 octobre, les ajouter au moment
  du point 33.
- **50.** **`hausdorff()` à borne infinie** (suite de #41) : environ 14 ns au
  lieu de 4,5 ns, la formule unique ayant été gardée. Décidé le 3 octobre :
  ajouter la sortie anticipée (+oo dès qu'une borne n'est infinie que d'un
  côté).

### S. Les exceptions de GAOL (14)

- **14.** **Suites de `what()` des exceptions** (#32). `what()` renvoie
  l'explication. Restent : les constructeurs à `const char*` des classes
  dérivées transmettent tel quel à `std::string` un pointeur nul (comportement
  indéfini, qu'aucun appel de GAOL ne fait) : le traiter comme une absence
  d'explication (décidé le 3 octobre) ; le manuel ne documente que ces
  constructeurs. Décidé le 3 octobre : sans explication, `what()` renvoie le nom
  de la classe dérivée (`input_format_error`…), de même quand le texte C de
  l'explication est vide (un NUL en tête), pour que `what()` ne soit jamais
  vide ; la forme courte d'`operator<<` reste, sans réserve là où la
  documentation dit que GAOL 4 donnait `std::exception`.

### T. Les exemples (39, 66)

- **39.** **Les restes de Goldstein-Price** (le +1 et « encloses the range »
  sont corrigés par #53) : `examples/03_dependency_problem.cpp` et
  `examples/06_global_optimization.cpp` écrivent le maximum sur [-2, 2]² comme
  un double, 2,97e-11 sous le vrai, si bien que leur vérification « l'enveloppe
  contient l'image » teste un intervalle trop étroit : le lire avec
  `textToInterval`, qui arrondit vers l'extérieur. L'exemple 16 garde le style
  de GAOL 4 dans `main()`. Les phrases d'`examples/examples.md` qui en parlent
  (des sorties identiques sur tous les builds, aucun avertissement avec `-Wall
  -Wextra`, vrai seulement en `-isystem`) sont pour la pull request de synthèse.
- **66.** **Les textes des exemples** (#53, #55) : `examples/CMakeLists.txt`
  (l. 18, « which the autotools and meson builds also compile ») semble renvoyer
  à l'enveloppe ; la section 3 de `examples/examples.md` (l. 533) parle encore
  de « three formatting slips » (il y en avait 29, corrigés) et donne comme
  ouverts `chi([0,0]) = 0`, les 400 bits et `[[nodiscard]]` en C++17 seulement.

### U. Petites erreurs (40, 64)

- **40.** **Petites erreurs, suite** (la plupart sont corrigées par #55).
  Restent : un test `nodiscard_discard_cxx*` qui reste en échec une fois son
  objet compilé ; `test_input()` de `tests/input_output.cpp`, dont le `try`
  avale une exception et saute six assertions. Décidé le 3 octobre : le
  programme qui compare les 88 sorties du manuel au programme
  (`run_examples.py`, hors du dépôt) va dans `manual/`. Non vérifié :
  `GAOL_NODISCARD` sous Visual C++ 2017 15.8 et 15.9.
- **64.** **La mise en page de `doc/tests.md`** (#32, #39, #41, #42) : cinq
  lignes de plus de 100 colonnes (l. 43, 72, 298, 300 et 308) parmi des lignes
  d'environ 80, une ligne orpheline (l. 281, « With flush-to-zero,
  denormals-are-zero or both set in MXCSR ») et « The reading of » seul sur une
  ligne (l. 476).

### V. Un ordre total pour les conteneurs (22)

Spécialiser ou non `std::less` est reporté (#70) ; la mise en garde se fait avec
le point I.

- **22.** **Pas d'ordre total pour les conteneurs** : `<`, `<=`, `>` et `>=`
  sont les relations « certainement » d'IEEE 1788, vraies dès qu'un des deux
  intervalles est vide : `std::set` perd les intervalles qui se chevauchent, et
  `std::sort` d'un vecteur contenant un intervalle vide lit au-delà de sa fin.
  Correction : un `gaol::lexicographic_less` (spécialiser aussi `std::less` ou
  non : reporté le 3 octobre, issue #70), et une mise en garde dans
  `doc/using.md` et le manuel contre `std::sort`, `std::set`, `std::max`,
  `std::min` et `std::clamp` sans comparateur.

### W. Des décorations (26)

Reporté (#67).

- **26.** **Des décorations**, ou au moins un indicateur disant qu'un argument
  est sorti du domaine : `sqrt` d'une boîte négative est vide et passe tous les
  tests d'inclusion, `interval(DBL_MAX*10)`, `x + INFINITY` et `x * NAN` sont
  vides sans rien dire, et `gaol_ieee1788::textToInterval` rend l'ensemble vide
  pour un texte mal formé. GAOL n'a que des intervalles nus (clause 11 d'IEEE
  1788-2015). À choisir : les décorations, ou un simple indicateur (reporté le
  3 octobre, issue #67).

### X. YalAA et VNODE-LP (28)

- **28.** **Des en-têtes optionnels au-dessus du cœur scalaire** : promouvoir
  `examples/box.h`, `examples/dual.h` et `examples/affine.h` (un type boîte, la
  différentiation directe, les formes affines, dont se servent les exemples 03 à
  14 et qui ne sont pas installés) en `gaol/box.h`, `gaol/dual.h` et
  `gaol/affine.h` ; ou des traits GAOL pour YalAA et un backend intervalle pour
  VNODE-LP. Décidé le 3 octobre : les traits pour YalAA et le backend pour
  VNODE-LP, et non la promotion des en-têtes des exemples.

## La version

### Y. Préparer et publier v5.0.0 (32, 33, 34, 71)

En dernier, dans cet ordre : la pull request de synthèse, une fois les branches
d'« En cours » fusionnées ; la licence ; le rapport de couverture (32) et les
temps (33) au commit de la version ; les fusions et l'étiquette (34) ; l'annonce
(71).

- **La pull request de synthèse**, une fois les branches de « En cours »
  fusionnées : `ChangeLog` et `doc/differences.md`, que rien n'a touchés depuis
  le 28 septembre (les textes proposés sont dans `todo-notes/` de la branche
  `todo-status`, et dans la description de #71 pour le point C ; pour le
  point K, à écrire d'après #72 et #73 ; dans celles de #74, #75 et #76 pour les
  points L, `make distclean` et J, et pour les suites du point J, d'après les
  commits de `configure-clean` du 4 octobre) ;
  `examples/examples.md` (marquer **Fixed** les n° 3, 4, 5, 8, 10, 12, 15, 19,
  20 et 21 de la section 5 et de l'annexe B, le n° 10 comme
  corrigé par la suppression des constructeurs à partir de chaînes (#30), et la
  ligne vide du n° 14 ; reprendre ce qu'il dit des points corrigés) ; une seule
  régénération de `manual/v5/gaol.pdf`, dont la dernière date du 28 septembre,
  après avoir corrigé dans `gaol.tex` les entrées d'index en conflit de
  « canonical interval » et l'`Overfull \hbox` des l. 511-515, et en vérifiant
  que l'entrée de `what()` ne laisse pas son en-tête seul en bas de page (macro
  `\defmethod`). Décidé le 3 octobre, pour les points 7 et 10 : la puce de
  `atanh` dans `doc/differences.md` reste, en disant que GAOL 4 n'est pas
  mesuré ; les puces de `hausdorff()` sont fusionnées ; un seul `\newinvfive`
  dans l'entrée `hausdorff` du manuel, et aucun sur la phrase du domaine de
  `atanh`.
- **La licence** (décidé le 3 octobre) : GAOL v5 reste sous LGPL, et
  `3rd/math-core` (CORE-MATH) sous MIT ; le dire clairement dans `README.md`, à
  côté de `COPYING.LIB` et dans le manuel.
- **32.** **Le rapport de couverture**
  ([coverage/README.md](coverage/README.md)), refait le 2026-09-27 à `605728e`
  (87,2 % des lignes de `gaol/`), ne correspond plus aux sources : 47 commits
  ont changé `gaol/` depuis. Le job « Coverage » de `linux.yml` le refait à
  chaque push, mais pas la copie du dépôt. À refaire au commit de la version
  (`-DGAOL_COVERAGE=ON`, cible `coverage`, gcovr) et à enregistrer.
- **33.** **Les temps de
  [doc/compare/performance.md](doc/compare/performance.md)** ont été remesurés
  le 2026-09-27 à `605728e`, sur un portable où d'autres programmes tournaient,
  et les opérations ont changé depuis (#37, #50, #54). À remesurer en dernier,
  sur le commit propre de la version, la machine ne faisant rien d'autre
  (`doc/compare/code/run_bench.sh`, ou `make perf`), puis reprendre le tableau
  de `doc/compare/README.md`.
- **34.** **Les recettes FetchContent récupèrent GAOL 4** : `doc/using.md`, le
  manuel et `tests/fetch_content` prennent la branche `master` de
  `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par défaut.
  `master` est GAOL 4.3.2, avec lequel les programmes de la documentation ne
  compilent pas, et il n'y a pas d'étiquette `v5.0.0`. Décidé le 3 octobre : une
  fois `configure-clean` fini, fusionner `configure-clean` dans `MATH-CORE`,
  puis `MATH-CORE` dans `master`, puis étiqueter `v5.0.0` ; les recettes et le
  test gardent `master` jusqu'à l'étiquette, puis la citent. Après v5.0.0, le
  développement continue dans `configure-clean`, et `master` ne reçoit que les
  versions.
- **71.** **Annoncer GAOL v5.0.0 à Frédéric Goualard**, l'auteur de GAOL, une
  fois l'étiquette posée (tâche 8.3 de `TODO_mistral.md`) : pas d'annonce
  publique, et pas de GitHub Release, l'étiquette suffisant (décidé le
  3 octobre).

## Ménage

- **Branches à supprimer sur GitHub** : celles d'« En cours », une fois
  fusionnées (les fusionnées, les jetables et `fix-path-core-math` l'ont été le
  3 octobre, celles de C, K, L, J, de `make distclean`, de #77 et de #78 après
  leur fusion).
- **Les lignes de crédit** : celles des descriptions de #50, #51, #53 à #57 et
  #59, d'un commentaire de #59 et de l'issue #49 ont été retirées le 3 octobre.
  Il en reste dans les descriptions de #60 à #63 et dans un commentaire de
  chacune de #58 et #60 à #63.
- **L'issue #49** reste ouverte comme suivi d'ensemble ; les décisions du
  3 octobre et les liens vers les issues #64 à #70 y sont en commentaire.

## Table des anciens numéros

1 : B ; 2 : B ; 3 : A ; 4 : Q ; 5 : G ; 6 : A ; 7 : R ; 8 : B ; 9 : M ; 11 : F ;
12 : E ; 14 : S ; 15 : F ; 18 : F ; 21 : D ; 22 : V ; 23 : H ; 24 : D ; 25 : P ;
26 : W ; 27 : H ; 28 : X ; 30 : P ; 31 : A ; 32 : Y ; 33 : Y ; 34 : Y ; 35 : I ;
36 : H ; 37 : I ; 38 : I ; 39 : T ; 40 : E, G et U ; 41 : O ; 42 : O ; 44 : N ;
45 : Q ; 46 : F ; 47 : A ; 48 : H ; 50 : R ; 51 : B ; 52 : O ; 53 : G ; 54 : G ;
55 : G ; 56 : A ; 57 : M ; 58 : F ; 59 : F ; 60 : F ; 61 : F ; 62 : D ; 63 : N ;
64 : U ; 65 : O ; 66 : T ; 70 : I ; 71 : Y.

## Ordre proposé pour les tâches restantes

1. **Débloquer la fin du parser : D, puis la décision parser de Q.** Traiter les
   changements du constructeur et préciser la correction DAZ du lecteur avant
   la régénération finale du parser. Les branches E et H peuvent avancer en
   parallèle sur leurs parties indépendantes.
2. **Terminer et fusionner E, puis H.** Inclure dans E la gestion récupérable de
   l'échec d'allocation du scanner ; régénérer le parser une fois les décisions
   de Q prises, puis relire les branches et résoudre le conflit de `doc/using.md`.
3. **Finir Q et les corrections de puissance B.** Faire d'abord A.3, prérequis
   noté dans B.2, puis le travail restant de B dans l'ordre indiqué par ce point.
   Compléter ensuite les tests DAZ concernés par Q.
4. **Achever les autres corrections mathématiques : M et R.** Garder les
   mesures et les tests avec les changements de bornes concernés.
5. **Fermer les suites de lecture et de build : F, G, N, O et le reste de A.**
   Faire F après la régénération du parser ; corriger `3rd/README.md` avant tout
   envoi amont pour A et valider les plateformes prises en charge.
6. **Finir les exceptions, petites corrections et documentation : S, U, T, V,
   puis I.** Intégrer dans I la décision et la documentation sur la concurrence
   des réglages de format ; coordonner V avec l'avertissement sur les
   comparateurs.
7. **Publier v5.0.0 avec Y**, après fusion des branches restantes, mise à jour
   de la documentation, couverture et mesures de performance ; faire ensuite
   le ménage listé plus haut.
8. **Après la version, reprendre les ajouts de portée plus large : P, W et X.**
   Ils portent surtout sur de nouvelles fonctionnalités ou de nouveaux
   mécanismes, plutôt que sur les corrections nécessaires à v5.0.0.
