# À faire

Ce qui reste à faire sur GAOL v5. Ce qui est fait est dans
[What differs from GAOL](doc/differences.md).

Les points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md), et de la vérification des
corrections qui l'ont suivie. « Revue n° n » renvoie au numéro n de sa
section 5, dont l'annexe B donne la correction et un test de régression,
validés sur des builds SSE2 et FPU de travail mais pas appliqués. Ses numéros
1, 6, 9, 10, 11, 13, 14 et 18 sont corrigés.

Les points 41 à 44 viennent de la vérification du fichier `VERSION.txt`, le
2026-09-28 : ce que les agents ont trouvé et qui n'est pas corrigé.

## Code

1. **Le pow de la norme est écrit deux fois.** `gaol_ieee1788::pow`
   (`gaol/gaol_interval.cpp`) intersecte x avec [0, +oo] et traite à part
   x = {0}, et `gaol_pow_hybrid()` le refait. Correction : faire de la moitié
   « norme » de `gaol_pow_hybrid()`, le pow de la table 9.1 pour un exposant
   intervalle, une fonction de `gaol/gaol_interval.cpp` ; `gaol_pow_hybrid()`
   n'y ajoute que le cas pown d'un exposant entier dégénéré, et
   `gaol_ieee1788::pow` appelle directement cette moitié, sauf pour ses
   exposants entiers au-delà des int.

2. **Le sens d'arrondi est encore vérifié plus d'une fois** par `pow(x, y)`
   quand il passe par exp(y log x) (une borne infinie, ou une base partant de
   0 avec un exposant qui n'est pas au-dessus de 0) : `log()`, `*` et `exp()`
   le vérifient trois fois ; par `nth_root(x, q)` pour q < 0, `inverse()` de
   `nth_root(x, -q)`, deux fois ; et par `modulo_k_pi()`, deux fois.
   Correction : appeler les corps de ces opérations après une seule
   vérification, comme le font maintenant `tan()`, les fonctions
   relationnelles et les puissances négatives (`uipow_rounded_upward()`,
   `interval::inverse_upward()`) ; les corps de `log()`, `exp()` et `*` sans
   la vérification restent à écrire.

3. **`pow(x, y)` a la largeur d'un double là où la puissance en un coin est un
   double.** La borne inférieure est le double sous la valeur de CORE-MATH
   arrondie vers le haut, même quand cette valeur est exacte : `pow([4], 0.5)`
   vaut [2 − 2^-52, 2], et les cas 149 à 152 et 163 de
   [doc/compare/special_cases.md](doc/compare/special_cases.md) sont plus
   larges que le résultat d'IEEE 1788 pour cette seule raison. Correction :
   garder la valeur comme borne inférieure quand la puissance est exacte, comme
   le font `exp2`, `log2`, `nth_root(x, 3)` et les fonctions de la table 10.5.

## Bornes fausses

4. **Le flush-to-zero et le denormals-are-zero rendent les bornes fausses**
   (revue n° 2) : avec eux, `[1e-300] * [1e-20]` vaut [0, 0]. Un programme lié
   avec `-Ofast`, `-ffast-math` ou `-funsafe-math-optimizations` les reçoit de
   `crtfastmath.o`, que GCC lie alors et dont le constructeur s'exécute après
   l'initialisation de GAOL. Le `-fno-fast-math` de `gaol.pc` et de
   `gaol::gaol` ne l'empêche pas, et fait taire le `#error` de
   `gaol_config.h` à la compilation, GCC appliquant d'abord les options `-O`.
   Charger un plug-in ou un module Python compilé avec `-Ofast` les active
   aussi. La sonde de chaque opération, `1.0 + tiny == 1.0` dans
   `round_upward_if_needed()` (`gaol/gaol_fpu.h`), ne voit que le sens
   d'arrondi : `tiny` vaut 2^-60, un double normal. Correction :
   - sonder avec un sous-normal, `1.0 + (subnormal + 0.0) == 1.0` avec
     2^-1060, qui voit le sens, FTZ et DAZ en une comparaison, puis effacer
     aussi FTZ et DAZ dans MXCSR (`& ~0x8040`). Mesuré sans coût sur un
     i7-1185G7 ; à mesurer d'abord sur les processeurs de l'intégration
     continue, une addition avec un opérande sous-normal étant lente sur
     certains processeurs x86. C'est la seule défense contre les plug-ins ;
   - passer `-mno-daz-ftz`, qui empêche GCC de lier `crtfastmath.o`, dans les
     options d'édition de liens de `gaol.pc` et de `gaol::gaol`, là où le
     compilateur l'accepte (GCC 12 et plus récents sur x86 ; non vérifié :
     GCC 9.4 et Clang 18 le refusent) ;
   - dire dans `doc/using.md`, `doc/three-builds.md` et le manuel que lier
     avec ces options, ou charger du code compilé avec elles, rend les bornes
     fausses.

