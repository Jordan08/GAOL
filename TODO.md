# À faire

Ce qui reste à faire sur GAOL v5 au commit `61c7503` de `configure-clean`. Les
points gardent les numéros de la liste d'origine : un numéro qui manque est un
point fait. Le détail de chaque point, les questions ouvertes des pull requests
fusionnées et le suivi (rapports, avancement) sont dans la branche `todo-status`
(`todo-notes/2026-10-02/reste-detaille.md`, `TODO.md`, `todo-notes/`) ; la
pull request de synthèse mettra à jour `ChangeLog` et `doc/differences.md`.
Les relectures des pull requests fusionnées ont aussi laissé de petites
corrections de commentaires, de documentation et de tests, rangées par point
dans `reste-detaille.md` ; seules celles de `3rd/README.md`, à faire avant
l'envoi à CORE-MATH, sont reprises ici (point 31).

Les points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md) (« revue n° n » renvoie au numéro
n de sa section 5) ; les points 41 à 44, de la vérification de `VERSION.txt`
(2026-09-28) ; 45 à 49, des corrections et des relectures.

## En cours

Ces branches sont poussées, mais pas fusionnées dans `configure-clean`.

- **3**, `todo-03-pow-exact-corner` (defcd7a) : terminé ; restent la
  relecture, la CI et la pull request ; conflit avec la branche du 8.
- **4**, `todo-04-ftz-daz` (ec0ea03) : terminé ; restent la fusion de
  `configure-clean` (conflit), la relecture, la CI et la pull request ;
  conflit avec la branche du 36 dans `doc/using.md`.
- **8**, `todo-08-pow-large-n` (d217170) : terminé ; restent la relecture, la
  CI et la pull request.
- **12**, `todo-12-long-sums` (553e649) : inachevé (commits « WIP ») ; restent
  la fin du travail, son rapport, la relecture, la CI et la pull request.
- **17**, `todo-17-output-speed` (3b9311c) : inachevé (« WIP ») ; restent les
  mesures, la vérification du texte écrit, le rapport, la relecture, la CI et
  la pull request.
- **24, armhf**, `fix-24b-armhf-fe-invalid` (375a822) : corrige la CI armhf,
  rouge depuis #50 ; validé sous qemu ; restent la relecture et la pull
  request.
- **36**, `todo-36-upward-rounding-effects` (1559af5) : inachevé (« WIP » fait
  sur `a2ca992`, conflits dans `doc/using.md`) ; voir le point 36 ; restent
  aussi la fusion de `configure-clean`, la relecture et la pull request.

## Code

