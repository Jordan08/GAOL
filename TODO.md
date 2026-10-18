# À faire

Ce qui reste à faire sur GAOL v5 au commit `ce2bf3f` de `configure-clean`.
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
CORE-MATH (5 octobre) ; 77 à 103, de la relecture de `configure-clean` du
14 octobre ; 104 et 105, des décisions du point AA (14 octobre) ; 106 à
112, des mesures de la CI de #124 (17 octobre). Les décisions du
3 octobre sont écrites dans chaque point (« Décidé le 3 octobre ») ; une
question reportée renvoie à son issue. Fait
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
`GAOL_PREFER_AVX512` ; la partie manuel du 36 est reportée au point I ;
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

Fait le 14 octobre aussi : la troisième pull request du point B (les anciens 8,
51, 72, 73 et 74), par #121, qui finit le point B. `pown` donne dans les `int`
les bornes les plus serrées, de tout signe : les deux coins à la fois, dans les
deux voies d'un registre SSE2 avec FMA ou d'une paire de doubles, par des
produits exacts h + l et une borne d'erreur K prouvée d'avance (0 pour x^2, qui
est `sqr(x)`) ; x^-n par l'inverse corrigé ; les puissances qui sont des
doubles exactes ; les coins hors de la plage calculés seuls, mis à l'échelle.
Au-delà des `int`, le `pow` de CORE-MATH aux coins (`gaol_pown_ll()`), et pour
les expressions `pow(e, [h])·e^l` au-delà de 2^53. x^3 prend 4,9 ns, contre 8,8
avec GAOL 4.2.3. Aussi : les deux espaces de noms, `gaol` et `gaol_ieee1788`,
dans `README.md`, `doc/using.md` et le manuel ; `doc/compare`, ITF1788 et
tang2021 relancés avec Clang 21 sous `gaol_ieee1788` (tous les cas de `pow` et
de `pown` d'ITF1788 les plus serrés) ; `check.py` et `run.sh` de tang2021
contournent deux défauts de gmpy2 2.2.1 (`context.div()` arrondi au plus près,
et les `mpfr` jamais libérés) ; la division et l'inverse SSE2 d'un diviseur à
borne nulle, et l'inverse FPU sous AppleClang, ne lèvent plus la division par
zéro. Décidé pendant le travail (13 et 14 octobre) : la méthode du prototype
P1y pour tout n dans les `int`, `cr_pow` au-delà seulement ; un double de trop
accepté où x^n est à moins de 2K d'un double ; le nom `gaol_pown_ll` ; la borne
restée dans la plage gardée la plus serrée ; x^(2^31 − 1) à 1,64 fois le temps
de GAOL 4.2.3 accepté ; le produit pour les expressions au-delà de 2^53, sans
nœud `long long` ; le test `__int128` limité aux modes GNU ; les mesures avec
Clang 21. Les textes pour `ChangeLog` et `doc/differences.md` sont dans la
description de #121.

Fait le 15 octobre : le déplacement du parser et des expressions dans
`gaol/parser/` (13 fichiers, les trois builds, la CI et la documentation ;
détail dans sa propre section ci-dessous, question CC.q4 répondue) ; le ménage
du point 93 (`gaol_exact.h`, `gaol_exact.c` et `sysdeps/`, `gaol_flags.h`
avec `prec_t` déplacé dans `gaol/parser/gaol_expression.h`, `s_nextafter.c`)
et de `gaol_assert.h` (`GAOL_ASSERT` remplacé par `assert`, détail dans
« Ménage ») ; les textes qui disaient que les builds Debug vérifient les
assertions de GAOL avec `GAOL_DEBUGGING` disent maintenant que l'absence de
`NDEBUG` garde `assert()` actif (`doc/building.md` à trois endroits,
`doc/three-builds.md`, `doc/differences.md`, le manuel), et `doc/using.md`
liste `gaol_assert.h` parmi les en-têtes de GAOL 4 retirés. Relecture de la
branche : audit des fichiers restants de `gaol/` et de `gaol/parser/` (un
seul sans aucun référenceur, CC.m6), cohérence des trois installations (les
mêmes 23 en-têtes), des scripts de CI et de `doc/using.md` « The headers » ;
trouvailles CC.m4 et CC.m5, questions CC.q5 à CC.q7. Demandé par le
mainteneur (réponses CC.q5 à CC.q7) : les intervalles de floats et l'outil
de l'auteur dans `gaol/float/` (5 fichiers, détail dans sa propre section
ci-dessous), le cast explicite de `gaol_profile.cpp:52` (CC.m4), et la mise
à jour de l'issue #49 pour le 15 octobre (faite).