5. **`-ffinite-math-only` n'est pas refusé** (revue n° 4). Le compilateur
   suppose alors que les NaN et les infinis n'arrivent jamais, y compris dans
   les fonctions inline des en-têtes, compilées avec les options du
   programme : l'ensemble vide ayant des bornes NaN,
   `([1,2] & [3,4]).is_empty()` est faux, et l'enveloppe de l'ensemble vide et
   de [1, 2] est vide. Le `-fno-fast-math` de `gaol.pc` ne protège que s'il
   vient après l'option sur la ligne de commande.
   `-funsafe-math-optimizations`, et `-ffast-math -fno-finite-math-only`,
   qu'aucune macro ne révèle, laissent GCC réduire la sonde `1 + tiny == 1` en
   `tiny == 0` dans le code inline, si bien que `width()` après un changement
   de sens est sous la largeur exacte. Correction : un `#error` sur
   `__FINITE_MATH_ONLY__` dans `gaol_config.h`, à côté de celui sur
   `__FAST_MATH__`, une ligne dans les options refusées de
   `doc/three-builds.md`, et un test de compilation comme dans
   `tests/fp_strict`.

6. **`pow` rate un résultat sous-normal quand l'unité x87 et MXCSR ne sont pas
   d'accord** (revue n° 3) : sur x86-64 avec la glibc, l'unité x87 arrondissant
   au plus proche et MXCSR vers le haut, comme les laisse `exactinit()` des
   prédicats de Shewchuk et de Triangle, dans 205 cas aléatoires sur 400.
   CORE-MATH arrondit lui-même ces résultats, dans le sens que donne
   `fegetround()`, que la glibc lit dans l'unité x87. Correction : dans
   `gaol/core_math_port.h`, sur x86-64, définir `fegetround()` comme une
   lecture de MXCSR, comme le fait `get_rounding_mode()` de CORE-MATH dans
   `rsqrt.c`, `asinpi.c` et `cbrt.c`, et ajouter `pow` avec des résultats
   sous-normaux à `tests/rounding_direction.cpp`, qui liste déjà cet état.

7. **`atanh([1, x])` vaut [DBL_MAX, +oo]** au lieu de l'ensemble vide (revue
   n° 5), atanh étant défini sur (−1, 1) ; de même `atanh([1])`, et
   `atanh_rel` et `tanhRev` en héritent, alors que `atanh([-5, -1])` est vide.
   Correction : l'ensemble vide quand `J.left() == 1.0 || J.right() == -1.0`,
   J étant `I & [-1, 1]`, et le domaine (−1, 1) dans `doc/accuracy.md` et le
   manuel.

