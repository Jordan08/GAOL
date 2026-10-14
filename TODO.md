# À faire

Ce qui reste à faire sur GAOL v5 au commit `01a0252` de `configure-clean`.
Depuis le 3 octobre, les points sont regroupés et nommés par des lettres : un
point réunit ce qui touche le même code ou le même fichier, ou ce qu'un ordre
impose de faire ensemble. Chacun garde, en sous-points, les numéros de
l'ancienne liste, auxquels renvoient les pull requests, les issues (#49, #64 à
#70, #80, #98, #104, #107) et les rapports des agents (ce qui en sert encore
est dans [todo-notes/synthese.md](todo-notes/synthese.md)) ; la table à la fin
donne la lettre de chaque ancien numéro, et un numéro qui n'y est pas est un
point fait.

Les anciens points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro n
de sa section 5) ; 41 à 44, de la vérification de `VERSION.txt` (2026-09-28) ;
45 à 49, des corrections et des relectures ; 50, d'une décision du 3 octobre ;
51 à 67, des relectures des pull requests ; 68 à 71, du tri de
`TODO_mistral.md` ; 72 à 74, d'une comparaison de `pown` avec le `pow` de
CORE-MATH (5 octobre). Les décisions du 3 octobre sont écrites dans chaque point
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
CMake le supposait déjà. Fait aussi le 4 octobre : `TODO.md`, `process.md` et
`todo-notes/` hors de l'archive des sources de CPack, par #79 ; les outils qui
ont vérifié `pow` au point 1, nettoyés, dans `tests/tools/pow/`, par #81.
Fait le 5 octobre : les anciens 24 et 62 du point D (les exceptions flottantes
et leur documentation), par #83. Décidé en fin de travail, et fait par #83 :
sous Visual C++ x64 et x86, les comparaisons silencieuses de GAOL sont des
`ucomisd` en ligne (`detail::quiet_less()`... de `gaol/gaol_port.h`), plus des
appels à `_dpcomp()` ; `div_rel` des intervalles SSE2 ne calcule plus la moitié
de division qu'il jette (+oo/+oo, ou un réel divisé par une borne nulle) ; sur
ARM 32 bits et POWER9, le constructeur des intervalles FPU teste
`std::isunordered()` d'abord, comme `is_empty()` ; `GAOL_TESTS_CHOICES` retiré,
des jobs armhf et POWER9 compilés pour la taille et un job POWER9 vérifiant les
choix sur `is_empty()` ; le bug de GCC 12.1 à 12.3 et 13.1 à 13.2 (avec
`-frounding-math` sur x86, `denorm_min()` dans un tableau statique de
structures) dit dans `doc/using.md` et le manuel ; le `<` de `cr_pow` sur un NaN
et la vectorisation des comparaisons silencieuses par GCC commentés dans #65 et
#80. Les textes pour `ChangeLog` et `doc/differences.md` sont dans la
description de #83.
Fait le 5 octobre aussi : l'ancien 21 du point D, les entiers au-delà de 2^53,
par #84, et les fonctions à exposant entier autres que `pow` (`nth_root`,
`rootn`, `pownRev` et les puissances d'expressions pour tout entier, les racines
d'ordre au-delà des `unsigned` encadrées par `pow`) par #85 ; avec #84, les
fonctions internes de GAOL passent de `gaol_core::detail` (#77) à `gaol_detail`,
qu'un `using namespace gaol` n'amène pas : un `namespace detail` du programme
était ambigu. Décidé en fin de travail : une borne `long double` à côté d'un
entier reste arrondie, ce que la documentation dit, et `x + n` pour un n au-delà
de 2^53 reste un encadrement, sans être le plus étroit. Les textes pour
`ChangeLog` et `doc/differences.md` sont dans les descriptions de #84 et de #85.
Fait le 5 octobre aussi : les anciens 23, 36 et 48 du point H, par #86 :
`cancel_minus` et `cancel_plus` exacts à `-O3` (les termes de leur TwoSum
passés par `rnd_keep()`, avec un test qui lève `FE_INVALID` sans la
correction), `gaol::restore_rounding()` appelable autant de fois que voulu, la
garde `gaol_core::rounding_guard` qui protège le lecteur, `operator<<` et
`intervalToText()` d'un `std::bad_alloc` qui laissait l'arrondi au plus proche,
et la section « What the upward rounding does to the program » de
`doc/using.md` achevée : le coût de `GAOL_PRESERVE_ROUNDING` mesuré (2,2 à 3,9
fois sur les quatre opérations, `exp` 1,7, `sin` et `cos` 1,3) et les mesures
de TwoSum et TwoProd en arrondi dirigé dans l'exemple 13. Décidé en fin de
travail, et fait par #86 : la formule de Dekker de TwoProd à quatre termes
(celle à trois est fausse même au plus proche pour des opérandes de
magnitudes très différentes) ; TwoProd mesuré sur des paires de [1, 2), celles
d'exposants très différents n'y perdant pas leur exactitude ; le nom
`gaol_core::rounding_guard`. Fait le 5 octobre : le point H (ancien 27),
par #88. Décidé en fin de travail : le chemin AVX-512 des opérations de base
(+, -, *, /, sqrt) ne touche plus au sens d'arrondi, pris à l'exécution par
`GAOL_PREFER_AVX512` ; sqr et pown passent par uipow et gardent le
comportement actuel ; la partie manuel du 36 est reportée au point I ;
le « about 7 ns » d'`examples/examples.md` (l. 509) pour un bloc au plus proche
était contredit par la mesure : corrigé en « about 13 ns » (2d80cc4). Les
textes pour `ChangeLog` et `doc/differences.md` sont dans la description
de #86.
Fait le 5 octobre aussi : le nettoyage des en-têtes, qui finit le point D, par
#87. `__GAOL_PUBLIC__` devient `GAOL_PUBLIC`, vide sous Visual C++ et clang-cl,
où GAOL est toujours une bibliothèque statique : plus de `__GAOL_PUBLIC__=` à
passer, et plus de branche `_COMPILING__GAOL_PUBLIC__`. `gaol` nomme un par un
les noms de `gaol_core`, au lieu de `using namespace gaol_core;` : les noms de
GAOL 4 que GAOL v5 a encore (`NaN_val` et `rnd_keep()` de 4.3.2 compris), les
fonctions sur les intervalles de GAOL v5 et les nœuds de leurs expressions,
`restore_rounding()` et `exact_string()` ; pas les fonctions de l'arrondi et du
flush, ni les fonctions qui calculent les puissances, ni les internes que le
manuel de GAOL 4 ne documentait pas (`f_negate()`, `uintdouble`,
`Interval_struct`, `the_null_expr`, `prec_t`, `modulo_k_pi()`, `reset_fpu_cw()`,
les `round_*_sse()`...), qui restent dans `gaol_core`. `gaol_allocator.h` est
supprimé, et les en-têtes installés n'ont plus de cast à la C, ce que
`headers.sh` vérifie avec `-Wold-style-cast`. Les textes pour `ChangeLog` et
`doc/differences.md` sont dans la description de #87.
Fait le 5 octobre aussi : le point M (anciens 9, 57), par #89.
Fait le 5 octobre aussi : le point S (ancien 14), par #90.
Fait le 6 octobre : le point R (anciens 7 et 50), par #92 : `hausdorff()`
rend +oo avant de vérifier le sens d'arrondi quand une borne n'est infinie que
d'un côté (2,5 à 4,4 fois plus rapide dans ce cas), et `atanh_rel` est comparé
à ses bornes infinies ; `atanh([1])` et `atanh([-1])` dans
`doc/compare/special_cases.md` restent au point 33. Décidé en fin de travail :
la phrase de `doc/using.md` (l. 445) et du manuel qui dit que chaque opération
laisse le sens d'arrondi vers le haut est à compléter au point I ; la double
vérification de `asinh_rel()` et `atanh_rel()` va au point B.2. Les textes
pour `ChangeLog` et `doc/differences.md` sont dans la description de #92.
Fait le 6 octobre aussi : le point G (anciens 5, 53, 54, 55, 40), par #91, et
par c7915a7 et ccf694a, entrés dans `configure-clean` sans pull request (une
branche qui suivait `origin/configure-clean` y a été poussée) : le remède des
deux `#error`, qui est de redonner tous les drapeaux de GAOL après l'option, et
ce que GCC 9.4 fait des sondes du sens d'arrondi et du flush. Décidé le
5 octobre : les tests `refused_*` dans les trois builds
(`tests/refused_options.sh` pour autotools et meson). Décidé le 6 octobre : la
vérification à l'exécution n'est pas gardée ; son prototype (une fonction
`static` par unité de traduction, à la fin de `gaol/gaol_interval.h`) voyait
`-funsafe-math-optimizations`, `-fno-signed-zeros` et `-fno-honor-nans`, pour
55 ns par unité au démarrage ; si la question revenait, un message seulement,
sans `abort()`. Trouvé en fin de travail : avec Clang, `-ffast-math` et `-Ofast`
coupent `-frounding-math`, que `-fno-fast-math` ne rétablit pas (documenté dans
`doc/three-builds.md`). La description de #91, écrite avant la fin du travail,
ne donne pas les textes pour `ChangeLog` et `doc/differences.md`.
Fait le 6 octobre aussi : le point N (anciens 44 et 63), par #94 :
`cpack_stale_configure` passe aux copies de l'arbre les compilateurs avec leurs
arguments (`CC="ccache gcc"`) et le programme du générateur (un Ninja hors du
`PATH`), ce que vérifie un job de `linux.yml`, et se dit ignoré sans `sh` ou
sans liens symboliques ; `package_source` reconfigure quand `VERSION.txt` donne
une autre version que la configuration (`cmake/gaol_package_source.cmake`, que
CPack inclut, dès CMake 3.14 : il ne fallait pas CMake 3.19), avec les
générateurs Makefile aussi. Décidé en fin de travail : la reconfiguration reste
limitée à ce changement de version ; ajouté à #94, `CPACK_VERBATIM_VARIABLES`,
sans lequel CMake 3 avertissait (CMP0010) et CMake 4 relisait
`CPACK_SOURCE_IGNORE_FILES` sans ses barres obliques inverses (`\.lo$` lu
`.lo$`). Les textes pour `ChangeLog` et `doc/differences.md` sont dans la
description de #94.

Fait le 6 octobre aussi : le point T (anciens 39 et 66), par #93 : le
maximum de Goldstein-Price, qu'aucun double ne contient (le plus proche est
2,97e-11 dessous), lu avec `textToInterval` dans les exemples 03 et 06, qui
vérifient maintenant l'image réelle ; l'exemple 16 garde la mise en page de
GAOL 4 dans `main()`, ses constantes suivant le style des autres exemples ;
`examples/CMakeLists.txt` dit que les trois builds compilent les seize
exemples ; les phrases d'`examples.md` qui donnaient comme ouverts les 29
« formatting slips », `chi([0,0])`, les 400 bits et `[[nodiscard]]` en
C++17 seulement marquent ce qui est corrigé. Les textes pour `ChangeLog` et
`doc/differences.md` sont dans la description de #93.

