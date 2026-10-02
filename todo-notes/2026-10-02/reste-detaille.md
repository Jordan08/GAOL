# À faire

Ce qui reste à faire sur GAOL v5, au commit `61c7503` de `configure-clean`.
Les points gardent les numéros de la liste d'origine, pour que les pull
requests, les rapports et l'issue #49 y renvoient encore : un numéro qui
manque est un point fait. Ce qui est fait est dans l'historique des pull
requests fusionnées ; son suivi (avancement, rapports de chaque point,
relectures) est dans la branche `todo-status`, dans son `TODO.md` et dans
`todo-notes/`, d'où viennent les chemins `todo-notes/…` cités ici.
[doc/differences.md](doc/differences.md) et `ChangeLog` seront mis à jour par
la pull request de synthèse (voir « Ménage »).

Les points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md). « Revue n° n » renvoie au
numéro n de sa section 5, dont l'annexe B donne une correction et un test de
régression ; ses numéros encore ouverts sont 2, 7, 16, 17 et 22. Les points
41 à 44 viennent de la vérification du fichier `VERSION.txt`, le 2026-09-28,
et les points 45 à 49 des corrections et des relectures des autres points,
les 30 septembre et 1er octobre.

## En cours

Le travail de ces branches est poussé, mais pas fusionné dans
`configure-clean`. Chacune s'y fusionne sans conflit, sauf mention contraire.

- **3**, `todo-03-pow-exact-corner` (defcd7a) : terminé et validé localement.
  Restent la relecture indépendante, la CI et la pull request. Conflit avec
  la branche du 8 dans `doc/accuracy.md` et `manual/v5/gaol.tex`. Le point 2
  attend sa fusion.
- **4**, `todo-04-ftz-daz` (ec0ea03) : terminé et validé localement.
  Restent la relecture indépendante, la fusion de `configure-clean` (conflit
  dans `tests/gaol_tests.h`), la CI et la pull request. Conflit avec la
  branche du 36 dans `doc/using.md`.
- **8**, `todo-08-pow-large-n` (d217170) : terminé et validé localement.
  Restent la relecture indépendante, la CI et la pull request ; conflit avec
  la branche du 3.
- **12**, `todo-12-long-sums` (553e649) : inachevé (commits « WIP » 35be26e et
  553e649, `configure-clean` fusionné en 7d615d4). Reste à finir le travail,
  à écrire son rapport, la relecture, la CI et la pull request.
- **17**, `todo-17-output-speed` (3b9311c) : inachevé (commit « WIP »
  fe4f555), `configure-clean` fusionné avec le 16. Restent les mesures, la
  vérification du texte écrit, le rapport, la relecture, la CI et la pull
  request.
- **36**, `todo-36-upward-rounding-effects` (1559af5) : inachevé (un commit
  « WIP », fait sur `a2ca992`, non relu ; conflit dans `doc/using.md`).
  Restent le coût de `GAOL_PRESERVE_ROUNDING`, le manuel, TwoProd, la fusion
  de `configure-clean`, la relecture et la pull request.
- **24, armhf**, `fix-24b-armhf-fe-invalid` (375a822) : la correction de
  l'`operator&=` des intervalles FPU sur ARM 32 bits, dont le défaut rend
  rouge la CI armhf de `configure-clean` depuis #50 ; validée sous qemu.
  Restent la relecture indépendante et la pull request.

## Code