8. **`pow(x, n)` pour n grand** (revue n° 7). La borne inférieure perd le
   carré du reste que porte `ipow_exact_dn()` : 8 doubles sous la plus serrée
   pour n = 2^28 − 1, 1962 pour 2^32 − 1, là où le manuel promet la plus serrée
   ou un double au-delà. Correction :
   `nl = std::fma(-h,h,p) + nl*(2.0*h - nl);`. Là où la puissance d'une borne
   dépasse la plage des doubles (vers 0 ou vers l'infini), les deux bornes
   prennent les produits arrondis, que les builds SSE2 et FPU multiplient dans
   des ordres différents : leurs bornes diffèrent dans 190 cas aléatoires sur
   1600, contre « les mêmes bornes sur toutes les machines » de
   `doc/accuracy.md`. Les mettre d'accord, ou le dire là.

9. **`tan(interval(-M_PI_2, M_PI_2))` vaut [-oo, +oo]** là où la plus serrée
   est ±1,63·10^16 (revue n° 8). Correction :
   `narrower_than_pi = (w <= pi_dn)` : w, la largeur arrondie vers le haut,
   au plus le double sous π, prouve que la largeur exacte est sous π.

10. **`hausdorff()` sur les bornes infinies, `nb_fp_numbers()` sur −0** (revue
    n° 19 et n° 20). `hausdorff(x, x)` vaut +oo pour x = [1, +oo], de même que
    `hausdorff([1, +oo], [2, +oo])`, dont la distance vaut 1 : une boucle de
    point fixe sur une boîte à un côté non borné ne s'arrête jamais.
    Correction, dans les deux builds : des bornes égales, infinies comprises,
    sont à distance 0. `nb_fp_numbers(-0.0, 1.0)` fait le tour et vaut
    13830554455654793217, et `[1, 2] - 1` a pour borne inférieure −0.
    Correction : numéroter les doubles par les bits de `std::fabs(a)` et
    `std::fabs(b)`.

11. **Le lecteur donne un encadrement sous-normal faux avec le flush-to-zero et
    le denormals-are-zero** : `textToInterval("1e-310")` vaut
    [0x0.fffffffffffffp-1022, 0x1p-1022], qui ne contient pas 1e-310 (GAOL s'y
    bloquait avant la bissection sur les bits). `gaol_compare_number()`
    (`gaol/gaol_interval_lexer.lpp`) teste `!(x > 0.0)` et prend
    `std::frexp()` du double candidat, que DAZ lit comme 0. Correction :
    prendre l'exposant et la mantisse du candidat dans ses bits.

## Plantages

12. **Les longues sommes débordent la pile** (revue n° 17) : une somme de
    100 000 termes lue dans une chaîne, et des expressions aussi longues
    construites par l'API, font une récursion par terme dans l'évaluation et
    dans la destruction de l'arbre. Correction : dans
    `gaol_interval_parser.ypp`, réduire chaque opération binaire dès sa
    lecture, comme le sont déjà les appels de fonctions, et régénérer le
    parser commité ; pour les expressions de l'API, une évaluation et une
    destruction sans récursion.

## Flux et texte

13. **Un intervalle ponctuel est écrit sous une forme que le lecteur refuse**
    (revue n° 12) : `<0.1, 0.1000000000000001>` dans le format par défaut, que
    le lecteur refuse (« bounds of degenerate interval do not evaluate to the
    same value »), si bien que
    `textToInterval(intervalToText(interval(0.1)))` est l'ensemble vide,
    contrairement au manuel. Correction : dans `display_bounds()`, écrire
    `<a, a>` seulement quand les deux textes sont égaux, et `[l, r]` sinon, et
    corriger le manuel là où il montre `<0.1, 0.1000000000000001>`.

14. **`gaol_exception` ne redéfinit pas `what()`** (revue n° 15) :
    `catch (const std::exception& e)` affiche `std::exception`, de même qu'une
    erreur non rattrapée. Correction : `what()` renvoie l'explication, et
    `operator<<` des exceptions n'affiche plus les deux.

15. **`operator>>` lève une exception sur une ligne vide**, et sur
    `in >> d >> x`, où l'intervalle lit le reste vide de la ligne du nombre :
    un fichier qui finit par une ligne vide termine `while (in >> x)` par
    `input_format_error`. Correction : lire avec
    `std::getline(is >> std::ws, buffer)`, qui saute les blancs comme la
    lecture d'un nombre, ou dire dans le manuel qu'une ligne vide est mal
    formée. Lire un intervalle par mot plutôt que par ligne permettrait aussi
    de relire ce qu'écrit `os << x << ' ' << y`.

16. **Les formats largeur et centre** (revue n° 21) affichent un centre et un
    rayon arrondis au plus proche, qui ne contiennent pas forcément
    l'intervalle ([1, 1 + 2^-52] s'écrit `1 (+/- 1.11e-16)`), et un rayon nul
    pour [0, 5·10^-324], contrairement au manuel ; le manuel et
    `gaol/gaol_interval.h` décrivent le format largeur comme le centre et la
    largeur, alors qu'il affiche le rayon. Les deux formats écrivent
    l'ensemble vide `empty`, là où le manuel dit que tous les formats écrivent
    `[empty]`. `gaol_ieee1788::intervalToText` suit le format global et la
    locale du flux, si bien que son texte n'est pas toujours un littéral
    d'IEEE 1788. Correction : documenter les deux formats comme des formats
    d'affichage, ou arrondir leur rayon vers le haut ; faire écrire à
    `intervalToText` le format des bornes, avec un point.

