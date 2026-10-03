# À faire

Ce qui reste à faire sur GAOL v5 au commit `a29c7b3` de `configure-clean`. Les
points gardent les numéros de la liste d'origine : un numéro qui manque est un
point fait. Le détail de chaque point, les questions ouvertes des pull requests
fusionnées et le suivi (rapports, avancement) sont dans la branche `todo-status`
(`todo-notes/2026-10-02/reste-detaille.md`, `TODO.md`, `todo-notes/`) ; la
pull request de synthèse mettra à jour `ChangeLog` et `doc/differences.md`.
Les relectures des pull requests fusionnées ont aussi laissé de petites
corrections de commentaires, de documentation et de tests : celles qui
restaient à faire le 3 octobre sont les points 51 à 67, et celles de
`3rd/README.md`, à faire avant l'envoi à CORE-MATH, sont au point 31.

Les points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro n
de sa section 5) ; les points 41 à 44, de la vérification de `VERSION.txt`
(2026-09-28) ; 45 à 49, des corrections et des relectures ; 50, d'une décision
du 3 octobre ; 51 à 67, des relectures des pull requests. Les questions ouvertes
ont été tranchées le 3 octobre : la décision est écrite dans chaque point
(« Décidé le 3 octobre »), et une question reportée renvoie à son issue
(#64 à #70).

## En cours

Ces branches sont poussées, mais pas fusionnées dans `configure-clean`. Les
points 3, 4 et 8, et la correction d'armhf du point 24, ont été fusionnés le
3 octobre (#63, #62, #61 et #60).

- **12**, `todo-12-long-sums` (553e649) : inachevé (commits « WIP ») ; restent
  la fin du travail, son rapport, la relecture, la CI et la pull request.
- **17**, `todo-17-output-speed` (3b9311c) : inachevé (« WIP ») ; restent les
  mesures, la vérification du texte écrit, le rapport, la relecture, la CI et
  la pull request.
- **36**, `todo-36-upward-rounding-effects` (1559af5) : inachevé (« WIP » fait
  sur `a2ca992`, conflit avec `configure-clean` dans `doc/using.md`) ; voir le
  point 36 ; restent aussi la fusion de `configure-clean`, la relecture et la
  pull request.

## Code

1. **Suites du pow de la norme écrit une fois** (#37). `pow_standard()`
   (`gaol/gaol_interval.cpp`) est la seule copie du pow de la table 9.1. Deux
   vérifications sont inutiles : la garde de l'exposant `[±oo]` de
   `gaol_pow_hybrid()`, inatteignable, et le cas `at_upper == 1.0` du bloc
   au-delà des int de `pow_standard()`. Les retirer reprendrait une part des 2
   à 3 % perdus par `gaol::pow` à exposant non entier (mesurés avec GCC 9.4
   seulement) ; le `is_empty()` de `gaol_pow_hybrid()` est à garder.
   Décidé le 3 octobre : le bloc au-delà des int et le pown dans les int sur x
   coupé à [0, +oo] restent dans `pow_standard()`, où `gaol_pow_hybrid()`,
   qui prend `[n]` avant, ne les atteint jamais, et non dans
   `gaol_ieee1788::pow`.

2. **Le sens d'arrondi est encore vérifié plus d'une fois** : trois fois par
   `pow(x, y)` quand `pow_standard()` passe par exp(y log x) (une borne
   infinie, ou une base partant de 0 avec un exposant qui n'est pas au-dessus
   de 0), deux fois par `nth_root(x, q)` pour q < 0 et par `modulo_k_pi()`.
   Correction : appeler les corps de ces opérations après une seule
   vérification, comme `tan()` et les puissances négatives ; les corps de
   `log()`, `exp()` et `*` sans la vérification restent à écrire. Décidé le
   3 octobre : les deux, dans cet ordre ; pour `pow`, après le point 3,
   étendre ensuite l'analyse des coins aux bornes infinies et à une base
   partant de 0 : plus d'exp(y log x), et des boîtes
   serrées (aujourd'hui, `pow([4, +oo], 0.5)` vaut [2 − 2^-52, +oo], et
   d'autres ont des centaines de doubles de trop), avec des tests aux limites.

3. **`pow(x, y)` a la largeur d'un double là où la puissance en un coin est un
   double** : `pow([4], 0.5)` vaut [2 − 2^-52, 2], et les cas 149 à 152 et 163
   de [doc/compare/special_cases.md](doc/compare/special_cases.md) sont plus
   larges que le résultat d'IEEE 1788. Fait par #63 : `pow_is_double()` prouve
   par des opérations exactes que x^y est un double, et `pow_lo()` le prend
   alors comme borne inférieure ; la colonne GAOL des cas 184 à 188 de
   `special_cases.md` et le commentaire de `pow_standard()` sont corrigés.
   Restent, décidés le 3 octobre (#63) : la racine carrée de SSE2 sur tout x86
   32 bits, et non plus sur Windows 32 bits seulement ; pas de recherche de cas
   durs pour MinGW-w64 x86 avant #63.

<!-- -->

50. **`hausdorff()` à borne infinie** (suite de #41) : environ 14 ns au lieu
    de 4,5 ns, la formule unique ayant été gardée. Décidé le 3 octobre :
    ajouter la sortie anticipée (+oo dès qu'une borne n'est infinie que d'un
    côté).

## Bornes fausses

4. **Le flush-to-zero et le denormals-are-zero rendent les bornes fausses**
   (revue n° 2) : `[1e-300] * [1e-20]` vaut [0, 0] dans un programme lié avec
   `-Ofast` (par `crtfastmath.o`) ou qui charge un plug-in compilé ainsi. Fait
   par #62 : une sonde sous-normale qui efface FTZ et
   DAZ (FZ sur ARM), `-mno-daz-ftz` à l'édition de liens, un test et la
   documentation. Décidé le 3 octobre : `gaol.pc` garde `-mno-daz-ftz`, qui
   arrête l'édition de liens avec Clang 18 ou un GCC antérieur à 11.4 (c'est
   documenté) ; GCC 9.4 sous `-funsafe-math-optimizations` réduit la sonde,
   même sous-normale, à `tiny == 0.0` : c'est seulement documenté, sans double
   `volatile` ; le moins unaire reste sans sonde (#62). Restent : mesurer sur
   les processeurs de la CI le coût de la sonde (+0,5 ns sur `x * y` sur un
   Xeon) ; ARM avec Visual C++ ou clang-cl, et FIZ, ne sont pas couverts ;
   `-fno-signed-zeros` fait sauter le `+ 0.0` de la sonde dans le code du
   programme (seulement documenté). La suite est
   au point 45.

5. **Suites du refus de `-ffinite-math-only`** (#39), fait avec ses tests
   CMake. Restent : avec GCC, `-Ofast` n'est pas refusé dès que
   `-fno-fast-math` est sur la ligne de commande, avant ou après, alors que
   `doc/three-builds.md` et `doc/using.md` le disent refusé ; le `#error` de
   `__FAST_MATH__` ne donne pas de remède, contrairement à celui de
   `__FINITE_MATH_ONLY__` ; `tests/refused_options.cpp` n'a pas de témoin
   positif (un test sans option, environ 1 s), et ses bornes constantes, et non
   `volatile`, laissent Clang calculer l'intersection à la compilation ; ce
   qu'aucune macro ne révèle (`-funsafe-math-optimizations`,
   `-fno-honor-nans`, les pragmas) n'est que documenté, et une vérification à
   l'exécution dans `gaol/gaol_init_cleanup.h` reste à essayer. Décidé le
   3 octobre : ajouter le témoin positif, et essayer cette vérification
   (prototype, ce qu'elle détecte et son coût, puis la garder ou non).

6. **Suites de `fegetround()` lu dans MXCSR et du refus de MinGW-w64** (#54,
   #51). Décidé le 3 octobre : ne pas lire MXCSR sur x86 32 bits avec SSE2 ;
   supprimer `GAOL_RND_MINGW_FENV_ONLY` (mingw-w64 11 i686 passe ctest sans
   lui sous wine) ; refuser mingw-w64 ARM64 avant 12 ou sans `_UCRT`, comme
   sur x86-64 (le refus actuel de mingw-w64 ARM avant 11 y est inclus) ;
   garder le refus du x86 32 bits avant 11, par version ; taire le `#warning`
   de `cbrt.c`, `rsqrt.c` et `asinpi.c` (changement `/* GAOL */`) en attendant
   le point 31. Reporté : signaler à mingw-w64 son `fma()` et son `round()`
   pour msvcrt (#69). Le `fma()` logiciel de `ucrtbase.dll` n'a jamais été
   vérifié.

7. **Suites de `atanh([1, x])`** (#31) : un `TEST_EQ` de `atanh_rel` à borne
   infinie (`atanh_rel([0.5, 1], [0, +oo])`), possible depuis le point 10 ;
   `atanh([1])` et `atanh([-1])` manquent à `doc/compare/special_cases.md`
   (relancer les cinq bibliothèques) : décidé le 3 octobre, les ajouter au
   moment du point 33.

8. **`pow(x, n)` pour n grand** (revue n° 7) : dans `ipow_exact_dn()`, la borne
   inférieure perd le carré du reste (1962 doubles sous la plus serrée pour
   n = 2^32 − 1), et les builds SSE2 et FPU multiplient les produits arrondis
   dans des ordres différents, contre « les mêmes bornes sur toutes les
   machines » de `doc/accuracy.md`. Fait par #61. Une borne nulle, ou une seule borne hors de
   la plage, envoie les deux bornes aux produits arrondis (jusqu'à 10 doubles de
   trop). Décidé le 3 octobre : rendre `ipow_exact_dn(0)` exact, sans traiter
   chaque borne à part ; garder l'ancien code SSE2 sous `#if 0` ; garder la
   garantie 5 n log2(n) 2^-104. `pown([-2, 3], 100)` différait entre SSE2 et FPU
   avec GCC 9.4 (#37) : à revérifier.

9. **Suites de `tan([-M_PI_2, M_PI_2])`** (#36). `tan()` donne maintenant
   ±1,63·10^16. Un intervalle sans pôle dont la largeur exacte est entre
   `pi_dn` et π donne encore [-oo, +oo] ; on n'en connaît aucun, et le rendre
   serré demanderait de comparer `r - l` à π en double-double. Décidé le
   3 octobre : `doc/accuracy.md` dit le cas théorique, et `tan()` garde
   `!(w <= pi_dn)` sans le drapeau `narrower_than_pi`. Que les tests
   `w < pi_dn` de `cos_or_sin()` donnent le plus serré est mesuré, pas
   démontré : décidé, le dire mesuré (29 400 intervalles) dans le commentaire
   et dans `doc/accuracy.md`.

<!-- -->

11. **Suites du lecteur sous denormals-are-zero** (#40). Le lecteur est juste
    sous FTZ et DAZ (comparaisons sur les bits) ; le reste est au point 45.
    Restent : vérifier une fois, avec `ctest -V -R numbers`, si les tests DAZ
    de `numbers` s'exécutent ou se sautent sur macOS x86_64 sous Rosetta et
    avec l'UCRT en Release ; dans le manuel, la phrase sur ces modes
    garde sa marque `\newinvfive` (décidé le 3 octobre). Décidé aussi (#40) :
    faire tourner les tests DAZ de `numbers` sur AArch64 et ARM32 (FPCR,
    FPSCR), avec l'outil de `tests/gaol_tests.h` venu de #62.

<!-- -->

45. **Le denormals-are-zero fausse encore des bornes hors du lecteur** (suite
    du point 11) : des fonctions comparent des sous-normaux, que DAZ lit comme
    0, avant toute sonde. Le parser accepte `<1e-310, 1e-309>`,
    `interval(0x1p-1073, 0x1p-1074)` n'est pas vide, `bound_to_text()` écrit
    une borne sous-normale au plus proche (à un chiffre, [22·2^-1074] s'écrit
    `[1e-322]`, relu plus petit), et `log`, `sqrt`, `abs`, `div_rel`, `mid()`
    et les relations se trompent : depuis #62 (point 4), avant la
    première opération qui sonde, et à chaque appel avec
    `GAOL_PRESERVE_ROUNDING`. Correction : comparer les bits, ou sonder avant
    de comparer, et régénérer le parser. Question de #58 : retirer DAZ et FTZ
    le temps de l'écriture, gdtoa et le runtime Debug de Visual C++ écrivant 0
    un sous-normal sous DAZ. Les tests DAZ ne tournent pas sur ARM (FPCR.FZ).
    Reporté le 3 octobre : la correction et la question de #58 sont dans
    l'issue #68. Sous DAZ, `pow([0.5], [2^-1074])` vaut [1, 1] depuis le
    point 1 (#63) : vérifier s'il le vaut encore après #62 ; s'il le vaut, il
    relève de ce point.

<!-- -->

47. **clang-cl : ce qui reste après le correctif de `cbrt`, `rsqrt` et
    `asinpi`** (#59). Restent : refuser clang-cl sans `/fp:strict` (il compile
    alors en `-fno-rounding-math -ffp-contract=on`, et `gaol_config.h` ne refuse
    que Visual C++ ; `_M_FP_STRICT` existe à partir de Clang 16), et le vérifier
    dans `tests/fp_strict` (décidé le 3 octobre) ; Cygwin x64, hors CI, reste
    faux (`FE_*` de newlib, pas de `_WIN32`) : décidé le 3 octobre, prendre le
    correctif 6 de `3rd/README.md` en entier plutôt que refuser Cygwin ;
    citer clang-cl x64 dans le manuel et `doc/three-builds.md` une fois ses
    jobs verts (point 49) ; réserver `/Zc:strictStrings-` à Visual C++ (11
    avertissements par build clang-cl, dans `CMakeLists.txt` et
    `gaol/meson.build`). Décidé aussi (#59) : ajouter à la CI le clang-cl de
    Visual Studio 2026 x64, et clang-cl x86 et arm64.

48. **`cancel_minus` et `cancel_plus` sont faux avec la bibliothèque compilée
    par GCC à `-O3`** (le build Release par défaut) : dans
    `difference_at_least()` (`gaol/gaol_interval.cpp`), GCC déplace les termes
    d'erreur de `two_sum` après `GAOL_RND_NEAREST_LEAVE()`, où TwoSum n'est
    plus exact. `cancel_minus([0.5], [4.9e-324, 1e-300])` donne
    [0x1.fffffffffffffp-2, 0.5] au lieu de [-oo, +oo], et
    `cancel_minus([DBL_MAX], [0.5])` [-oo, +oo]. Correction (essayée) : faire
    passer `s1`, `e1`, `s2` et `e2` par `gaol_core::rnd_keep()` avant
    `GAOL_RND_NEAREST_LEAVE()` (`GAOL_RND_KEEP` ne corrige rien), un test avec
    ces cas et `cancel_minus([DBL_MAX], [1])`, qui lève aussi FE_INVALID, et
    le commentaire de `GAOL_RND_NEAREST_ENTER` (`gaol/gaol_fpu.h`) à corriger.

49. **Compilé par clang-cl, `GAOL_INFINITY` n'est pas l'infini quand le
    programme arrondit vers le bas ou vers zéro** : `gaol/gaol_port.h` le
    définit comme `HUGE_VAL`, que l'UCRT écrit `((double)(float)1e+300)`, et
    clang-cl fait cette conversion à l'exécution sous `/fp:strict`, ce qui donne
    FLT_MAX : `interval()` vaut [-FLT_MAX, FLT_MAX], `interval(1e300)` est vide
    (37 échecs de `rounding_direction` sous wine). Le défaut est aussi dans le
    code en ligne des en-têtes publics. Les deux jobs clang-cl de `windows.yml`
    en sont le test et restent rouges d'ici là. Décidé le 3 octobre :
    `std::numeric_limits<double>::infinity()`, dans sa propre pull request,
    pas encore faite.

## Plantages

12. **Les longues sommes débordent la pile** (revue n° 17) :
    `textToInterval()` d'une somme de 100 000 termes plante encore, comme les
    expressions aussi longues construites par l'API (une récursion par nœud
    dans l'évaluation et la destruction). La correction est dans la branche
    `todo-12-long-sums`, inachevée : chaque opération calculée dès sa lecture
    par le parser régénéré, l'évaluation et la destruction des expressions par
    des boucles, l'imbrication bornée par `YYMAXDEPTH` (au-delà,
    `input_format_error`) et `operator<<` d'une expression encore récursif
    (limite documentée). À la régénération, faire taire l'avertissement de
    Clang 18 sur `gaol_nerrs` (décidé le 3 octobre).

## Flux et texte

14. **Suites de `what()` des exceptions** (#32). `what()` renvoie
    l'explication. Restent : les constructeurs à `const char*` des classes
    dérivées transmettent tel quel à `std::string` un pointeur nul
    (comportement indéfini, qu'aucun appel de GAOL ne fait) : le traiter comme
    une absence d'explication (décidé le 3 octobre) ; le manuel ne documente
    que ces constructeurs. Décidé le 3 octobre : sans explication, `what()`
    renvoie le nom de la classe dérivée (`input_format_error`…), de même
    quand le texte C de l'explication est vide (un NUL en tête), pour que
    `what()` ne soit jamais vide ; la forme
    courte d'`operator<<` reste, sans réserve là où la documentation dit que
    GAOL 4 donnait `std::exception`.

15. **Suites de la lecture des lignes vides par `operator>>`** (#46), qui lit
    par `std::getline(is >> std::ws, buffer)` : une ligne vide est sautée, et
    `while (in >> x)` finit sans exception. Sous `exceptions(eofbit)`, un
    intervalle lu en fin d'entrée lève avec `eofbit` seul (un double reçoit
    `eofbit | failbit`), et une dernière ligne sans fin de ligne est perdue ;
    `std::ws` saute `\v` et `\f`, que le lecteur ne prend pas pour des blancs.
    Décidé le 3 octobre : on garde les deux comportements (le premier est
    documenté, et le lecteur ne change pas).

16. **Suites des formats d'affichage** (#57). Restent, vérifiés dans
    `61c7503` : dans le format `width`, `+2 (+/- +1)` sous `std::showpos`, un
    milieu groupé et un rayon non groupé sous une locale qui groupe les
    chiffres, et un milieu `-0` ; le format `agreeing`, qui écrit [1, 10]
    `1~[., 0.]` et les zéros avec leur signe, et ne donne de chiffres communs
    qu'aux intervalles positifs de rapport au plus 10 ; des tests (le format
    `width` sous une locale à virgule, un ordre d'évaluation non spécifié dans
    `tests/rounding_direction.cpp`, `gaol_tests::hex()` sous la locale
    globale) ; le `#include <sstream>`, inutile, à retirer de
    `gaol/gaol_ieee1788.h`. Décidé le 3 octobre : corriger les formats `width`
    (`std::showpos`, groupement, milieu `-0`) et `agreeing` (sa condition, et
    les zéros écrits `[0]`) ; `intervalToText` suit `interval::precision()` ;
    le format `hexa` écrit lui aussi un point `[a]`, comme le décimal, et non
    plus `[a, a]` bit à bit (à vérifier sous `std::hexfloat`), et un point nul
    s'écrit `[0]` quels que soient les signes ; sous
    une locale à virgule, où -2.5 s'écrit `[-2,5, -2,5]`, que le lecteur
    refuse, la garde de `display_bounds()` teste `numpunct::decimal_point()` et
    le texte s'écrit avec un point ; pas de format dont le rayon absorbe
    l'arrondi du milieu.

17. **`operator<<` est plus lent depuis qu'il écrit dans un
    `std::ostringstream` à lui** : environ 450 ns de plus par intervalle
    (mesurés avant #57), et `std::internal` complète comme `std::right`. Le
    travail est dans la branche `todo-17-output-speed`, inachevée : le texte
    est formé dans une chaîne, et `std::internal` remplit entre le signe et
    les chiffres du milieu. Reste à mesurer le gain sur des programmes qui
    écrivent beaucoup d'intervalles, sur une machine au repos, et à vérifier
    que le texte reste identique octet pour octet à celui de `configure-clean`
    (formats, précisions, drapeaux, locales).

18. **Suites de la lecture des longs nombres** (#42). Le texte d'un nombre est
    analysé une fois : 20 000 caractères se lisent en 0,02 s sous toutes les
    locales, et l'analyse des chiffres reste quadratique (décision du 30
    septembre). Décidé le 3 octobre : cette décision vaut aussi pour la forme
    incertaine, dont les sommes passent par `gaol_decimal_add_sub()`, qui
    insère en tête d'une `std::string` (100 000 chiffres : 1,1 s, contre
    0,46 s) ; on n'y touche pas.

<!-- -->

46. **`operator>>` ne relit pas `os << x << ' ' << y`** (suite du point 15) :
    il lit un intervalle par ligne, et la fin d'un intervalle dans un flux est
    ambiguë, un intervalle contenant des espaces et la grammaire admettant des
    littéraux imbriqués et des expressions (`[[1, 2], 3]`, `1 -2`). Il faut
    d'abord décider de la syntaxe acceptée : un intervalle par ligne (ce qui
    est documenté), les seuls littéraux lus jusqu'au crochet fermant, ou une
    fonction à part. Reporté le 3 octobre : issue #64.

## En-têtes

19. **`gaol::sin(0.5)` ne compile plus** (revue n° 16) : chaque fonction de
    l'espace de noms `gaol` sur un double est ambiguë entre les surcharges pour
    les intervalles et pour les expressions, parce que `gaol/gaol` inclut
    `gaol_ieee1788.h`, qui inclut `gaol_expression.h` ; GAOL 4 le compilait.
    Correction : ne plus inclure `gaol_expression.h` depuis `gaol_ieee1788.h`,
    et déplacer ses deux surcharges d'expressions (`pown(e, n)`, `pow(e1, e2)`)
    à la fin de `gaol_expression.h`, en gardant le choix des surcharges dans les
    deux ordres d'inclusion et en corrigeant le commentaire d'en-tête de
    `gaol_ieee1788.h`.

20. **Les en-têtes publics débordent dans le programme** (revue n° 22). Les
    trois builds installent tous les en-têtes de `gaol/`, internes compris :
    `gaol_interval_parser.h` ne compile pas quand on l'inclut, ni
    `gaol_allocator.h` seul. `gaol_exceptions.h` met `using std::exception;`
    et `using std::string;` à la portée globale, et les en-têtes définissent
    des macros sans préfixe (`INLINE`, `MEMALIGN`, `__HI`, `HAVE_FENV_H`…).
    Avec `-Wall -Wextra` et un simple `-I`, un programme reçoit 35
    avertissements de GCC 13 (`-Wunused-parameter` dans `gaol_expr_visitor.h`,
    `-Wdeprecated-copy` à chaque `x = ...;`). Correction : ne plus installer
    les en-têtes internes, supprimer les `using`, préfixer les macros, déclarer
    l'affectation de copie par défaut, ne pas nommer les paramètres inutilisés.

## Interface

21. **Les entiers au-delà de 2^53** : `interval(0)`, `interval(0, 0)`,
    `x = 0`, `x < 0` et `max(x, 0)` compilent, les constructeurs
    `interval(double)` et `interval(double, double)` n'étant pas `explicit`
    (essayé le 28 septembre puis retiré) : l'entier devient un double, et un
    entier au-delà de 2^53 un double qui ne le contient pas. Correction : des
    constructeurs templates sur les types entiers, contraints, dans les
    en-têtes, sans changement d'ABI ; pas un simple `interval(int)`, qui rend
    `interval(5L)` ambigu.

22. **Pas d'ordre total pour les conteneurs** : `<`, `<=`, `>` et `>=` sont
    les relations « certainement » d'IEEE 1788, vraies dès qu'un des deux
    intervalles est vide : `std::set` perd les intervalles qui se chevauchent,
    et `std::sort` d'un vecteur contenant un intervalle vide lit au-delà de sa
    fin. Correction : un `gaol::lexicographic_less` (spécialiser aussi
    `std::less` ou non : reporté le 3 octobre, issue #70), et une mise en garde
    dans `doc/using.md` et
    le manuel contre `std::sort`, `std::set`, `std::max`, `std::min` et
    `std::clamp` sans comparateur.

23. **Gérer le sens d'arrondi** : `gaol::cleanup()` ne le restaure qu'à son
    premier appel, il n'y a pas de garde à portée, et `rnd_keep()` n'est pas
    présenté comme une barrière. Correction : un `gaol::restore_rounding()`
    qu'on peut appeler autant de fois qu'on veut, une garde qui calcule un
    bloc au plus proche (environ 7 ns) et `rnd_keep()` documenté ; à faire
    avec le point 36, dont la branche documente déjà `rnd_keep()` et écrit une
    telle garde dans `examples/13_rounding_environment.cpp`.

24. **Exceptions flottantes : ce qui reste après #47 et #50.** Après
    #60, `is_empty()` reste compilé en
    `vcmpe` sur armhf dans le code du programme : l'écrire avec
    `std::isunordered()` sur ARM 32 bits (décidé le 3 octobre).
    Avec `GAOL_PRESERVE_ROUNDING`, les opérations SSE2 masquent de nouveau les
    exceptions du programme et effacent ses indicateurs : à corriger. `0 × oo`
    dans l'`operator*=` SSE2 et le `pow` de CORE-MATH pour un exposant extrême
    lèvent FE_INVALID, sans test : les corriger, avec un test et le coût de `*`
    mesuré (décidé le 3 octobre). `gaol_intervalf.h` appelle
    `std::islessequal` sans inclure `<cmath>`. Décidé le 3 octobre : la
    première comparaison de `interval(l, r)` devient silencieuse, ce qui rend
    gratuits `floor`, `ceil` et `integer` (`interval(NAN)`, `x += NAN` et
    `set_contains(NAN)` ne lèvent plus FE_INVALID) ; `x &= y` s'écrit avec un
    seul `std::isunordered()` (à vérifier avec Clang 18, et son coût sous
    Visual C++ à mesurer) ; les intervalles de flottants (`gaol_intervalf.h`,
    `gaol_interval2f.h`) gardent leurs comparaisons signalantes, mais
    `intervalf::is_empty()` reste silencieux (#47). Décidé aussi (#60) :
    `is_empty()` écrit avec `std::isunordered()` sous `_ARCH_PWR9`, comme sur
    ARM 32 bits ; les relations en ligne (`set_le`, `set_strictly_contains`,
    `less`…) corrigées pour ne plus lever FE_INVALID, même dans une boucle
    vectorisée (AArch64, intervalles FPU x86-64) ; les trois vérifications de
    choix de #60, fragiles par nature, gardées.

25. **Les outils que chaque algorithme réécrit** : `mulRevToPair` (un
    `div_rel` en deux morceaux), `inflate`, `bisect(ratio)` et
    `is_bisectable()` (`split()` coupe au milieu, ±DBL_MAX pour une
    demi-droite), un constructeur milieu-rayon, `hull()` et `intersect()`
    comme fonctions, un encadrement de la largeur (`width()` est un majorant),
    un littéral `_iv` dans un espace de noms `gaol::literals`, `erf` et `erfc`
    (leurs sources sont dans `3rd/math-core`, mais aucun build ne les
    compile), et les fonctions réciproques de `atan2`, `pow(x, y)`, `max`,
    `min`, `sign` et `floor`, qu'IBEX écrit lui-même.

26. **Des décorations**, ou au moins un indicateur disant qu'un argument est
    sorti du domaine : `sqrt` d'une boîte négative est vide et passe tous les
    tests d'inclusion, `interval(DBL_MAX*10)`, `x + INFINITY` et `x * NAN`
    sont vides sans rien dire, et `gaol_ieee1788::textToInterval` rend
    l'ensemble vide pour un texte mal formé. GAOL n'a que des intervalles nus
    (clause 11 d'IEEE 1788-2015). À choisir : les décorations, ou un simple
    indicateur (reporté le 3 octobre, issue #67).

27. **Un sens d'arrondi qui ne fuit pas** : l'arrondi porté par chaque
    instruction (AVX-512, le FPCR d'AArch64 en assembleur), comme le fait
    inari, trois fois plus rapide sur les additions ; dans la direction de
    P2746. Aujourd'hui GAOL laisse l'arrondi vers le haut au programme
    (point 36) ou, avec `GAOL_PRESERVE_ROUNDING`, le change et le rend à
    chaque opération, plusieurs fois plus lentement.

28. **Des en-têtes optionnels au-dessus du cœur scalaire** : promouvoir
    `examples/box.h`, `examples/dual.h` et `examples/affine.h` (un type
    boîte, la différentiation directe, les formes affines, dont se servent les
    exemples 03 à 14 et qui ne sont pas installés) en `gaol/box.h`,
    `gaol/dual.h` et `gaol/affine.h` ; ou des traits GAOL pour YalAA et un
    backend intervalle pour VNODE-LP. Décidé le 3 octobre : les traits pour
    YalAA et le backend pour VNODE-LP, et non la promotion des en-têtes des
    exemples.

## Tests et intégration continue

29. **Le test sous une locale à virgule n'est vérifié que dans `linux.yml`**
    (#35) : macOS, Windows, les conteneurs et `build-systems.yml` ne génèrent ni
    ne vérifient de locale à virgule (Debian demande le paquet `locales` ;
    Alpine et manylinux n'en ont pas). Décidé le 3 octobre : un contrôle
    (`.github/scripts/comma-locale.sh check`) après les tests là où la locale
    existe. Au passage : `numbers` n'a pas de `TIMEOUT` dans
    `tests/fetch_content` et `tests/find_package`, et prend 235 s de ses 300 sur
    macOS x86_64 Debug ASan+UBSan (délai dépassé une fois dans #50) : décidé, un
    `TIMEOUT` de 400 s pour ce job, sans alléger les tests ; le passage de 300 à
    30 tirages en Debug (`d78af72`) reste (décidé le 3 octobre), mais son
    commentaire, qui l'attribue à la lenteur des flux, est à corriger : #58 a
    montré que les jobs Visual Studio Debug attendaient une boîte de dialogue ;
    les jobs Ubuntu 22.04 de `linux.yml` passent sur `ubuntu-24.04` (décidé)
    avant le 17 avril 2027.

30. **Des suites de tests et des bancs d'essai où GAOL est absent** : passer
    ITF1788 (toutes les opérations d'IEEE 1788 ; seuls les cas des fonctions
    réciproques sont repris, dans `tests/reverse_values.py`) et le banc
    d'essai de Tang et al. (2021) sur GAOL v5, et ajouter à `doc/compare/`
    Boost.Interval, la bibliothèque que les utilisateurs prennent d'abord,
    dont les fonctions élémentaires ne sont pas sûres.

## CORE-MATH

31. **Envoyer à CORE-MATH les correctifs écrits dans
    [3rd/README.md](3rd/README.md)**, dont la section « Changes to propose
    upstream » donne six correctifs contre son master `b1a4badf`, vérifiés
    avec `./check.sh --worst` et `--special` ; la glibc n'a aucun de ces
    défauts. Rien n'est envoyé. Décidé le 3 octobre : le correctif 6 sous sa
    forme large (la table), après examen de `binary80/pow/powl.c` ; changer
    dès maintenant les `sinpi.c` et `log1p.c` embarqués (`/* GAOL */`, comme
    sinh, cosh et tanh) ; envoyer aussi le patch 7 (`exact_pow()`, #63).
    Reportés : qui envoie et comment (merge request sur
    gitlab.inria.fr, ou `git format-patch` à core-math@inria.fr), #65 ; y
    joindre ou non l'échec du master à `./check.sh --worst --rndd pow` et la
    borne de `ss` dans `asinpi_acc()`, #66.
    Corriger d'abord les phrases inexactes de `3rd/README.md` relevées par les
    relectures de #54, #56 et #59.

## Documentation

32. **Le rapport de couverture** ([coverage/README.md](coverage/README.md)),
    refait le 2026-09-27 à `605728e` (87,2 % des lignes de `gaol/`), ne
    correspond plus aux sources : 47 commits ont changé `gaol/` depuis. Le job
    « Coverage » de `linux.yml` le refait à chaque push, mais pas la copie du
    dépôt. À refaire au commit de la version (`-DGAOL_COVERAGE=ON`, cible
    `coverage`, gcovr) et à enregistrer.

33. **Les temps de [doc/compare/performance.md](doc/compare/performance.md)**
    ont été remesurés le 2026-09-27 à `605728e`, sur un portable où d'autres
    programmes tournaient, et les opérations ont changé depuis (#37, #50,
    #54). À remesurer en dernier, sur le commit propre de la version, la
    machine ne faisant rien d'autre (`doc/compare/code/run_bench.sh`, ou
    `make perf`), puis reprendre le tableau de `doc/compare/README.md`.

34. **Les recettes FetchContent récupèrent GAOL 4** :
    `doc/using.md`, le manuel et `tests/fetch_content` prennent la branche
    `master` de `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la
    branche par défaut. `master` est GAOL 4.3.2, avec lequel les programmes de
    la documentation ne compilent pas, et il n'y a pas d'étiquette `v5.0.0`.
    Décidé le 3 octobre : une fois `configure-clean` fini, fusionner
    `configure-clean` dans `MATH-CORE`, puis `MATH-CORE` dans `master`, puis
    étiqueter `v5.0.0` ; les recettes et le test gardent `master` jusqu'à
    l'étiquette, puis la citent.

35. **Un premier programme avant les détails** : `README.md` n'a ni code C++
    ni renvoi à `examples/`, le premier programme est à la ligne 96 de
    `doc/using.md`, et aucun document ne montre un algorithme sur les
    intervalles. Correction : un programme de dix lignes dans `README.md` qui
    renvoie à `examples/` et à `examples/examples.md`, et un court tutoriel
    (encadrement d'image et subdivision, Newton avec `%` ou `div_rel`, un
    contracteur avec les fonctions `*_rel`, séparation et évaluation), tiré
    des exemples 03, 05, 06 et 07.

36. **Ce que l'arrondi vers le haut fait au programme**, dans `doc/using.md`
    et dans « Common errors » du manuel, avec la table de la section 2.8 de
    `examples/examples.md` (`printf`, `strtod`, `lrint`, les allers-retours
    par le texte, TwoSum, un double calculé avant `cleanup()` et réutilisé
    après), et `GAOL_PRESERVE_ROUNDING` avec son coût. Commencé dans la
    branche `todo-36-upward-rounding-effects` (une section de `doc/using.md`
    et des ajouts à `examples/13_rounding_environment.cpp`). Restent : le coût
    de `GAOL_PRESERVE_ROUNDING` (la section finit sur `XXCOSTXX`), le manuel,
    et les mesures de TwoSum et TwoProd en arrondi dirigé (exacts avec `fma`).

37. **Une table des noms** pour les utilisateurs d'IBEX, de Codac, de C-XSC,
    de Boost et d'IEEE 1788, dans `doc/using.md` ou le manuel, avec ce que
    `<`, `<=` et `==` signifient dans chacun : dans C-XSC et PROFIL/BIAS, `<=`
    est l'inclusion, et le code porté depuis eux compile et change de sens.
    La section 2.3 de `examples/examples.md` en donne une pour GAOL v5,
    `gaol_ieee1788`, IBEX et Codac seulement.

38. **Les pièges dans les erreurs courantes du manuel** (« Common errors » de
    `manual/v5/gaol.tex`, section 5.3 de `examples/examples.md`) : `sqrt(2)`
    sur un nombre ; `interval(m - r, m + r)` avec des doubles ; l'ensemble
    vide qui passe tous les tests ; le domaine sans décorations ; `split()`
    d'un intervalle canonique ; `/` contre `%` dans la méthode de Newton ;
    `pow(x, 2)` qui prend le pow de GAOL quand les deux espaces de noms sont
    ouverts ; un ordre non entier de `nth_root` dans un texte, qui lève
    `invalid_action_error` ; `gaol_ieee1788::textToInterval`, qui rend
    l'ensemble vide pour un texte mal formé ; une borne nulle écrite `-0`,
    dont le signe diffère entre les builds SSE2 et FPU.

39. **Les restes de Goldstein-Price** (le +1 et « encloses the range » sont
    corrigés par #53) : `examples/03_dependency_problem.cpp` et
    `examples/06_global_optimization.cpp` écrivent le maximum sur [-2, 2]²
    comme un double, 2,97e-11 sous le vrai, si bien que leur vérification
    « l'enveloppe contient l'image » teste un intervalle trop étroit : le lire
    avec `textToInterval`, qui arrondit vers l'extérieur. L'exemple 16 garde
    le style de GAOL 4 dans `main()`. Les phrases d'`examples/examples.md` qui
    en parlent (des sorties identiques sur tous les builds, aucun
    avertissement avec `-Wall -Wextra`, vrai seulement en `-isystem`) sont
    pour la pull request de synthèse.

40. **Petites erreurs, suite** (la plupart sont corrigées par #55). Restent :
    le commentaire Doxygen « Parse a string to create an interval » de
    `gaol/gaol_parser.h`, placé avant l'énumération et non avant
    `parse_interval()`, et qui ne liste que les formats de GAOL 4 ; les tests
    `refused_*` de `tests/CMakeLists.txt`, faux quand GAOL est un sous-projet
    (ils prennent `CMAKE_BINARY_DIR` et `CMAKE_SOURCE_DIR`) ; un test
    `nodiscard_discard_cxx*` qui reste en échec une fois son objet compilé ;
    `test_input()` de `tests/input_output.cpp`, dont le `try` avale une
    exception et saute six assertions. Décidé le 3 octobre : le programme qui
    compare les 88 sorties du manuel au programme (`run_examples.py`, hors du
    dépôt) va dans `manual/`. Non vérifié : `GAOL_NODISCARD` sous Visual C++
    2017 15.8 et 15.9.

## Les trois builds

41. **GAOL ne peut pas être un sous-projet meson** : `meson.build` appelle
    `add_global_arguments`, que meson refuse dans un sous-projet (de 0.53.2 à
    1.11.2), alors que le commentaire de `gaol_dep` (`gaol/meson.build`)
    promet cet usage. Correction : `add_project_arguments` (essayé avec meson
    1.4.1), et mettre dans les `compile_args` de `gaol_dep` les options dont
    le code utilisant GAOL a besoin (celles de `pc_cflags`, que `gaol.pc`
    donne déjà : `-frounding-math`, `-ffp-contract=off`…), les arguments du
    projet ne passant pas au projet parent (décidé le 3 octobre, plutôt que
    retirer la promesse).

42. **Suites du faux Python du Microsoft Store** (#44) : que `WindowsApps`
    ait un alias `python.exe` en plus de `python3.exe`, comme le disent
    `meson.build`, `doc/building.md` et le manuel, n'a pas été vérifié sous
    Windows : décidé le 3 octobre, adoucir le texte. Ajouter aussi au manuel le
    cas résiduel que donne `doc/building.md` (l. 263-268) : un profil dont le
    répertoire diffère de `USERPROFILE`. La documentation plutôt qu'un
    changement de l'ordre `python3`, `python` (#44) est confirmée.

43. **Suites du `VERSION.txt` à marque d'ordre des octets** (#45) : sous
    Windows, seul le job meson MSYS2 lance `.github/scripts/version-file.sh` ;
    décidé le 3 octobre, il suffit, sans étape native (meson avec Visual C++,
    sous `pwsh`). Décidé aussi : limiter l'étape « VERSION.txt read by
    autoconf, as by configure » de `build-systems.yml` à
    `matrix.cfg.configure == ''`, et garder les `?` du message de
    `configure.ac` sur un `VERSION.txt` refusé, et laisser le `.strip()` de
    meson, qui retire aussi les espaces Unicode (#45).

44. **Les suites du test `cpack_stale_configure`** (#33), qui vérifie que
    CMake avertit d'un `configure` généré pour une autre version. Il échoue à
    tort quand le compilateur du parent porte un argument (`CC="ccache gcc"`)
    ou que Ninja est hors du `PATH` : décidé le 3 octobre, passer les
    compilateurs par `cmake -E env`. Sans `sh` ou sans liens symboliques, il
    s'arrête sur `FATAL_ERROR` au lieu de se dire ignoré. Avec les générateurs
    Makefile, `package_source` ne relance pas CMake après un changement de
    `VERSION.txt` (documenté ; le vérifier à l'archive demanderait CMake
    3.19) : décidé, faire dépendre `package_source` d'une reconfiguration ; le
    contrôle reste un test CTest (jusqu'à 52 s sous QEMU).

## Corrections laissées par les relectures

Relevées dans les descriptions et les relectures des pull requests
fusionnées, et vérifiées le 3 octobre au commit `a29c7b3`.

51. **Les commentaires et les textes de pow** (suite du point 1, #37) : le
    commentaire d'en-tête de `pow_standard()` (`gaol/gaol_interval.cpp`)
    raconte l'histoire (« This was the second half… ») au lieu de dire ce que
    fait la fonction ; le Doxygen de `gaol_pow_hybrid()`
    (`gaol/gaol_interval.h`, l. 835) dit la partie standard exp(J log(I)),
    alors que les bornes finies viennent des coins du `pow` de CORE-MATH ;
    `doc/tests.md` (l. 187 et 327) et `tests/ieee1788.cpp` (l. 133) disent
    les bornes vérifiées « bit for bit », alors que `==` ne distingue pas −0
    de +0, et racontent l'histoire : dire plutôt que les littéraux ont été
    relevés sur l'ancien code et vérifiés avec mpmath à 500 bits, ce que
    `tests/gaol_tests.h` doit citer aussi.

52. **La raison de C++17 dans les tests** (#37) : `tests/CMakeLists.txt`,
    `tests/Makefile.am` et `tests/meson.build` la donnent par les littéraux
    de `elementary_values.h` seulement, alors que `ieee1788.cpp`,
    `arithmetic.cpp`, `core_math.cpp` et d'autres en ont aussi : parler des
    tests en général.

53. **Le commentaire du refus de `-ffinite-math-only`** (suite du point 5,
    #39, #47) : `gaol/gaol_config.h` (l. 208) dit encore que `is_empty()` lit
    l'ensemble vide comme `!(left() <= right())`, alors que c'est
    `!std::islessequal(left(), right())` depuis #47. La phrase qui dit
    `([1, 2] & [3, 4]).is_empty()` faux (« GCC 9, Clang 18 » dans
    `gaol_config.h`, « GCC 9.4, Clang 18 » dans `doc/three-builds.md`, « with
    GCC 9 and Clang 18 » dans `doc/tests.md`) est à préciser : avec
    `-ffinite-math-only`, GCC 13 le rend faux à -O0, -O2 et -O3, Clang 18 à
    -O0, et à -O2 et -O3 seulement avec des bornes `volatile`.

54. **L'annexe B n° 4 de `examples/examples.md`** (#39, l. 1072) dit qu'un
    test de compilation vérifie le message avec `PASS_REGULAR_EXPRESSION`,
    « as `tests/fp_strict` does » : c'est faux, `tests/fp_strict` fait un
    `try_compile()` à la configuration, pour Visual C++ seulement.

55. **`.github/audit`** (#39) : `compare.py` (l. 25) et `make_probe.py`
    (l. 15) comparent `__FAST_MATH__` mais pas `__FINITE_MATH_ONLY__` ; sans
    effet tant que `-fno-fast-math` lui-même est comparé.

56. **Les commentaires du sens d'arrondi avec mingw-w64** (suite du point 6,
    #54) : le commentaire de `round_upward_if_needed()` (`gaol/gaol_fpu.h`,
    l. 334) dit que l'unité x87 « computes none of GAOL's doubles », ce qui
    est faux avec mingw-w64 x64, dont `ldexp()` est le `fscale` x87 : écrire
    « none of GAOL's doubles but exact ones » ; « as the get_rounding_mode()
    of cbrt.c, rsqrt.c and asinpi.c does » (`gaol/core_math_port.h`, l. 232)
    ne vaut que pour GCC et Clang, pas pour Visual C++ x64.

57. **Le commentaire de `tan()`** (suite du point 9, #36) : au-dessus de
    `const double w` (`gaol/gaol_interval.cpp`), « the test was w < pi_dn »
    est inexact (l'ancien code testait `!(w < pi_up)`, puis `w < pi_dn` pour
    le drapeau), et « though none holds a pole » se lit comme si aucun
    intervalle de cette largeur n'avait de pôle, alors que `[0, pi_dn]`
    contient π/2. L'entrée `tan` du manuel ne parle pas des largeurs entre
    `pi_dn` et π : une phrase.

58. **Deux restes du lecteur sous DAZ** (suite du point 11, #40) : le manuel
    (`gaol.tex`, l. 3579) écrit `\code{-Ofast}` au lieu de `\option{-Ofast}` ;
    `tests/numbers.cpp` teste `#if GAOL_TESTS_HAVE_MXCSR` (l. 176, 238…),
    macro indéfinie hors x86, qui avertit sous `-Wundef` : écrire `#ifdef`.

59. **Les textes d'`operator>>`** (suite du point 15, #46) :
    `doc/differences.md` (l. 402-403) et `examples/examples.md` (l. 62, 459
    et 634) disent encore qu'une ligne vide fait lever `input_format_error` ;
    le commentaire d'`operator>>` (`gaol/gaol_interval.cpp`, l. 668, « std::ws
    is no extraction: it constructs no sentry ») et `doc/tests.md` (l. 259)
    donnent pour général ce qui est vrai du `std::ws` de libstdc++ 9 et 10 :
    écrire « le `std::ws` de libstdc++ » ; `doc/tests.md` (l. 250, « with or
    without `std::noskipws` ») se lit comme si un nombre sautait les blancs
    sous `noskipws` ; l'exemple du manuel (`gaol.tex`, l. 3501) lit et écrit
    `x` dans la même expression : en faire deux instructions, et couper en
    deux la phrase voisine (ligne vide « avec ou sans `std::noskipws` »,
    intervalle sur plusieurs lignes) ; le Doxygen d'`operator>>`
    (`gaol/gaol_interval.h`) ne dit pas, comme le manuel, qu'avec les
    exceptions désactivées une ligne refusée écrit un message sur `cerr` et
    arrête le programme.

60. **`stream_without_buffer()` en dernier** (#46) : `tests/numbers.cpp`
    l'appelle en dernier (l. 1891) ; en cas de régression, le programme meurt
    par SIGSEGV sans afficher les échecs précédents, stdout étant tamponné :
    `std::fflush(stdout)` avant l'appel.

61. **Les commentaires de la lecture des longs nombres** (suite du point 18,
    #42) : le commentaire du membre `beyond` de `gaol_number`
    (`gaol/gaol_interval_lexer.lpp`, l. 216, une ligne de 119 colonnes)
    laisse croire qu'il est positionné dès que le nombre est sous le plus
    petit double positif ou au-dessus du plus grand, alors qu'il ne l'est que
    loin au-delà (10^311 et plus, ou sous 10^-330) : le corriger, au-dessus
    du membre ; le commentaire de `gaol_enclose_number()` (l. 368-372) dit
    qu'une lecture sous une locale à virgule prend le temps de la locale C :
    écrire « à peu près » (rapport mesuré de 1,0 à 1,2) ; dans
    `tests/numbers.cpp`, le message d'échec du test de temps est construit
    par `std::to_string` sous la locale à virgule (« 2,112549 s ») : le
    construire sous la locale C.

62. **La documentation des exceptions flottantes** (suite du point 24, #47,
    #50) : la liste des opérations qui, sous `GAOL_PRESERVE_ROUNDING`,
    masquent de nouveau les exceptions du programme (`doc/using.md` l. 527,
    manuel l. 1235) oublie `%`, `div_rel` et `+= d`, `-= d`, `*= d`, `/= d`,
    `%= d`, et ne dit pas qu'elles effacent les indicateurs ;
    « `fetestexcept(FE_INEXACT)` is raised whatever the result »
    (`doc/using.md` l. 513, manuel l. 1219) est trop fort (`-X`, `abs`, `&`,
    `|`, `floor`, `max(X, Y)` ne le lèvent pas) : « after most operations » ;
    la ligne courte de `doc/accuracy.md` (l. 60) ; `\newinvfive` sur les
    derniers paragraphes de la section 3.3 du manuel ; la structure
    `EmptySet` de `tests/rounding_direction.cpp` (l. 252), qui porte aussi
    `nonempty_sets` : un nom neutre.

63. **Le commentaire de `cpack_stale_configure`** (suite du point 44, #33) :
    « The script removes what it made in this directory, and nothing else »
    (`tests/cpack_stale_configure.cmake`, l. 46) est inexact,
    `file(REMOVE_RECURSE)` retirant tout le répertoire (la suppression est
    sûre, c'est le commentaire qui est faux) ; `doc/tests.md` (l. 595) dit
    « CMake build » sans dire pourquoi autotools et meson n'ont pas ce test
    (seul CMake fait l'archive, et le job autotools compare déjà
    `configure --version` à `VERSION.txt`).

64. **La mise en page de `doc/tests.md`** (#32, #39, #41, #42) : cinq lignes
    de plus de 100 colonnes (l. 43, 72, 298, 300 et 308) parmi des lignes
    d'environ 80, une ligne orpheline (l. 281, « With flush-to-zero,
    denormals-are-zero or both set in MXCSR ») et « The reading of » seul
    sur une ligne (l. 476).

65. **Les `open()` de `meson.build`** (#45) : `open(sys.argv[-1], …).read(…)`
    (l. 32, 53 et 60) ne ferme pas le fichier ; sans effet sous CPython, un
    `with` le ferait.

66. **Les textes des exemples** (#53, #55) : `examples/CMakeLists.txt`
    (l. 18, « which the autotools and meson builds also compile ») semble
    renvoyer à l'enveloppe ; la section 3 de `examples/examples.md` (l. 533)
    parle encore de « three formatting slips » (il y en avait 29, corrigés)
    et donne comme ouverts `chi([0,0]) = 0`, les 400 bits et `[[nodiscard]]`
    en C++17 seulement.

67. **Le Doxygen du format `hexa`** (#43) : « same as "bounds" except… »
    (`gaol/gaol_interval.h`, l. 84) était déjà inexact ; le réécrire avec la
    décision du point 16 (un point s'écrit `[a]` en hexadécimal aussi).

## Décisions sans point

Décidé le 3 octobre :

- **Les choix « à confirmer » de #29** (la référence HTML de Doxygen, les
  options de meson, la CI sans les exemples, `make test` qui n'écarte les
  exemples qu'à partir de CMake 3.17 et meson 0.57, `make perf`) sont
  acceptés.
- **La licence** : GAOL v5 reste sous LGPL, et `3rd/math-core` (CORE-MATH)
  sous MIT. Reste à le dire clairement dans `README.md`, à côté de
  `COPYING.LIB` et dans le manuel.

## Questions ouvertes

Les questions reportées le 3 octobre ont chacune leur issue : #64 (point 46),
#65 et #66 (point 31), #67 (point 26), #68 (point 45), #69 (point 6) et #70
(point 22).

GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` (#34) : le contournement
(`no-inline-functions`) ne vaut que pour GCC 12 sur aarch64, et d'autres
versions pourraient être touchées sans que la CI le montre. Décidé le
3 octobre : on le laisse tel quel, sans réduire le cas ni le signaler.

## Ménage

- **Branches à supprimer sur GitHub** : celles d'« En cours », une fois
  fusionnées (les fusionnées, les jetables et `fix-path-core-math` l'ont été
  le 3 octobre).
- **La CI tourne deux fois par pull request**, sur `push` (toutes les
  branches) et sur `pull_request` : le push de `61c7503` a lancé 114 jobs,
  environ 380 minutes de runner. Ne lancer `push` que sur les branches
  principales diviserait la charge par deux : décidé le 3 octobre, `push`
  seulement sur `master`, `MATH-CORE` et `configure-clean`.
- **Les lignes de crédit** : celles des descriptions de #50, #51, #53 à #57
  et #59, d'un commentaire de #59 et de l'issue #49 ont été retirées le
  3 octobre. Il en reste dans les descriptions de #60 à #63 et dans un
  commentaire de chacune de #58 et #60 à #63.
- **La pull request de synthèse**, une fois les branches de « En cours »
  fusionnées : `ChangeLog` et `doc/differences.md`, que rien n'a touchés depuis
  le 28 septembre (les textes proposés sont dans `todo-notes/` de la branche
  `todo-status`) ; `examples/examples.md` (marquer **Fixed** les n° 3, 4, 5, 8,
  10, 12, 15, 19, 20 et 21 de la section 5 et de l'annexe B, le n° 10 comme
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
- **L'issue #49** reste ouverte comme suivi d'ensemble ; les décisions du
  3 octobre et les liens vers les issues #64 à #70 y sont en commentaire.