Fait le 16 octobre : les tests compilés avec `-Wno-psabi` là où le
compilateur l'accepte, par #123, dans les trois builds, les trois projets de
test et `.github/scripts/tests.sh`. Les notes de GCC sur le passage des
arguments (« parameter passing for argument of type ... changed in GCC ... »),
une centaine à trois cents par build sur armhf et une ou deux sur aarch64 et
ppc64le, ne remplissent plus les journaux de la CI. Ni la bibliothèque, ni
`gaol.pc`, ni les exemples ne reçoivent l'option. Décidé pendant le travail :
l'option plutôt que les tests en C++14, qui n'enlève pas les notes d'armhf.
Rien pour `ChangeLog` ni `doc/differences.md` (description de #123).

Fait le 17 octobre : le point AA (ancien 76), par #122. Shekel 5 prend
263 ns dans le banc de `doc/compare` sur le Xeon E3-1220 v6, contre 358 avant
et 274 avec GAOL 4.2.3, sans qu'une borne change : `sqr()` des intervalles
SSE2 calcule son produit lui-même, et les puissances arrondies qu'il était seul
à appeler (`uipow_rounded()`, `uipow_upup()`, `uipow_dnup()`) sont retirées ;
`+`, `-`, `/` et les opérateurs avec un double appellent le chemin AVX-512 en
appel terminal, sans cadre de pile. Ce chemin se décide à la configuration
(`GAOL_AVX512_ARITHMETIC`), sur la machine qui construit, jamais en
compilation croisée ni avec GCC pour Windows (bug 54412 de GCC) ;
`gaol_core::avx512_arithmetic` disparaît, `gaol::init()` arrête avec un
message un programme lancé sur un processeur sans AVX-512, et l'option
`enable-prefer-avx512` de meson devient un `combo` `auto`/`true`/`false`.
L'écart qui reste avec GAOL 4.2.3 sur `+` et la ligne arithmétique est la
vérification du sens d'arrondi de chaque opération (points H et Q). Décidé
pendant le travail : le chemin à la configuration et le défaut gardé, une
bibliothèque qui prend le chemin ne tournant alors que sur des processeurs
AVX-512 (15 octobre) ; le `combo` de meson, l'arrêt dans `init()` et GCC pour
Windows écarté (16 octobre) ; `operator*=` de deux intervalles laissé tel
quel, plus lent en appel terminal. Décidé le 14 octobre : deux points le
suivent, AG et AH, et la phrase « sqr et pown passent par uipow » de la
décision du 5 octobre (point H) est retirée. Les textes pour `ChangeLog` et
`doc/differences.md` sont dans la description de #122.

Fait le 18 octobre : le point AG (ancien 104, le banc de `doc/compare` sur
l'i7-1185G7) et le point ZZ (la vitesse de `+`, `-`, `*`, `/` et `sqr()`,
demandée le 17 octobre), par #124. Shekel 5 prend 108 ns dans le banc, contre
189 avec GAOL 4.2.3 (210 contre 170 avant), `x * y` 2,3 ns contre 10,0, et les
dix lignes de `+ - * /` du banc sont plus rapides qu'avec GAOL 4.2.3, sans
qu'une borne change, hors le signe d'une borne nulle d'un produit ou d'un
quotient (`doc/accuracy.md`) : les opérateurs des intervalles SSE2 appellent des
fonctions de la bibliothèque (`add_bounds()`, `sub_bounds()`, `mul_bounds()`,
`div_bounds()`, `sqr_bounds()`) qui prennent les bornes dans un registre, `*`,
`/` et `sqr()` sans `switch` sur les signes, et le test du sens d'arrondi est
une comparaison. Le chemin AVX-512 prend aussi `sqr()`, les puissances entières
(`pown_large_er()`) et `sqrt()`, par deux racines scalaires ; avec
`GAOL_PRESERVE_ROUNDING` sur x86-64 avec FMA, `GAOL_RND_ENTER()` n'écrit plus
que MXCSR. Le banc a dix lignes d'opérateurs (`run_lines.sh`),
`gaol_performance` cinq, et `performance.md` une colonne « rounding direction
preserved » (`MERGE=1`). Les avertissements de la CI sont corrigés (tests,
parser, `meson.build`, workflows). Décidé pendant le travail : le point AI de ce
fichier, la vitesse sur macOS, Windows et ARM, relevée par la CI. Les textes
pour `ChangeLog` et `doc/differences.md` sont dans la description de #124.

## En cours

Ces branches sont poussées, mais pas fusionnées dans `configure-clean`.

Aucune.

## Même fichier de test ou de documentation

### I. La documentation pour l'utilisateur (35, 37, 38, 70)

`README.md`, `doc/using.md` et « Common errors » du manuel : les écrire
ensemble, avec la mise en garde du point V et la partie manuel du point H, pour
ne régénérer le PDF qu'une fois (point Y) ; le site (70) vient après.
Décidé le 6 octobre (#92) : y compléter aussi la phrase de `doc/using.md`
(l. 445) et du manuel selon laquelle chaque opération laisse le sens d'arrondi
vers le haut : celles qui rendent avant leur vérification (un opérande vide,
`hausdorff()` quand une borne n'est infinie que d'un côté) le laissent tel
qu'elles l'ont trouvé. La relecture du 14 octobre en ajoute : celles qui
n'arrondissent rien (`-x`, `abs`, `floor`, `min`, `max`, `invabs_rel`), et
`nth_root(x, 0)` et `nth_root(x, 1)` ; la phrase est maintenant aux l. 749-750
de `doc/using.md`.

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

## Trouvé par la relecture de `configure-clean` du 14 octobre

Relecture de `configure-clean` à `533992f`, le 14 octobre au soir, par six
relecteurs indépendants : les puissances entières, le `pow` réel et ses
réciproques, les autres opérations, la lecture et l'écriture, les trois
builds, la documentation. Chaque défaut ci-dessous a été reproduit par un
programme, sauf ceux dits non vérifiés, avec les builds CMake SSE2 et FPU
(GCC 15.2), Clang 18 et 21, et ASan + UBSan. Les références sont calculées à
part : rationnels exacts, mpmath, MPFR par gmpy2. Aucune borne fausse : sur
environ 1,3 million de puissances entières, 550 000 `pow` réels, 600 000
autres opérations et 760 lectures de nombres, tout résultat contient la valeur
exacte. À faire avant Y ; AF avec le point I, pour ne régénérer le manuel
qu'une fois.

### AB. Des exceptions flottantes levées à tort (77, 78, 79, 80)

- **77.** **`pow` lève l'exception d'opération invalide près du dépassement de
  capacité** : `gaol::pow([2], [1023.9999])` a les bornes les plus serrées,
  mais lève `FE_INVALID`, et un programme qui l'a activée
  (`feenableexcept(FE_INVALID)`) meurt sur SIGFPE. De même `pow(I, d)`, les
  puissances entières au-delà des `int` (`gaol_pown_ll()`, par exemple
  `pow([0x1.000003f82a1fp+0], 3000000001LL)`), `pow_rel` et
  `pow_exponent_rel` (environ 0,5 % de tirages au hasard), et
  `textToInterval("pow([2,2],1023.9999)")`, dans les deux espaces de noms et
  avec tous les builds. La cause est dans CORE-MATH : `exp_1()`
  (`3rd/math-core/src/binary64/pow/pow.c`, l. 1037-1053) écarte
  rh > RHO2 = 0x1.62e42e709a95bp+9, mais son k atteint 2^22 dès
  rh ≥ 709,7826283, et M = 2047 donne 2^1024 = +∞ : la première phase de
  `cr_pow` calcule alors ∞ − ∞ (l. 1780), avant que la seconde rende la bonne
  valeur. Les puissances entre 1,797541e308 et 1,797611e308 environ sont
  touchées. Le `pow.c` de la branche `master` de CORE-MATH a encore ce code le
  14 octobre. Décidé le 14 octobre : le correctif dans CORE-MATH, passer à
  la seconde phase quand M atteint 0x7ff (ou abaisser RHO2), ligne marquée
  `/* GAOL */` et décrite dans `3rd/README.md` comme correctif à proposer en
  amont (#65), et `pow([2], [1023.9999])` dans `extreme_powers[]` de
  `tests/rounding_direction.cpp`.
- **78.** **`cancel_minus` et `cancel_plus` lèvent `FE_INVALID` quand la
  différence des bornes déborde** : `cancel_minus([DBL_MAX], [-DBL_MAX])`,
  l'exemple de la note de 12.12.5 d'IEEE 1788-2015, rend bien [DBL_MAX, +∞],
  mais `two_sum()` (`gaol/gaol_interval.cpp`, l. 4178) calcule ∞ − ∞ avant
  de traiter la somme infinie ; de même `cancel_plus([DBL_MAX], [DBL_MAX])`.
  Le test de `tests/rounding_direction.cpp` (l. 2085) ne prend que
  [DBL_MAX] − [1], qui ne déborde pas. Correction : e = 0 quand la somme est
  infinie, avant le calcul de ses termes, et [M] − [−M] dans le test.
- **79.** **`FE_OVERFLOW` levé pour des résultats sans borne infinie** :
  `sqrt([1, DBL_MAX])` (le produit de vérification u·u = 2^1024),
  `nth_root([DBL_MAX], 3)`, `sin([-1, DBL_MAX])`, `tan([DBL_MAX])`,
  `div_rel([1], [2^-1074, 1], [1, 2])`, `sqrt_rel([DBL_MAX], [-3, 0])`, et
  `cos`, `nth_root_rel`, `atan_rel`, `atan2_rel_x` et `atan2_rel_y` dans des
  tirages au hasard. `doc/using.md` (l. 1170-1172) et le manuel
  (l. 1361-1363) disent que `FE_OVERFLOW` ou `FE_DIVBYZERO` signalent une
  borne infinie de l'intervalle. Décidé le 14 octobre : corriger le code, et
  non les textes ; mettre à l'échelle les vérifications de `sqrt` et des
  racines, et chercher les autres débordements intermédiaires (`sin`, `cos`,
  `tan`, `div_rel`, les fonctions `*_rel`), sans calcul qui puisse déborder
  pour un résultat fini.
- **80.** **`1/[0, 1]` ne lève plus la division par zéro depuis #121**, mais
  `doc/using.md` (l. 1077-1078) et le manuel (l. 1252-1254) le donnent encore
  en exemple avec les intervalles SSE2, contre `doc/using.md`
  (l. 1153-1159) et `doc/tests.md` (l. 186-190). Correction : garder
  `log([0, 1])` seul dans l'exemple.

### AC. `nth_root` d'un sous-normal et réciproques de `pow` (81, 82)

- **81.** **`nth_root(x, n)` et `rootn` d'un x sous-normal donnent des bornes
  très loin des plus serrées** : `nth_root([2^-1074], 33)` est
  [0x1.5ecf527769956p-33, 0x1.5ecf527771187p-33], et le plus serré
  [0x1.5ecf52776a115p-33, 0x1.5ecf52776a116p-33] (mpmath à 600 bits) : 1 983
  doubles sous la borne inférieure, 28 785 au-dessus de la supérieure. On
  trouve 6,9e8 doubles pour n = 65, 5,9e9 pour n = 2^20 + 1, 68 739 pour
  x = 2^-1060 et n = 65, et de même avec `rootn(x, q)` pour q < 0.
  L'encadrement reste juste ; seules les racines d'ordre n ≥ 4 d'un
  sous-normal sont touchées, et les puissances exactes restent exactes.
  `doc/accuracy.md` (l. 129, « the tightest or one double beyond, whatever the
  magnitude of x »), le commentaire de `newton_root()` et
  `doc/differences.md` (l. 502) promettent le contraire. Cause : les preuves
  (`ipow_hi()` et `ipow_lo()`, vers la l. 2911 de `gaol/gaol_interval.cpp`,
  appelées par `is_proved_below_root()` et `is_proved_above_root()`)
  calculent par exponentiation binaire une puissance intermédiaire
  sous-normale : pour n = 33 et x = 2^-1074, l^32 ≈ 2^-1040 n'a plus
  qu'environ 34 bits. Correction : prouver avec une puissance mise à
  l'échelle, qui ne devient jamais sous-normale, par exemple celle de
  `pown_point()`, qui ramène déjà c à [1, 2) en gardant l'exposant à part ; ou
  corriger les textes.
- **82.** **`pow_rel` et `pow_exponent_rel` (`powRev1`, `powRev2`) n'ont pas
  la garde de `[±oo]` de `pow`** : `powRev1([+oo], [2], R)` rend
  [1, 1 + 2^-52], `powRev2([+oo], [2], R)` [0, 2^-1074],
  `powRev1([2], [+oo], R)` [DBL_MAX, +∞] et `powRev1([-oo], [0, 1], R)`
  [1, +∞], là où `pow(x, [±oo])` est vide, et ces réciproques avec lui. Ces
  résultats restent des encadrements, et ces arguments ne se construisent
  qu'avec `interval(__m128d)`, dans les builds SSE2. Correction : le test
  `!(l < +oo && u > -oo)` de `pow_standard_upward()` dans
  `pow_rel_upward()` et `pow_exponent_rel_upward()`.

### AD. La lecture et l'écriture des intervalles (83 à 89)

- **83.** **Un exposant entier hors des `int` est refusé dans un texte** :
  `gaol::textToInterval("nth_root(8,3000000000)")` lève
  `invalid_action_error` (« nth_root used with non integral 2nd arg. »), et
  `gaol_ieee1788::textToInterval` rend l'ensemble vide pour
  `"rootn(8,3000000000)"` et `"pown([2,3],3000000000)"`, alors que
  `nth_root(8, 3000000000LL)` et `pow([2, 3], 3000000000LL)` sont calculés
  depuis #121 ([1.000000000693146, 1.000000000693148] et [DBL_MAX, +∞]).
  Cause : `gaol_call` (`gaol/gaol_interval_parser.ypp`, l. 567-571) passe
  par `is_an_int()` (`gaol/gaol_interval.h`, l. 1215), qui exige un `int`.
  Correction : les surcharges `long long` pour un entier dégénéré hors des
  `int` (en saturant comme `integer_as_long_long()`), ou la limite dite dans
  `gaol/gaol_parser.h` et le manuel (l. 4295 et 4932).
- **84.** **`[l, u]` avec l > u est lu comme un intervalle non vide** quand
  les deux nombres s'arrondissent au même double ou se chevauchent :
  `[9007199254740993, 9007199254740992]` donne [2^53], qui ne contient pas
  2^53 + 1 ; `[0x1.999999999999ap-4, 0.1]` donne [0x1.999999999999ap-4], qui
  ne contient pas 0,1 ; `[1.00000000000000001, 1]` donne [1] ; dans les deux
  espaces de noms, alors que `[2, 1]` et `interval(2^53 + 1, 2^53)` sont
  vides. IEEE 1788-2015 (9.7.4 a et 12.12.7) veut l ≤ u, le vide sinon, et
  `doc/accuracy.md` (l. 196) promet le plus serré pour ces littéraux.
  Correction (`gaol_literal`, `gaol/gaol_interval_parser.ypp`, l. 685) :
  comparer les valeurs exactes des deux nombres quand leurs encadrements se
  touchent, et rendre le vide si l > u. Décidé le 14 octobre : le vide dans
  les deux espaces de noms, `gaol::textToInterval` compris, dont le manuel
  décrit aujourd'hui la lecture borne par borne (à mettre à jour).
- **85.** **Le plafond de l'exposant déborde un `long` de 32 bits** (Windows,
  i386, armhf) : `k`, `k2`, `kb`, `exponent` et `q` sont des `long`
  (`gaol/gaol_interval_lexer.lpp`, l. 282, 324 et 634), mais le plafond
  dépasse 2^31 − 1 pour un lexème de plus de 536 870 411 caractères, en deçà
  des 2^29 caractères au-delà desquels le lexeur lève `std::bad_alloc` :
  `"1e"` suivi de 536,8 millions de 9 serait lu [0, 2^-1074] au lieu de
  [DBL_MAX, +∞]. Non vérifié, faute de chaîne 32 bits sur la machine.
  Correction : ces variables en `long long`.
- **86.** **`_iv` d'un entier hexadécimal, octal ou binaire au-delà de 2^64
  n'est pas le plus serré** : `0xFFFFFFFFFFFFFFFFF_iv` (2^68 − 1) donne
  [0x1.fffffffffffffp+67, 0x1.0000000000003p+68], le plus serré finissant à
  0x1p+68 ; un binaire de 65 uns finit 11 doubles trop haut, l'octal 8^25 − 1
  à 0x1.0000000000007p+75. `doc/using.md` (l. 488-490), le commentaire de
  `gaol/gaol_literals.h` et le `\brief` d'`operator""_iv` disent
  « tightest ». Cause : les chiffres sont ajoutés un à un en arithmétique
  d'intervalles (l. 83). Correction : écrire les chiffres en texte
  hexadécimal flottant (`"0x…p0"`, les chiffres octaux ou binaires
  regroupés), que `textToInterval` lit exactement.
- **87.** **Un nombre incertain juste après un `+` ou un `-` binaire est
  refusé** : `10 - 3.56?1` est lu, mais `10-3.56?1`, `10+3.56?1` et
  `[1,2]+3?` lèvent `input_format_error` (le vide avec `gaol_ieee1788`), le
  lexeur prenant `-3.56?1` pour un seul lexème (l. 749), alors que le manuel
  dit que ces littéraux se combinent aux opérateurs dans n'importe quel
  ordre. À décider : le documenter, ou sortir le signe du lexème en le
  gardant pour les directions `u` et `d` (`-10?u` est [−10, −9,5]).
- **88.** **Lire un long nombre prend un temps quadratique en ses chiffres** :
  `"0."` suivi de n chiffres 3 prend 0,026 s pour n = 20 000, 0,37 s pour
  80 000 et 4,4 s pour 320 000 (environ 45 s attendues pour un million), et
  la forme incertaine 2,3 fois plus (`gaol_take_apart`, l. 298 du lexeur, un
  chiffre à la fois, et `gaol_natural_mul_pow5`). Un texte venu d'ailleurs
  peut ainsi bloquer un programme. Correction : garder environ 770 chiffres
  significatifs et un chiffre non nul « collant », aucun double ne séparant
  alors les deux valeurs, et convertir 9 chiffres par multiplication.
- **89.** **Le format `agreeing` écrit un intervalle ponctuel comme un nombre
  nu** : `interval(4.0)` s'écrit `4.000000000000000`, quand le format
  `bounds` écrit `[4]` ; ni le commentaire d'`interval_format`
  (`gaol/gaol_interval.h`) ni le manuel (l. 4457) ne le disent. À
  documenter.

### AE. Les trois builds (90 à 95)

- **90.** **autotools et meson ne vérifient que le compilateur C++ pour Clang
  sur ARM 32 bits** (`configure.ac`, l. 228-233 ; `meson.build`,
  l. 214-219), quand CMake vérifie aussi le compilateur C
  (`CMakeLists.txt`, l. 220). Or CORE-MATH est compilé en C (dans 3rd/,
  `s_nextafter.c` étant parti), et le `#error` de `gaol/gaol_config.h`
  n'atteint que le C++ : avec `CC=clang CXX=g++` sur armhf, configure et
  meson accepteraient, et CORE-MATH, compilé par un Clang qui ignore le sens
  d'arrondi sur cette cible, donnerait des bornes fausses. La vérification de
  `-frounding-math` ne regarde que le compilateur C++ dans les trois builds
  (`CMakeLists.txt`, l. 360 compris). Non vérifiable ici, faute de chaîne
  ARM. Correction : les deux vérifications avec le compilateur C aussi.
- **91.** **`tests/refused_options.sh` se dit sauté quand le programme ne
  compile pas** : il reconnaît GCC ou Clang en prétraitant le source du test
  avec `-dM -E` (l. 21) ; si les en-têtes de GAOL manquent, g++ n'écrit rien,
  et le script dit « SKIP: neither GCC nor Clang » (77) au lieu d'échouer
  (vérifié avec `GAOL_CXXFLAGS="-std=c++17 -frounding-math"`). Correction :
  `$GAOL_CXX $GAOL_CXXFLAGS -dM -E -x c++ /dev/null`.
- **92.** **Les sources C n'ont pas les mêmes drapeaux dans les trois
  builds** : CMake leur donne `-msse2 -msse3 -fvisibility=hidden`
  (`CMakeLists.txt`, l. 757), autotools et meson au C++ seulement
  (`configure.ac`, l. 304, 310 et 559 ; `meson.build`, l. 273, 326 et 330) ;
  `-ffloat-store` va au C et au C++ avec CMake et meson, au C++ seul avec
  autotools (l. 574) ; le C est gnu99 avec CMake, c99 ou gnu11 avec meson,
  celui du compilateur avec autotools. Effet visible : les bibliothèques
  partagées d'autotools et de meson exportent `get_inexact` et
  `clear_inexact`, pas celle de CMake. `.github/audit` ne compare que les
  drapeaux de `gaol_interval.cpp`. Correction : les mêmes drapeaux C
  partout, et un source C dans `probe_build.py`.
- **93.** **Du code mort exporté sous des noms C génériques** :
  **décidé et fait le 15 octobre**, retirés la famille `gaol_exact`
  et `s_nextafter.c`, dont `nm -D` montrait les trois symboles
  (`get_inexact`, `clear_inexact`, `s_nextafter`) exportés par les
  bibliothèques partagées d'autotools et meson, pas celle de CMake
  (point 92). Aucun appelant : `get_inexact()` et `clear_inexact()`
  depuis la réécriture v5 du lecteur de nombres (commit `73af2fe`, le
  nombre est pris par ses bits) et la suppression de gdtoa (commit
  `520450f`), qui les employaient (strtod arrondi bas puis haut, le
  nombre exact distingué par le drapeau inexact) ; `s_nextafter()`
  depuis que le `#define nextafter s_nextafter` de la v4 est parti
  (son commentaire le disait). Supprimés : `gaol/gaol_exact.{h,c}`,
  `gaol/sysdeps/gaol_exact_{c99,msvc}.h` (tout `gaol/sysdeps/`
  disparaît ; `gaol_exact_msvc.h` était mort de naissance, inclus
  seulement par `gaol_exact.c` sous `_MSC_VER`, jamais compilé pour
  MSVC) et `gaol/s_nextafter.c`. Retirés aussi : l'include vestigial
  du lexeur (`gaol/parser/gaol_interval_lexer.lpp:45`, .cpp régénéré,
  le diff n'étant que la ligne et les `#line`), `CMakeLists.txt`
  (GAOL_SOURCES et 2 commentaires), `gaol/Makefile.am` (+ `Makefile.in`
  régénéré, qui perd ses règles C : GAOL n'a plus de source C,
  CORE-MATH de 3rd/ reste le seul), `gaol/meson.build` (gaol_SOURCES,
  EXTRA_DIST, `incdir` qui pointait sur `sysdeps/`), `configure.ac`,
  `meson.build` et `configure` (commentaires), `doc/using.md`,
  `doc/differences.md` (2 endroits), `coverage/README.md`,
  `process.md`, et les listes de « pas installés » de
  `.github/workflows/linux.yml` et `build-systems.yml`. La détection
  `GAOL_HAVE_FENV_H` reste (`gaol_fpu.h` l'utilise). `nm`/`nm -D` sur
  les bibliothèques des trois builds : les trois symboles ont disparu,
  de la statique comme de la partagée. Revalidé le 15 octobre :
  CMake 57/57, autotools 29 passed + 2 sautés et 16 exemples, meson
  29 + 2 sautés, sous-projet meson 8/8, find_package 6/6,
  `make distclean` rend l'arbre, `<gaol/gaol>` seul + `textToInterval`
  OK sur les installations CMake statique et partagée. Vérifié en
  même temps, sur la question du mainteneur sur `gaol_flags.h` : il ne
  déclarait pas des drapeaux IEEE mais `prec_t`, les priorités
  d'opérateurs, qu'emploie `gaol/parser/gaol_expression.cpp` (les
  deux autres mentions de `prec_t` sont un commentaire de
  `gaol/gaol_interval.h` et le test négatif `tests/misc.cpp`, qui
  vérifie que `gaol::prec_t` n'existe pas : il passe). **Décidé et
  fait le 15 octobre** : `prec_t` est défini dans
  `gaol/parser/gaol_expression.h`, qui n'incluait `gaol_flags.h` que
  pour cela, et `gaol_flags.h` — un en-tête installé — est supprimé
  des sources et des trois listes d'installation ; les en-têtes
  installés compilent seuls (`-Wall -Wextra -Wold-style-cast -Wundef`,
  GCC et Clang 18, sauf `gaol_interval_fpu.h` et `gaol_interval_sse.h`,
  qui ne compilent pas seuls par conception), et une expression
  construite avec les en-têtes installés a la priorité
  `prec_t::plus_prec`. `coverage/coverage.html`, rapport ligne à
  ligne, cite encore `sysdeps/gaol_exact_c99.h` et `s_nextafter.c` :
  périmé jusqu'à la cible `coverage` (CC.m3).
- **94.** **Les listes de macros de l'audit ne s'accordent pas** :
  `.github/audit/compare.py` (l. 25) compare `GAOL_HAVE_AVX512_TARGET` et
  `GAOL_PREFER_AVX512`, que `make_probe.py` (l. 9-12) ne sonde pas, si bien
  que la comparaison ne peut jamais différer, et `GAOL_RND_SSE_REGISTER` est
  sondé sans être comparé. Correction : accorder les deux listes.
- **95.** **Les détections compilent dans un build et lient dans les
  autres** : la sonde d'AVX-512 lie avec CMake (l. 663), mais ne fait que
  compiler avec autotools (l. 351) et meson (l. 356), alors que
  `__builtin_cpu_supports` demande `__cpu_model` à l'édition de liens ; la
  sonde de FMA, à l'inverse, ne vérifie que le C++ avec CMake, et le C et le
  C++ avec édition de liens dans les deux autres. Non vérifié sur une chaîne
  où le résultat changerait. Correction : lier, et vérifier les deux
  langages, dans les trois builds.

### AF. Des textes que le code contredit (96 à 103)

À faire avec le point I, pour ne régénérer le manuel qu'une fois.

- **96.** **« tightest » sans la limite acceptée au point B** : le
  commentaire de `gaol_pown()` (`gaol/gaol_interval.h`, l. 1436-1440) ; le
  manuel (l. 1494, « Both give the tightest bounds », et l. 5039) ;
  `doc/accuracy.md`, l. 96, colonne Tightness de `pow(x, y)`, « for any y »
  (`gaol_ieee1788::pow` d'un exposant entier dégénéré passe par `pown`, et
  `gaol_ieee1788::pow([0x1.fffffffffffffp+1], [7])` a sa borne inférieure un
  double sous la plus serrée), et l. 95, colonne Tests, « for all the
  doubles », quand `tests/arithmetic.cpp` en tire 5 000 ; `doc/using.md`,
  l. 346-350 (au-delà de 2^53, pour un n qui n'est pas un double, les bornes
  sont à trois doubles au plus). Y ajouter « or one double beyond where x^n
  is within 2K of a double », et les trois doubles au-delà de 2^53.
- **97.** **`doc/accuracy.md`, l. 180, dit `mulRevToPair` absent**, alors
  qu'il est fourni depuis le point P (`gaol/gaol_ieee1788.h`, l. 482 ;
  `doc/using.md`, l. 411 ; 172 cas d'ITF1788 réussis) ; et l. 172, `pownRev`
  est absent « for p < 0 », quand le code et `doc/compare/itf1788.md` disent
  p ≤ 0.
- **98.** **`doc/accuracy.md`, l. 95 : `[|x|^−n, +∞]`** où x contient 0 pour
  un n < 0 pair : le signe de l'exposant est faux (`pow([-3, 2], -2)` est
  [1/9, +∞], soit 3^-2) ; écrire `[mag(x)^n, +∞]`.
- **99.** **`doc/tests.md`, l. 37 : « 23 powers »** à une borne 0 ou infinie
  ou au-delà des doubles, quand `powers_at_special_values()` de
  `tests/arithmetic.cpp` en a 30 (24, et 6 coins calculés seuls).
- **100.** **`doc/building.md`, l. 358 : `GAOL_USING_ASM`** serait
  l'assembleur du « 32-bit x86 Linux and macOS » ; il vaut 1 avec `GAOL_ASM`
  hors de Visual C++, et le `f_negate` x87 de `gaol/gaol_fpu_fenv.h`
  (l. 256-262) sert aussi en x86-64, où `GAOL_IX86_LINUX` est défini
  (`gaol/gaol_config.h`, l. 100).
- **101.** **`doc/three-builds.md`, l. 44-47 et 97-110** : `pow(x, 3)` en
  14,5 ns, 21,1 ns sans FMA, et le gain de 1,5 fois de FMA, mesurés avant le
  point B (`doc/compare/performance.md` donne 6,0 ns). À remesurer avec les
  temps de Y (33), ou à dater.
- **102.** **« keeps its names and its results »** (`README.md`, l. 61 ;
  `doc/using.md`, l. 250 ; le manuel) : les mêmes documents listent des
  résultats changés depuis GAOL 4 : `pow(I, 2.5)` tronquait l'exposant,
  `pow([-4, -1], [0.5])` était [−1, 2], et un exposant entier au-delà des
  `int` donnait [−∞, +∞]. Écrire par exemple « keeps its names and the
  meaning of its pow ».
- **103.** **Les chemins AVX-512 et SSE2 diffèrent par le signe d'une borne
  nulle** : pour x = [+0, −0] et un double d < 0, `x * d` donne [−0, +0] avec
  AVX-512 et [+0, −0] avec SSE2 (`gaol/gaol_interval_sse.cpp`, l. 668), la
  seule différence sur 12,2 millions de paires, alors que
  `gaol/gaol_interval_avx512.cpp` (l. 14-18) et `doc/using.md` (l. 918)
  promettent les mêmes bornes « bit for bit », signes compris. Vérifié en
  mettant à vrai `gaol_core::avx512_arithmetic`, un symbole exporté, que
  #122 a retiré : le chemin se décide à la configuration
  (`GAOL_AVX512_ARITHMETIC`). Pour le vérifier, construire avec
  `-DGAOL_PREFER_AVX512=ON` sur une machine qui a AVX-512 ; les lignes et
  les commentaires cités ont changé avec #122. Correction : multiplier
  voie par voie dans la branche AVX-512 d'`operator*=(double)`, comme le
  fait SSE2, ou dire l'exception.

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

### AH. Les sauts et les frontières de 32 octets (105)

- **105.** **Étudier `-mbranches-within-32B-boundaries` pour les builds de
  GAOL**, décidé le 14 octobre à la fin du point AA, sur le Xeon E3-1220 v6
  et sur l'i7-1185G7. L'erratum des sauts des processeurs Skylake à Comet
  Lake y déplace une opération jusqu'à 1 ns selon le seul placement du code
  (#122 : `x * y`, au code inchangé, passe de 4,96 à 4,69 ns ; `[1] / p`, de
  7,0 à 8,1 ns quand seul `sqr()` change). À mesurer : le gain sur le Xeon, le
  coût sur l'i7, qui n'a pas l'erratum, et ce que les trois builds peuvent
  donner aux compilateurs qui prennent l'option.

### AI. La vitesse sur macOS, Windows et ARM (106 à 112)

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
  aucune cible.

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
    écritures de MXCSR, jusqu'à 27 % de plus que sans (`exp`) (CORE-MATH
    calcule avec le sens matériel, aucun arrondi embarqué n'y est possible) ;
    un mode de CORE-MATH où le sens serait un paramètre les supprimerait, au
    prix de modifier `3rd/math-core` ;
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
    `7716fec`, et aucun test ne le voit, qui comparent avec `==`.

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

## Relecture du 14 octobre 2026 (branche `todo-b-pown`, #121)

Relecture de la branche `todo-b-pown` (troisième pull request du point B, #121),
faite le 14 octobre 2026. La branche est à `7f31f6b`, 20 commits au-dessus de
`origin/configure-clean` à `88a0436`, 41 fichiers modifiés.

### Vérifié

- **Compilation** : CMake SSE2 avec GCC 15.2, tests et exemples, réussi ;
  `-Wall -Wextra -Werror` avec GCC 15.2, réussi.
- **Tests** : 57 sur 57 réussis (`ctest -j4`), `intervalf` et `interval2f`
  sautés par construction.
- **Commits** : auteur et committer `Jordan08 <jordan.ninin@gmail.com>`, pas de
  `Co-Authored-By`, pas de mention de Claude, sujets en anglais.
- **Trois builds** : pas de nouveau fichier source, `CMakeLists.txt`,
  `Makefile.am`, `meson.build` inchangés, pas à mettre à jour.
- **Documentation** : `README.md`, `doc/using.md`, `doc/accuracy.md`,
  `doc/tests.md`, `manual/v5/gaol.tex` mis à jour, cohérents avec le code.
- **Tests** : `tests/arithmetic.cpp`, `tests/ieee1788.cpp`,
  `tests/elementary.cpp`, `tests/expressions.cpp`, `tests/rounding_direction.cpp`,
  `tests/misc.cpp` mis à jour, avec des tests de non-régression qui ont des
  dents (mutants dans `tests/tools/pow/mutate.py`, 79 mutants).

### À faire

- **B.121.1** : merger `origin/configure-clean` dans `todo-b-pown`. Le commit
  `4d8942a` (« TODO.md: point B, the decisions of 14 October at the end of
  #121 ») a été poussé sur `origin/configure-clean` le 14 octobre à 11:00, après
  le dernier merge de la branche (`9a2dcef`, 14 octobre 00:10). La branche est
  en retard d'un commit sur `TODO.md` : le diff montre une « suppression » de la
  section « Décidé le 14 octobre », qui n'est que ce retard. **Décidé le
  14 octobre : attendre**, le merge se fera plus tard.

- **B.121.2** : deux sujets de commit dépassent 100 caractères (la règle dit
  « sous 100 caractères environ ») :
  - `80e03ea` : « pown after review: x^2 is sqr(x), lanes on |c|, n split
    below its ulp, expressions beyond 2^53 as x^h x^l » (105 caractères) ;
  - `57f2cab` : « doc/compare: GAOL v5 through gaol_ieee1788 in the benchmark,
    besides gaol in the cases and the enclosure » (104 caractères).
  **Décidé le 14 octobre : laisser tels quels** (4 à 5 caractères de trop,
  la règle dit « environ 100 »).

### Non vérifiable ici

- Windows (Visual C++, clang-cl, MinGW-w64, MSYS2), macOS (AppleClang arm64 et
  x86_64), ARM 32 et 64 bits, ppc64le, s390x, riscv64, Alpine : la CI les
  couvre.
- Le test des exposants de plus de 64 bits (`__int128`) ne tourne que dans les
  modes GNU de GCC et Clang : vérifié à la main par l'auteur de la PR, pas dans
  la CI (décision du 14 octobre).
- La matrice complète de `process.md` 6.2 (autotools, meson, FPU, Clang 18,
  sanitizers, préservation du sens d'arrondi, `headers.sh`, manuel) n'a pas été
  relancée ici : elle est documentée dans la PR comme faite par l'auteur à
  `80e03ea`.

### Questions pour le mainteneur

1. **B.121.q1** : faut-il merger `origin/configure-clean` dans `todo-b-pown`
   maintenant (pour récupérer `4d8942a`), ou attendre que la PR soit relue ?
   **Réponse (14 octobre) : attendre.**
2. **B.121.q2** : les deux sujets de commit à 104 et 105 caractères sont-ils
   acceptables (la règle dit « environ 100 »), ou faut-il les raccourcir ?
   **Réponse (14 octobre) : laisser tels quels.**
3. **B.121.q3** : la relecture a porté sur le code à `7f31f6b`. Faut-il que je
   fasse d'autres vérifications (autotools, meson, FPU, Clang 18, sanitizers,
   manuel) avant que tu fusionnes ?
   **Réponse (14 octobre) : non, c'est suffisant.**

## Relecture du 14 octobre 2026 (branche `configure-clean`)

Relecture de `configure-clean` au commit `533992f`, faite le 14 octobre 2026
après la fusion de #120 (point Z) et #121 (point B, 3/3).

### Vérifié (matrice complète de `process.md` 6.2)

- **CMake SSE2 (GCC 15.2)** : compilation réussie, **57/57 tests** réussis.
- **CMake FPU (GCC 15.2, `GAOL_SIMD=OFF`)** : compilation réussie, **57/57
  tests** réussis.
- **CMake Clang 18** : compilation réussie, **57/57 tests** réussis.
- **`-Wall -Wextra -Werror`** (GCC et Clang 18) : compilation réussie sans
  avertissement.
- **Debug ASan + UBSan** (`-fsanitize=address,undefined`) : 10 tests clés
  (arithmetic, ieee1788, elementary, expressions, rounding_direction, misc,
  relations, trigonometric, non_arithmetic, core_math) : **10/10 réussis**.
  Le test `numbers` non lancé (trop long sous les sanitizers).
- **Autotools** : `configure --with-tests --with-examples`, `make -j4`,
  `make check` (**16/16 exemples** réussis), `make install`, `make distclean`
  (rend l'arbre tel que git l'a).
- **Meson 1.10** : `meson setup -Dwith-tests=true -Dwith-examples=true`,
  `meson compile -j4`, `meson test` : **29 réussis, 2 sautés** (`intervalf`,
  `interval2f`), 0 échec.
- **GAOL installé (meson, shared)** : `tests.sh` depuis la racine des sources :
  **5 881 289 vérifications, 0 échec**, avec `pkg-config --cflags --libs gaol`
  aussi.
- **En-têtes sous `-Wundef`** : `headers.sh` sur l'installation (GCC) : chaque
  en-tête installé compile seul avec `-std=c++11 -Wall -Wextra -Wold-style-cast
  -Wundef -Werror` ; 89 macros, toutes en `GAOL_` ou `gaol_`. Le point Z est
  bien fait.
- **Manuel** : `pdflatex` deux fois sur `manual/v5` : **137 pages, 0 erreur**,
  1 Overfull connu (l. 511-515, à corriger au point Y).
- **Scripts** :
  - `pown_bound.py` : 6001 valeurs de k, **0 inégalité échoue**, erreur/K au
    plus 0,727 pour c^m, 0,474 pour c^-m ;
  - `checkrows.py` sur la table de `tests/ieee1788.cpp` (117 boîtes) : **234
    résultats, 0 échec**, 232 à distance 0, 2 à distance 1 (le cas documenté
    `pow([0, 1+2^-52], -2)`) ;
  - `mutate.py --list` : 79 mutants listés, avec les survivants attendus
    marqués.
- **`TODO.md` à jour** : les commits `2a98ecd` (point Z, #120) et `533992f`
  (point B, #121) suivent la procédure de `process.md` section 9 : en-tête au
  commit courant, paragraphe « Fait le 14 octobre », retrait de la section du
  point, mise à jour de « En cours » (qui devient « Aucune »).
- **Issue #49 à jour** : « État au 14 octobre 2026 (commit 533992f de
  `configure-clean`) », #120 et #121 listées dans « Fusionné depuis le
  30 septembre ».
- **Branches supprimées** : `todo-b-pown` et `todo-z-wundef` ne sont plus sur
  GitHub (seules `configure-clean`, `master` et `MATH-CORE` restent).
- **Aucune PR ouverte** vers `configure-clean`.
- **CI GitHub** sur `533992f` : **tous les jobs verts** — Linux, macOS, Windows,
  Linux containers, Autotools and meson : tous `success`.

### Relecture approfondie du code (PRs récentes)

- **#117 (point B, 1/3)** : bien fait. `pown_upward()` sépare le corps de
  `gaol_pown()` après une seule vérification ; `upward_access` donne accès à
  `inverse_upward()` ; `rnd_keep_bounds()` évite la copie octet par octet.
  `nth_root(I, INT_MIN)` gère `|INT_MIN|` sans dépassement signé.
  `acosh_rel()`, `asinh_rel()`, `atanh_rel()` et `modulo_k_pi()` ont une
  seule vérification. Les attributs `always_inline` sont bien placés.
- **#118 (point B, 2/3)** : bien fait. `pow_lo()` et `pow_hi()` gèrent les
  limites (0, +∞) aux coins, ce qui évite d'appeler CORE-MATH avec 0 ou ±∞.
  Un coin par borne pour une base contenant 1. `root_of_large_order()` ni
  exportée ni `static`. `rnd_keep()` d'un intervalle FPU par une amie.
- **#119 (suites de l'audit)** : bien fait. Corrections de commentaires et de
  documentation (chemin AVX-512, ancien 9 dit mesuré, `<a, b>` par les bits).
- **#120 (point Z)** : bien fait. Les 13 options de `gaol_configuration.h`
  définies à 0 ou 1 dans les trois builds ; les macros de la cible définies à
  0 où elles ne l'étaient pas. `headers.sh` vérifie `-Wundef`. `bisect(ratio)`
  par les `quiet_*` de `gaol_detail`.
- **#114 (point F)** : bien fait. `ascii_lower()` remplace `std::tolower()`,
  indépendant de la locale (problème turc documenté et testé). Le lexeur a
  des commentaires améliorés. Les tests sous FZ/DAZ sont en place.
- **#105 (point E)** : bien fait. `eval_stack` a une croissance dynamique
  (heap) avec `std::bad_alloc` en cas de manque de mémoire. Les premiers
  éléments sont dans la pile (pas d'allocation pour les petites expressions).
  Le parser gère les longues sommes, NUL et `<a, b>` par les bits.

### Cohérence documentation/code

- `gaol_pown_ll` : documenté dans `doc/using.md`, déclaré dans
  `gaol_interval.h`, défini dans `gaol_interval.cpp`. OK.
- `K = 0` pour `m = 2` (x^2 = `sqr(x)`) : documenté dans TODO.md, implémenté
  ligne 314 de `gaol_interval.cpp`. OK.
- `ascii_lower` : défini dans `gaol_interval_parser.ypp`, testé dans
  `tests/expressions.cpp` (`names_under_turkish_locale`). OK.
- `eval_stack` : utilisé dans `gaol_expr_eval.h`, `std::bad_alloc` dans
  `gaol_eval_stack.h`. OK.
- `todo-notes/synthese.md` : 11 sections avec « In `configure-clean`:
  **no** » — les textes pour `ChangeLog` et `doc/differences.md` ne sont pas
  encore reportés, ce qui est normal (ils seront repris par la PR de synthèse
  du point Y).

### Points restants (rappel)

- **I** : la documentation pour l'utilisateur (35, 37, 38, 70).
- **W** : décorations (26), reporté (#67).
- **X** : YalAA et VNODE-LP (28).
- **AG** : le banc de `doc/compare` sur l'i7-1185G7 (104).
- **AH** : `-mbranches-within-32B-boundaries` (105).
- **Y** : préparer et publier v5.0.0 (32, 33, 34, 71), en dernier.

### Trouvailles mineures

- **CC.m1** : deux TODO/FIXME anciens dans `gaol/gaol_interval.cpp` (l. 30-31,
  reliques de GAOL 4 : « Rewrite as many functions as possible to use SIMD
  instructions », « Use of f_negate deprecates -ffloat-store? »). À nettoyer
  au point Y (publication v5.0.0), hors sujet pour les points A à Z.
- **CC.m2** : le test `numbers` n'a pas été lancé sous ASan/UBSan (trop long :
  158 s pour `arithmetic` seul sous les sanitizers). Les 10 autres tests clés
  couvrent les chemins modifiés récemment. À relancer complet au point Y si
  besoin.
- **CC.m3** : `coverage/coverage.html` (rapport gcovr commis) n'a pas été
  régénéré après le déplacement du parser dans `gaol/parser/` ni après les
  suppressions du 15 octobre (`gaol_exact.h`, `gaol_exact.c` et `sysdeps/`,
  `gaol_flags.h`, `s_nextafter.c`, `gaol_assert.h`) ni après la scission de
  `gaol_interval.h`/`.cpp` en `gaol_interval_core.h`/`.cpp` et
  `gaol_interval.h`/`.cpp` (15 octobre, commit `2ac9d4e`) : il cite encore
  `sysdeps/gaol_exact_c99.h` et `s_nextafter.c`. Il faudra le refaire tourner
  (`cmake --build <build> --target coverage`) au prochain bilan de
  couverture. Les chemins de `coverage/README.md` ont été mis à jour à la
  main à chaque fois ; après la scission, les compteurs et les numéros de
  ligne de `gaol_interval_core.cpp`/`.h` ne sont plus à jour.
  **Réponse (15 octobre) : non, pas tout de suite, le coverage sera généré
  plus tard.**
- **CC.m4** : avertissement `-Wconversion` de `gaol/gaol_profile.cpp:52` sur
  armhf, vu dans les journaux de la CI de #123 (qui le laisse tel quel, sans
  rapport avec sa branche) : `tv_sec` y est un `__time64_t`, et
  `tv_sec*1000` vers `long` tronque à 32 bits là où `long` en fait 32
  (dépassement après environ 24,8 jours). `gaol_profile.h` est installé,
  utilisé par l'exemple 16, testé par `tests/misc.cpp`, documenté (section
  « Debugging facilities » du manuel). **Réponse (15 octobre, CC.q6) :
  corrigé avec un cast explicite** (`return long(...)`, détail dans la
  section `gaol/float/` ci-dessous). **Caduc (15 octobre, CC.q9) :
  `gaol_profile.h` et `gaol_profile.cpp` sont supprimés** (section Ménage
  ci-dessous).
- **CC.m5** : `process.md` section 12 (« Où en est le travail ») date du
  13 octobre : B y est « en cours » (fait par #121) et Z « à faire » (fait
  par #120) ; AA est fait (#122) et #123 est fusionnée. À remettre à jour.
- **CC.m6** : `gaol/gaol_generator_mul_i2f.py` est le seul fichier de `gaol/`
  que rien ne référence (hors `doc/compare/`) : c'est un outil de l'auteur
  (Goualard, 2010, « No build runs it: it is a tool of the author »), qui
  écrit les 256 cas de la multiplication de `gaol_interval2f.cpp`.
  **Résolu (15 octobre, CC.q5) : déplacé dans `gaol/float/`** (section
  ci-dessous).

### Questions pour le mainteneur

1. **CC.q1** : la CI sur `configure-clean` (`533992f`) est en cours. Faut-il
   attendre qu'elle soit verte avant de commencer le point I ?
   **Réponse (14 octobre) : non, commencer le point I.**
2. **CC.q2** : le point I (documentation utilisateur) est le prochain selon
   l'ordre proposé. Faut-il le commencer maintenant, ou attends-tu d'autres
   relectures (W, X, AA) ?
   **Réponse (14 octobre) : non, faire juste une relecture complète, lancer
   tous les tests et les scripts.**
3. **CC.q3** : les 12 issues ouvertes (#64 à #70, #80, #101, #102, #104, #107,
   #49) : faut-il en traiter certaines avant le point Y (publication v5.0.0) ?
   **Réponse (14 octobre) : on verra ça plus tard.**
4. **CC.q4** : le déplacement du parser dans `gaol/parser/` (15 octobre)
   installe les 5 en-têtes publics (`gaol_expression.h`, `gaol_expr_eval.h`,
   `gaol_expr_visitor.h`, `gaol_eval_stack.h`, `gaol_parser.h`) dans
   `include/gaol/parser/` : un programme inclut désormais
   `gaol/parser/gaol_expression.h` et non plus `gaol/gaol_expression.h`.
   C'est une rupture de l'interface pour les programmes GAOL v5 écrits avant
   (v5 n'est pas encore publié). Faut-il garder ce sous-répertoire dans le
   chemin d'installation, ou installer les en-têtes directement dans
   `include/gaol/` (les sources restant dans `gaol/parser/`) ?
   **Réponse (15 octobre) : aucun problème, on garde `gaol/parser/`, tant
   pis pour la rétrocompatibilité. En pratique, `gaol_expression` est utilisé
   par le parser principalement ; l'objectif de GAOL n'est pas de fournir des
   expressions mais des intervalles, ça va du moment que `textToInterval`
   fonctionne à l'installation. Vérifié : un programme qui n'inclut que
   `<gaol/gaol>` (le parapluie n'inclut pas `gaol/parser/gaol_expression.h`)
   appelle `gaol::textToInterval("[1, 2]")`, `gaol::textToInterval(
   "pow([-4, -1], 2)")` (à travers le parser) et
   `gaol_ieee1788::textToInterval("[3, 4]")` ; compilé et lié avec les GAOL
   installés des trois builds (meson, CMake, autotools), en shared et en
   static : `[1, 2]`, `[1, 16]` et `[3, 4]`, comme `doc/using.md` le
   documente.**
5. **CC.q5** : `gaol/gaol_generator_mul_i2f.py` (CC.m6), outil de l'auteur
   de 2010 que rien ne référence : le garder tel quel dans `gaol/`, ou le
   déplacer dans `tests/tools/` avec les autres outils ?
   **Réponse (15 octobre) : déplacer ce qui concerne les intervalles de
   floats dans `gaol/float/` : `gaol_intervalf.*`, `gaol_interval2f.*` et
   `gaol_generator_mul_i2f.py` — fait (section ci-dessous).**
6. **CC.q6** : l'avertissement `-Wconversion` de `gaol/gaol_profile.cpp:52`
   sur armhf (CC.m4) : corriger (`long` en un type de 64 bits dans
   `gaol_profile.h` et `gaol_profile.cpp`, ou un cast explicite), ou laisser
   tel quel ?
   **Réponse (15 octobre) : corrigé avec un cast explicite — fait (section
   ci-dessous).** **Caduc (15 octobre, CC.q9) : `gaol_profile.cpp` est
   supprimé** (section Ménage ci-dessous).
7. **CC.q7** : l'issue #49 en est à « État au 14 octobre 2026 (commit
   62dad3a) » : faut-il la mettre à jour pour le 15 octobre (déplacement du
   parser, suppressions de `gaol_exact.h`, `gaol_flags.h`, `s_nextafter.c`
   et `gaol_assert.h`) ?
   **Réponse (15 octobre) : oui — fait (l'issue #49 est à jour au
   15 octobre).**
8. **CC.q8** : `gaol/dllmain.cpp` sert-il encore ? Non : aucun des trois
   builds ne le compile (`CMakeLists.txt` et `gaol/Makefile.am` ne le
   listent pas ; `gaol/meson.build` seulement dans `EXTRA_DIST`, qui ne
   compile pas), et il ne serait utile que pour une DLL Visual C++, or
   la bibliothèque est toujours statique avec MSVC (`CMakeLists.txt:769`,
   `gaol/meson.build` ; `GAOL_PUBLIC` y est vide, pas de
   `__declspec(dllexport)`), donc `DllMain` ne serait jamais appelé.
   L'initialisation et le nettoyage automatiques sont faits par les
   objets statiques `gaol_core::gaol_initializer` (`gaol/gaol_common.h`)
   avec tous les compilateurs, MSVC compris (`doc/differences.md`). Il ne
   compilerait même plus : il appelle `gaol::initialization_process()` et
   `gaol::cleanup_process()`, qui n'existent plus dans le namespace `gaol`
   (seulement `gaol_core`, dans `gaol/gaol_init_cleanup.h`, en-tête interne
   non installé). Hors MSVC, le fichier est vide (tout est sous
   `#if defined (_MSC_VER)`). **Réponse (15 octobre) : oui — supprimé**
   (`git rm gaol/dllmain.cpp`, sa ligne de `EXTRA_DIST` retirée de
   `gaol/meson.build`, la phrase de `doc/differences.md` mise à jour ;
   aucun build ne le compilait, meson reconfiguré et testé : 29 OK +
   2 skip).
9. **CC.q9** : `gaol/gaol_profile.h` et `gaol/gaol_profile.cpp` sont-ils
   encore utilisés ? Oui, dans le dépôt : `examples/16_Goldstein_Price.cpp`
   (l. 77, 82 : `reset_time()`, `elapsed_time()` pour le temps affiché),
   `tests/misc.cpp` (l. 55 : `GAOL_NAMES_CLASS(timepiece)`, le nom de GAOL
   4 dans le namespace `gaol`) et le parapluie `gaol/gaol` (l. 37, qui
   l'inclut, et les `using` de `gaol` le nomment). La bibliothèque
   elle-même ne les utilise pas (0.0 % de couverture). **Question
   (15 octobre) : supprimer quand même**, en réécrivant la mesure de
   temps de `16_Goldstein_Price` (par `std::chrono` ?) et en retirant la
   ligne de `tests/misc.cpp` et l'include du parapluie, ou **garder**
   l'API publique (`timepiece`, `get_time`, `reset_time`, `elapsed_time`,
   `intermediate_elapsed_time`, `GAOL_PUBLIC`, installée) ?
   **Réponse (15 octobre) : supprimer — fait** (`git rm` des deux fichiers,
   l'include retiré du parapluie `gaol/gaol`, la mesure de temps de
   l'exemple 16 réécrite avec `std::chrono`, la ligne de `tests/misc.cpp`
   retirée, les trois builds, `doc/building.md`, `coverage/README.md`, le
   chapitre « Profiling » du manuel v5 et les scripts `.github/audit/` mis
   à jour ; détail dans la section Ménage ci-dessous).

### Déplacement du parser et des expressions dans `gaol/parser/` (15 octobre 2026)

Demandé par le mainteneur le 15 octobre : tout ce qui concerne le parser de
GAOL et les expressions dans `gaol/parser/` (exemple donné :
`gaol/gaol_expr_visitor.h`). Fait sur `configure-clean` :

- **13 fichiers déplacés** (`git mv`) : `gaol_expr_visitor.h`,
  `gaol_expression.h`, `gaol_expression.cpp`, `gaol_expr_eval.h`,
  `gaol_eval_stack.h`, `gaol_interval_parser.ypp/.cpp/.h`,
  `gaol_interval_lexer.lpp/.cpp`, `gaol_parser.h`, `gaol_parser.cpp`,
  `regenerate_parser.sh`.
- **Includes et commentaires** mis à jour dans les sources (`gaol_common.cpp`,
  `gaol_interval.cpp`, `gaol_interval.h`, `gaol_common.h`, `gaol_ieee1788.h`,
  `gaol_exceptions.h`, les fichiers de `gaol/parser/` eux-mêmes) et dans les
  tests (`misc.cpp`, `debugging.cpp`, `expressions.cpp`, `ieee1788.cpp`).
- **Les trois builds** : `CMakeLists.txt` (sources, en-têtes installés, cible
  `parser`), `Makefile.am` de `gaol/` et de la racine (sources, en-têtes
  installés, cible `parser`), `Makefile.in` régénérés avec automake 1.18.1 ;
  `meson.build` de `gaol/` et de la racine (sources, en-têtes installés,
  `run_target('parser')`).
- **Installation** : les 5 en-têtes publics dans `include/gaol/parser/`
  (`gaol_interval_parser.h` reste non installé) ; vérifié avec les trois
  builds. Question CC.q4 sur ce chemin, **résolue** (réponse ci-dessous :
  on garde `gaol/parser/`).
- **Fichiers générés** : `gaol_interval_lexer.cpp`,
  `gaol_interval_parser.cpp` et `gaol_interval_parser.h` régénérés par
  `sh gaol/parser/regenerate_parser.sh` (flex 2.6.4, bison 3.8.2) ; la cible
  `parser` des trois builds donne les mêmes fichiers octet pour octet.
- **CI et scripts** : `.github/workflows/linux.yml` et `build-systems.yml`
  (contrôles d'installation), `.github/scripts/headers.sh` (teste chaque
  en-tête de `include/gaol/parser/` seul), `.github/scripts/tests.sh`,
  `tests/tools/pow/mutate.py`, `todo-notes/orchestration/bin/check_branch`,
  `cmake/msvc/unistd.h`.
- **Documentation** : `doc/using.md`, `doc/building.md`,
  `doc/differences.md`, `doc/continuous-integration.md`, `doc/tests.md`,
  `examples/examples.md`, `manual/v5/gaol.tex`, `manual/check_examples.py`,
  `coverage/README.md`, `process.md`.
- **Corrigé en cours de route** : `make distclean` laissait le répertoire
  `gaol/parser/.deps` (les sources du parser étant dans un sous-répertoire,
  `subdir-objects`) ; `distclean-local` de `gaol/Makefile.am` le supprime
  maintenant.

Validé (matrice complète de `process.md` 6.2, 2 cœurs) : CMake SSE2, FPU et
Clang 18, tests et exemples, **57/57 tests** chacun ; `-Wall -Wextra -Werror`
GCC et Clang 18 sans avertissement ; ASan/UBSan (Debug) **41/41 tests** ;
autotools `configure`/`make`/`make install`/`make check` (29 réussis, 2
skippés par construction, 16 exemples), `make distclean` rend l'arbre tel que
git l'a ; meson `compile`/`install`/`test` (29 réussis, 2 skippés) ; le GAOL
installé : `tests.sh` shared et static, `pkg-config`, `headers.sh` (GCC et
Clang, chaque en-tête de `include/gaol/parser/` compilé seul) ; la cible
`parser` des trois builds régénère les trois fichiers octet pour octet ;
`find_package` 6/6 ; sous-projet meson 8/8 ; `check_examples.py` 104/104 ;
l'audit `.github/audit` donne la même configuration aux trois builds ;
`comma-locale.sh` OK ; le manuel (`pdflatex` deux fois) : zéro erreur, pas de
nouvel « Overfull » (le paragraphe flex/bison a été restructuré en bloc
`cmdshell` pour cela).

### Déplacement des intervalles de floats dans `gaol/float/` (15 octobre 2026)

Demandé par le mainteneur le 15 octobre (réponse CC.q5) : ce qui concerne
les intervalles de floats dans `gaol/float/`. Fait sur `configure-clean` :

- **5 fichiers déplacés** (`git mv`) : `gaol_intervalf.h`,
  `gaol_intervalf.cpp`, `gaol_interval2f.h`, `gaol_interval2f.cpp` et
  `gaol_generator_mul_i2f.py` (l'outil de l'auteur de 2010, CC.m6, qui écrit
  les 256 cas de la multiplication de `gaol_interval2f.cpp`).
- **Includes et commentaires** mis à jour : `gaol_interval.cpp` (qui inclut
  les deux `.cpp` sous `GAOL_FLOAT_INTERVALS`), `gaol_config.h` et
  `gaol_port.h` (commentaires), les trois tests (`tests/intervalf.cpp`,
  `tests/interval2f.cpp`, `tests/other_functions.cpp`), et les fichiers
  déplacés eux-mêmes.
- **Les trois builds** : `gaol/Makefile.am` (`noinst_HEADERS`) et
  `Makefile.in` régénéré (automake 1.18.1, le diff n'est que la liste) ;
  `gaol/meson.build` (EXTRA_DIST) ; `CMakeLists.txt` (un commentaire). Aucun
  des cinq fichiers n'est installé ni distribué (le `.py` n'est dans aucun
  EXTRA_DIST), comme avant.
- **Correction en même temps** (CC.q6, CC.m4) : `gaol/gaol_profile.cpp:52`
  retourne `long(...)` explicitement : sur armhf, `tv_sec` est un
  `__time64_t` de 64 bits et `long` en fait 32, la conversion avertissait
  (`-Wconversion`, journaux de la CI de #123). Le comportement est le même.
  Vérifié en simulant la conversion 64 bits vers 32 (`-m32 -Wconversion`) :
  l'avertissement est là sans le cast, absent avec.

Validé (2 cœurs) : CMake Release **57/57** sans avertissement, et un build
avec `-DGAOL_FLOAT_INTERVALS=1` où `intervalf` et `interval2f` ne sont plus
sautés et passent (**57/57**) — le code déplacé compile et tourne ;
autotools `configure`/`make`/`make check` (**29 PASS + 2 SKIP, 16
/exemples**), `make install` (les mêmes **23 en-têtes** que CMake et meson,
aucun des fichiers déplacés), `make distclean` rend l'arbre tel que git l'a ;
meson **29 OK + 2 skip** ; `headers.sh` OK (88 macros) ; `tests.sh` **0
échec** (static et pkg-config).

### Scission de `gaol_interval.h`/`.cpp` (15 octobre 2026)

Demandé par le mainteneur le 15 octobre : renommer `gaol_interval.h` en
`gaol_interval_core.h` et `gaol_interval.cpp` en `gaol_interval_core.cpp`,
et mettre dans un nouveau `gaol_interval.h` tout ce qui concerne le
namespace `gaol`, comme le namespace `gaol_ieee1788` est en entier dans
`gaol_ieee1788.h`. Fait sur `configure-clean` (commit `2ac9d4e`) :

- **`git mv`** : `gaol_interval.h` → `gaol_interval_core.h` (la classe
  `interval`, les fonctions et `gaol_detail` de `gaol_core`, 2645 lignes)
  et `gaol_interval.cpp` → `gaol_interval_core.cpp` (leurs définitions,
  plus celles de `gaol_ieee1788`, 5924 lignes).
- **Nouveau `gaol_interval.h`** (210 lignes) : le commentaire explicatif
  et le namespace `gaol` en entier (les using-déclarations, `pow` inline,
  `textToInterval` déclaré), comme `gaol_ieee1788.h` ; il inclut
  `gaol_interval_core.h`, et les deux sont installés.
- **Nouveau `gaol_interval.cpp`** (91 lignes) : les définitions des deux
  `gaol::textToInterval`, sorties de `gaol_interval_core.cpp`.
- **`text_in_message()`** : le helper du namespace anonyme de
  `gaol_interval.cpp`, utilisé par `operator>>` (gaol_core) et par
  `textToInterval` (gaol), est devenu `gaol_detail::text_in_message()`,
  inline dans `gaol_interval_core.h` ; les deux `.cpp` l'appellent. La
  déclaration `extern` de `gaol_parse_string` est dans les deux `.cpp`
  (définie dans le parser, déclarée dans aucun en-tête).
- **Les trois builds** : `CMakeLists.txt` (`GAOL_SOURCES`, `GAOL_HEADERS`),
  `gaol/Makefile.am` (`libgaol_la_SOURCES`, `library_include_HEADERS`) et
  `Makefile.in` régénéré (automake 1.18.1, le diff n'est que les deux
  listes), `gaol/meson.build` (`gaol_SOURCES`, `gaol_HEADERS`).
- **Commentaires et références** mis à jour là où ils citent les anciens
  chemins : `gaol_ieee1788.h`, `gaol_common.h`, `gaol_fpu.h`,
  `gaol_fpu_fenv.h`, `gaol_fpu_msvc.h`, `gaol_exceptions.h`,
  `gaol_port.h`, `gaol_config.h`, `gaol/parser/gaol_expression.h`,
  `gaol_interval_sse.h` et `gaol_interval_fpu.h` (garde et `#error` :
  ils sont inclus par `gaol_interval_core.cpp`, la garde vérifie
  `GAOL_INTERVAL_CORE_H`), `gaol_interval_sse.cpp`,
  `gaol_interval_fpu.cpp`, `tests/core_math.cpp`, `tests/reverse.cpp`,
  `tests/rounding_direction.cpp`, `tests/static_initialization.cpp`,
  `tests/other_functions.cpp`, `tests/tools/pow/mutate.py` (`FILE`, et le
  mutant de `integer_power` édite `gaol_interval_core.h` maintenant),
  `pown_bound.py`, `README.md`, `doc/accuracy.md`, `doc/differences.md`,
  `doc/tests.md`, `examples/examples.md`,
  `todo-notes/orchestration/points_w4.py` et `coverage/README.md`.
  Les commentaires qui citent le namespace `gaol` gardent
  `gaol/gaol_interval.h` ; `doc/compare/*/work/` n'est pas touché (copies
  figées) ; `doc/differences.md` garde sa phrase sur GAOL 4 (historique).
- **CC.m3** : les chemins de `coverage/README.md` sont à jour
  (`gaol_interval_core.cpp`/`.h`), les compteurs et les numéros de ligne ne
  le sont plus ; le coverage sera régénéré plus tard (réponse du 15
  octobre).

Validé (2 cœurs) : CMake Release **57/57** sans avertissement, et un
build avec `-DGAOL_FLOAT_INTERVALS=1` **57/57** (`intervalf` et
`interval2f` actifs) ; autotools `configure`/`make` (0 avertissement)/
`make check` (**29 PASS + 2 SKIP, 16 exemples**), `make install` (les
mêmes **24 en-têtes** que CMake et meson, les deux `gaol_interval*.h`
compris), `make distclean` rend l'arbre tel que git l'a ; meson **29 OK +
2 skip** ; `headers.sh` sur l'installation (chaque en-tête compile seul,
g++ et clang++-18, hors les trois qui échouent seuls par conception) ;
`tests.sh` **0 échec** (static et pkg-config).

## Ménage

- **Branches à supprimer sur GitHub** : celles d'« En cours », une fois
  fusionnées (les fusionnées, les jetables et `fix-path-core-math` l'ont été le
  3 octobre, celles de C, K, L, J, de `make distclean`, de #77, #78, #79, #81,
  #83, #84, #85, #86, #87, #88, #89, #90, #91, #92, #93, #94, #95, #96, #97,
  #99, #100, #103, #105, #108, #109, #110, #111, #112, #113, #114, #115, #117,
  #118, #119, #120, #121, #122, #123 et #124 après leur fusion, et la jetable de
  #116).
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
- **`GAOL_ASSERT` supprimé** (décidé le 15 octobre, question du mainteneur) :
  les deux usages `GAOL_ASSERT(d >= 0.0)` (`ipow_up()` l. 72 et `ipow_dn()`
  l. 102 de `gaol/gaol_double_op.h`, en-tête interne, non installé) ont été
  remplacés par `assert` classique, avec `#include <cassert>` ; l'en-tête
  installé et documenté `gaol/gaol_assert.h` est supprimé des sources et
  des trois listes d'installation (`CMakeLists.txt`, `gaol/Makefile.am`,
  `gaol/meson.build`), et ses mentions retirées du manuel v5, de
  `doc/using.md`, `doc/building.md`, `tests/debugging.cpp`, des trois
  modèles `gaol_configuration*.h.in` et de `configure.ac`. Sémantique :
  l'ancien `GAOL_ASSERT(a)` valait `assert(a)` avec `GAOL_DEBUGGING` et
  rien sinon ; `assert` classique est actif tant que `NDEBUG` n'est pas
  défini. Équivalent dans les configurations usuelles (Debug : les deux
  actifs, Release : les deux inactifs) ; `GAOL_DEBUGGING` ne gouverne plus
  que `GAOL_DEBUG_VERBOSE` (`gaol/gaol_common.h`). Divergence acceptée :
  `GAOL_DEBUG=ON` en build Release (`NDEBUG` défini, `assert` inactif là où
  `GAOL_ASSERT` était actif). Vérifié sémantiquement : `ipow_dn(-1.0, 2)`
  avorte (SIGABRT, « Assertion `d >= 0.0' failed », `gaol_double_op.h:102`)
  sans `NDEBUG`, n'avorte pas avec `-DNDEBUG`. Les trois systèmes de build
  passent (CMake Release 57/57, CMake Debug compile avec `GAOL_DEBUGGING`
  et sans `NDEBUG`, autotools 29 PASS + 2 SKIP + 16 exemples, meson 29 OK +
  2 skip), l'installation ne contient plus `gaol_assert.h`, les en-têtes
  installés recompilent un par un (g++ et clang++-18, hors les trois
  en-têtes qui échouent seuls par conception), et le manuel v5 est
  reconstruit sans erreur (1 Overfull, comme la baseline).
- **`gaol_profile.h` et `gaol_profile.cpp` supprimés** (décidé le 15
  octobre, CC.q9) : l'en-tête installé déclarait la classe `timepiece` et
  les fonctions `get_time`, `reset_time`, `elapsed_time` et
  `intermediate_elapsed_time` (`GAOL_PUBLIC`), le source les définissait
  avec `getrusage()` ou `clock()` ; la bibliothèque ne les utilisait pas
  (0.0 % de couverture), et le chapitre « Profiling » du manuel v5 les
  documentait. `git rm` des deux fichiers, l'include retiré du parapluie
  `gaol/gaol`, la mesure de temps de `examples/16_Goldstein_Price.cpp`
  réécrite avec `std::chrono::steady_clock` (même sortie « Elapsed time:
  X ms »), la ligne `GAOL_NAMES_CLASS(timepiece)` retirée de
  `tests/misc.cpp`. Les trois builds nettoyés : `gaol_profile` retiré des
  listes de sources et d'en-têtes de `CMakeLists.txt`, `gaol/Makefile.am`
  (`Makefile.in` régénéré, automake 1.18.1, le diff n'est que les deux
  listes) et `gaol/meson.build`, et la macro `GAOL_HAVE_GETRUSAGE` retirée
  des trois modèles `gaol_configuration*.h.in`, de `configure.ac`
  (`configure` régénéré, autoconf 2.72, le diff n'est que le test
  `getrusage`), de `CMakeLists.txt` et de `meson.build` : elle n'avait
  d'autre utilisateur que `gaol_profile.cpp`, et le mainteneur a demandé
  de supprimer de l'API publique. `doc/building.md` (ligne du tableau),
  `.github/audit/compare.py` et `make_probe.py` (listes de macros) mis à
  jour. `coverage/README.md` édité à la main (les deux lignes de
  `gaol_profile.cpp` retirées, les totaux ajustés : 3653 lignes, 439 non
  exécutées, 88.0 %), en attendant la prochaine régénération du coverage.
  Le chapitre « Profiling » du manuel v5 supprimé en entier (les quatre
  fonctions, leur exemple, la classe `timepiece` et son exemple) ; une
  entrée ajoutée à `doc/differences.md`, la phrase historique sur
  `gaol_profile.cpp` déclarant `getrusage()` gardée. `CC.m4` et `CC.q6`
  sont caducs. `manual/v4` non touché (figé).

  Validé (2 cœurs) : CMake Release **57/57** sans avertissement, et
  l'exemple 16 affiche « Elapsed time: 113 ms » ; autotools
  `configure`/`make` (0 avertissement)/`make check` (**29 PASS + 2 SKIP,
  16 exemples**), `make install` (**23 en-têtes**, sans `gaol_profile.h`),
  `make distclean` rend l'arbre tel que git l'a ; meson **29 OK + 2 skip** ;
  `headers.sh` sur l'installation CMake (chaque en-tête compile seul, g++ et
  clang++-18, hors les trois qui échouent seuls par conception) ; `tests.sh`
  **0 échec** (static et pkg-config) ; `check_examples.py` **104/104** ; le
  manuel v5 reconstruit sans erreur (140 pages, 1 Overfull, comme la
  baseline, « Profiling » absent de la table des matières) ; les trois
  installations ont les **mêmes 24 fichiers** (19 dans `gaol/` dont le
  parapluie, 5 dans `gaol/parser/`) et les mêmes définitions actives de
  `gaol_configuration.h`.

## Table des anciens numéros

26 : W ; 28 : X ; 32 : Y ; 33 : Y ; 34 : Y ; 35 : I ; 37 : I ; 38 : I ;
70 : I ; 71 : Y ; 77 : AB ; 78 : AB ; 79 : AB ; 80 : AB ; 81 : AC ; 82 : AC ;
83 : AD ; 84 : AD ; 85 : AD ; 86 : AD ; 87 : AD ; 88 : AD ; 89 : AD ;
90 : AE ; 91 : AE ; 92 : AE ; 93 : AE ; 94 : AE ; 95 : AE ; 96 : AF ;
97 : AF ; 98 : AF ; 99 : AF ; 100 : AF ; 101 : AF ; 102 : AF ; 103 : AF ;
105 : AH ; 106 : AI ; 107 : AI ; 108 : AI ; 109 : AI ;
110 : AI ; 111 : AI ; 112 : AI.

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
   est fait par #111, et B par #117, #118 et #121. Q est fait par #100.
4. **Achever les autres corrections mathématiques : M et R.** Garder les
   mesures et les tests avec les changements de bornes concernés. Faits par #89
   et #92.
5. **Fermer les suites de lecture et de build : F, G, N, O et le reste de A.**
   G est fait par #91, N par #94, O par #95, A par #111 et #113, F par #114.
6. **Finir les exceptions, petites corrections et documentation : S, U, T, V,
   puis I.** Intégrer dans I la décision et la documentation sur la concurrence
   des réglages de format ; coordonner V avec l'avertissement sur les
   comparateurs. S, T, U et V sont faits par #90, #93, #97 et #96, Z par
   #120, AA (Shekel 5) par #122, AG (le banc de `doc/compare`) par #124. AH, sa
   suite, peut se faire à tout moment avant Y, comme AB à AE, trouvés par la
   relecture du 14 octobre ; AF se fait avec I. AI, trouvé par la CI de #124
   (17 octobre), est à ordonner par le mainteneur.
7. **Publier v5.0.0 avec Y**, après fusion des branches restantes, mise à jour
   de la documentation, couverture et mesures de performance ; faire ensuite
   le ménage listé plus haut.
8. **Après la version, reprendre les ajouts de portée plus large : W et X.**
   Ils portent surtout sur de nouvelles fonctionnalités ou de nouveaux
   mécanismes, plutôt que sur les corrections nécessaires à v5.0.0. P est fait
   par #99.