17. **`operator<<` est environ 450 ns plus lent par intervalle** depuis qu'il
    écrit dans un `std::ostringstream` à lui (13 % pour le format des bornes,
    2,2 fois pour le format centre), et `std::internal` complète maintenant
    comme `std::right`. À mesurer sur des programmes qui écrivent beaucoup
    d'intervalles ; formater dans un tampon de caractères plutôt que dans un
    flux en économiserait l'essentiel.

18. **Lire un long nombre sous une locale à virgule est lent** : chacune des
    125 comparaisons au plus de `gaol_enclose_number()` réanalyse le texte, en
    un temps quadratique en sa longueur : 2,5 s pour 20 000 caractères, contre
    0,08 s sous la locale C. Correction : analyser une fois la mantisse et
    l'exposant et les comparer à chaque double, ou donner à `strtod()` une
    copie du texte avec le séparateur décimal de `localeconv()`, pour que sa
    valeur soit de nouveau juste et que deux comparaisons suffisent.

## En-têtes

19. **`gaol::sin(0.5)` ne compile plus** (revue n° 16) : chaque fonction de
    l'espace de noms `gaol` sur un double est ambiguë entre les surcharges pour
    les intervalles et pour les expressions, depuis que `gaol_ieee1788.h`
    inclut `gaol_expression.h` ; GAOL 4 le compilait. Correction : ne plus
    inclure `gaol_expression.h` depuis `gaol_ieee1788.h`, et déplacer les deux
    surcharges d'expressions de `gaol_ieee1788` (`pown(e, n)`, `pow(e1, e2)`)
    à la fin de `gaol_expression.h`.