1. **Suites du pow de la norme écrit une fois** (#37). `pow_standard()`
   (`gaol/gaol_interval.cpp`) est la seule copie du pow de la table 9.1. Deux
   vérifications sont inutiles : la garde de l'exposant `[±oo]` de
   `gaol_pow_hybrid()`, inatteignable, et le cas `at_upper == 1.0` du bloc
   au-delà des int de `pow_standard()`. Les retirer reprendrait une part des 2
   à 3 % perdus par `gaol::pow` à exposant non entier (mesurés avec GCC 9.4
   seulement) ; le `is_empty()` de `gaol_pow_hybrid()` est à garder. À
   confirmer : le bloc au-delà des int et le pown dans les int sur x coupé à
   [0, +oo] sont dans `pow_standard()`, où `gaol_pow_hybrid()`, qui prend
   `[n]` avant, ne les atteint jamais, et non dans `gaol_ieee1788::pow`.

2. **Le sens d'arrondi est encore vérifié plus d'une fois** : trois fois par
   `pow(x, y)` quand `pow_standard()` passe par exp(y log x) (une borne
   infinie, ou une base partant de 0 avec un exposant qui n'est pas au-dessus
   de 0), deux fois par `nth_root(x, q)` pour q < 0 et par `modulo_k_pi()`.
   Correction : appeler les corps de ces opérations après une seule
   vérification, comme `tan()` et les puissances négatives ; les corps de
   `log()`, `exp()` et `*` sans la vérification restent à écrire. Pour `pow`,
   après le point 3, on peut plutôt étendre l'analyse des coins aux bornes
   infinies et à une base partant de 0 : plus d'exp(y log x), et des boîtes
   serrées (aujourd'hui, `pow([4, +oo], 0.5)` vaut [2 − 2^-52, +oo], et
   d'autres ont des centaines de doubles de trop), avec des tests aux limites.

3. **`pow(x, y)` a la largeur d'un double là où la puissance en un coin est un
   double** : `pow([4], 0.5)` vaut [2 − 2^-52, 2], et les cas 149 à 152 et 163
   de [doc/compare/special_cases.md](doc/compare/special_cases.md) sont plus
   larges que le résultat d'IEEE 1788. Fait dans la branche
   `todo-03-pow-exact-corner` : `pow_is_double()` prouve par des opérations
   exactes que x^y est un double, et `pow_lo()` le prend alors comme borne
   inférieure. À corriger avec elle : la colonne GAOL des cas 184 à 188 de
   `special_cases.md` ([−0] au lieu de [0] depuis #37), et une phrase du
   commentaire de `pow_standard()` qui devient fausse.

## Bornes fausses

4. **Le flush-to-zero et le denormals-are-zero rendent les bornes fausses**
   (revue n° 2) : `[1e-300] * [1e-20]` vaut [0, 0] dans un programme lié avec
   `-Ofast` (par `crtfastmath.o`) ou qui charge un plug-in compilé ainsi. Fait
   dans la branche `todo-04-ftz-daz` : une sonde sous-normale qui efface FTZ et
   DAZ (FZ sur ARM), `-mno-daz-ftz` à l'édition de liens, un test et la
   documentation. Restent : décider si `gaol.pc` garde `-mno-daz-ftz`, qui
   arrête l'édition de liens avec Clang 18 ou un GCC antérieur à 11.4 ; mesurer
   sur les processeurs de la CI le coût de la sonde (+0,5 ns sur `x * y` sur un
   Xeon) ; ARM avec Visual C++ ou clang-cl, et FIZ, ne sont pas couverts ;
   GCC 9.4 sous `-funsafe-math-optimizations` réduit la sonde, même
   sous-normale, à `tiny == 0.0` (ranger la somme dans un double `volatile` la
   garderait) ; `-fno-signed-zeros` fait sauter le `+ 0.0` de la sonde dans le
   code du programme (seulement documenté). La suite est au point 45.

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
   l'exécution dans `gaol/gaol_init_cleanup.h` reste à essayer.

6. **Suites de `fegetround()` lu dans MXCSR et du refus de MinGW-w64** (#54,
   #51). Restent des choix de portabilité : lire MXCSR aussi sur x86 32 bits
   avec SSE2 ; garder ou supprimer `GAOL_RND_MINGW_FENV_ONLY` (mingw-w64 11 i686
   passe ctest sans lui sous wine) ; refuser mingw-w64 ARM64 avant 12 ou sans
   `_UCRT`, comme sur x86-64 ; garder ou lever le refus de mingw-w64 ARM
   avant 11, que rien de testé ne justifie, et restreindre ou non celui du x86
   32 bits avant 11 à la chaîne WinLibs qui le prouve ; taire le `#warning` de
   `cbrt.c`, `rsqrt.c` et `asinpi.c` en attendant le point 31 ; signaler à
   mingw-w64 son `fma()` et son `round()` pour msvcrt. Le `fma()` logiciel de
   `ucrtbase.dll` n'a jamais été vérifié.

7. **Suites de `atanh([1, x])`** (#31) : un `TEST_EQ` de `atanh_rel` à borne
   infinie (`atanh_rel([0.5, 1], [0, +oo])`), possible depuis le point 10 ;
   `atanh([1])` et `atanh([-1])` manquent à `doc/compare/special_cases.md`
   (relancer les cinq bibliothèques).

8. **`pow(x, n)` pour n grand** (revue n° 7) : dans `ipow_exact_dn()`, la
   borne inférieure perd le carré du reste (1962 doubles sous la plus serrée
   pour n = 2^32 − 1), et les builds SSE2 et FPU multiplient les produits
   arrondis dans des ordres différents, contre « les mêmes bornes sur toutes
   les machines » de `doc/accuracy.md`. Fait dans la branche
   `todo-08-pow-large-n`. À décider : une borne nulle, ou une seule
   borne hors de la plage, envoie les deux bornes aux produits arrondis
   (jusqu'à 10 doubles de trop) : rendre `ipow_exact_dn(0)` exact, ou traiter
   chaque borne à part ; supprimer l'ancien code SSE2 gardé sous `#if 0` ;
   relire la garantie 5 n log2(n) 2^-104. `pown([-2, 3], 100)` différait entre
   SSE2 et FPU avec GCC 9.4 (#37) : à revérifier.

9. **Suites de `tan([-M_PI_2, M_PI_2])`** (#36). `tan()` donne maintenant
   ±1,63·10^16. Un intervalle sans pôle dont la largeur exacte est entre
   `pi_dn` et π donne encore [-oo, +oo] ; on n'en connaît aucun, et le rendre
   serré demanderait de comparer `r - l` à π en double-double : garder la
   phrase de `doc/accuracy.md`, ou dire le cas théorique. Que les tests
   `w < pi_dn` de `cos_or_sin()` donnent le plus serré est mesuré, pas
   démontré.

<!-- -->

11. **Suites du lecteur sous denormals-are-zero** (#40). Le lecteur est juste
    sous FTZ et DAZ (comparaisons sur les bits) ; le reste est au point 45.
    Restent : vérifier une fois, avec `ctest -V -R numbers`, si les tests DAZ
    de `numbers` s'exécutent ou se sautent sur macOS x86_64 sous Rosetta et
    avec l'UCRT en Release ; dans le manuel, retirer `\newinvfive` de la
    phrase sur ces modes, ou tester GAOL 4 sous DAZ.

<!-- -->

45. **Le denormals-are-zero fausse encore des bornes hors du lecteur** (suite
    du point 11) : des fonctions comparent des sous-normaux, que DAZ lit comme
    0, avant toute sonde. Le parser accepte `<1e-310, 1e-309>`,
    `interval(0x1p-1073, 0x1p-1074)` n'est pas vide, `bound_to_text()` écrit
    une borne sous-normale au plus proche (à un chiffre, [22·2^-1074] s'écrit
    `[1e-322]`, relu plus petit), et `log`, `sqrt`, `abs`, `div_rel`, `mid()`
    et les relations se trompent : une fois le point 4 fusionné, avant la
    première opération qui sonde, et à chaque appel avec
    `GAOL_PRESERVE_ROUNDING`. Correction : comparer les bits, ou sonder avant
    de comparer, et régénérer le parser. À décider (#58) : retirer DAZ et FTZ
    le temps de l'écriture, gdtoa et le runtime Debug de Visual C++ écrivant 0
    un sous-normal sous DAZ. Les tests DAZ ne tournent pas sur ARM (FPCR.FZ).

<!-- -->

47. **clang-cl : ce qui reste après le correctif de `cbrt`, `rsqrt` et
    `asinpi`** (#59). Restent : refuser clang-cl sans `/fp:strict` (il compile
    alors en `-fno-rounding-math -ffp-contract=on`, et `gaol_config.h` ne refuse
    que Visual C++ ; `_M_FP_STRICT` existe à partir de Clang 16), et le vérifier
    dans `tests/fp_strict` ; Cygwin x64, hors CI, reste faux (`FE_*` de newlib,
    pas de `_WIN32`) : prendre le correctif 6 de `3rd/README.md` en entier, ou
    refuser Cygwin ; citer clang-cl x64 dans le manuel et `doc/three-builds.md`
    une fois ses jobs verts (point 49) ; réserver `/Zc:strictStrings-` à
    Visual C++ (11 avertissements par build clang-cl, dans `CMakeLists.txt` et
    `gaol/meson.build`).

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
    (limite documentée). À la régénération, faire taire ou non l'avertissement
    de Clang 18 sur `gaol_nerrs`.

## Flux et texte

14. **Suites de `what()` des exceptions** (#32). `what()` renvoie
    l'explication. Restent : les constructeurs à `const char*` des classes
    dérivées transmettent tel quel à `std::string` un pointeur nul
    (comportement indéfini, qu'aucun appel de GAOL ne fait) : le traiter comme
    une absence d'explication, ou le dire ; le manuel ne documente que ces
    constructeurs. À choisir : le texte de `what()` sans explication, la forme
    courte d'`operator<<`, et une réserve là où la documentation dit que GAOL 4
    donnait `std::exception` (`Unknown exception` sous Visual C++).

15. **Suites de la lecture des lignes vides par `operator>>`** (#46), qui lit
    par `std::getline(is >> std::ws, buffer)` : une ligne vide est sautée, et
    `while (in >> x)` finit sans exception. À décider : sous
    `exceptions(eofbit)`, un intervalle lu en fin d'entrée lève avec `eofbit`
    seul (un double reçoit `eofbit | failbit`), et une dernière ligne sans fin
    de ligne est perdue ; `std::ws` saute `\v` et `\f`, que le lecteur ne prend
    pas pour des blancs.

16. **Suites des formats d'affichage** (#57). Restent, vérifiés dans
    `61c7503` : dans le format `width`, `+2 (+/- +1)` sous `std::showpos`, un
    milieu groupé et un rayon non groupé sous une locale qui groupe les
    chiffres, et un milieu `-0` ; le format `agreeing`, qui écrit [1, 10]
    `1~[., 0.]` et les zéros avec leur signe, et ne donne de chiffres communs
    qu'aux intervalles positifs de rapport au plus 10 ; des tests (le format
    `width` sous une locale à virgule, un ordre d'évaluation non spécifié dans
    `tests/rounding_direction.cpp`, `gaol_tests::hex()` sous la locale
    globale) ; le `#include <sstream>`, inutile, à retirer de
    `gaol/gaol_ieee1788.h`. À décider : la précision d'`intervalToText` ;
    l'écriture d'un point en hexadécimal (non vérifiée sous `std::hexfloat`) ;
    `[0]` ou `[-0]` pour un point nul ; sous une locale à virgule, -2.5 s'écrit
    `[-2,5, -2,5]`, que le lecteur refuse ; un format dont le rayon absorbe
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
    septembre). À décider : si cette décision vaut aussi pour la forme
    incertaine, dont les sommes passent par `gaol_decimal_add_sub()`, qui
    insère en tête d'une `std::string` (100 000 chiffres : 1,1 s, contre
    0,46 s).

<!-- -->

46. **`operator>>` ne relit pas `os << x << ' ' << y`** (suite du point 15) :
    il lit un intervalle par ligne, et la fin d'un intervalle dans un flux est
    ambiguë, un intervalle contenant des espaces et la grammaire admettant des
    littéraux imbriqués et des expressions (`[[1, 2], 3]`, `1 -2`). Il faut
    d'abord décider de la syntaxe acceptée : un intervalle par ligne (ce qui
    est documenté), les seuls littéraux lus jusqu'au crochet fermant, ou une
    fonction à part.

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
    fin. Correction : un `gaol::lexicographic_less`, peut-être une
    spécialisation de `std::less`, et une mise en garde dans `doc/using.md` et
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
    `fix-24b-armhf-fe-invalid` (« En cours »), `is_empty()` reste compilé en
    `vcmpe` sur armhf dans le code du programme : le documenter, l'écrire avec
    `std::isunordered()` sur ARM 32 bits, ou le signaler à GCC (bogue 52258).
    Avec `GAOL_PRESERVE_ROUNDING`, les opérations SSE2 masquent de nouveau les
    exceptions du programme et effacent ses indicateurs : à corriger. `0 × oo`
    dans l'`operator*=` SSE2 et le `pow` de CORE-MATH pour un exposant extrême
    lèvent FE_INVALID, sans test : corriger, ou laisser documenté.
    `gaol_intervalf.h` appelle `std::islessequal` sans inclure `<cmath>`. À
    décider : le coût de `floor`, `ceil`, `integer` et `x &= y` (non mesuré
    sous Visual C++), et les comparaisons signalantes des constructeurs
    (`interval(NAN)`) et des intervalles de flottants (`gaol_intervalf.h`,
    `gaol_interval2f.h`).

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
    indicateur.

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
    backend intervalle pour VNODE-LP.

## Tests et intégration continue

29. **Le test sous une locale à virgule n'est vérifié que dans `linux.yml`**
    (#35) : macOS, Windows, les conteneurs et `build-systems.yml` ne génèrent
    ni ne vérifient de locale à virgule (Debian demande le paquet `locales` ;
    Alpine et manylinux n'en ont pas). À décider : un contrôle
    (`.github/scripts/comma-locale.sh check`) après les tests là où la locale
    existe, une variable d'environnement qui fasse échouer `numbers` lui-même,
    ou rien. Au passage : `numbers` n'a pas de `TIMEOUT` dans
    `tests/fetch_content` et `tests/find_package`, et prend 235 s de ses 300
    sur macOS x86_64 Debug ASan+UBSan (délai dépassé une fois dans #50) ; le
    passage de 300 à 30 tirages en Debug (`d78af72`) est à remesurer, #58
    ayant montré que les jobs Visual Studio Debug attendaient une boîte de
    dialogue ; les jobs Ubuntu 22.04 de `linux.yml` sont à remplacer avant le
    17 avril 2027.

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
    défauts. Rien n'est envoyé. À décider : qui envoie et comment (merge
    request sur gitlab.inria.fr, ou `git format-patch` à core-math@inria.fr) ;
    la forme du correctif 6 (la table, ou la petite forme de la copie de
    GAOL), et `binary80/pow/powl.c`, non examiné ; y joindre l'échec du master
    à `./check.sh --worst --rndd pow` et la borne de `ss` dans `asinpi_acc()` ;
    changer aussi les `sinpi.c` et `log1p.c` embarqués, ou attendre l'amont.
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

34. **Les recettes FetchContent récupèrent GAOL 4** (décision à prendre) :
    `doc/using.md`, le manuel et `tests/fetch_content` prennent la branche
    `master` de `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la
    branche par défaut. `master` est GAOL 4.3.2, avec lequel les programmes de
    la documentation ne compilent pas, et il n'y a pas d'étiquette `v5.0.0`.
    Correction : une fois #29 fusionnée, étiqueter `v5.0.0` ou fusionner
    `MATH-CORE` dans `master`, et écrire l'étiquette dans les recettes et le
    test.

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
    exception et saute six assertions. À décider : où garder le programme qui
    compare les 88 sorties du manuel au programme (`run_examples.py`, hors du
    dépôt). Non vérifié : `GAOL_NODISCARD` sous Visual C++ 2017 15.8 et 15.9.

## Les trois builds

41. **GAOL ne peut pas être un sous-projet meson** : `meson.build` appelle
    `add_global_arguments`, que meson refuse dans un sous-projet (de 0.53.2 à
    1.11.2), alors que le commentaire de `gaol_dep` (`gaol/meson.build`)
    promet cet usage. Correction : `add_project_arguments` (essayé avec meson
    1.4.1), et mettre dans les `compile_args` de `gaol_dep` les options dont
    le code utilisant GAOL a besoin (celles de `pc_cflags`, que `gaol.pc`
    donne déjà : `-frounding-math`, `-ffp-contract=off`…), les arguments du
    projet ne passant pas au projet parent ; sinon, retirer la promesse.

42. **Suites du faux Python du Microsoft Store** (#44) : que `WindowsApps`
    ait un alias `python.exe` en plus de `python3.exe`, comme le disent
    `meson.build`, `doc/building.md` et le manuel, n'a pas été vérifié sous
    Windows : le vérifier, ou adoucir le texte.

43. **Suites du `VERSION.txt` à marque d'ordre des octets** (#45) : sous
    Windows, seul le job meson MSYS2 lance `.github/scripts/version-file.sh` ;
    une étape native (meson avec Visual C++, sous `pwsh`) est-elle voulue ?

44. **Les suites du test `cpack_stale_configure`** (#33), qui vérifie que
    CMake avertit d'un `configure` généré pour une autre version. Il échoue à
    tort quand le compilateur du parent porte un argument (`CC="ccache gcc"`)
    ou que Ninja est hors du `PATH` : ne pas l'enregistrer dans ces cas, ou
    passer les compilateurs par `cmake -E env`. Sans `sh` ou sans liens
    symboliques, il s'arrête sur `FATAL_ERROR` au lieu de se dire ignoré. À
    décider : avec les générateurs Makefile, `package_source` ne relance pas
    CMake après un changement de `VERSION.txt` (documenté ; le vérifier à
    l'archive demanderait CMake 3.19) ; garder le contrôle en test CTest
    (jusqu'à 52 s sous QEMU), ou en faire une étape de la CI.

## Décisions à prendre

Les décisions propres à un point sont dans son texte (voir surtout les points
29, 31, 34 et 46). Une seule n'a pas de point :

- **Les choix « à confirmer » de #29** (la référence HTML de Doxygen, les
  options de meson, la CI sans les exemples, `make test` qui n'écarte les
  exemples qu'à partir de CMake 3.17 et meson 0.57, `make perf`, la licence
  MIT alors que GAOL v5 est sous LGPL) : acceptés, abandonnés ou reportés ?

## Questions ouvertes

GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` (#34) : le contournement
(`no-inline-functions`) ne vaut que pour GCC 12 sur aarch64, et d'autres
versions pourraient être touchées sans que la CI le montre. Réduire le cas et
le signaler à GCC ?

## Ménage

- **Branches à supprimer sur GitHub.** Fusionnées : `todo-01-pow-norm-half`
  (#37), `todo-05-finite-math-only` (#39), `todo-29-ci-comma-locale` (#35) et
  `remove-string-constructor` (#30). Jetables : `ci-debug-numbers-arm64`
  (#34), `ci-debug-mingw-numbers` (#48) et les branches de #38 et #52,
  fermées. Abandonnée : `fix-path-core-math`, dont le travail a été repris ou
  remplacé. Puis chaque branche de « En cours » une fois fusionnée.
- **La CI tourne deux fois par pull request**, sur `push` (toutes les
  branches) et sur `pull_request` : le push de `61c7503` a lancé 114 jobs,
  environ 380 minutes de runner. Ne lancer `push` que sur les branches
  principales diviserait la charge par deux.
- **Les descriptions des pull requests** #50, #51, #53 à #57 et #59 (et un
  commentaire de #59) finissent par une ligne de crédit et un lien : les
  retirer, et étendre aux pull requests la règle qui l'interdit dans les
  commits ?
- **La pull request de synthèse**, une fois les branches de « En cours »
  fusionnées : `ChangeLog` et `doc/differences.md`, que rien n'a touchés depuis
  le 28 septembre (les textes proposés sont dans `todo-notes/` de la branche
  `todo-status`) ; `examples/examples.md` (marquer **Fixed** les n° 3, 4, 5,
  8, 10, 12, 15, 19, 20 et 21 de la section 5 et de l'annexe B, le n° 10
  comme corrigé par la suppression des constructeurs à partir de chaînes
  (#30), et la ligne vide du n° 14 ; reprendre ce qu'il dit des points
  corrigés) ; une seule régénération de `manual/v5/gaol.pdf`, dont la
  dernière date du 28 septembre, après avoir corrigé dans `gaol.tex` les
  entrées d'index en conflit de « canonical interval » et l'`Overfull \hbox`
  des l. 511-515.
- **L'issue #49** (suivi d'ensemble) est restée au 30 septembre : y reporter
  #58, #59, le point 49 et la CI de `61c7503` (rouge sur armhf et sur les deux
  jobs clang-cl seulement), et remplacer sa liste de branches ; ou la fermer
  en renvoyant à ce `TODO.md`.