1. **Suites du pow de la norme écrit une fois** (#37). `pow_standard()`
   (`gaol/gaol_interval.cpp`) est la seule copie du pow de la table 9.1 ;
   restent les remarques des relectures :
   - `gaol_pow_hybrid()` refait une vérification inutile : sa garde de
     l'exposant `[+oo]`/`[-oo]` est inatteignable (`interval(±oo)` est vide,
     pris d'abord par `is_empty()`) ; dans `pow_standard()`, le cas
     `at_upper == 1.0` du bloc au-delà des int est redondant (`pow(1, n)` de
     CORE-MATH vaut exactement 1). Les retirer reprendrait une part des +2 à
     3 % (2 à 3 ns) de `gaol::pow` à exposant non entier, mesurés avec GCC
     9.4 seulement. Le `is_empty()` de `gaol_pow_hybrid()` est à garder : pour
     un exposant entier au-delà des int, la fonction rend `interval::universe()`
     sans regarder I, et `gaol::pow(interval::emptyset(), interval(1e10))`
     vaudrait [-oo, +oo] sans lui ;
   - le commentaire d'en-tête de `pow_standard()` raconte l'histoire (« This
     was the second half of gaol_pow_hybrid()… ») au lieu de dire ce que
     fait la fonction ; le Doxygen de `gaol_pow_hybrid()`
     (`gaol/gaol_interval.h`) dit que la partie standard est exp(J log(I)),
     alors que les bornes finies viennent des coins du `pow` de CORE-MATH ;
   - `doc/tests.md` (test `ieee1788`) et `tests/ieee1788.cpp` (en-tête,
     commentaire de `pow_on_boxes()`) disent les bornes vérifiées « bit for
     bit » alors que `==` ne distingue pas −0 de +0, et racontent l'histoire
     (« when each had its own copy of the pow » dans `doc/tests.md`, « The two
     were written apart » et « before the pow of the standard was written
     once » dans `tests/ieee1788.cpp`) : dire plutôt que les littéraux ont été
     relevés sur l'ancien code et vérifiés avec mpmath à 500 bits ;
   - les commentaires de `tests/CMakeLists.txt`, `tests/Makefile.am` et
     `tests/meson.build` justifient C++17 par les littéraux de
     `elementary_values.h` seulement, alors que `ieee1788.cpp`,
     `arithmetic.cpp`, `core_math.cpp` et d'autres en ont aussi : parler des
     tests en général.

2. **Le sens d'arrondi est encore vérifié plus d'une fois** par `pow(x, y)`
   quand `pow_standard()` (`gaol/gaol_interval.cpp`) passe par
   `exp(y*log(base))` (une borne infinie, ou une base partant de 0 avec un
   exposant qui n'est pas au-dessus de 0) : `log()`, `*` et `exp()` le
   vérifient trois fois ; par `nth_root(x, q)` pour q < 0, `inverse()` de
   `nth_root(x, -q)`, deux fois ; et par `modulo_k_pi()`, dont chacune des
   deux divisions `x/interval::pi()` le vérifie. Correction : appeler les
   corps de ces opérations après une seule vérification, comme le font
   `tan()`, les fonctions relationnelles et les puissances négatives
   (`uipow_rounded_upward()`, `interval::inverse_upward()`) ; les corps de
   `log()`, `exp()` et `*` sans la vérification restent à écrire. Pour `pow`,
   fusionner d'abord le point 3 (branche `todo-03-pow-exact-corner`), puis
   choisir : (a) écrire ces corps ; (b) étendre l'analyse des coins aux
   bornes infinies et à une base partant de 0, avec les limites de x^y, ce
   qui supprime le recours à exp(y log x). La voie (b) rendrait aussi ces
   boîtes serrées : le chemin exp(y log x) peut être large de centaines de
   doubles (`gaol_ieee1788::pow([-oo, 2^-1000], [-oo, -1])` vaut
   [0x1.ffffffffffd97p+999, +oo], 617 doubles sous 2^1000), et
   `pow([4, +oo], 0.5)` vaut [2 − 2^-52, +oo] au lieu de [2, +oo]. Elle
   change des valeurs attendues de `pow_on_boxes()` (`tests/ieee1788.cpp`,
   boîtes [0.25, 0.5] × [−oo, −1] et [2, 4] × [1, +oo]) et demande des tests
   aux limites.

3. **`pow(x, y)` a la largeur d'un double là où la puissance en un coin est un
   double.** Dans `pow_standard()` (`gaol/gaol_interval.cpp`), `pow_lo()`
   prend toujours le double sous la valeur de CORE-MATH arrondie vers le
   haut, même quand elle est exacte : `pow([4], 0.5)` vaut [2 − 2^-52, 2], et
   les cas 149 à 152 et 163 de
   [doc/compare/special_cases.md](doc/compare/special_cases.md) sont plus
   larges que le résultat d'IEEE 1788. Fait dans la branche
   `todo-03-pow-exact-corner` (voir « En cours ») : `pow_is_double()` prouve
   par des entiers et des opérations exactes que x^y est un double
   (puissances de deux à t/p, puissances 2^k-ièmes parfaites à a/2^k), et
   `pow_lo()` le prend alors comme borne inférieure ; `pow([4], 0.5)` vaut
   [2], et 275 cas spéciaux sur 286 sont ceux d'IEEE 1788 ; tests dans
   `tests/core_math.cpp` (`pow_exact_at_corners()`, 81 422 paires),
   `elementary.cpp`, `ieee1788.cpp` (la boîte [0.1, 2] × [−2, 0.5] devient
   `[0x1p-2, 100]`) et `other_functions.cpp`. À corriger avec elle : la
   colonne GAOL des cas 184 à 188 de `doc/compare/special_cases.md` écrit
   [−0], alors que GAOL donne [0] depuis #37 ; la phrase « where the corners
   below are one double wide » du commentaire de `pow_standard()`, qui reste
   dans la branche et devient fausse là où x^y est un double. Les bornes
   infinies gardent exp(y log x) : voir le point 2. Voir
   `todo-notes/2026-10-01/03.md`.

## Bornes fausses

4. **Le flush-to-zero et le denormals-are-zero rendent les bornes fausses**
   (revue n° 2) : avec eux, `[1e-300] * [1e-20]` vaut [0, 0]. Un programme
   lié avec `-Ofast`, `-ffast-math` ou `-funsafe-math-optimizations` les
   reçoit de `crtfastmath.o` (GCC et Clang, sur x86 et sur ARM), et un
   plug-in compilé ainsi les active à son chargement. La sonde
   `1.0 + tiny == 1.0` de `round_upward_if_needed()` (`gaol/gaol_fpu.h`,
   `tiny` = 2^-60) ne voit que le sens d'arrondi. Fait dans la branche
   `todo-04-ftz-daz` (voir « En cours »), validé en SSE2, FPU, Clang 18,
   autotools, meson, et sur AArch64 et armhf sous qemu :
   - la sonde `1.0 + (2^-1060 + 0.0) == 1.0` efface FTZ et DAZ de MXCSR, et
     FZ (et FIZ) d'ARM ;
   - les intervalles FPU et les opérations à opérande double sondent avant
     de comparer les bornes ;
   - `-mno-daz-ftz` passe à l'édition de liens par `gaol.pc`, et par
     `gaol::gaol` pour le compilateur de GAOL, de sa version ou plus récent
     (GCC 11.4, 12.4, 13.1 et plus sur x86) ;
   - le test `tests/fast_math_link.cpp` est nouveau, et la documentation est
     écrite.

   Questions de la branche :
   - `gaol.pc` porte `-mno-daz-ftz` : un programme lié par Clang 18 ou par un
     GCC avant 11.4 avec le `gaol.pc` d'un GAOL compilé par GCC 13 s'arrête
     sur l'option. Le garder (c'est documenté), ou ne le laisser que dans
     `gaol::gaol` ;
   - coût : `x * y` prend 0,5 ns de plus (3,45 → 4,0 ns) sur un Xeon Cascade
     Lake. Les processeurs de la CI ne sont pas mesurés : comparer le
     tableau de `gaol_performance` de la CI à celui de `configure-clean` ;
   - ne sont pas couverts : ARM avec Visual C++ (FPCR par `_ReadStatusReg`)
     et clang-cl ; FIZ (Armv8.7) n'est pas essayé ;
   - GCC 9.4 avec `-funsafe-math-optimizations` réduit la sonde en
     `tiny == 0.0` malgré `volatile`, et la sonde sous-normale de la même
     façon. Ranger la somme dans un double `volatile` avant de comparer
     garderait l'addition (coût non mesuré ; question de #39).
     `-fno-signed-zeros` fait sauter le `+ 0.0` de la sonde dans le code du
     programme, ce qui n'est que documenté ;
   - avant la première opération qui sonde, et toujours avec
     `GAOL_PRESERVE_ROUNDING`, les fonctions qui comparent les bornes avant
     leur sonde se trompent sous DAZ ou FZ : voir le point 45.

   Voir `todo-notes/2026-10-01/04.md`.

5. **Suites du refus de `-ffinite-math-only`** (#39). Le `#error` sur
   `__FINITE_MATH_ONLY__` (`gaol/gaol_config.h`), la ligne de
   `doc/three-builds.md` et les tests de CMake `refused_finite_math_only` et
   `refused_fast_math` sont faits. Restent :
   - le commentaire de ce refus dans `gaol/gaol_config.h` dit encore que
     `is_empty()` lit l'ensemble vide comme `!(left() <= right())` ; depuis
     #47 c'est `!std::islessequal(left(), right())`. La phrase qui dit
     `([1, 2] & [3, 4]).is_empty()` faux (« GCC 9, Clang 18 » dans
     `gaol/gaol_config.h`, « GCC 9.4, Clang 18 » dans `doc/three-builds.md`,
     « with GCC 9 and Clang 18 » dans `doc/tests.md`, sans compilateur dans
     `tests/refused_options.cpp`) est à préciser : dans `61c7503`, avec
     `-ffinite-math-only`, GCC 13 le rend faux à -O0, -O2 et -O3, Clang 18 à
     -O0, et à -O2 et -O3 seulement avec des bornes `volatile` (des bornes
     constantes lui font calculer l'intersection à la compilation) ;
   - avec GCC, `-Ofast` n'est pas refusé dès que `-fno-fast-math` est sur la
     ligne de commande, avant ou après (GCC 9.4 et 13), alors que le tableau
     de `doc/three-builds.md` et `doc/using.md` (« does nothing when it comes
     before them, where the compilation stops ») le disent refusé. Le
     préciser, et lever l'ambiguïté de « refused when the option follows
     it » (`doc/three-builds.md`) : « quand `-ffast-math` ou
     `-ffinite-math-only` vient après `-fno-fast-math` » ;
   - le `#error` de `__FAST_MATH__` ne donne pas de remède, celui de
     `__FINITE_MATH_ONLY__` en donne un ;
   - `tests/refused_options.cpp` n'a pas de témoin positif : il n'est compilé
     qu'avec les options refusées. Ajouter un test sans option (environ 1 s),
     ou retirer la phrase qui le dit juste sans elles. Ses bornes constantes
     laissent Clang calculer l'intersection à la compilation, alors que des
     doubles `volatile` montreraient le défaut avec les deux compilateurs ;
   - l'annexe B n° 4 de `examples/examples.md` dit qu'un test de compilation
     vérifie le message avec `PASS_REGULAR_EXPRESSION`, « as `tests/fp_strict`
     does ». C'est faux : `tests/fp_strict` fait `try_compile()` à la
     configuration, pour Visual C++ seulement ;
   - ce qu'aucune macro ne montre n'est que documenté :
     `-funsafe-math-optimizations` et `-ffast-math -fno-finite-math-only`
     (GCC), `-fno-honor-nans` (Clang), et sans doute `#pragma GCC optimize`,
     `__attribute__((optimize))` et `#pragma clang fp` (non testés). Une idée
     non essayée : une vérification à l'exécution dans l'objet statique de
     `gaol/gaol_init_cleanup.h`, compilé avec les options de chaque fichier
     qui inclut GAOL.

   La sonde réduite par `-funsafe-math-optimizations` est au point 4.

6. **Suites de `fegetround()` lu dans MXCSR et du refus de MinGW-w64** (#54,
   #51). Sont faits : la lecture de MXCSR dans `gaol/core_math_port.h` sur
   x86-64, le test de `pow` à résultat sous-normal dans
   `tests/rounding_direction.cpp`, la correction de `cbrt.c` sous mingw-w64
   x64, et le refus sur x86-64 de tout mingw-w64 avant 12 ou lié à
   `msvcrt.dll`. Restent :
   - à corriger dans les textes :
     - le commentaire de `round_upward_if_needed()` (`gaol/gaol_fpu.h`) dit
       que l'unité x87 « computes none of GAOL's doubles ». C'est faux avec
       mingw-w64 x64, dont `ldexp()` est le `fscale` x87 : écrire « none of
       GAOL's doubles but exact ones » ;
     - « as the get_rounding_mode() of cbrt.c, rsqrt.c and asinpi.c does »
       (`gaol/core_math_port.h`) ne vaut que pour GCC et Clang, pas pour
       Visual C++ x64 ;
     - dans `3rd/README.md`, « How the changes are checked » dit que chaque
       fonction dont le fichier a changé donne les bits de l'amont, ce que
       `cbrt.c` ne fait pas, par construction, avec mingw-w64 x64. Dire que
       le changement 6 est vérifié par `tests/core_math.cpp` dans les jobs
       MinGW-w64 et MSYS2 x64 ;
   - à décider :
     - étendre la lecture de MXCSR au x86 32 bits avec SSE2
       (`__i386__ && __SSE2_MATH__`, Visual C++ x86 `/arch:SSE2`), en défense
       de plus, sans pouvoir le tester ici ;
     - garder ou supprimer `GAOL_RND_MINGW_FENV_ONLY`
       (`gaol/gaol_fpu_fenv.h`) : le mingw-w64 11 i686 passe tout ctest sans
       lui sous wine32 ;
     - refuser mingw-w64 ARM64 avant 12 ou sans `_UCRT`, comme sur x86-64, ou
       laisser ;
     - garder ou lever le refus de mingw-w64 ARM avant 11, que rien de testé
       ne justifie ;
     - garder le refus du x86 32 bits par version (avant 11), ou le
       restreindre à la chaîne de WinLibs qui le prouve ;
     - taire localement le `#warning` de `cbrt.c`, `rsqrt.c` et `asinpi.c`,
       qui s'affiche toujours avec mingw-w64 et que le correctif 6 de
       `3rd/README.md` retire en amont, en attendant son envoi (point 31) ;
     - signaler à mingw-w64 son `fma()` et son `round()`, encore là pour
       msvcrt dans les versions 13 et 14 ;
   - limites gardées et écrites : un programme qui définit lui-même `_UCRT`
     en se liant à `msvcrt.dll` n'est pas refusé, ni un instantané de
     mingw-w64 « 12 » antérieur au déplacement de `fma.c` et `round.c`
     (théorique) ; seul `tests/core_math.cpp` les attraperait. Le `fma()`
     logiciel de `ucrtbase.dll` (processeur sans FMA3) n'a jamais été
     vérifié.

<!-- -->

8. **`pow(x, n)` pour n grand** (revue n° 7). Dans `ipow_exact_dn()`
   (`gaol/gaol_interval.cpp`), la borne inférieure perd le carré du reste
   (`nl = std::fma(-h,h,p) + (2.0*h)*nl`) : elle est 8 doubles sous la plus
   serrée pour n = 2^28 − 1, et 1962 pour 2^32 − 1. Là où les deux bornes
   prennent les produits arrondis (une borne 0 ou infinie, ou dont la
   puissance sort de la plage des doubles), les intervalles SSE2 multiplient
   depuis le bit de poids fort de n (`MSB_position()`, `reverse_bits()`) et
   les FPU depuis le bit de poids faible : leurs bornes diffèrent, contre
   « les mêmes bornes sur toutes les machines » de `doc/accuracy.md`. Fait
   dans la branche `todo-08-pow-large-n` (voir « En cours ») :
   - `nl = std::fma(-h,h,p) + nl*(2.0*h - nl)` ;
   - les intervalles SSE2 dans l'ordre des FPU (le code de GAOL qui était
     sous `#if 0`, et qui est plus rapide) ;
   - la garantie écrite « à 5 n log2(n) 2^-104 près d'un double » au lieu de
     n 2^-104, déjà dépassé à n = 4 ;
   - les tests `large_powers()` et `rounded_powers_as_in_both_builds()` de
     `tests/arithmetic.cpp`.

   Questions de la branche :
   - une borne nulle (`pow([0, b], n)`, `pow([a, 0], n)`) envoie les deux
     bornes aux produits arrondis : la borne haute est au-dessus de la plus
     serrée pour 88 % des b de [0.5, 2] (n = 3 à 10), jusqu'à 10 doubles.
     `ipow_exact_dn(0)` pourrait rendre 0 exactement, en une ligne : à
     décider. Plus généralement, une seule borne hors de la plage (0,
     infinie, ou dont la puissance sort des doubles) envoie les deux bornes
     aux produits arrondis, et celle qui y reste peut s'éloigner de la plus
     serrée : décider s'il faut traiter chaque borne à part ;
   - supprimer l'ancien code SSE2 (`MSB_position()`, `reverse_bits()`), gardé
     sous `#if 0` ;
   - relire la nouvelle garantie, 5 n log2(n) 2^-104 (bornée par
     (3s + 2m) n 2^-104, mesurée 1,7 log2(n) n 2^-104).

   Le signe d'une borne nulle diffère encore entre SSE2 et FPU (`sqr([-2, 3])`
   vaut [-0, 9] et [0, 9]) : c'est laissé ainsi selon la décision du 30
   septembre, et écrit dans `doc/accuracy.md`. Voir
   `todo-notes/2026-10-01/08.md`.

9. **Suites de `tan([-M_PI_2, M_PI_2])`** (#36). `tan()` teste maintenant
   `!(w <= pi_dn)` et donne ±1,63·10^16. Restent :
   - le commentaire de `tan()` au-dessus de `const double w`
     (`gaol/gaol_interval.cpp`) dit « the test was w < pi_dn », alors que
     l'ancien code testait `!(w < pi_up)`, puis `w < pi_dn` pour le drapeau
     `narrower_than_pi`. Son « though none holds a pole » se lit comme si
     aucun intervalle de cette largeur n'avait de pôle, alors que
     `[0, pi_dn]` contient π/2 ;
   - un intervalle sans pôle dont la largeur exacte est strictement entre
     `pi_dn` et π donne encore [-oo, +oo] (w s'arrondit à `pi_up`). Le rendre
     serré demanderait de comparer `r - l` à π en double-double ; on n'en
     connaît aucun (600 premiers pôles, 120 000 en relecture). Garder
     « above π̲ and below π » dans `doc/accuracy.md`, ou dire le cas
     théorique ; l'entrée `tan` du manuel ne parle pas de ces largeurs ;
   - `cos_or_sin()` garde ses tests `w < pi_dn` et `w < 2.0*pi_dn` : qu'ils
     donnent le résultat le plus serré est mesuré (29 400 intervalles de
     largeur voisine de π, aucun mauvais résultat), pas démontré.

<!-- -->

11. **Suites du lecteur sous denormals-are-zero** (#40). Le lecteur est juste
    sous flush-to-zero et denormals-are-zero : `gaol_compare_number()`
    (`gaol/gaol_interval_lexer.lpp`) prend les doubles par leurs bits ; ce
    que DAZ fausse encore hors du lecteur est le point 45. Restent les
    remarques des relectures :
    - dans le manuel (`manual/v5/gaol.tex`, paragraphe « Numbers »,
      l. 3418-3422), la phrase sur les deux modes écrit `\code{-Ofast}` au
      lieu de `\option{-Ofast}` (l. 345 et 348), et porte `\newinvfive` sans
      dire ce que faisait GAOL 4, qui lisait par `strtod()` en arrondi dirigé
      et n'a pas été testé sous DAZ : retirer la marque, ou tester GAOL 4
      (`GAOL_V1`) ;
    - dans `tests/numbers.cpp`, `#if GAOL_TESTS_HAVE_MXCSR` (l. 176, 238 et
      324) teste une macro indéfinie hors x86 et avertit sous `-Wundef` :
      écrire `#ifdef` ;
    - vérifier une fois, avec `ctest -V -R numbers`, si les tests DAZ de
      `numbers` s'exécutent ou se sautent avec leur message : sur les jobs
      macOS x86_64 sous Rosetta, et avec l'UCRT en Release, où
      `subnormal_output()` saute si la bibliothèque C écrit un sous-normal
      autrement sous DAZ.

    Voir `todo-notes/11.md`.

<!-- -->

45. **Le denormals-are-zero fausse encore des bornes hors du lecteur** (suite
    du point 11). Le point 11 a rendu le lecteur de nombres indépendant de DAZ
    (comparaisons sur les bits). #57 a fait de même pour `display_bounds()`
    (commit `6ad025a`) : sous DAZ, [0, 5e-324] n'est plus écrit comme un
    point. D'autres fonctions comparent encore des doubles sous-normaux comme
    des doubles, avant toute sonde de `round_upward_if_needed()` :
    - l'action de `gaol/gaol_interval_parser.ypp` (l. 484) qui vérifie le
      littéral `<a, b>` (`l.left() != r.right()`) : sous DAZ,
      `<1e-310, 1e-309>` et `<0x1p-1074, 0x1p-1073>` sont acceptés au lieu
      d'être refusés ;
    - le constructeur `interval(l, r)` (`gaol_interval_sse.h` l. 85,
      `gaol_interval_fpu.h` l. 97) : `l <= r` étant aplati, des bornes
      sous-normales dans le mauvais ordre ([0x1p-1073, 0x1p-1074]) ne donnent
      pas l'ensemble vide ;
    - `bound_to_text()` (`gaol/gaol_interval.cpp` l. 656 et 665) compare avec
      `0.0`. Sous DAZ, une borne sous-normale passe donc par le chemin de la
      bibliothèque C, et elle est écrite au plus proche au lieu d'être
      arrondie vers l'extérieur. À 16 chiffres, le texte relu contient encore
      l'intervalle, mais à un chiffre [22·2^-1074] s'écrit `[1e-322]`, relu
      [20·2^-1074, 21·2^-1074]. Une bibliothèque C peut aussi écrire 0 pour
      tout sous-normal sous DAZ : c'est le cas de gdtoa (FreeBSD, macOS) et
      du runtime Debug de Visual C++, d'après `subnormal_output()` de
      `tests/numbers.cpp`, qui n'y vérifie rien. [5e-324] s'y écrit alors
      `[0]` ;
    - les fonctions qui comparent les bornes avant leur sonde (relevé au point
      4) : `log([1e-310, 1e-309])` et `sqrt([-1e-310, 4])` donnent l'ensemble
      vide, et `abs([-1e-309, -1e-310])` reste négatif ; de même pour
      `div_rel`, le `mid()` des intervalles FPU et les relations. La branche
      `todo-04-ftz-daz` fait effacer FTZ/DAZ par la sonde de chaque opération.
      Une fois fusionnée, il ne reste pour ces fonctions que la fenêtre avant
      la première opération qui sonde ; avec `GAOL_PRESERVE_ROUNDING`, qui
      rend les modes au programme après chaque opération, chaque appel.

    Correction : les rendre indépendantes du mode, comme le lecteur (comparer
    les bits), ou leur faire exécuter la sonde avant de comparer ; régénérer
    le parser commité (bison 3.5.1) pour le premier. Comparer les bits ne
    suffit pas pour `bound_to_text()` : son chemin propre fait aussi écrire
    les chiffres par la bibliothèque C (`out << magnitude`), et gdtoa ou le
    runtime Debug de Visual C++ y écrivent 0 un sous-normal sous DAZ. Ce
    runtime déclenche même sa propre assertion avant d'écrire 0 (`cfout.cpp`,
    une boîte de dialogue qui bloque le programme Debug), y compris par
    `operator<<`. GAOL doit-il retirer DAZ (et FTZ) de MXCSR le temps de
    l'écriture ? C'est la question de #58 ; la branche `todo-04-ftz-daz` ne
    touche pas `operator<<`. Le même mode existe sur AArch64 et ARM32
    (FPCR.FZ), où les tests DAZ de `tests/numbers.cpp` ne tournent pas
    (MXCSR seulement). Le `tests/gaol_tests.h` de la branche `todo-04-ftz-daz`
    sait écrire FPCR et FPSCR. Voir `todo-notes/11.md` et
    `todo-notes/2026-10-01/04.md`.

<!-- -->

47. **clang-cl : ce qui reste après le correctif de `cbrt`, `rsqrt` et
    `asinpi`** (#59). Le `get_rounding_mode()` de `cbrt.c`, `rsqrt.c` et
    `asinpi.c` prend maintenant la branche de Windows sous `_WIN32` aussi
    (changement n° 7 de `3rd/README.md`), et `windows.yml` a deux jobs
    clang-cl x64 (Release et Debug). Restent :
    - refuser clang-cl sans `/fp:strict` ? `gaol/gaol_config.h` (l. 236) ne
      refuse que Visual C++ (`!defined(__clang__)`), et `tests/fp_strict` ne
      vérifie que lui. Sans `/fp:strict`, clang-cl compile avec
      `-fno-rounding-math -ffp-contract=on`. Il définit `_M_FP_STRICT` à
      partir de Clang 16 : la garde pourrait tester `defined(_MSC_VER) &&
      (!defined(__clang__) || __clang_major__ >= 16)`, et `tests/fp_strict`
      vérifier clang-cl aussi. C'est plus urgent maintenant que clang-cl est
      dans la CI ;
    - Cygwin x64 reste faux. Les `FE_*` de newlib valent 0 à 3, et Cygwin ne
      définit ni `__WIN32__` ni `_WIN32` : les trois fonctions y prennent les
      arrondis vers le bas et vers le haut pour l'arrondi vers zéro. Prendre
      le correctif 6 de `3rd/README.md` en entier (la table, juste quelle que
      soit la bibliothèque C), ou refuser Cygwin dans `gaol_config.h` ? Cygwin
      n'est pas dans la CI ;
    - citer clang-cl x64 dans le manuel (Prerequisites, `manual/v5/gaol.tex`
      l. 307) et dans `doc/three-builds.md` une fois ses jobs verts : ils
      échouent à l'étape des tests sur `configure-clean`, à cause du point 49.
      Le clang-cl x64 de Visual Studio 2026 (concerné par le correctif, non
      testé), et clang-cl x86 et arm64 (non concernés : pas de `__x86_64__`),
      ne sont pas dans la CI ;
    - chaque build clang-cl affiche 11 avertissements « argument unused
      during compilation: '/Zc:strictStrings-' ». `CMakeLists.txt` (l. 686) le
      donne sous `if(MSVC)`, qui est vrai pour clang-cl ;
      `$<$<CXX_COMPILER_ID:MSVC>:...>` le réserverait à Visual C++
      (`gaol/meson.build` l. 164 le donne aussi).

    Voir `todo-notes/2026-10-01/47.md`.

48. **`cancel_minus` et `cancel_plus` sont faux avec la bibliothèque compilée
    par GCC à `-O3`.** Dans `difference_at_least()`
    (`gaol/gaol_interval.cpp` l. 2309), GCC déplace les termes d'erreur de
    `two_sum` après `GAOL_RND_NEAREST_LEAVE()` : ils sont calculés vers le
    haut, où TwoSum n'est plus exact. Reproduit sur `configure-clean` avec
    GCC 13.3 en Release (build par défaut) :
    `cancel_minus([0.5], [4.9e-324, 1e-300])` donne
    [0x1.fffffffffffffp-2, 0.5] au lieu de [-oo, +oo], et
    `cancel_minus([DBL_MAX], [0.5])` donne [-oo, +oo] au lieu de
    [pred(DBL_MAX), DBL_MAX]. `cancel_minus([DBL_MAX], [1])` donne aussi
    [-oo, +oo] et lève FE_INVALID. En Debug et avec Clang, les résultats sont
    justes. Correction : faire passer `s1`, `e1`, `s2` et `e2` par
    `gaol_core::rnd_keep()` avant `GAOL_RND_NEAREST_LEAVE()`, comme aux
    lignes 3001 et 3116. `GAOL_RND_KEEP`, proposé par les relectures, est vide
    sans `GAOL_PRESERVE_ROUNDING` et ne corrige rien dans ce build ; essayé :
    `rnd_keep()` donne les trois bons résultats, sans FE_INVALID. Ajouter un
    test de non-régression avec ces trois cas (le test aléatoire de
    `tests/arithmetic.cpp` ne les voit pas). Corriger aussi le commentaire de
    `GAOL_RND_NEAREST_ENTER` (`gaol/gaol_fpu.h` l. 126-137), qui dit qu'entre
    les deux macros une opération ne fait que changer des signes et comparer.
    Relevé par les relectures des points 24 et 24b (#47, #50).

49. **Compilé par clang-cl, `GAOL_INFINITY` n'est pas l'infini quand le
    programme arrondit vers le bas ou vers zéro.** `gaol/gaol_port.h`
    (l. 145) le définit comme `HUGE_VAL`, que l'UCRT (SDK 10.0.26100) écrit
    `((double)(float)1e+300)`. Sous `/fp:strict`, clang-cl fait cette
    conversion à l'exécution (`vcvtsd2ss`/`vcvtss2sd`) : elle donne FLT_MAX
    vers le bas ou vers zéro, et lève dépassement et inexact dans les quatre
    sens. `interval()` et `[1]/[-1, 1]` valent alors
    `[-0x1.fffffep+127, 0x1.fffffep+127]`, `pow(x, n)` donne `[-nan, nan]`, et
    `interval(1e300)` est vide. `rounding_direction` a 37 échecs avec
    clang-cl 18 sous wine, et aucun avec `__builtin_huge_val()`.
    `GAOL_INFINITY` est aussi dans le code en ligne des en-têtes publics, donc
    dans le programme qui utilise GAOL. Visual C++, GCC, Clang et MinGW ne sont
    pas concernés. Les deux jobs clang-cl de `windows.yml`, fusionnés avec
    #59, en sont le test : ils échouent à l'étape des tests sur
    `configure-clean` tant que ce n'est pas corrigé. À décider, dans sa propre
    pull request : `GAOL_INFINITY` = `__builtin_huge_val()` sous `__GNUC__` ou
    `__clang__` (`HUGE_VAL` sinon), ou
    `std::numeric_limits<double>::infinity()`, que le commentaire de
    `gaol_port.h` (l. 141-144) écarte pour d'anciennes versions de libc++.
    Trouvé par la relecture du point 47 (`todo-notes/2026-10-01/47.md`,
    `review-47.md`).

## Plantages

12. **Les longues sommes débordent la pile** (revue n° 17) : dans
    `configure-clean`, `textToInterval()` d'une somme de 100 000 termes plante
    encore (vérifié avec une pile de 8 Mo), comme les expressions aussi
    longues construites par l'API : l'évaluation et la destruction de l'arbre
    font une récursion par nœud. La correction est dans la branche
    `todo-12-long-sums`, inachevée (voir « En cours ») :
    `gaol_interval_parser.ypp` calcule chaque opération dès sa lecture
    (`gaol_operation()`), le parser commité est régénéré, et les expressions
    de l'API sont évaluées (`gaol/gaol_expr_eval.h`, `gaol/gaol_eval_stack.h`)
    et détruites (`gaol/gaol_expression.cpp`) par des boucles, avec des piles
    dans le tas ; l'imbrication d'une chaîne est bornée par la pile du parser
    (`YYMAXDEPTH` 10000 : environ 10 000 parenthèses, crochets ou signes et
    5 000 appels imbriqués, au-delà `input_format_error`), et `operator<<`
    d'une expression reste récursif (limite documentée).

## Flux et texte

14. **Suites de `what()` des exceptions** (#32). `gaol_exception::what()`
    renvoie l'explication, ou `gaol_exception` sans explication, et
    `operator<<` ne l'écrit plus qu'une fois. Restent : le manuel
    (`manual/v5/gaol.tex`, l. 4451-4452) ne documente que les constructeurs à
    `const char* e`, alors que `gaol_exception` prend une `const string&` et
    ses classes dérivées les deux ; les constructeurs à `const char*` des
    classes dérivées (`input_format_error`, `unavailable_feature_error`,
    `invalid_action_error`) passent un pointeur nul à `std::string`, ce qui
    est un comportement indéfini (libstdc++ lève `std::logic_error`, y
    compris par `gaol_ERROR(excep, NULL)`), qu'aucun appel de GAOL ne fait :
    le traiter comme une absence d'explication, ou le dire ; dans
    `doc/tests.md` (paragraphe `expressions`, l. 391), « The reading of »
    reste seul sur une ligne : refaire les retours à la ligne. À choisir :
    le texte de `what()` sans explication (`gaol_exception`,
    `GAOL exception` ou le nom de la classe), la forme courte
    `file, line n: explication` d'`operator<<`, et une réserve là où
    `doc/using.md` et le manuel disent que GAOL 4 donnait `std::exception`
    (texte de libstdc++ et de libc++ ; `Unknown exception` sous Visual C++).
    L'entrée `what()` du manuel peut laisser son en-tête seul en bas de page
    (macro `\defmethod`) : à revoir quand le PDF sera régénéré.

15. **Suites de la lecture des lignes vides par `operator>>`** (#46).
    `operator>>` (`gaol/gaol_interval.cpp`) lit maintenant par
    `std::getline(is >> std::ws, buffer)`, après la sentinelle de toute
    extraction : une ligne vide est sautée, et `in >> d >> x` comme
    `while (in >> x)` lisent ce qu'il y a et finissent sans exception. La
    lecture par mot est le point 46. Restent les remarques de la relecture :
    - des textes disent encore qu'une ligne vide fait lever
      `input_format_error` : la puce « operator>> ends with the input » de
      `doc/differences.md` (l. 402-403, « a blank one included »), et
      `examples/examples.md` (l. 62-63, l. 457-459 et la ligne 14 du tableau
      5.2, l. 634) ;
    - le commentaire d'`operator>>` (`gaol/gaol_interval.cpp`, l. 541,
      « std::ws is no extraction: it constructs no sentry ») et
      `doc/tests.md` (l. 204, « std::ws alone does none of this ») donnent
      pour général ce qui est vrai du `std::ws` de libstdc++ 9 et 10 : en
      C++11, `ws` est une entrée non formatée qui construit une sentinelle,
      et libc++ le fait ; écrire « le std::ws de libstdc++ » ;
    - `doc/tests.md` l. 194-195 (« Blank lines have to be skipped, as the
      blanks before a number are, with or without std::noskipws ») se lit
      comme si un nombre sautait les blancs sous `noskipws` ;
    - l'exemple du manuel `cout << d << ' ' << x << ' ' << (in >> x ?
      "more" : "end");` (`gaol.tex` l. 3342) lit et écrit `x` dans la même
      expression : en faire deux instructions ; la phrase des l. 3311-3313
      (ligne vide « avec ou sans `std::noskipws` », intervalle sur plusieurs
      lignes) est à couper en deux ;
    - le commentaire Doxygen d'`operator>>` (`gaol/gaol_interval.h`,
      l. 791-801) ne dit pas, contrairement au manuel, qu'avec les exceptions
      désactivées une ligne refusée écrit un message sur `cerr` et arrête le
      programme ;
    - `tests/numbers.cpp` appelle `stream_without_buffer()` en dernier
      (l. 1891) : en cas de régression le programme meurt par SIGSEGV sans
      afficher les échecs précédents, stdout étant tamponné ; ajouter
      `std::fflush(stdout)` avant l'appel.

    À décider : avec `exceptions(eofbit)` seul, un intervalle lu en fin
    d'entrée lève depuis `std::ws` avec `eofbit` seul, là où un double
    reçoit `eofbit | failbit`, et une dernière ligne sans fin de ligne est
    perdue (accepter, c'est documenté, ou ajouter le code) ; `std::ws` saute
    `\v` et `\f`, que le lexeur ne compte pas comme blancs (règle `SPACE`,
    `[ \t\r\n]+`) : aligner le lexeur, ou laisser. Voir `todo-notes/15.md`.

16. **Suites des formats d'affichage** (#57). Les formats `width` et
    `center` écrivent le `midpoint()` au plus proche et le `rad()` vers le
    haut, documentés comme des formats d'affichage ; l'ensemble vide s'écrit
    `[empty]` dans tous les formats ; `gaol_ieee1788::intervalToText` écrit
    le format des bornes avec un point, et `[a]` pour un point dont les deux
    bornes s'écrivent pareil (un zéro `[0]`, décision du 13). Restent des
    défauts, vérifiés dans `61c7503` :
    - sous `std::showpos`, [1, 3] s'écrit `+2 (+/- +1)` dans le format
      `width` ; sous une locale qui groupe les chiffres, le milieu est groupé
      (il passe par le `num_put` du flux) et le rayon non
      (`bound_to_text()`) ;
    - le milieu s'écrit `-0` quand `midpoint()` vaut -0 :
      [-2^-1073, 2^-1074] donne `-0 (+/- 9.881312916824931e-324)`, alors que
      le manuel montre 0 pour [-0, 0] et que le format des bornes écrit
      `[0]` ;
    - le format `agreeing` écrit [1, 10] `1~[., 0.]` (`I.right() >
      10*I.left()` est faux à l'égalité, et le préfixe commun `1` coupe
      `1.000000000000000` et `10.00000000000000`), et les zéros avec leurs
      signes, contre la règle `[0]` : `~[-0., 0.]` pour `interval::zero()`
      en SSE2, `-0.000000000000000` pour `interval(-0.0)`. Sa condition
      `I.right() > 10*I.left()` est vraie pour tout intervalle négatif, tout
      intervalle contenant 0 et [0, x] : seuls les intervalles positifs avec
      r ≤ 10 l sont écrits avec leurs chiffres communs. À corriger ou à
      documenter ;
    - le format `width` sous une locale à virgule n'est pas testé (il
      écrirait `0,2 (+/- 0,1000000000000001)`) ;
    - `tests/rounding_direction.cpp` (l. 490) tire deux `random.uniform()`
      dans les arguments d'un même `std::make_pair` : l'ordre d'évaluation
      non spécifié donne d'autres opérandes selon le compilateur ;
    - sous une locale à virgule, `gaol_tests::hex()` (`tests/gaol_tests.h`)
      suit la locale globale et écrit `0x1,4p+2` dans les messages d'échec.

    À décider : `intervalToText` garde 16 chiffres ou suit
    `interval::precision()` (une ligne) ; le format hexa écrit un point
    `[a, a]`, bit à bit, avec les signes des zéros ; un point nul s'écrit
    `[0]` quels que soient les signes, ou `[-0]` quand les deux bornes sont
    -0 ; retirer `#include <sstream>` de `gaol/gaol_ieee1788.h`, inutile
    depuis qu'`intervalToText` est dans libgaol ; sous une locale à virgule,
    la garde de `display_bounds()` teste le texte (`left.find(',')`) et non
    `numpunct::decimal_point()`, et -2.5 s'écrit `[-2,5, -2,5]`, que le
    lecteur refuse ; un format de plus, dont le rayon absorbe l'arrondi des
    chiffres du milieu (`0.1 (+/- 5.6e-18)` pour le point 0.1). Que le
    runtime Debug de Visual C++ bloque sur son assertion, puis écrive 0, pour
    une borne sous-normale sous DAZ (#58) se décide avec le point 45.
    `examples/examples.md` décrit encore la revue n° 21 comme ouverte (ligne
    21 du tableau, l. 641, et l. 1186) et le format `width` comme « midpoint
    and width » (l. 566). Voir `todo-notes/2026-09-30/16.md`.

17. **`operator<<` est plus lent depuis qu'il écrit dans un
    `std::ostringstream` à lui** : environ 450 ns de plus par intervalle,
    mesurés avant #57 (13 % pour le format des bornes, 2,2 fois pour le
    format centre), et `std::internal` complète comme `std::right` (le
    manuel le dit, `gaol.tex` l. 3735). Le travail est dans la branche
    `todo-17-output-speed`, inachevée (voir « En cours ») : le texte est
    formé dans une chaîne de caractères plutôt que dans un flux, et
    `std::internal` met le remplissage entre le signe du milieu et ses
    chiffres dans les formats `width` et `center` ; `tests/numbers.cpp`
    vérifie 25 cas, sous les drapeaux, le remplissage et une locale du flux,
    contre les textes de l'ancien `operator<<`. Reste à mesurer le gain sur
    des programmes qui écrivent beaucoup d'intervalles, sur une machine au
    repos, et à vérifier que le texte reste identique octet pour octet à
    celui de `configure-clean` (formats, précisions, drapeaux, locales).

18. **Suites de la lecture des longs nombres** (#42). Le texte d'un nombre
    est analysé une fois (`gaol_take_apart()`,
    `gaol/gaol_interval_lexer.lpp`) et non à chaque comparaison : 20 000
    caractères se lisent en 0,02 s sous la locale C comme sous une locale à
    virgule (vérifié dans `61c7503`). L'analyse des chiffres reste
    quadratique (100 000 chiffres : 0,46 s) : décision du 30 septembre, on
    n'y touche pas. Restent : dire si cette décision vaut aussi pour la forme
    incertaine, dont `gaol_read_uncertain()` fait les sommes par
    `gaol_decimal_add_sub()`, qui insère en tête d'une `std::string`
    (100 000 chiffres : 1,1 s, contre 0,46 s pour la forme simple) ; le
    commentaire du membre `beyond` de `gaol_number` (l. 216) laisse croire
    qu'il est positionné dès que le nombre est sous le plus petit double
    positif ou au-dessus du plus grand, alors qu'il ne l'est que loin
    au-delà (10^311 et plus, ou sous 10^-330), et la ligne fait 119
    colonnes : mettre le commentaire au-dessus du membre ; le commentaire de
    `gaol_enclose_number()` (l. 371-372) dit que la lecture sous une locale
    à virgule prend le temps de la locale C : écrire « à peu près » (rapport
    mesuré de 1,0 à 1,2) ; dans `tests/numbers.cpp` (l. 1867-1868), le
    message d'échec du test de temps est construit par `std::to_string`
    sous la locale à virgule et affiche « 2,112549 s » : le construire avant
    les `setlocale`, ou dans un flux de la locale C. Voir `todo-notes/18.md`.

<!-- -->

46. **`operator>>` ne relit pas `os << x << ' ' << y`** (suite du point 15).
    Il lit un intervalle par ligne (`std::getline(is >> std::ws, buffer)`,
    `gaol/gaol_interval.cpp`). Un intervalle contient des espaces (`[1, 2]`)
    et la grammaire admet des littéraux imbriqués et des expressions
    (`[[1, 2], 3]`, `sqrt(2)+1`, `1 -2`), si bien que la fin d'un intervalle
    dans un flux est ambiguë. Écrire avec un saut de ligne
    (`os << x << '\n' << y`) se relit. Options :
    - garder un intervalle par ligne (documenté, c'est ce que fait le
      point 15) ;
    - n'accepter dans `operator>>` que les littéraux (`[..]`, `<..>`,
      `empty`, nombres), lus jusqu'au crochet fermant, en gardant la lecture
      par ligne pour le reste ;
    - une fonction à part.

    Il faut d'abord décider de la syntaxe acceptée. Voir `todo-notes/15.md`.

## En-têtes

19. **`gaol::sin(0.5)` ne compile plus** (revue n° 16) : chaque fonction de
    l'espace de noms `gaol` sur un double est ambiguë entre les surcharges
    pour les intervalles et pour les expressions (GCC 13 : « call of
    overloaded 'sin(double)' is ambiguous », entre `sin(const interval&)` de
    `gaol_interval.h` et `sin(const expression&)` de `gaol_expression.h`),
    parce que `gaol/gaol` inclut `gaol_ieee1788.h`, qui inclut
    `gaol_expression.h` (l. 70) ; GAOL 4 le compilait. Correction : ne plus
    inclure `gaol_expression.h` depuis `gaol_ieee1788.h`, et déplacer les
    deux surcharges d'expressions de `gaol_ieee1788` (`pown(e, n)` et
    `pow(e1, e2)`, l. 161-176) à la fin de `gaol_expression.h`. Le
    commentaire d'en-tête de `gaol_ieee1788.h` (l. 27-29) dit que
    `gaol_expression.h` y est inclus d'abord pour que ses using-declarations
    prennent les surcharges des expressions quel que soit l'ordre des
    inclusions : garder ce résultat dans les deux ordres, et corriger le
    commentaire.

20. **Les en-têtes publics débordent dans le programme** (revue n° 22).
    Les trois builds installent tous les en-têtes de `gaol/` (CMake par
    `file(GLOB gaol/*.h)`, `CMakeLists.txt` l. 712-728 ;
    `gaol/Makefile.am` l. 64-70 ; `gaol/meson.build`), internes compris :
    `gaol_interval_parser.h` ne compile pas quand on l'inclut
    (« 'Interval_struct' does not name a type »), et lui et
    `gaol_init_cleanup.h` sont internes ; `gaol_allocator.h` ne compile pas
    seul (`MEMALIGN` non déclaré). `gaol_exceptions.h` met
    `using std::exception;` et `using std::string;` à la portée globale
    (l. 36 et 41), et les en-têtes définissent des macros sans préfixe :
    `INLINE` (`gaol_config.h`), `MEMALIGN`, `__HI` et `__LO`
    (`gaol_port.h`), `HAVE_FENV_H` (`gaol_configuration.h` généré). Avec
    `-Wall -Wextra` et un simple `-I` (pkg-config, ou FetchContent, dont le
    répertoire d'inclusion n'est pas `SYSTEM`), un programme de quelques
    lignes reçoit 35 avertissements avec GCC 13 : 30 `-Wunused-parameter`
    dans `gaol_expr_visitor.h`, et un `-Wdeprecated-copy` à chaque
    `x = ...;`, y compris dans les fonctions en ligne de `gaol_interval.h`
    (l. 742-743) : le constructeur de copie est écrit
    (`gaol_interval_sse.h` l. 98, `gaol_interval_fpu.h` l. 106) et
    l'affectation de copie implicite. Correction : ne plus installer les
    en-têtes internes, supprimer les `using`, préfixer les macros, déclarer
    l'affectation de copie par défaut et laisser sans nom les paramètres
    inutilisés.

## Interface

21. **Les entiers au-delà de 2^53** : `interval(0)`, `interval(0, 0)`,
    `x = 0`, `x < 0`, `max(x, 0)` et `T(0)` compilent, GAOL v5 n'ayant plus de
    constructeur à partir de chaînes et ses constructeurs `interval(double)`
    et `interval(double, double)` (`gaol/gaol_interval.h`) n'étant pas
    `explicit` (essayé le 28 septembre, 46ff4ff, puis retiré, c02e15d) :
    l'entier devient un double. Un entier au-delà de 2^53 devient ainsi un
    double qui ne le contient pas (`interval(9007199254740993LL)`, section
    2.1 de `examples/examples.md`). Correction : des constructeurs templates
    sur les types entiers, contraints (vérifié sur tous les programmes de
    test et les sources de la bibliothèque ; dans les en-têtes, sans
    changement d'ABI). Pas un simple `interval(int)`, qui rend `interval(5L)`
    ambigu.

22. **Pas d'ordre total pour les conteneurs.** `<`, `<=`, `>` et `>=` sont les
    relations « certainement » d'IEEE 1788 (`operator<` est `certainly_le()`,
    `strictPrecedes`, dans `gaol/gaol_interval.h`), vraies dès qu'un des deux
    intervalles est vide, donc pour (∅, ∅) : `std::set` perd les intervalles
    qui se chevauchent, et `std::sort` d'un vecteur contenant un intervalle
    vide lit au-delà de sa fin. Ni `doc/using.md` ni le manuel n'en
    préviennent. Correction : un `gaol::lexicographic_less`, peut-être une
    spécialisation de `std::less`, et une mise en garde dans la documentation
    contre `std::sort`, `std::set`, `std::max`, `std::min` et `std::clamp`
    sans comparateur.

23. **Gérer le sens d'arrondi.** `gaol::cleanup()` ne le restaure qu'à son
    premier appel (`_already_cleaned`, `gaol/gaol_common.cpp`), ce que
    `doc/using.md` dit maintenant ; il n'y a pas de garde à portée, et
    `rnd_keep()` (`gaol/gaol_fpu.h`, espace de noms `gaol_core`) n'est pas
    présenté comme une barrière. Correction : un `gaol::restore_rounding()`
    qu'on peut appeler autant de fois qu'on veut, une garde qui calcule un
    bloc au plus proche (environ 7 ns), et `rnd_keep()` documenté. La branche
    `todo-36-upward-rounding-effects` (point 36, inachevée) documente déjà
    `gaol::rnd_keep()` dans `doc/using.md` et écrit une telle garde,
    `nearest_scope`, dans `examples/13_rounding_environment.cpp` : faire les
    deux ensemble.

24. **Exceptions flottantes : ce qui reste après #47 et #50.** `is_empty()`,
    `interval::emptyset()` et toutes les opérations sur un opérande vide ne
    lèvent plus FE_INVALID, et `doc/using.md` et le manuel (section 3.3)
    disent que les exceptions restent masquées pendant que GAOL calcule.
    Restent :
    - **armhf** : la CI de `configure-clean` y est rouge depuis #50 (6 échecs
      de `rounding_direction` : `x & empty`, `intersection(x, empty)` et des
      fonctions réciproques), GCC 12 à 14 pour ARM 32 bits compilant une
      comparaison silencieuse de l'`operator&=` FPU en `vcmpe`. La correction
      est dans la branche `fix-24b-armhf-fe-invalid` (voir « En cours »). Le
      même mécanisme atteint `is_empty()` dans le code du programme
      (`x.is_empty() ? p : q` donne `vcmpe` avec GCC 13.3) : le documenter,
      écrire `is_empty()` avec `std::isunordered()` sur ARM 32 bits, ou le
      signaler à GCC (bogue 52258).
    - **Opérandes non vides** : `0 × oo` dans l'`operator*=` des intervalles
      SSE2 (`[0]*[1, +oo]`, `pow([1], [1, +oo])`) et le `pow` de CORE-MATH
      pour un exposant de magnitude extrême (`pow([1, 2], [4.9e-324])`,
      `[1e300, 1e308]`) lèvent FE_INVALID : documenté, non corrigé, sans test.
      Le corriger, ou le laisser documenté.
    - **`GAOL_PRESERVE_ROUNDING` avec les intervalles SSE2** :
      `GAOL_RND_ENTER_SSE()` (`gaol/gaol_fpu.h`) appelle `round_upward_sse()`,
      qui écrit MXCSR avec `GAOL_SSE_MASK | _MM_ROUND_UP`, et
      `GAOL_RND_LEAVE_SSE()` ne rend que les bits d'arrondi : `+`, `-`, `*`,
      `/`, `sqr()`, `inverse()`, `%`, `div_rel` et les formes à opérande
      double masquent de nouveau les exceptions du programme (MXCSR 0x1d80
      avant, 0x1f80 après) et effacent ses indicateurs. Ce build ne devrait
      changer que les bits d'arrondi (proche du point 27).
    - **Documentation** (`doc/using.md`, fin de « The floating-point
      exceptions », et `manual/v5/gaol.tex`, section 3.3) : la liste de ces
      opérations oublie `%`, `div_rel` et `+= d`, `-= d`, `*= d`, `/= d`,
      `%= d` (écrire aussi qu'elles effacent les indicateurs) ; « after an
      operation of GAOL, `fetestexcept(FE_INEXACT)` is raised whatever the
      result » est trop fort (`-X`, `abs`, `&`, `|`, `floor`, `max(X, Y)` ne
      le lèvent pas, et sous `GAOL_PRESERVE_ROUNDING` un `X+Y` exact laisse
      les indicateurs à zéro) : écrire « after most operations ». Petites
      reprises : la ligne courte de `doc/accuracy.md` (« hold on every
      architecture and with »), `\newinvfive` sur les derniers paragraphes de
      la section 3.3, et `EmptySet` de `tests/rounding_direction.cpp`, qui
      porte aussi `nonempty_sets` (`SampleInterval`). Le commentaire de
      `gaol/gaol_config.h` sur `is_empty()` est au point 5.
    - **À décider** : `interval(NAN)`, `interval(NAN, 1)`, `x += NAN` et
      `set_contains(NAN)` lèvent FE_INVALID (comparaisons signalantes des
      constructeurs) : les garder, ou (a) rendre silencieuse la première
      comparaison de `interval(l, r)`, ce qui rendrait aussi gratuits `floor`,
      `ceil` et `integer`, qui coûtent 0,17 ns de plus depuis leur
      `std::isunordered()` (`gaol/gaol_interval.h`) ; ou (b) rendre ces trois
      fonctions gratuites sans toucher au constructeur, en construisant
      `[floor(l), floor(r)]` sans lui (amies, `_mm_set_pd`, `lb_`/`rb_`) :
      `floor` et `ceil` sans comparaison, `integer` avec le seul
      `std::islessequal(ceil(l), floor(r))`, `interval(NaN)` gardant son
      piège (la forme que recommande la relecture de #50). Avec GCC 13,
      `x &= y` est de 13 à 29 % plus lent (une forme à un seul
      `std::isunordered(lb_, I.lb_)`, mesurée pour armhf, est plus rapide avec
      GCC 13 mais pas partout avec Clang 18) ; le coût de `std::islessequal()`
      avec Visual C++ n'est pas mesuré (`gaol_performance` ne mesure ni `&=`
      ni les relations) : si c'est un appel de fonction, un assistant GAOL
      avec `_mm_ucomile_sd` (pour `is_empty()` aussi) rendrait l'instruction
      unique ; les intervalles de flottants (`gaol_intervalf.h`,
      `gaol_interval2f.h`) gardent des comparaisons signalantes dans `&=` et
      les relations, et `gaol_intervalf.h` appelle `std::islessequal` sans
      inclure `<cmath>`.

25. **Les outils que chaque algorithme réécrit** : `mulRevToPair` (un
    `div_rel` en deux morceaux), `inflate`, `bisect(ratio)` et
    `is_bisectable()` (`split()` coupe au milieu, ±DBL_MAX pour une
    demi-droite ; l'exemple 05 écrit sa coupe à 45 %, comme IBEX), un
    constructeur milieu-rayon, `hull()` et `intersect()` comme fonctions (il
    n'y a que `|`, `&` et les `convexHull` et `intersection` de
    `gaol_ieee1788`), un encadrement de la largeur (`width()` est un
    majorant), un littéral `_iv` dans un espace de noms `gaol::literals`
    (l'exemple 02 l'écrit lui-même), `erf` et `erfc` (leurs sources sont dans
    `3rd/math-core/src/binary64/erf` et `erfc`, mais aucun des trois builds
    ne les compile), et les fonctions réciproques de `atan2`, `pow(x, y)`,
    `max`, `min`, `sign` et `floor`, qu'IBEX écrit lui-même.
    `gaol/gaol_ieee1788.h` liste `mulRevToPair`, `powRev1`, `powRev2`,
    `atan2Rev1` et `atan2Rev2` parmi les opérations absentes. Le découpage
    prévu de ce point (25a à 25k), du 26 (un indicateur local au thread), du
    27 (une étude de faisabilité, puis un backend AVX-512 optionnel), du 28
    (28a à 28c) et du 30 (30a à 30c) est dans
    `todo-notes/orchestration/points_w4.py` (branche `todo-status`).

26. **Des décorations**, ou au moins un indicateur disant qu'un argument est
    sorti du domaine : `sqrt` d'une boîte négative est vide et passe tous les
    tests d'inclusion, et `interval(DBL_MAX*10)`, `x + INFINITY` et
    `x * NAN` sont vides sans rien dire. GAOL n'a que des intervalles nus
    (`gaol/gaol_ieee1788.h` : ni décorations ni opérations décorées, clause 11
    d'IEEE 1788-2015), et `gaol_ieee1788::textToInterval` rend l'ensemble vide
    pour un texte mal formé, qu'on ne distingue pas de `[empty]` (à
    documenter en attendant, point 38). À choisir : les décorations, ou un
    simple indicateur.

27. **Un sens d'arrondi qui ne fuit pas** : l'arrondi porté par chaque
    instruction (AVX-512, le FPCR d'AArch64 en assembleur), comme le fait
    inari, trois fois plus rapide sur les additions ; dans la direction de
    P2746. Aujourd'hui GAOL laisse l'arrondi vers le haut pour tout le
    programme (point 36) ou, avec `GAOL_PRESERVE_ROUNDING`, le lit, le change
    et le rend à chaque opération, plusieurs fois plus lentement, ses
    opérations SSE2 masquant alors de nouveau les exceptions du programme
    (point 24).

28. **Des en-têtes optionnels au-dessus du cœur scalaire**, à partir de
    `examples/` : un type boîte, la différentiation directe sur les
    intervalles et les formes affines, que `examples/box.h`,
    `examples/dual.h` et `examples/affine.h` définissent pour les exemples 03
    à 14 sans être installés, à promouvoir en `gaol/box.h`, `gaol/dual.h` et
    `gaol/affine.h` ; ou des traits GAOL pour YalAA et un backend intervalle
    pour VNODE-LP.

## Tests et intégration continue

29. **Le test sous une locale à virgule n'est vérifié que dans
    `linux.yml`** (#35) : macOS, Windows, les conteneurs (`containers.yml` ;
    le `locale-gen` de Debian ne prend pas de nom : `apt-get install locales`
    puis une ligne dans `/etc/locale.gen` ou `localedef` ; Alpine et les
    images manylinux n'ont pas de locale à virgule) et `build-systems.yml`
    (journaux `tests/numbers.log`, `build-meson/meson-logs/testlog.txt` et
    `numbers.log`) ne génèrent ni ne vérifient rien. À décider :
    `sh .github/scripts/comma-locale.sh check <build>` après les tests là où
    la locale existe, ou une variable (`GAOL_TESTS_REQUIRE_COMMA_LOCALE`)
    lue par `tests/numbers.cpp`, qui ferait échouer le test lui-même partout
    (écartée tant que le point 18 éditait ce code ; il est fusionné), ou
    rien. Au passage : `tests/fetch_content` et `tests/find_package` ne
    donnent pas de `TIMEOUT` à `numbers` (300 s dans `tests/CMakeLists.txt`
    seulement), et les images Ubuntu 22.04, dépréciées depuis le 17
    septembre, ne seront plus prises en charge d'ici le 17 avril : que
    prendre à la place des jobs 22.04 de `linux.yml` (x86_64 GCC et Clang,
    arm64 GCC, arm64 Clang 14 refusé, CMake 3.14, GCC 9) ?

30. **Des suites de tests et des bancs d'essai où GAOL est absent** : passer
    ITF1788 (toutes les opérations d'IEEE 1788) et le banc d'essai de Tang et
    al. (2021) sur GAOL v5, et ajouter à `doc/compare/` (aujourd'hui
    libieeep1788, filib++, PROFIL/BIAS et Solaris Studio) Boost.Interval, la
    bibliothèque que les utilisateurs prennent d'abord, dont les fonctions
    élémentaires ne sont pas sûres. Seuls les cas des fonctions réciproques
    des tests « minimal » de libieeep1788, devenus ceux d'ITF1788, sont déjà
    repris (`tests/reverse_values.py`).

## CORE-MATH

31. **Envoyer à CORE-MATH les correctifs écrits dans
    [3rd/README.md](3rd/README.md)** (décision à prendre). Sa section
    « Changes to propose upstream » (#56, complétée par #59) donne six
    correctifs contre le master de CORE-MATH `b1a4badf` (29 septembre),
    vérifiés avec `./check.sh --worst` et `--special` : les masques
    `~0ul>>12` (`sinh.c`, `cosh.c`, `tanh.c`, et aussi `sinpi.c`, `log1p.c`
    et `lgamma.c`, sans résultat changé), le décalage de `asinpi_acc()`, le
    `__builtin_expect` de `rsqrt.c`, le `__builtin_roundeven` de `sin.c`, le
    sens d'arrondi que `pow.h` lit pour ses résultats sous-normaux, et le
    champ d'arrondi de MXCSR lu quelles que soient les valeurs `FE_*` dans
    `cbrt.c`, `rsqrt.c` et `asinpi.c` (mingw-w64, clang-cl, Cygwin). Les
    décalages de `cospi.c` sont corrigés en amont, et la glibc n'a aucun de
    ces défauts : rien à lui envoyer. Rien n'est envoyé. À décider :
    - qui envoie, et comment : merge request sur gitlab.inria.fr (compte
      nécessaire) ou `git format-patch` à core-math@inria.fr, avec ou sans
      `Signed-off-by` ;
    - le correctif 6 sous sa forme large (la table, proposée), ou la petite
      que fait la copie de GAOL (`mode = fegetround();` dans `cbrt.c`,
      `|| defined(_WIN32)` pour clang-cl) ; y ajouter `binary80/pow/powl.c`,
      qui a la même chaîne de branches (non examiné) ;
    - signaler en même temps l'échec du master à son propre
      `./check.sh --worst --rndd pow` (underflow parasite en
      x = -0x1.10a688680a753p-93, y = 11), et la borne de ss de
      `asinpi_acc()` (ss = 76 en ±(1 − 2^-53) vers le bas et vers zéro quand
      la fonction est appelée directement, ce que `cr_asinpi()` ne fait pas) ;
    - donner aux `sinpi.c` et `log1p.c` embarqués, qui gardent `~0ul>>12`, le
      même changement `/* GAOL */` que sinh, cosh et tanh, ou attendre
      l'import du correctif amont.

    Les `~0ul` et `1ul<<52` de `binary80/atan2/atan2l.c` et
    `binary128/expm1/expm1q.c` (formats que GAOL n'utilise pas) n'ont pas été
    examinés. À corriger dans `3rd/README.md` : « Four of the changes above …
    (2 to 5) » (l. 277) ne compte pas les changements 6 et 7 (`cbrt` avec
    mingw-w64, les trois fichiers avec clang-cl), qui sont aussi des
    correctifs ; le statut du correctif 5 (« Present in master. », l. 844) ne
    dit pas que la copie de GAOL le compense par le `fegetround()` de
    `gaol/core_math_port.h`, qui lit MXCSR sur x86-64.

## Documentation

32. **Le rapport de couverture** ([coverage/README.md](coverage/README.md) et
    `coverage/coverage.html`) a été refait le 2026-09-27 à `605728e`
    (38ee6a7 : 87,2 % des lignes de `gaol/`, 67,1 % des branches), après la
    grammaire unique, le lexer et le parser réentrants et la suppression des
    relations `possibly_*`. Depuis, 47 commits (hors fusions) ont changé
    `gaol/` (60 fichiers, +2030 −1562 lignes : `interval(const char*)`
    supprimé, `gaol_interval.cpp`, `gaol_expression.cpp`,
    `gaol_exceptions.cpp`, `gaol_common.cpp`…), si bien que ses listes de
    lignes non exécutées ne correspondent plus aux sources. Le job
    « Coverage » de `.github/workflows/linux.yml` le refait à chaque push
    (résumé du job, artefact, seuil de 80 %), mais la copie du dépôt n'est
    pas mise à jour. À refaire au commit de la version avec
    `-DGAOL_COVERAGE=ON` et la cible `coverage`, qui demandent gcovr, et à
    enregistrer dans le dépôt.

33. **Les temps de [doc/compare/performance.md](doc/compare/performance.md)**
    ont été remesurés le 2026-09-27 à `605728e` (38e3b60), après que le sens
    d'arrondi n'est plus vérifié qu'une fois par fonction d'intervalles, mais
    sur un portable où d'autres programmes tournaient (certaines opérations
    jusqu'à dix fois plus lentes dans un tour, dit la page), et
    `doc/compare/code/machine.txt`, qui garde avec `results.csv` le dernier
    passage complet, dit `v4.3.2-125-g605728e-dirty` là où `performance.md`
    dit `605728e`. Depuis, les opérations ont changé (comparaisons
    silencieuses sur un opérande vide, #50 ; `pow_standard()`, #37 ;
    `fegetround()` de CORE-MATH lu dans MXCSR, #54). À remesurer en dernier,
    sur le commit propre de la version, la machine ne faisant rien d'autre
    (`doc/compare/code/run_bench.sh`, ou GAOL v5 seul avec `make perf`), puis
    reprendre le tableau que `doc/compare/README.md` en tire.

34. **Les recettes FetchContent récupèrent GAOL 4** (décision à prendre).
    `doc/using.md` (l. 53), le manuel (`manual/v5/gaol.tex`, l. 879-880) et
    `tests/fetch_content/CMakeLists.txt` (quand
    `FETCHCONTENT_SOURCE_DIR_GAOL` n'est pas donné) prennent `GIT_TAG master`
    de `Jordan08/GAOL`, et le `git clone` de `doc/building.md` (l. 137) la
    branche par défaut. `master` est GAOL 4.3.2 (`v4.3.2-6-g406d850`, avec
    `3rd/mathlib` et sans `gaol_ieee1788.h`), 137 commits derrière
    `MATH-CORE` et 286 derrière `configure-clean` : les programmes de la
    documentation ne compilent pas avec elle. Les étiquettes s'arrêtent à
    `v4.3.2` : il n'y a pas de `v5.0.0` à fixer, alors que `VERSION.txt` dit
    5.0.0. Correction : une fois #29 (`configure-clean` vers `MATH-CORE`)
    fusionnée, étiqueter `v5.0.0`, ou fusionner `MATH-CORE` dans `master`, et
    écrire l'étiquette dans les recettes et dans le test.

35. **Un premier programme avant les détails.** `README.md` ne contient pas de
    code C++ et ne renvoie pas à `examples/` ; le premier programme est à la
    ligne 96 de `doc/using.md`, après les recettes CMake et pkg-config ;
    aucun document ne montre un algorithme sur les intervalles (le manuel ne
    nomme ni Newton, ni la bissection, ni un contracteur, ni la séparation et
    évaluation). Correction : un programme de dix lignes dans `README.md` qui
    renvoie à `examples/` et à `examples/examples.md`, et un court tutoriel
    (encadrement d'image et subdivision, Newton avec `%` ou `div_rel`, un
    contracteur avec les fonctions `*_rel`, séparation et évaluation), que
    les exemples 03, 05, 06 et 07 contiennent déjà.

36. **Ce que l'arrondi vers le haut fait au programme**, dans `doc/using.md`
    et dans les erreurs courantes du manuel (« Common errors » de
    `manual/v5/gaol.tex`), avec la table de la section 2.8 de
    `examples/examples.md` : `printf`, `strtod`, `lrint`, les allers-retours
    par le texte, TwoSum, et GCC qui réutilise après `cleanup()` un double
    calculé avant ; `GAOL_PRESERVE_ROUNDING` comme moyen de garder l'arrondi
    du programme, avec son coût mesuré (4,8 fois sur x + y, 6,7 fois sur
    x/y, 1,4 à 1,6 fois sur exp et sin, selon la revue). Commencé dans la
    branche `todo-36-upward-rounding-effects` (voir « En cours ») : une
    section « What the upward rounding does to the program » dans
    `doc/using.md` (la table, les autres conséquences, ce que le programme
    peut faire) et `examples/13_rounding_environment.cpp`, qui calcule et
    vérifie chaque ligne de la table. Restent : le coût de
    `GAOL_PRESERVE_ROUNDING`, la section finissant sur `XXCOSTXX` ; le
    manuel, que la branche ne touche pas ; les mesures de TwoSum et TwoProd
    en arrondi dirigé (erreur inexacte dans 0,9 % et 3,8 % des cas, toujours
    trop petite ; exacte avec `fma`, issue #49). `todo-04-ftz-daz` ajoute
    aussi une section à la fin de « The rounding direction » de
    `doc/using.md`.

37. **Une table des noms** pour les utilisateurs d'IBEX, de Codac, de C-XSC,
    de Boost et d'IEEE 1788, dans la documentation de l'utilisateur
    (`doc/using.md` ou le manuel), avec ce que `<`, `<=` et `==` signifient
    dans chacun : dans C-XSC et PROFIL/BIAS, `<=` est l'inclusion, et le code
    porté depuis eux compile et change de sens ; GAOL v5 n'a pas de `==` sur
    les intervalles, et ses `<`, `<=`, `>`, `>=` sont les relations
    « certainement ». La section 2.3 de `examples/examples.md` en donne une
    pour GAOL v5, `gaol_ieee1788`, IBEX et Codac seulement ; aucun document
    de l'utilisateur n'en a.

38. **Les pièges dans les erreurs courantes du manuel** (« Common errors » de
    `manual/v5/gaol.tex`, qui ne traite que des constantes comme `0.1`, du
    zéro, des deux espaces de noms et des options de compilation ;
    recommandation 15 et section 5.3 de `examples/examples.md`) :
    - `sqrt(2)` sur un nombre, qui donne un double ;
    - `interval(m - r, m + r)` avec des doubles ;
    - l'ensemble vide qui passe tous les tests ; le domaine sans décorations ;
    - `split()` d'un intervalle canonique, dont la moitié droite est
      l'intervalle lui-même ([1, 1 + 2^-52] donne [1] et [1, 1 + 2^-52]) ;
    - `/` contre `%` dans la méthode de Newton ;
    - `pow(x, 2)` qui compile et prend le pow de GAOL quand les deux espaces
      de noms sont ouverts (`[-0, 9]` pour x = [-2, 3], où le pow de la norme
      donne [0, 9]), le manuel ne citant que `pow(x, x)`, ambigu ;
    - un texte refusé pour un ordre non entier de `nth_root`, qui lève
      `invalid_action_error` et non `input_format_error` ;
    - `gaol_ieee1788::textToInterval`, qui donne l'ensemble vide pour un
      texte mal formé, là où `gaol::textToInterval` lève
      `input_format_error` ;
    - une borne nulle écrite `-0`, les zéros n'étant pas normalisés
      (décision du 30 septembre, point 10), et dont le signe diffère entre
      les builds SSE2 et FPU : `sqr([-1, 2])` s'écrit [-0, 4] avec SSE2 et
      [0, 4] avec le code FPU, `[1, 2] - 1` [-0, 1] avec les deux
      (`interval::zero()` s'écrit `[0]` dans les deux depuis #57).

39. **Les restes de Goldstein-Price** (le +1 du manuel et de
    `examples/16_Goldstein_Price.cpp`, et « encloses the range », sont
    fusionnés par #53). `examples/03_dependency_problem.cpp` (l. 237) et
    `examples/06_global_optimization.cpp` (l. 338) écrivent le maximum de la
    fonction sur [-2, 2]² comme un double (1015690.2717980589, 2,97e-11 sous
    le vrai maximum) : leur vérification « l'enveloppe contient l'image »
    teste un intervalle un peu plus étroit que l'image ; `textToInterval`,
    comme dans l'exemple 16, arrondirait vers l'extérieur. L'exemple 16 écrit
    ce maximum 1,2e-20 sous la vraie valeur (le lecteur arrondit vers
    l'extérieur ; `…0829884232` rendrait le texte lui-même majorant,
    cosmétique), et garde dans `main()` le style de GAOL 4 (`x(-2,2)`,
    `z(0.1)`, `interval(0.,0.)` au lieu de `interval(0.0)`). Dans
    `examples/examples.md`, la section 1.1 dit que les sorties sont les mêmes
    sur tous les builds (l'exemple 16 imprime un temps) et que les 16
    exemples compilent sans avertissement avec `-Wall -Wextra`, ce qui n'est
    vrai qu'avec les en-têtes de GAOL en `-isystem` (en `-I`, GCC 13.3 en
    donne de 33 à 62 par exemple, venus des en-têtes : point 20) ; la
    section 3 décrit le programme du manuel sans le +1 sans dire qu'il est
    corrigé. Dans `examples/CMakeLists.txt` (l. 18), « which the autotools
    and meson builds also compile » semble renvoyer à l'enveloppe.

40. **Petites erreurs, suite** (le commentaire de `chi()`, les bits de
    `tests/gaol_tests.h`, `jail_parser.h`, `GAOL_NODISCARD` avant C++17 et
    les exemples du manuel sans `boolalpha` sont corrigés par #55).
    - Le commentaire Doxygen « Parse a string to create an interval » de
      `gaol/gaol_parser.h` est placé avant `enum class parsing_names` et non
      avant `parse_interval()`, si bien que Doxygen le donne à l'énumération,
      et il ne liste que les formats de GAOL 4 : ni `<a, b>`, ni les
      expressions, `[l, f]` pour `[l, r]`, `\emp inf` (la branche
      `todo-12-long-sums` ajoute une note au même commentaire).
    - Les tests de compilation `refused_finite_math_only` et
      `refused_fast_math` (`gaol_refused_test()` de `tests/CMakeLists.txt`)
      prennent `CMAKE_BINARY_DIR` et `CMAKE_SOURCE_DIR` comme répertoires
      d'inclusion, faux quand GAOL est un sous-projet avec `WITH_TESTS` ; les
      tests nodiscard prennent `PROJECT_*`.
    - Un test `nodiscard_discard_cxx*` reste en échec une fois son objet
      compilé avec succès (en développement seulement ; effacer l'objet, par
      `FIXTURES_SETUP` ou `cmake -E rm -f`, non essayé sous les générateurs
      Visual Studio).
    - La section 3 de `examples/examples.md` parle encore de « three
      formatting slips » (il y en avait 29, corrigés) et donne comme ouverts
      `chi([0,0]) = 0`, les 400 bits et `[[nodiscard]]` en C++17 seulement.
    - `tests/gaol_tests.h` ne cite pas la vérification à 500 bits (mpmath)
      des boîtes de `pow` de `tests/ieee1788.cpp`, dont les bornes sont
      enregistrées et non calculées par un script.

    À décider : où garder le programme qui compare les 88 sorties du manuel
    au programme (`run_examples.py`, hors du dépôt), avec le point 39 ou dans
    `manual/`. Non vérifié : la garde `_MSC_VER >= 1924` de `GAOL_NODISCARD`
    donne `_Check_return_` à Visual C++ 2017 15.8 et 15.9, absents de
    Compiler Explorer.

## Les trois builds

41. **GAOL ne peut pas être un sous-projet meson.** `meson.build` appelle
    `add_global_arguments` (17 fois, à partir de la ligne 207), que meson
    refuse dans un sous-projet : un projet parent qui fait `subproject('gaol')`
    s'arrête sur « Function 'add_global_arguments' cannot be used in
    subprojects », avec meson 0.53.2, 1.4.1 et 1.11.2. Le commentaire de
    `gaol_dep` (`gaol/meson.build`, l. 192-193) dit pourtant qu'il sert à un
    projet meson qui prend GAOL en sous-projet. Correction :
    `add_project_arguments` (essayé avec meson 1.4.1 : le sous-projet se
    construit et le programme du parent se lie), et mettre dans les
    `compile_args` de `gaol_dep`, qui n'a que `gaol_public_args`, les options
    dont le code utilisant GAOL a besoin. Les arguments du projet ne passent
    pas au projet parent : son programme est compilé sans `-frounding-math`
    ni `-ffp-contract=off`. Ce sont les options de `pc_cflags` (`meson.build`
    l. 204), que `gaol.pc` donne déjà. Sinon, retirer la promesse du
    commentaire.

<!-- -->

44. **Les suites du test `cpack_stale_configure`** (#33). CMake avertit
    maintenant, quand il configure, d'un `configure` généré pour une autre
    version que celle de `VERSION.txt` (bloc CPack de `CMakeLists.txt`), et
    `tests/cpack_stale_configure.cmake` le vérifie. La relecture de #33 a
    laissé ces remarques :
    - le CMake imbriqué ne reçoit que le générateur, `CMAKE_C_COMPILER` et
      `CMAKE_CXX_COMPILER` (l. 102-105). Deux cas configurent bien mais font
      échouer le test (« CMake did not configure the copy ») : un parent dont
      le compilateur porte un argument (`CC="ccache gcc"`, donc
      `CMAKE_*_COMPILER_ARG1` non vide), et un Ninja hors du `PATH`
      (`CMAKE_MAKE_PROGRAM`). Correction : ne pas enregistrer le test dans ces
      cas (`tests/CMakeLists.txt` l. 147), ou passer les compilateurs par
      `cmake -E env CC=... CXX=...` ;
    - sans `sh` ou sans liens symboliques, le script s'arrête sur
      `FATAL_ERROR`. La règle du dépôt est pourtant de vérifier à l'exécution
      ce qu'un test exige de la plate-forme, et de le dire ignoré. Aucun job
      de la CI n'est concerné ;
    - le commentaire « The script removes what it made in this directory, and
      nothing else » (l. 46) est inexact : `file(REMOVE_RECURSE
      "${GAOL_WORK_DIR}")` retire tout le répertoire, avec pour seule garde le
      suffixe `/cpack_stale_configure`. La suppression est sûre, c'est le
      commentaire qu'il faut corriger ;
    - `doc/tests.md` (l. 510) dit « CMake build » sans dire pourquoi autotools
      et meson n'ont pas ce test : seul CMake fait l'archive, et le job
      autotools compare déjà `configure --version` à `VERSION.txt`.

    À décider : avec les générateurs Makefile, `package_source` ne relance pas
    CMake après un changement de `VERSION.txt` (Ninja le fait). L'archive porte
    alors le nom de l'ancienne version et contient le nouveau `VERSION.txt`.
    C'est documenté dans `doc/building.md`. Une vérification au moment de
    l'archive demanderait `CPACK_PRE_BUILD_SCRIPTS` (CMake 3.19, alors que le
    minimum est 3.14) ou une dépendance de `package_source` sur une
    reconfiguration. Garder aussi le contrôle comme test CTest (de 1,3 à 4,4 s
    sous Linux, 52 s sous QEMU ppc64le ; absent sous Windows et MSYS2), ou en
    faire une étape des workflows. Voir `todo-notes/44.md`.

## Décisions à prendre

- **#29 et le point 34.** La pull request #29 (`configure-clean` vers
  `MATH-CORE`) est toujours ouverte. Une fois fusionnée : étiqueter `v5.0.0`
  ou fusionner `MATH-CORE` dans `master`, pour que les recettes FetchContent,
  le `git clone` de `doc/building.md` et `tests/fetch_content` prennent
  GAOL v5 et non GAOL 4.
- **Les choix « à confirmer » de #29**, que la liste ne reprend plus depuis
  sa réécriture (`a2ca992`) : la référence HTML de Doxygen supprimée ; meson
  sans `enable-debug` ni `enable-optimize` (le type de build les remplace) ;
  la CI qui ne lance que `make test`, sans compiler les exemples ;
  `make test` qui n'écarte les exemples qu'à partir de CMake 3.17 et meson
  0.57 ; le premier `make perf` qui déplace les colonnes des autres
  bibliothèques dans `results.csv` ; la licence MIT (GAOL v5 est toujours
  sous LGPL). Acceptés, abandonnés, ou reportés ?
- **31, l'envoi des correctifs à CORE-MATH** : qui envoie et comment (merge
  request sur gitlab.inria.fr ou `git format-patch` à core-math@inria.fr,
  avec ou sans `Signed-off-by`), la forme du correctif 6, et ce qu'on
  signale avec eux (point 31).
- **49, `GAOL_INFINITY` sous clang-cl** : `__builtin_huge_val()` sous GCC et
  Clang, ou `std::numeric_limits<double>::infinity()`, dans sa propre pull
  request. Les deux jobs clang-cl de `windows.yml` restent rouges d'ici là,
  et le point 47 attend qu'ils soient verts pour citer clang-cl.
- **46, la syntaxe d'`operator>>`** : un intervalle par ligne, les seuls
  littéraux lus jusqu'au crochet fermant, ou une fonction à part.
- **29, la locale à virgule hors de Linux** : étendre le contrôle aux autres
  workflows, faire échouer `numbers` lui-même par une variable
  d'environnement, ou le laisser à `linux.yml`.

## Questions ouvertes

Les questions laissées par les pull requests fusionnées et par leurs
relectures, quand elles ne sont pas déjà dans le texte d'un point ci-dessus,
avec le numéro de la pull request. Le détail est dans les rapports de
`todo-notes/` (branche `todo-status`).

- **1** :
  - Choix de conception à confirmer : le bloc « au-delà des int » et le
    pown « dans les int » sur x coupé à [0, +oo] sont dans `pow_standard()`,
    où `gaol_pow_hybrid()` ne les atteint jamais puisqu'il prend `[n]`
    avant, et non dans `gaol_ieee1788::pow`, qui aurait dû refaire la coupe
    de x et le cas x = {0}. Dire si l'autre choix est voulu. (#37)
  - `pown([-2, 3], 100)` différait entre SSE2 et FPU avec GCC 9.4 à
    `a2ca992` (borne haute `…cdddp+158` contre `…cddbp+158`) ; GCC 13 donne
    `…cddbp+158` partout. À revérifier avec GCC 9.4 : le point 8 ne met
    d'accord que les produits arrondis. (#37)
- **5** :
  - `.github/audit` (`compare.py:25`, `make_probe.py:15`) compare
    `__FAST_MATH__` mais pas `__FINITE_MATH_ONLY__`, sans effet tant que
    `-fno-fast-math` lui-même est comparé. (#39)
  - Facultatif : le tableau des cibles de `doc/building.md` (l. 280) ne dit
    pas que le `make test` de CMake lance aussi les tests de compilation,
    ce que disent `doc/tests.md` et `doc/three-builds.md`. (#39)
- **7** :
  - La puce proposée pour `doc/differences.md` (GAOL v5 donnait
    `[DBL_MAX, +oo]` pour `atanh([1])` et `atanh([1, 5])` ; GAOL 4 non
    mesuré) : la garder, l'adapter ou la retirer dans la pull request de
    synthèse. (#31)
  - Manuel, entrée `atanh` : la phrase sur le domaine ]−1, 1[ doit-elle
    porter `\newinvfive` ? Seul le paragraphe CORE-MATH qui suit la porte.
    (#31)
  - `doc/compare/special_cases.md` n'a que `atanh([-1, 1])` et
    `atanh([2, 3])` (cas 124 et 125) ; ajouter `atanh([1])` et `atanh([-1])`
    à `doc/compare/code/cases.py` oblige à relancer les cinq bibliothèques.
    (#31)
  - Tests de `atanh_rel` (`tests/reverse_mappings.cpp`) : un `TEST_EQ` à
    borne infinie (`atanh_rel([0.5, 1], [0, +oo])`) est possible depuis le
    point 10 ; aucun test ne protège la clause `J.right() == -1.0` de
    `atanh()`, ces cas étant déjà vides par le constructeur. (#31)
- **9** : le drapeau `narrower_than_pi` est supprimé et `tan()` teste
  `!(w <= pi_dn)` (même comportement) : le garder, ou rétablir le drapeau
  avec `(w <= pi_dn)`, à la lettre de la correction proposée. (#36)
- **10** :
  - `hausdorff()` à borne infinie coûte environ 14 ns au lieu de 4,5 ns ;
    une sortie anticipée est possible (+oo dès qu'une borne n'est infinie
    que d'un côté), la formule unique a été gardée. (#41)
  - `doc/differences.md` (l. 288, puce `hausdorff()`) : les bornes infinies
    et `nb_fp_numbers()` avec −0 en puce à part, ou fusionnés, dans la pull
    request de synthèse ? (#41)
  - Manuel, entrée `hausdorff` (`gaol.tex` l. 3028 et 3033) : deux
    `\newinvfive` de suite ; fondre le paragraphe des bornes infinies dans
    la première phrase ? (#41)
- **13** : `[a]` suppose que `bound_to_text()` arrondit exactement vers
  l'extérieur (deux textes égaux sont le double) ; non vérifié sous
  `std::hexfloat`, où la bibliothèque C écrit les chiffres dans le sens
  d'arrondi courant. (#43)
- **14** :
  - `what()` rend `explanation_.c_str()` : une explication contenant un NUL
    est coupée (vide si le NUL est en tête), contre le « never empty » du
    commentaire ; GAOL n'en lève jamais : question de formulation. (#32)
  - `operator<<` d'une exception appelle `explanation()` deux fois (deux
    copies de la chaîne) ; sans conséquence. (#32)
- **16** : `d78af72` (30 tirages au lieu de 300 sans `NDEBUG`) attribue les
  300 s des jobs Visual Studio Debug à la lenteur des flux, alors que #58 a
  montré que `numbers` attendait sur la boîte de dialogue du runtime Debug.
  Remesurer, puis revenir à 300 tirages ou corriger le commentaire. (#57,
  #58)
- **39** : makeindex avertit d'entrées en conflit pour « canonical
  interval » dans le manuel (`\cindex{canonical interval|hyperpagebf}` et
  `\cindex{canonical interval}`, l. 1191 et 1199) ; le défaut est antérieur
  au point 39. (#53)
- **42** : que `WindowsApps` contienne un alias `python.exe` en plus de
  `python3.exe` n'a pas été vérifié sous Windows, alors que `meson.build`
  (l. 20), `doc/building.md` et le manuel l'affirment : le vérifier, ou
  adoucir le texte. (#44)
- **43** :
  - Aucune étape Windows native (meson avec Visual C++, sous `pwsh`) ne
    lance `.github/scripts/version-file.sh` ; seul le job meson MSYS2 le
    fait. Une étape `pwsh` est-elle voulue ? (#45)
  - Les `open(sys.argv[-1], …).read(…)` de `meson.build` (l. 32, 53 et 60)
    ne ferment pas le fichier ; sans effet sous CPython, un `with` au
    besoin. (#45)
- **Hors liste** :
  - GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` : le contournement
    (`no-inline-functions`) ne vaut que pour `__GNUC__ == 12 &&
    __aarch64__`, et d'autres versions pourraient être touchées sans que la
    CI le montre. Le défaut n'est pas signalé à GCC, faute de cas minimal :
    le réduire et le signaler ? (#34)
  - Clang 18 : `-Wunused-but-set-variable` sur `gaol_nerrs` dans le parser
    de bison 3.5.1 (`gaol/gaol_interval_parser.cpp` l. 1538). Le faire taire
    ou le laisser, à la régénération du parser que demandent les points 12
    et 45 ? (#39, #40, #41, #42, #43)
  - Manuel : `Overfull \hbox` de 22,4 pt dans le paragraphe du paquet
    `libgaol-dev_version_arch.deb` (`gaol.tex` l. 511-515). (#40, #44)
  - `doc/tests.md` : des lignes de plus de 100 colonnes (34, 62, 243, 245,
    253) parmi des lignes d'environ 80, et une ligne orpheline (226 : « With
    flush-to-zero, denormals-are-zero or both set in MXCSR »). (#39, #41,
    #42)
  - `tests/input_output.cpp`, `test_input()` : `<-inf,-inf>` lève
    `input_format_error`, que le `try` attrape et se contente d'afficher ;
    les six assertions suivantes ne s'exécutent jamais. Retirer le
    `try/catch`, et corriger ou retirer les deux cas `<inf,...>`. (#43)
  - `numbers` sur « macOS 15 x86_64 GCC Debug ASan+UBSan » : 235 s pour un
    délai de 300 s dans `61c7503`, malgré les 30 tirages de `d78af72` en
    Debug, et un run de #50 a dépassé le délai. L'alléger encore, ou relever
    le délai ? (#50)
  - L'étape de CI « VERSION.txt read by autoconf, as by configure »
    (`build-systems.yml`, `if: runner.os == 'Linux'`) coûte environ 10 s sur
    quatre entrées Linux, dont les deux à sens d'arrondi préservé : la
    limiter à `matrix.cfg.configure == ''` ? (commit direct)
  - Le message d'autoconf sur un `VERSION.txt` refusé écrit `?` pour tout
    caractère autre que chiffres, points et blancs (`tr -c '0-9. \n' '?'`,
    `configure.ac` l. 52), pour protéger m4 ; les octets en hexadécimal
    restent exacts. Une citation exacte demanderait des quadrigraphes : la
    garder ainsi ? (commit direct)
  - Le manuel ne donne pas le cas résiduel de WindowsApps (profil dont le
    répertoire diffère de `USERPROFILE`) que donne `doc/building.md`
    (l. 263-268) ; son « turn off the aliases » le couvre : l'ajouter, ou
    laisser. (commit direct)

## Ménage

- **Branches à supprimer sur GitHub.** Fusionnées : `todo-01-pow-norm-half`
  (#37), `todo-05-finite-math-only` (#39), `todo-29-ci-comma-locale` (#35)
  et `remove-string-constructor` (#30). Jetables : `ci-debug-numbers-arm64`
  (#34), `ci-debug-mingw-numbers` (#48),
  `copilot/fix-visual-studio-2026-x86-release-job` (#38, fermée) et
  `claude/todo-md-contents-xfervz` (#52, fermée). Abandonnée :
  `fix-path-core-math` (21 septembre, travail `todo-review-fixes` jamais
  fusionné) : `exact_string` en a été repris par 636daf7, sa refonte de
  `pow` remplacée le soir même par les espaces de noms `gaol_core`, `gaol`
  et `gaol_ieee1788` (d6f5270) puis par `pow_standard()` (#37), et ses
  `3rd/core-math-patches` par les correctifs écrits dans `3rd/README.md`
  (#56). Puis chaque branche de « En cours » une fois fusionnée.
- **L'issue #49** (suivi d'ensemble) est restée à l'état du 30 septembre au
  soir : 47 « en relecture », ni #58 ni #59, pas de point 49, la correction
  du 48 par `GAOL_RND_KEEP` (qui ne corrige rien, voir le point 48),
  « Vérifier la CI de `configure-clean` après les fusions de #53 à #57 »
  non coché, et une liste de branches à
  supprimer dont il ne reste que `todo-01-pow-norm-half`,
  `todo-05-finite-math-only`, `todo-29-ci-comma-locale` et
  `ci-debug-numbers-arm64`. Y reporter #58, #59, le point 49 et la CI de
  `61c7503` (rouge sur armhf et sur les deux jobs clang-cl seulement), et
  remplacer sa liste de branches ; ou la fermer en renvoyant à ce
  `TODO.md`.
- **Les scripts et consignes de la méthode**, dans
  `todo-notes/orchestration/` de la branche `todo-status` : les garder ou
  les supprimer.
- **La CI tourne deux fois par pull request** : chaque workflow se lance sur
  `push` (toutes les branches) et sur `pull_request`. Le push de `61c7503` a
  lancé 114 jobs (25 Linux, 31 Windows, 11 macOS, 17 conteneurs, 30
  autotools et meson), environ 380 minutes de runner (durées des jobs
  additionnées) ; ne lancer `push` que sur les branches principales
  diviserait la charge par deux.
- **Les descriptions des pull requests** #50, #51, #53 à #57 et #59, et un
  commentaire de #59, finissent par une ligne qui crédite l'outil qui les a
  écrites et un lien : les retirer, et étendre aux pull requests la règle
  qui l'interdit dans les commits ?
- **La pull request de synthèse**, une fois les branches de « En cours »
  fusionnées :
  - consigner les changements dans `ChangeLog` et `doc/differences.md`, que
    rien n'a touchés depuis le 28 septembre (les textes proposés sont dans
    les rapports de `todo-notes/`) ;
  - mettre `examples/examples.md` à jour : marquer **Fixed** les n° 3, 4, 5,
    8, 10, 12, 15, 19, 20 et 21 de la section 5 et de l'annexe B, et la
    ligne vide du n° 14 ; l'annexe B n° 10 (l. 1117-1122) propose encore une
    correction d'`interval(const char*)`, qui n'existe plus : dire que le
    n° 10 est corrigé par la suppression des constructeurs à partir de
    chaînes (#30) ; les phrases qui comptent les numéros corrigés (l. 607,
    903 et 1010) ; les points écrits `[a]` et non `<a, b>` (l. 62,
    §2.7 l. 464, annexe B l. 1131) ; les lignes vides lues (l. 61-63,
    457-459, 634) ; le format `width` (l. 566) ; et ce que disent les points
    5, 15, 16, 39 et 40 ;
  - régénérer une seule fois `manual/v5/gaol.pdf`, dont la dernière version
    date du 28 septembre.