20. **Les en-têtes publics débordent dans le programme** (revue n° 22).
    `gaol_interval_parser.h`, installé, ne compile pas quand on l'inclut, et
    lui et `gaol_init_cleanup.h` sont internes ; `gaol_allocator.h` ne compile
    pas seul. Ils mettent `using std::exception; using std::string;` à la
    portée globale et définissent des macros sans préfixe (`INLINE`,
    `HAVE_FENV_H`, `MEMALIGN`, `__HI`…). Avec `-Wall -Wextra` et un simple
    `-I` (pkg-config, ou FetchContent, dont le répertoire d'inclusion n'est
    pas `SYSTEM`), un programme reçoit environ 35 avertissements : 30
    `-Wunused-parameter` dans `gaol_expr_visitor.h`, et un
    `-Wdeprecated-copy` à chaque `x = ...;` (un constructeur de copie écrit
    par l'utilisateur avec une affectation de copie implicite). Correction :
    ne plus installer les en-têtes internes, supprimer les `using`, préfixer
    les macros, déclarer l'affectation de copie par défaut et laisser sans nom
    les paramètres inutilisés.

## Interface

21. **Les entiers au-delà de 2^53** : `interval(0)`, `interval(0, 0)`,
    `x = 0`, `x < 0`, `max(x, 0)` et `T(0)` compilent, GAOL v5 n'ayant plus de
    constructeur à partir de chaînes : l'entier devient un double. Un entier
    au-delà de 2^53 devient ainsi un double qui ne le contient pas.
    Correction : des constructeurs templates sur les types entiers, contraints
    (vérifié sur tous les programmes de test et les sources de la
    bibliothèque ; dans les en-têtes, sans changement d'ABI). Pas un simple
    `interval(int)`, qui rend `interval(5L)` ambigu.

22. **Pas d'ordre total pour les conteneurs.** `<`, `<=`, `>` et `>=` sont les
    relations « certainement » d'IEEE 1788, vraies pour (∅, ∅) : `std::set`
    perd les intervalles qui se chevauchent, et `std::sort` d'un vecteur
    contenant un intervalle vide lit au-delà de sa fin. Correction : un
    `gaol::lexicographic_less`, peut-être une spécialisation de `std::less`,
    et une mise en garde dans la documentation contre `std::sort`,
    `std::set`, `std::max`, `std::min` et `std::clamp` sans comparateur.

23. **Gérer le sens d'arrondi.** `cleanup()` ne le restaure qu'une fois, il
    n'y a pas de garde à portée, et `rnd_keep()` n'est pas présenté comme une
    barrière. Correction : un `gaol::restore_rounding()` qu'on peut appeler
    autant de fois qu'on veut, une garde qui calcule un bloc au plus proche
    (environ 7 ns), et `rnd_keep()` documenté.

24. **Indicateurs et exceptions flottantes.** Avec les exceptions matérielles
    activées (`feenableexcept`), `1/[0,1]`, `log([0,1])`, `[1e308]*10` et
    chaque `is_empty()` d'un ensemble vide calculé meurent sur SIGFPE : les
    résultats non bornés viennent de dépassements ou de divisions par zéro, et
    `is_empty()` compare des bornes NaN avec une comparaison signalante.
    Chaque opération lève l'indicateur inexact. Correction : `is_empty()` avec
    `std::islessequal`, sans coût, et un mot dans la documentation pour dire
    que les exceptions matérielles doivent être désactivées.

25. **Les outils que chaque algorithme réécrit** : `mulRevToPair` (un
    `div_rel` en deux morceaux), `inflate`, `bisect(ratio)` et
    `is_bisectable()` (`split()` coupe au milieu, ±DBL_MAX pour une
    demi-droite), un constructeur milieu-rayon, `hull()` et `intersect()`
    comme fonctions, un encadrement de la largeur (`width()` est un majorant),
    un littéral `_iv` dans un espace de noms `gaol::literals`, `erf` et `erfc`
    (ceux de CORE-MATH sont embarqués), et les fonctions réciproques de
    `atan2`, `pow(x, y)`, `max`, `min`, `sign` et `floor`, qu'IBEX écrit
    lui-même.

26. **Des décorations**, ou au moins un indicateur disant qu'un argument est
    sorti du domaine : `sqrt` d'une boîte négative est vide et passe tous les
    tests d'inclusion, et `interval(DBL_MAX*10)`, `x + INFINITY` et
    `x * NAN` sont vides sans rien dire.

27. **Un sens d'arrondi qui ne fuit pas** : l'arrondi porté par chaque
    instruction (AVX-512, le FPCR d'AArch64 en assembleur), comme le fait
    inari, trois fois plus rapide sur les additions ; dans la direction de
    P2746.

28. **Des en-têtes optionnels au-dessus du cœur scalaire**, à partir de
    `examples/` : un type boîte (`box.h`), la différentiation directe sur les
    intervalles (`dual.h`), les formes affines (`affine.h`) ; ou des traits
    GAOL pour YalAA et un backend intervalle pour VNODE-LP.

## Tests et intégration continue

29. **Le test sous une locale à virgule ne tourne pas dans l'intégration
    continue** : `tests/numbers.cpp` lit sous `fr_FR.UTF-8` ou `de_DE.UTF-8`
    seulement là où l'une est installée. Correction :
    `sudo locale-gen fr_FR.UTF-8` dans `.github/workflows/linux.yml`.

30. **Des suites de tests et des bancs d'essai où GAOL est absent** : passer
    ITF1788 (toutes les opérations d'IEEE 1788) et le banc d'essai de Tang et
    al. (2021) sur GAOL v5, et ajouter à `doc/compare/` Boost.Interval, la
    bibliothèque que les utilisateurs prennent d'abord, dont les fonctions
    élémentaires ne sont pas sûres.

## CORE-MATH

31. **Proposer en amont les corrections des sources embarquées.**
    [3rd/README.md](3rd/README.md) liste cinq modifications des sources de
    CORE-MATH ; quatre sont des corrections plutôt que des adaptations à
    GAOL : les masques `~0ull` de `sinh.c`, `cosh.c` et `tanh.c`, les
    décalages signés de `cospi.c`, le décalage sur 128 bits de `asinpi.c`, et
    le `__builtin_expect` sur 64 bits de `rsqrt.c`, qui prenait les
    sous-normaux pour +0 partout où `long` fait 32 bits.
    Une cinquième est faite dans `gaol/core_math_port.h` plutôt que dans les
    sources : depuis sa réécriture (commit amont `6b84457`), `sin.c` appelle
    `__builtin_roundeven()` sans garde, ce que GCC n'a pas avant la version
    10, et GCC 9.4 ne l'éditait pas ; les autres sources ne prennent ce
    builtin qu'à partir de GCC 10 et de Clang 17 et arrondissent elles-mêmes
    avant (`roundeven_finite()` de `exp.c`), ce que `sin.c` pourrait faire
    aussi.
    C'est aussi par là que le travail de GAOL v5 atteint la glibc, qui importe
    les fonctions de CORE-MATH (glibc 2.41 à 2.44 ; `cosh`, `sinh` et `tanh`
    en 2.44) et tourne sur des cibles Linux 32 bits, où `long` fait aussi
    32 bits. À vérifier : si les copies de la glibc ont encore les masques
    `~0ul` et le `__builtin_expect` sur 64 bits ; si oui, envoyer les mêmes
    corrections à la glibc (libc-alpha, avec une ligne `Signed-off-by` : plus
    de cession de copyright à la FSF depuis août 2021).

## Documentation

32. **Le rapport de couverture** ([coverage/README.md](coverage/README.md)) a
    été écrit le 2026-09-20 : ses lignes non exécutées ne correspondent plus
    aux sources, qui ont changé depuis (une seule grammaire pour le lecteur de
    chaînes, le lexer et le parser rendus réentrants, les relations
    `possibly_*`, `certainly_eq` et `certainly_neq` supprimées). À refaire
    avec `-DGAOL_COVERAGE=ON` et la cible `coverage`, qui demandent gcovr.

33. **Les temps de [doc/compare/performance.md](doc/compare/performance.md)**
    ont été mesurés à `bb6f7e4` avec des fichiers modifiés hors des sources
    (`bb6f7e4-dirty`), avant que le sens d'arrondi ne soit vérifié une seule
    fois par fonction d'intervalles (`exp()` 5,7 % plus rapide depuis, `sin()`
    3,6 %, `pow(x, y)` 3,4 %), et une seule fois dans `tan()`, les fonctions
    relationnelles, les puissances négatives et `sqrt_rel()`. À remesurer sur
    un commit propre, la machine ne faisant rien d'autre
    (`doc/compare/code/run_bench.sh`, ou GAOL v5 seul avec `make perf`).

34. **Les recettes FetchContent récupèrent GAOL 4.** Celles de `doc/using.md`
    et du manuel, et le `git clone` de `doc/building.md`, prennent la branche
    `master` de `Jordan08/GAOL`, qui le 2026-09-27 est GAOL 4.2.3, 121 commits
    derrière `MATH-CORE` : les programmes de la documentation ne compilent pas
    avec elle, et il n'y a pas d'étiquette `v5.0.0` à fixer. Correction :
    étiqueter `v5.0.0`, ou fusionner `MATH-CORE` dans `master`, et écrire
    l'étiquette dans les recettes.

35. **Un premier programme avant les détails.** `README.md` ne contient pas de
    code C++, le premier programme est à la ligne 92 de `doc/using.md`, et
    aucun document ne montre un algorithme sur les intervalles. Correction :
    un programme de dix lignes dans `README.md` qui renvoie à `examples/`, et
    un court tutoriel (encadrement d'image et subdivision, Newton avec `%` ou
    `div_rel`, un contracteur avec les fonctions `*_rel`, séparation et
    évaluation), que les exemples 03, 05, 06 et 07 contiennent déjà.

36. **Ce que l'arrondi vers le haut fait au programme**, dans `doc/using.md`
    et dans les erreurs courantes du manuel, avec la table de la section 2.8
    de la revue : `printf`, `strtod`, `lrint`, les allers-retours par le
    texte, TwoSum, et GCC qui réutilise après `cleanup()` un double calculé
    avant. `GAOL_PRESERVE_ROUNDING` comme moyen de garder l'arrondi du
    programme, avec son coût mesuré (4,8 fois sur x + y, 1,4 à 1,6 fois sur
    exp et sin).

37. **Une table des noms** pour les utilisateurs d'IBEX, de Codac, de C-XSC,
    de Boost et d'IEEE 1788 (section 2.3 de la revue), avec ce que `<`, `<=`
    et `==` signifient dans chacun : dans C-XSC et PROFIL/BIAS, `<=` est
    l'inclusion, et le code porté depuis eux compile et change de sens.

38. **Les pièges dans les erreurs courantes du manuel** : `sqrt(2)` sur un
    nombre ; `interval(m - r, m + r)` avec des doubles ; l'ensemble vide qui
    passe tous les tests ; le domaine sans décorations ; `split()` d'un
    intervalle canonique ; `/` contre `%` dans la méthode de Newton ;
    `pow(x, 2)` qui prend le pow de GAOL quand les deux espaces de noms sont
    ouverts ; un texte refusé pour un ordre non entier de `nth_root`, qui lève
    `invalid_action_error` et non `input_format_error` ; `textToInterval` qui
    donne l'ensemble vide pour un texte mal formé ; une borne nulle écrite
    `-0`, dont le signe diffère entre les builds SSE2 et FPU
    (`sqr([-1, 2])`, `interval::zero()`).

39. **La fonction de Goldstein-Price du manuel oublie le +1**
    ((x + y)² au lieu de (x + y + 1)²), comme `examples/16_Goldstein_Price.cpp`,
    l'exemple de GAOL 4, et le manuel dit que f « parcourt » l'encadrement,
    environ 150 fois plus large que l'image de la vraie fonction.
    Correction : dire que l'encadrement contient l'image, et donner la vraie
    fonction ou dire que ce n'est pas elle.

40. **Petites erreurs.** Le commentaire d'en-tête de `chi()`
    (`gaol/gaol_interval.h`) dit `chi([0,0]) = 0` là où le code et le manuel
    disent −1 ; `tests/gaol_tests.h` dit que les références utilisent 400 bits
    là où `tests/elementary_values.py` en utilise 2000 ; le commentaire de
    `interval(const char*)` nomme un `jail_parser.h` qui n'existe pas ;
    `GAOL_NODISCARD` fonctionne à partir de C++17 et `gaol::gaol` ne fixe pas
    de norme du langage, si bien qu'un projet CMake avec GCC 9, en C++14, n'a
    pas d'avertissement pour `sqrt(x);` ; trois exemples du manuel montrent
    `true`/`false` là où le programme affiche 1/0 (pas de `std::boolalpha`),
    et `nan` là où il affiche `-nan`.

## Les trois builds

41. **GAOL ne peut pas être un sous-projet meson.** `meson.build` appelle
    `add_global_arguments` (17 fois, à partir de la ligne 171), que meson
    refuse dans un sous-projet : un projet parent qui fait `subproject('gaol')`
    s'arrête sur « Function 'add_global_arguments' cannot be used in
    subprojects », avec meson 0.53.2 comme avec 1.11.2. Le défaut est
    antérieur à `VERSION.txt`. Le commentaire de `gaol_dep`
    (`gaol/meson.build`) dit pourtant qu'il sert à un projet meson qui prend
    GAOL en sous-projet. Correction : `add_project_arguments`, et mettre dans
    les `compile_args` de `gaol_dep` les options dont le code utilisant GAOL a
    besoin (`-frounding-math`, `-ffp-contract=off`..., celles de `gaol.pc`),
    que les arguments du projet ne transmettent pas au projet parent ; ou
    retirer la promesse du commentaire.

42. **meson 0.53 sous Windows peut prendre le faux Python du Microsoft
    Store.** `find_program('python3', 'python', native: true)`, qui lit
    `VERSION.txt` dans `project()`, peut trouver le `python3.exe` de
    `WindowsApps`, qui ne fait qu'ouvrir le Store et que meson 0.53 n'écarte
    pas : `run_command` échoue et `meson setup` s'arrête. Non reproduit (pas
    de Windows ici) ; l'intégration continue n'est pas touchée, son meson
    venant de pip et étant récent. Correction : chercher `python` avant
    `python3` sous Windows, ou le dire dans la section meson de
    `doc/building.md`.

43. **Un `VERSION.txt` qui commence par une marque d'ordre des octets**
    (UTF-8 avec BOM, comme l'enregistrent certains éditeurs sous Windows) est
    refusé par les trois builds avec « VERSION.txt holds "7.3.11" », dont le
    caractère fautif ne se voit pas. Correction : retirer une marque d'ordre
    des octets en tête avant de lire la version, ou dire dans le message que le
    fichier ne doit contenir que des chiffres et des points.

44. **L'archive des sources de CPack ne voit pas un `configure` en retard.**
    `cmake --build <build> --target package_source` prend le `configure`
    commité tel quel : après un changement de `VERSION.txt` sans nouvelle
    génération de `configure`, `gaol-<version>.tar.gz` contient un
    `configure` dont `--version` donne l'ancienne version (il avertit quand il
    tourne) ; seule l'intégration continue le signale. Correction : dans le
    bloc CPack de `CMakeLists.txt`, lire la ligne `PACKAGE_VERSION=` de
    `configure` et écrire un `message(WARNING)` quand elle diffère de
    `PROJECT_VERSION`.