Fait le 6 octobre aussi : le point V (ancien 22), par #96 :
`gaol::lexicographic_less`, un foncteur qu'un `std::set`, un `std::map`, un
`std::sort`... prennent comme comparateur, ordre total strict qui met
l'ensemble vide d'abord puis les intervalles par leurs bornes ; `<` est
`strictPrecedes` d'IEEE 1788 et n'en est pas un. `std::sort` avec `<` d'un
vecteur contenant un intervalle vide lit au-delà de sa fin (vérifié sous
AddressSanitizer), et un `std::set` avec `<` perd les intervalles qui se
chevauchent. La spécialisation de `std::less` reste reportée (#70), et la
mise en garde dans `doc/using.md` et le manuel se fera avec le point I. Les
textes pour `ChangeLog` et `doc/differences.md` sont dans la description
de #96.

Fait le 6 octobre aussi : le point O (anciens 41, 42, 52 et 65), par #95 : GAOL
peut être un sous-projet meson (`add_project_arguments`, les drapeaux de
`gaol.pc` dans `gaol_dep`, plus de configuration de test dans un sous-projet ;
`tests/meson_subproject` et un job de `build-systems.yml` le vérifient) ; le
faux Python du Microsoft Store dit non vérifié ; la raison de C++17 des tests ;
les `open()` de `meson.build` fermés par `with`. Décidé pendant le travail, et
fait par #95 : GAOL se compile en Release par défaut dans les trois builds, en
projet comme en sous-projet (meson avant 1.8.4 : Release forcé sur les
bibliothèques, sauf avec Visual C++ avant meson 0.57 et le backend Visual
Studio, ce que la documentation dit ; CMake sans type de build : CORE-MATH
n'était pas optimisé ; avec plusieurs configurations, Release en tête pour GAOL
seul, `cmake --build` de Visual Studio restant en Debug sans `--config`) ;
`enable-debug` (meson) et `GAOL_DEBUG` (CMake) compilent les seules
bibliothèques pour le débogage, quel que soit le type de build ; la macro
`GAOL_DEBUG(lvl, cmd)` devient `GAOL_DEBUG_VERBOSE`, sans alias ;
`set -o pipefail` dans les étapes des workflows qui passent par `tee` ; le
manuel ne dit plus que `gaol.pc` donne la bibliothèque mathématique.
`tests/release_flags.py` et le job `build-types` de `linux.yml` vérifient le
Release et le debug. Les textes pour `ChangeLog` et `doc/differences.md` sont
dans la description de #95.

Fait le 6 octobre aussi : le point U (anciens 40, hors sa partie du point E, et
64), par #97 : les tests de compilation qui doivent échouer (`refused_*`,
`nodiscard_discard_*`) compilent une copie de leur source, refaite et touchée
avant le build (`tests/compile_error.cmake`), et ne restent plus en échec quand
un en-tête ou leur source est rétabli avec son ancienne date ; `test_input()`
ne cache plus l'exception de `<-inf,-inf>`, et ses six vérifications suivantes
tournent ; `manual/check_examples.py` vérifie que les 88 exemples du manuel
impriment ce qu'il montre, dans un job de `linux.yml` ; la sortie hexadécimale
de `interval(0.1)` corrigée dans le manuel ; le commentaire du format
hexadécimal ; `doc/tests.md` sans ligne de plus de 80 colonnes. Décidé pendant
le travail, et fait par #97 : les tests compilent et passent sans exceptions
(`--disable-exceptions`, `-Denable-exception=false`), ce que deux jobs de
`build-systems.yml` vérifient, exemples compris, l'exemple 15 lisant alors avec
`gaol_ieee1788::textToInterval` ; les sorties du manuel hors des `@outputs`
restent non vérifiées. Reporté au point E : sans exceptions, un appel faux
arrête le programme dans `gaol_ieee1788::textToInterval` (#98).
`GAOL_NODISCARD` sous Visual C++ 2017 15.8 et 15.9 reste non vérifiable. Les
textes pour `ChangeLog`, `doc/differences.md` et `examples/examples.md` sont
dans la description de #97.

Fait le 6 octobre aussi : le point Q (anciens 4 et 45), par #100. Décidé au
début du travail (#68) : les fonctions qui comparent des bornes sans calculer
(le constructeur à deux bornes, les relations, `&`, `|`, `max`, `min`, `abs`,
`sign`, `mig`, `mag`, `invabs_rel`) les comparent par leurs bits là où les
doubles se comparent égaux (`gaol_detail::bound_less()`... de
`gaol/gaol_port.h`), sans écrire dans MXCSR, plutôt que par la sonde (3 à
4,5 ns de plus par appel) ; l'écriture, `midpoint()`, `split()` et `chi()`
effacent les modes le temps de l'opération (la question de #58) ; le test de
`<a, b>` du parser va au point E ; `gaol_performance` imprime dans la CI le
temps de la sonde avec 2^-1060 et avec 2^-60 : aussi long sur les 68 jobs qui
tournent sur un processeur, aucun ne prenant d'assistance microcode. Décidé en
fin de travail, et fait par #100 : les arrondis à un entier, `is_an_int`,
`is_canonical` (dont le `nextafter()` de l'UCRT et de macOS se trompait sous
DAZ), `feven`, `nb_fp_numbers`, et `inf` et `sup` de `gaol_ieee1788` par les
bits aussi ; le chemin AVX-512, dont l'arrondi porté par l'instruction
applique FTZ, calculait `x - y` après avoir rétabli FTZ avec Clang 18 et
`GAOL_PRESERVE_ROUNDING` (`[3e-308] - [2.9e-308]` valait [0, 0]) : corrigé,
avec un job Clang de `linux.yml` qui le prend ; là où GAOL ne peut pas
effacer les modes (Visual C++ pour ARM64), `operator<<` écrit une borne
sous-normale vers l'extérieur (2^-1022 ou 0), ce que `numbers` vérifie sous
`_controlfp_s()`. ARM avec Visual C++ hors de l'écriture, FIZ et
`-fno-signed-zeros` restent seulement documentés. Les textes pour `ChangeLog`
et `doc/differences.md` sont dans la description de #100.

Fait le 12 octobre : le point P (anciens 25 et 30), par #99 : les outils
`width_enclosure()`, `inflate()`, `interval::midrad()`, `bisect(ratio)`,
`is_bisectable()`, `hull()`, `intersect()` et `gaol_ieee1788::mulRevToPair` ;
le littéral `_iv` de `gaol/gaol_literals.h` ; `erf` et `erfc` ; les
réciproques `atan2_rel_y` et `atan2_rel_x` (`atan2Rev1`, `atan2Rev2`),
`pow_rel` et `pow_exponent_rel` (`powRev1`, `powRev2`), `max_rel`, `min_rel`,
`sign_rel` et `floor_rel` ; dans `doc/compare/`, ITF1788 (7 202 des 9 542 cas
réussis, aucun bogue), le banc d'essai de Tang et al. (2021), Boost.Interval et
un test d'encadrement. Décidé pendant le travail, et fait par #99 : `_iv`
encadre le nombre écrit (opérateur littéral brut), Visual C++ refusant les
entiers au-delà de 2^64 - 1 (documenté) ; `bisect` lève
`invalid_action_error` pour un ratio hors de ]0, 1[ ; des deux versions de
l'ancien 25i, `atan2_rel_y` et `atan2_rel_x` sont gardés ; `gaol_ldexp()`
(`gaol/gaol_ldexp.h`) remplace pour CORE-MATH, avec tous les compilateurs, le
`ldexp()` de la bibliothèque C, faux dans les arrondis dirigés avec l'UCRT
(`erfc` sous-normal) et sous macOS ; C4098 désactivé pour `erfc.c` plutôt que
de le modifier ; sous Windows 32 bits, `core_math_port.h` traduit chaque
`FE_*` en son bit de MXCSR (l'UCRT et mingw-w64 13 n'ont pas les valeurs de
glibc) ; `read_in_branch()` garde dans leur branche les opérations qu'un test
écarte (Clang et AppleClang pour ARM 64 bits les calculaient avant) ; les
tables de `tests/reverse.cpp` en données `constexpr` (GCC 12 pour armhf
manquait de mémoire) ; `headers.sh` en locale C ; « Jordan NININ » au lieu de
« Joran » dans les 61 fichiers de `68ec12c`. Les brouillons des signalements
à ITF1788, Boost.Interval et CORE-MATH sont dans l'issue #104, non envoyés.
Les questions restées ouvertes (l'exception de `bisect`, `010_iv`, la
politique de Boost, la colonne GAOL v5 de `doc/compare/performance.md`, la
précision de `pow` annoncée par `doc/accuracy.md`, le tableau des outils de
`process.md`) et les textes pour `ChangeLog` et `doc/differences.md` sont dans
la description de #99.

Fait le 12 octobre aussi : le point E (anciens 12, sa partie de 40 et la partie
du parser de 45 ; #98), par #105 : chaque opération calculée dès sa lecture,
l'évaluation et la destruction des expressions par des boucles (une somme de
100 000 termes débordait la pile), l'imbrication bornée par `YYMAXDEPTH`
(10 000 entrées, au-delà `input_format_error`) ; un `std::string` lu en entier
par `YY_INPUT`, un NUL n'étant le caractère d'aucun lexème (une chaîne de plus
de 2^32 caractères était lue comme ses premiers), et écrit `\0` dans les
messages d'erreur ; `std::bad_alloc`, et tout libéré, quand flex, bison, le
lexeur ou une action ne peuvent pas allouer (flex appelait `exit(2)`), que
`gaol_ieee1788::textToInterval`, passé dans la bibliothèque, laisse passer ;
sans exceptions, un appel faux qui fait échouer la lecture au lieu d'arrêter le
programme (#98, fermée) ; `<a, b>` comparé par les bits ; le commentaire
Doxygen de `parse_interval()` (40). Décidé en fin de travail : `YYMAXDEPTH`
reste à 10 000 ; `eval_stack` et `expr_eval` ne sont plus copiables ; un
lexème de plus de 2^29 caractères lève `std::bad_alloc` ; pas de test du temps
de lecture d'un long lexème (une limite de temps serait fragile) ; le parser
régénéré avec bison 3.8.2, celui d'Ubuntu 26.04, par #108 ;
`parse_interval(nullptr)` reporté (#107) ;
`process.md` mis à jour par une pull request à part. Les textes pour
`ChangeLog` et `doc/differences.md` sont dans la description de #105.
Fusionnés le 12 octobre aussi : #103, qui retire `.continue/rules`, #108, #109,
qui met `process.md` à jour (l'arbre `GAOL_V9`, les outils d'Ubuntu 26.04), et
#110, un job de la CI (`meson-oldest` de `build-systems.yml`) pour meson
0.53.2, la plus ancienne version prise en charge, que plus rien ne vérifiait.
Fait ensuite, le 12 octobre : le point A (anciens 31, 6, 56, 3 et 47), par #111
et #113 : le correctif 6 de CORE-MATH en entier dans `cbrt.c`, `rsqrt.c` et
`asinpi.c`, qui rend Cygwin juste, avec un job de la CI qui donne à CORE-MATH
les `FE_*` de Cygwin ; les masques de `sinpi.c` et `log1p.c` ;
`GAOL_RND_MINGW_FENV_ONLY` supprimé ; mingw-w64 pour ARM refusé avant 12 ou
sans `_UCRT` ; la racine carrée de SSE2 sur tout x86 32 bits ; les commentaires
du sens d'arrondi ; chaque ligne changée de CORE-MATH marquée `/* GAOL */`
(#113) ; dans `3rd/README.md`, le correctif 1 étendu à `expm1q.c` et
`atan2l.c`, le 6 à `powl.c`, et le 4 fait en amont. L'envoi reste reporté (#65,
#66), le signalement à mingw-w64 aussi (#69), et le `fma()` logiciel de
`ucrtbase.dll` n'a jamais été vérifié. Décidé à sa fin : `configure` régénéré
par `autoconf` seul quand seul `configure.ac` change, et `check_branch` qui
prend l'arbre courant, par #112 ; `atan2l.c` dans le correctif 1, son échec
dans les arrondis dirigés étant à signaler aux auteurs avec l'envoi. Le défaut
de `cr_pow` relevé par D.24 (#83) est corrigé en amont par `c01f08c5` : le
contournement de GAOL pourra tomber au prochain import de CORE-MATH. Les textes
pour `ChangeLog` et `doc/differences.md` sont dans la description de #111.

Fait le 13 octobre : le point F (anciens 11, 15, 18, 46, 58, 59, 60 et 61),
par #114, et `process.md` mis à jour par #115 (`GAOL_V8` et `GAOL_V9` se
valent ; le mainteneur ouvre les pull requests quand le jeton de `gh` ne le
peut pas). Les noms de fonctions sont lus quelle que soit la locale, pliés en
ASCII plutôt que par `std::tolower()`, qui faisait refuser `SIN(...)` sous une
locale turque de la glibc. Les tests de `numbers` sous les modes qui mettent
les sous-normaux à zéro tournent aussi sur ARM (FZ et FIZ avec GCC et Clang,
FZ posé par `_controlfp_s()` avec Visual C++ pour ARM64), et la vérification
unique du 11, faite par #116 (jetable, fermée), les montre exécutés sous
Rosetta, avec l'UCRT en Release, sur arm64, sur armhf et avec Visual C++ pour
ARM64 ; FIZ n'est honoré par aucun runner. Aussi : `\option{-Ofast}` dans le
manuel, `std::fflush(stdout)` avant le dernier test de `numbers`, son message
de temps sous la locale C, les commentaires du lexeur, et les textes
d'`operator>>` (`doc/tests.md`, `examples/examples.md`, le manuel, le
Doxygen) ; dans `examples/examples.md`, le format bounds qui se relit sous
toute locale et les points écrits entre crochets. Les anciens 15 et 18
n'avaient rien à faire (décidé le 3 octobre) ; le 46 reste reporté (#64).
Décidé pendant le travail : le test turc reste tel quel, et ne tourne dans
aucun job de la CI, faute de locale turque ; le `.h` du parser régénéré perd
les deux lignes d'en-tête ajoutées à la main (un `%code requires` exigerait
Bison 2.4, alors que le `.ypp` reste lisible par Bison 2.3) ; `-Wundef` devient
le point Z. Les textes pour `ChangeLog` et `doc/differences.md` sont dans la
description de #114.

Fait le 13 octobre aussi : la première des trois pull requests du point B
(l'ancien 1, et l'ancien 2 hors des coins de `pow`), par #117. Une seule
vérification du sens d'arrondi par appel des puissances entières de `pow`
(`pown_upward()`), de `nth_root(x, q)` pour q < 0, d'`acosh_rel()`,
`asinh_rel()`, `atanh_rel()` et `nth_root_rel()`, et de `modulo_k_pi()`, dont
les quotients par [π] sont calculés sur des doubles ; |q| sans dépassement
dans `nth_root(I, INT_MIN)` ; le test inutile `at_upper == 1.0` retiré. Décidé
pendant le travail : la classe amie `upward_access` ; la garde de `[±oo]` de
`gaol_pow_hybrid()` gardée, avec un test SSE2 ; une seule vérification aussi
pour `asinh_rel()` et `nth_root_rel()`, malgré 13 à 16 % de temps en plus
avec Clang 18, sans cause trouvée. Les textes pour `ChangeLog` et
`doc/differences.md` sont dans la description de #117.

Fait le 13 octobre aussi : la deuxième pull request du point B (l'ancien 2,
les coins de `pow`), par #118. `pow(x, y)` prend les coins de toute boîte, avec
les limites de x^y, 0 ou +∞, là où la base est 0 ou +∞ ou l'exposant infini,
au lieu d'exp(y log x) : les bornes les plus étroites partout (les 1347 cas de
`pow` d'ITF1788), une seule vérification du sens d'arrondi, et plus
d'exception de division par zéro. Un exposant dégénéré et une base partant de
0 ont leur branche ; une base autour de 1 prend un coin par borne quand
l'exposant est d'un seul signe (84 ns au lieu de 134 ; GAOL 4 : 137).
`gaol_ieee1788::pow` rend le vide pour un exposant `[±oo]`, comme la norme ;
`nth_root(I, long long)` et `nth_root(I, unsigned long long)` vérifient une
fois au-delà des `unsigned` ; `GAOL_RND_KEEP()` d'un intervalle passe par ses
deux bornes ; `fast_modulo_k_pi()` est retirée ; `doc/compare/performance.md`
est relancé en entier sous Ubuntu 26.04.1 (`setup.sh` : MPFR sans
`float128`). Décidé pendant le travail : `root_of_large_order()` ni exportée
ni `static` ; `rnd_keep()` d'un intervalle FPU par une amie ; les 5 ns de plus
de `gaol_ieee1788::pow(x, [n])` au-delà des `int` acceptés ; `huge_root()`
gardé au-delà de 64 bits ; `special_cases.md`, `enclosure.md` et tang2021
relancés à la fin de B ; Shekel 5 devient le point AA. Les textes pour
`ChangeLog` et `doc/differences.md` sont dans la description de #118.

Fait le 13 octobre aussi : un audit de ce que `TODO.md` et les descriptions
des pull requests annoncent pour les points faits (A, C à H, J à V) a vérifié
que le code de chacun est dans `configure-clean` (`f469c4e`), avec ses tests ;
ses suites, par #119 : les commentaires du chemin AVX-512 (les modes de flush
effacés comme par les autres opérations, depuis #100), l'ancien 9 dit
mesuré, pas démontré, dans le commentaire de `cos_or_sin()` et
`doc/accuracy.md` (et retiré du commentaire de `tan()`), `<a, b>` comparé par
ses bits dans `doc/using.md`, le manuel et `doc/tests.md`, et l'état de
`process.md`. `bisect(ratio)` par les `quiet_*` va au point Z (décidé le
13 octobre).

Fait le 14 octobre : le point Z (ancien 75), par #120. Les treize options de
`gaol/gaol_configuration.h` sont toujours définies, 1 ou 0, dans les trois
builds, par un même bloc `#ifndef` à la fin des trois gabarits, et les macros
de la cible (`GAOL_RND_*`, `GAOL_IX86_*`, `GAOL_ARM_MACOSX`,
`GAOL_WORDS_BIGENDIAN`...) valent 0 là où elles n'étaient pas définies : un
programme peut inclure GAOL sous `-Wundef -Werror`, ce que `headers.sh`
vérifie dans les jobs d'avertissements (GCC et Clang sous Linux, AppleClang
arm64). Les `defined()` et `#ifdef` que cela rendait toujours vrais lisent la
valeur (`GAOL_PUBLIC`, l'allocation alignée de macOS, l'exemple 13,
`diffpow.cpp`), et `tests/release_flags.py` lit `GAOL_DEBUGGING 1`.
`bisect(ratio)` compare le ratio par les `quiet_*` de `gaol_detail`. Décidé
pendant le travail : le bloc `#ifndef` plutôt que `#cmakedefine01`, qui
entrait en conflit avec le `-DGAOL_DEBUGGING=1` de CMake en Debug sous un
générateur multi-configuration ; `GAOL_SIZEOF_*` laissés à `<limits.h>` ; ni
les tests ni les `.cpp` de la bibliothèque. Les textes pour `ChangeLog` et
`doc/differences.md` sont dans la description de #120.

## En cours

Ces branches sont poussées, mais pas fusionnées dans `configure-clean`.

- `todo-b-pown` : la troisième pull request du point B, #121.

## Même changement, ou même code

### B. pow (1, 2, 8, 51, 72, 73, 74)

Tous dans `pow_standard()`, `gaol_pow_hybrid()`, `gaol_pown()` et
`ipow_exact_dn()` (`gaol/gaol_interval.cpp`), et `integer_power()`
(`gaol/gaol_interval.h`) pour le 74, avec leurs commentaires et les tests de
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
  0), deux fois par `nth_root(x, q)` pour q < 0, par `modulo_k_pi()`, et par
  `asinh_rel()` et `atanh_rel()`, qui appellent `asinh()` et `atanh()` après
  leur propre vérification (relevé par #92, décidé le 6 octobre). Correction :
  appeler les corps de ces opérations après une seule vérification, comme
  `tan()` et les puissances négatives ; les corps de `log()`, `exp()` et `*`
  sans la vérification restent à écrire. `nth_root(I, q)` calcule `|q|` par
  `-static_cast<long>(q)` pour inclure `INT_MIN`, mais `long` fait aussi 32
  bits sous MSVC et MinGW : `nth_root(I, INT_MIN)` déclenche alors un
  dépassement signé indéfini ; calculer la magnitude sans négation signée et
  couvrir ce cas dans les tests. Décidé le 3 octobre : les deux, dans cet ordre
  ; pour `pow`, après le point 3 (fait par #111), étendre ensuite l'analyse des
  coins aux bornes infinies et à une base partant de 0 : plus d'exp(y log x),
  et des boîtes serrées (aujourd'hui, `pow([4, +oo], 0.5)` vaut [2 − 2^-52,
  +oo], et d'autres ont des centaines de doubles de trop), avec des tests aux
  limites.
  Les outils qui ont vérifié `pow` au point 1 (différentiel entre deux builds,
  table de `pow_on_boxes()`, `checkrows.py` contre mpmath, mutations, temps)
  sont dans `tests/tools/pow/` (#81). Décidé le 4 octobre : ajouter avec ce
  changement à `pow_on_boxes()` les boîtes `{none, P(2147483648.0), none}` (le
  test du vide de `gaol::pow`) et `{P(1), P(0.1), P(1)}` (le coin en 1), que
  seul `elementary` protège aujourd'hui ; ne pas fixer le signe des zéros de
  `pow` (décision du 30 septembre) ; à la régénération de la table, la boîte 70
  s'écrit `0.25`. Les boîtes qui quittent exp(y log x) peuvent devenir plus
  lentes (70 à 79 ns, contre 90 à 112 ns aux coins), et les mutants des coins de
  `mutate.py` sont à réécrire.
- **8.** **`pow(x, n)` pour n grand** (revue n° 7) : dans `ipow_exact_dn()`, la
  borne inférieure perd le carré du reste (1962 doubles sous la plus serrée pour
  n = 2^32 − 1), et les builds SSE2 et FPU multiplient les produits arrondis
  dans des ordres différents, contre « les mêmes bornes sur toutes les
  machines » de `doc/accuracy.md`. Fait par #61. Une borne nulle, ou une seule
  borne hors de la plage, envoie les deux bornes aux produits arrondis (jusqu'à
  10 doubles de trop). Décidé le 3 octobre : rendre `ipow_exact_dn(0)` exact,
  sans traiter chaque borne à part ; garder l'ancien code SSE2 sous `#if 0` ;
  garder la garantie 5 n log2(n) 2^-104. `pown([-2, 3], 100)` différait entre
  SSE2 et FPU avec GCC 9.4 (#37) : à revérifier. Décidé le 4 octobre : avec
  `ipow_exact_dn(0)`, réécrire la phrase de `doc/accuracy.md` (l. 94) et du
  manuel (l. 4301) qui dit les deux bornes prises aux produits arrondis
  « where one is 0 or infinite or its power is below 2^-968 » : elle est
  approximative pour une puissance paire d'un intervalle qui contient 0, dont
  la plus petite borne n'est jamais élevée (`pow([-2^-400, 2], 4)` prend les
  produits exacts).
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
- **72.** **`pown(x, n)` pour n < 0 au plus serré, avec le `pow` de
  CORE-MATH** : `gaol_pown()` calcule x^-m comme 1/x^m (ou (1/x)^m où x^m
  déborde), soit deux arrondis. Sur 200 000 tirages par plage (x dans
  [0.5, 2] pour n de −3 à −100, x près de 1 pour n de −10^3 à −10^6), environ
  la moitié de chaque borne a 1 ou 2 doubles de trop (« within 2 doubles » de
  `doc/accuracy.md`, l. 93). CORE-MATH n'a ni pown ni rootn en binary64 au
  commit `6b84457`, mais `cr_pow(x, (double)n)` est un pown pour tout `int` :
  n est exact en double, et `cr_pow` donne à une base négative le signe de la
  parité de n (vérifié pour n impair : `cr_pow(-x, n)` vers le haut est
  l'opposé de `cr_pow(x, n)` vers le bas). Correction : les bornes de
  `pow_lo()` et `pow_hi()`, comme `pow_standard()` : la valeur de `cr_pow`
  arrondie vers le haut, et le double au-dessous, sauf où la puissance est un
  double, ce que `pow_is_double()` prouve déjà pour un exposant entier
  (k = 0). Le découpage de x selon le signe et le 0 intérieur reste celui de
  `gaol_pown()`, et une borne 0 est prise à part : `cr_pow(0, n)` vaut +oo
  pour n < 0 et lève la division par zéro. Coût pour un intervalle (x86-64,
  GCC, Release, deux appels de `cr_pow` sans `pow_is_double()`) : 55 ns au
  lieu de 24 pour n = −3, 54 au lieu de 40 pour n = −30, 58 au lieu de 81
  pour n = −1000. À décider : `cr_pow` pour tout n < 0 (les bornes les plus
  serrées, deux fois plus lent aux petits |n|), ou seulement au-dessus du
  seuil du point 73.
- **73.** **`pown(x, n)` pour n grand avec le `pow` de CORE-MATH** : pour
  n ≥ 3, les produits exacts de `ipow_exact_up()` et `ipow_exact_dn()`
  donnent déjà les bornes les plus serrées (aucun écart sur 200 000 tirages
  par plage pour n de 3 à 10^6 ; un double de trop à 0,003 % des bornes
  inférieures pour n de 2^28 à 2^31 − 1, x près de 1), mais leur coût croît
  avec log2(n) : pour un intervalle, 17 ns à n = 3, 54 ns à n = 100, 77 ns à
  n = 1000, 118 ns à n = 10^5 et 262 ns à n = 2·10^9, contre 51 à 90 ns pour
  deux appels de `cr_pow`, quel que soit n. Correction : au-dessus d'un seuil
  à mesurer (vers 100 à 200, sur plusieurs machines), les bornes de `cr_pow`
  comme au point 72 : plus rapides, toujours les plus serrées, les mêmes sur
  toutes les machines, sans la garantie 5 n log2(n) 2^-104 ni le repli sur
  les produits arrondis d'une borne nulle ou hors de la plage ; sous le seuil,
  les produits exacts. À faire avant la fin du point 8, dont le repli ne
  resterait que sous le seuil. Réécrire avec les points 72 et 73 la ligne de
  `pown` de `doc/accuracy.md` (l. 93) et le manuel.
- **74.** **`pow(x, n)` pour un entier au-delà des 32 bits** : `gaol::pow(x,
  n)` et `gaol_ieee1788::pown(x, n)` pour un entier hors des `int` et des
  `unsigned int` (`integer_power()`, `gaol/gaol_interval.h`, l. 1309), et
  `gaol::pow(x, y)` pour un `[n]` dégénéré entier hors des `int`
  (`gaol_pow_hybrid()`), donnent [−oo, +oo] ; seul `gaol_ieee1788::pow` les
  calcule, sur x ≥ 0 (le bloc au-delà des int de `pow_standard()`). x^n y est
  pourtant fini et non trivial pour x près de 1 : `pow(interval(1 + 2^-30),
  1LL << 33)`, environ e^8, vaut [−oo, +oo], alors que `cr_pow` donne
  [2980.9579759367944, 2980.9579759367948]. Correction : pour |n| ≤ 2^53, où
  n est un double, les bornes de `cr_pow` comme aux points 72 et 73, base
  négative comprise ; au-delà, où n n'est pas toujours un double, encadrer
  |x|^n, par exemple par x^h · x^l avec n = h + l, h et l des doubles (à
  vérifier aux bords de la plage), le signe venant de la parité de n lue sur
  l'entier. Prolonge le 21, fait par #84, qui a donné `pow(x, n)` pour tout
  type entier. Réécrire aussi la ligne de `pow` de `doc/accuracy.md` (l. 94),
  qui dit [−∞, +∞] pour `gaol::pow`.

Décidé le 12 octobre, pour tout le point B :

- **Le moins de `if` possible** dans `pow` et `pown`.
- **Le `pow` de CORE-MATH** (`cr_pow`) partout où il n'est pas encore pris :
  en binary64, l'amont n'a ni pown, ni rootn, ni compoundn (commit `dc0d7e0`,
  12 octobre).
- **Une seule vérification du sens d'arrondi par appel** de `pow` et de
  `pown`, sous chacune de leurs formes. Pour `pow`, elle vient de l'extension
  des coins du 2, qui retire exp(y log x) : les corps de `log()`, `exp()` et
  `*` sans vérification ne sont pas écrits. Un compteur des vérifications,
  dans les tests, le montre en local ; il est retiré du code poussé.
- **La norme** : `gaol_ieee1788::pow` et `gaol_ieee1788::pown` suivent
  IEEE 1788-2015. `gaol::pow` est `gaol_ieee1788::pow`, sauf pour un exposant
  entier, où il est `gaol_ieee1788::pown` : un type entier, un double entier
  ou un intervalle dégénéré `[n]`, au-delà des `int` compris (74).
- **Plus rapide que GAOL 4.2.3** pour chaque forme de puissance, mesuré comme
  dans `doc/compare/performance.md` (clang-18, sans préservation de
  l'arrondi), où `pow(x, 3)` prend 21,4 ns contre 13,8. Où le plus serré et
  la vitesse s'opposent (72, 73), à décider sur ces mesures.
- **Toutes les configurations de la CI.**
- **ITF1788 et tang2021 relancés** (`doc/compare/itf1788/run.sh`,
  `doc/compare/tang2021/run.sh`), et leurs pages mises à jour : 68 cas de
  `pow` et 15 de `pown` d'ITF1788 n'y sont pas les plus serrés.
- **Trois pull requests**, dans cet ordre : une vérification par appel (1, et
  le 2 hors des coins de `pow`, `nth_root(I, INT_MIN)` compris) ; les coins
  de `pow` (2) ; `pown` (8, 72, 73, 74). Les textes du 51 vont avec le code
  qu'ils décrivent.

Décidé aussi le 12 octobre, sur les mesures faites avant le code (clang-18,
i7-1185G7, GAOL 4.2.3 construit comme dans `doc/compare/code`) :

- **72** : sous le seuil du 73, l'inverse de x^m calculé à partir des
  produits exacts (h + l), corrigé par `fma` ; `cr_pow` au-dessus.
- **73** : le seuil se mesure sur cette machine seulement.
- **`pown` pour les grands |n|** : aucune méthode aux bornes les plus serrées
  ne bat GAOL 4 au-delà de x^30. x^1000 prend 14,9 ns avec GAOL 4, dont les
  bornes ont 1 300 à 1 500 doubles de trop, et 62,7 ns aujourd'hui. Un
  prototype, les produits exacts des deux bornes dans un registre SSE2 avec
  une borne d'erreur a priori, donne les bornes les plus serrées en 11,7 ns
  pour x^3 (GAOL 4 : 13,2) et 23,1 ns pour x^1000. L'étude continue, pour
  raccourcir sa chaîne de dépendances, avant d'écrire `pown`.
- **`acosh_rel()` et `nth_root_rel()`**, qui vérifient aussi le sens
  d'arrondi deux fois, sont corrigés dans la première pull request.
- **`inverse_upward()`**, privé, est donné à `pown` et à `nth_root(I, q)`
  pour q < 0 par une classe amie, `upward_access`, définie dans
  `gaol/gaol_interval.cpp` seul.

Décidé le 13 octobre, à la fin de l'étude (sur cette machine, Ubuntu 26.04.1,
sur secteur ; Clang 18 et Clang 21 donnent les mêmes temps à 3 % près, sauf
`cr_pow`, 18 % plus rapide avec Clang 21) :

- **`pown` dans les `int`** : les bornes les plus serrées pour tout n, par la
  méthode du prototype (les deux bornes dans un registre SSE2, les produits
  exacts h + l, une borne d'erreur a priori ; pour n < 0, l'inverse corrigé,
  et x^-1 par la division). Elle prend la moitié du temps de GAOL 4 jusqu'à
  |n| = 16 (x^3 : 4,1 ns contre 8,8), moins aux puissances de 2, et 1,1 à
  1,6 fois son temps au-delà (x^1000 : 14,2 ns contre 9,7), où les bornes de
  GAOL 4 ont 32 à 1 500 doubles de trop : l'exigence de vitesse n'y est pas
  tenue. Aucune méthode « à quelques doubles près » ne coûte moins : l'erreur
  d'une étape est doublée à chaque carré qui suit.
- **73** : pas de `cr_pow` dans les `int`, pas de seuil (deux appels de
  `cr_pow` ne battent la méthode qu'au-delà de 2^27 environ avec Clang 21) ;
  `cr_pow` au-delà des `int` seulement (74).
- **Les mesures de performance** se font avec Clang 21 : celles de la
  troisième pull request et `doc/compare/performance.md`, à remesurer avec
  lui.
- **Les deux espaces de noms**, à la fin de la troisième pull request :
  - `doc/compare` teste GAOL v5 par `gaol_ieee1788` : en plus de `gaol` dans
    les cas particuliers et l'encadrement, à la place de `gaol` dans les
    performances (GAOL 4 reste mesuré par son `gaol`) ;
  - ITF1788 et tang2021 relancés par `gaol_ieee1788` (tang2021 passe
    aujourd'hui par `gaol::interval`) ;
  - la documentation (`README.md` et partout où c'est utile) dit les deux
    espaces de noms : `gaol`, pour rester compatible avec GAOL 4, et
    `gaol_ieee1788`, pour suivre IEEE 1788-2015 ; la principale différence est
    dans `pow` et `pown`, avec une partie qui lui est consacrée.

Décidé aussi le 13 octobre, au début de la troisième pull request :

- **74** : la fonction exportée pour un exposant hors des `int` et des
  `unsigned int` s'appelle `gaol_pown_ll(const interval&, long long)` ;
  au-delà de 2^63, l'exposant est ramené à ±(2^63 − 1) ou ±(2^63 − 2) selon
  sa parité, toute puissance y débordant ou s'annulant déjà.
- **8** : quand une seule borne sort de la plage, l'autre garde la borne la
  plus serrée (chaque coin hors de la plage est calculé seul, mis à
  l'échelle, au plus serré lui aussi).
- **Un double de trop accepté** : la borne d'erreur a priori K de la méthode
  laisse un double de trop quand x^n est à moins de 2K d'un double. C'est
  rare au hasard (2^-46 des bornes pour n = 3), mais fréquent pour un x à
  quelques doubles d'une valeur dont la puissance est exacte : un cinquième
  des bornes pour x à moins de 8 doubles de 1 et n jusqu'à 200
  (`pow([1 + 2^-52], 3)` donne [1 + 2^-51, 1 + 2^-50] au lieu de
  [1 + 3·2^-52, 1 + 2^-50]). Trancher ces cas par `cr_pow` donnait toujours
  les plus serrées, mais coûtait 55 à 270 ns, ces puissances étant les cas
  difficiles de CORE-MATH : ce n'est pas fait, et c'est documenté.

Décidé le 14 octobre, à la fin de la troisième pull request (#121) :

- **Les puissances des expressions au-delà de 2^53**, pour un n qui n'est
  pas un double, restent le produit `pow(e, [h])·e^l` (n = h + l) : pas de
  nœud portant un `long long`, qui ajouterait une méthode à `expr_visitor`.
- **x^(2^31 − 1) à 1,64 fois le temps de GAOL 4.2.3** : accepté.
- **Le test des exposants de plus de 64 bits** (`__int128`) reste limité
  aux modes GNU, sans job de CI de plus.
- **La division par zéro des intervalles SSE2** est corrigée dans #121 :
  `inverse()` (donc `pow(x, -1)`), `/` et `%` d'un diviseur à borne nulle
  divisaient la moitié jetée par 0 et levaient l'exception, là où les
  intervalles FPU ne lèvent rien.

## Même fichier de test ou de documentation

### I. La documentation pour l'utilisateur (35, 37, 38, 70)

`README.md`, `doc/using.md` et « Common errors » du manuel : les écrire
ensemble, avec la mise en garde du point V et la partie manuel du point H, pour
ne régénérer le PDF qu'une fois (point Y) ; le site (70) vient après.
Décidé le 6 octobre (#92) : y compléter aussi la phrase de `doc/using.md`
(l. 445) et du manuel selon laquelle chaque opération laisse le sens d'arrondi
vers le haut : celles qui rendent avant leur vérification (un opérande vide,
`hausdorff()` quand une borne n'est infinie que d'un côté) le laissent tel
qu'elles l'ont trouvé.

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

### AA. Shekel 5, plus lent qu'avec GAOL 4 (76)

- **76.** **Shekel 5 prend moitié plus de temps avec GAOL v5 qu'avec GAOL
  4.2.3** (relevé au point B, #118, le 13 octobre). Le banc de
  `doc/compare/performance.md`, relancé ce jour-là pour toutes les
  bibliothèques (Ubuntu 26.04.1, Clang 18.1.8, `-O3 -mfma`, sur le processeur
  2), donne 277 ns contre 185. Les opérations dont Shekel 5 est fait coûtent à
  GAOL v5 10 à 23 % de plus qu'à GAOL 4.2.3 : + et − 2,7 ns contre 2,5, ÷ 9,7
  contre 8,4, le carré 7,6 contre 6,1. Shekel 5 (`doc/compare/code/bench_ops.h`)
  en fait 25 soustractions, dont les 20 `x_j - a_ij` d'intervalles dégénérés,
  20 additions, 20 carrés et 5 divisions de `[1]` : leurs écarts n'expliquent
  qu'environ 47 des 92 ns.
  - L'écart précède le point B : `configure-clean` du 12 octobre, avant #117,
    prend autant (258 ns, contre 249 sur la branche de #118, par le même
    programme, le même jour).
  - Le 27 septembre, sous Ubuntu 20.04, l'écart était de 12 % (315 contre
    280 ns), mais la colonne de GAOL v5 venait alors de `make perf` avec
    GCC 9.4, et celle de GAOL 4.2.3 de Clang 18.

  À chercher : ce qui coûte plus dans l'expression que dans le banc de chaque
  opération seule, par exemple la soustraction d'un intervalle dégénéré, la
  division de `[1]`, ou les dépendances entre les opérations, que le banc
  d'une opération seule ne crée pas. Mesurer les formes de Shekel 5 une à
  une, et comparer le code généré à celui de GAOL 4.2.3 (valgrind, `perf`
  n'étant pas permis sur cette machine). Corriger si la cause est dans GAOL.

## La version

### Y. Préparer et publier v5.0.0 (32, 33, 34, 71)

En dernier, dans cet ordre : la pull request de synthèse, une fois les branches
d'« En cours » fusionnées ; la licence ; le rapport de couverture (32) et les
temps (33) au commit de la version ; les fusions et l'étiquette (34) ; l'annonce
(71).

- **La pull request de synthèse**, une fois les branches de « En cours »
  fusionnées : `ChangeLog` et `doc/differences.md`, que rien n'a touchés depuis
  le 28 septembre (les textes proposés sont dans `todo-notes/synthese.md`, et
  dans la description de #71 pour le point C ; pour le
  point K, à écrire d'après #72 et #73 ; dans celles de #74, #75 et #76 pour les
  points L, `make distclean` et J, et pour les suites du point J, d'après les
  commits de `configure-clean` du 4 octobre) ;
  `examples/examples.md` (marquer **Fixed** les n° 2, 3, 4, 5, 7, 8, 10, 12,
  15, 16, 19, 20, 21 et 22 de la section 5 et de l'annexe B, le n° 10 comme
  corrigé par la suppression des constructeurs à partir de chaînes (#30) ;
  reprendre ce qu'il dit des points corrigés, la ligne vide du n° 14 l'étant
  par #114) ; une seule
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
  de `doc/compare/README.md`. Au même moment, ajouter `atanh([1])` et
  `atanh([-1])` aux cas de `doc/compare/special_cases.md` en relançant les cinq
  bibliothèques (ancien 7, décidé le 3 octobre).
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
  3 octobre, celles de C, K, L, J, de `make distclean`, de #77, #78, #79, #81,
  #83, #84, #85, #86, #87, #88, #89, #90, #91, #92, #93, #94, #95, #96, #97,
  #99, #100, #103, #105, #108, #109, #110, #111, #112, #113, #114, #115, #117,
  #118, #119 et #120 après leur fusion, et la jetable de #116).
- **Les lignes de crédit** : celles des descriptions de #50, #51, #53 à #57 et
  #59, d'un commentaire de #59 et de l'issue #49 ont été retirées le 3 octobre.
  Il en reste dans les descriptions de #60 à #63 et dans un commentaire de
  chacune de #58 et #60 à #63, et dans les messages des commits `a0d3871`,
  `790ca12` et `f1f4239` de #99, fusionnée sans squash le 12 octobre.
- **L'issue #49** reste ouverte comme suivi d'ensemble ; les décisions du
  3 octobre et les liens vers les issues #64 à #70 y sont en commentaire, comme
  celles du 4 octobre sur les questions restées ouvertes dans les rapports
  (#80).
- **`todo-notes/`** : ramené de la branche `todo-status`, supprimée ensuite ;
  ses rapports, relus le 4 octobre, sont retirés de l'arbre (ils restent au
  commit `16a2f60`), et ce qui en sert encore est dans
  `todo-notes/synthese.md`.

## Table des anciens numéros

1 : B ; 2 : B ; 8 : B ; 26 : W ; 28 : X ; 32 : Y ; 33 : Y ; 34 : Y ; 35 : I ;
37 : I ; 38 : I ; 51 : B ; 70 : I ; 71 : Y ; 72 : B ; 73 : B ; 74 : B ;
76 : AA.

## Ordre proposé pour les tâches restantes

1. **Débloquer la fin du parser.** Fait : le point D (#83, #84, #85 et #87),
   et la décision parser de Q (#100 : le test de `<a, b>` par les bits, au
   point E). Les branches E et H peuvent avancer en parallèle sur leurs
   parties indépendantes.
2. **Terminer et fusionner E.** Inclure dans E la gestion récupérable de
   l'échec d'allocation du scanner ; régénérer le parser avec le test de
   `<a, b>` du point Q, puis relire la branche. Le point H est fait (#86),
   hors le 27 et la partie manuel reportée au point I. E est fait par #105.
3. **Finir les corrections de puissance B.** A.3, prérequis noté dans B.2,
   est fait par #111 : faire le travail de B dans l'ordre indiqué par ce
   point. Q est fait par #100.
4. **Achever les autres corrections mathématiques : M et R.** Garder les
   mesures et les tests avec les changements de bornes concernés. Faits par #89
   et #92.
5. **Fermer les suites de lecture et de build : F, G, N, O et le reste de A.**
   G est fait par #91, N par #94, O par #95, A par #111 et #113, F par #114.
6. **Finir les exceptions, petites corrections et documentation : S, U, T, V,
   puis I.** Intégrer dans I la décision et la documentation sur la concurrence
   des réglages de format ; coordonner V avec l'avertissement sur les
   comparateurs. S, T, U et V sont faits par #90, #93, #97 et #96, Z par
   #120. AA (Shekel 5) peut se faire à tout moment avant Y.
7. **Publier v5.0.0 avec Y**, après fusion des branches restantes, mise à jour
   de la documentation, couverture et mesures de performance ; faire ensuite
   le ménage listé plus haut.
8. **Après la version, reprendre les ajouts de portée plus large : W et X.**
   Ils portent surtout sur de nouvelles fonctionnalités ou de nouveaux
   mécanismes, plutôt que sur les corrections nécessaires à v5.0.0. P est fait
   par #99.
