# À faire

Ce qui reste à faire sur GAOL v5. Ce qui est fait est dans
[What differs from GAOL](doc/differences.md).

Les points 4 à 30 et 34 à 40 viennent de la revue du 2026-09-27,
[examples/examples.md](examples/examples.md), et de la vérification des
corrections qui l'ont suivie. « Revue n° n » renvoie au numéro n de sa
section 5, dont l'annexe B donne la correction et un test de régression,
validés sur des builds SSE2 et FPU de travail mais pas appliqués. Ses numéros
1, 5, 6, 8, 9, 10, 11, 13, 14, 15 et 18 sont corrigés.

Les points 41 à 44 viennent de la vérification du fichier `VERSION.txt`, le
2026-09-28 : ce que les agents ont trouvé et qui n'est pas corrigé (les 42, 43
et 44 le sont depuis). Les points 45 à 47 viennent du travail du 2026-09-30.

## Avancement au 2026-09-30

Les points sont repris un par un, chacun dans sa branche `todo-NN-…` faite à partir de `configure-clean`, validé en local (builds
SSE2, FPU et Clang 18 selon le point, test de non-régression dont on a montré qu'il échoue sans la correction), relu par un relecteur
indépendant, poussé pour lancer la CI, puis proposé en pull request. Les points 1, 5, 7, 9, 10, 11, 13, 14, 15, 18, 24, 29, 42, 43 et
44, fusionnés, ne sont plus dans la liste ci-dessous. Les rapports détaillés de chaque point (résumé, tests, validation, questions
ouvertes, ce qu'il faudra consigner dans `ChangeLog` et `doc/differences.md`, remarques des relecteurs) sont dans
[todo-notes/](todo-notes/), et les scripts et consignes de la méthode dans [todo-notes/orchestration/](todo-notes/orchestration/) (à
supprimer si vous n'en voulez pas). Le suivi d'ensemble est l'issue #49.

Les messages de commit font une ou deux lignes et ne portent jamais de ligne `Co-Authored-By` : c'est écrit dans
`.claude/CLAUDE.md` (sur `master`, `configure-clean` et cette branche), que Claude Code lit à chaque session.

### Fusionné dans `configure-clean`

Seize pull requests avant le 30 septembre, puis #48 (CI Windows : MinGW-w64 x64 lance ses programmes avec son `bin` en tête du `PATH`).

| Point | Pull request | Branche |
| --- | --- | --- |
| 7 `atanh([1, x])` | #31 | `todo-07-atanh-domain` |
| 14 `what()` des exceptions | #32 | `todo-14-exception-what` |
| 44 archive CPack et `configure` en retard | #33 | `todo-44-cpack-stale-configure` |
| hors liste : le job Debian 12 arm64 | #34 | `fix-numbers-gcc12-aarch64` |
| 29 locale à virgule dans la CI | #35 | `todo-29-ci-comma-locale` |
| 9 `tan([-M_PI_2, M_PI_2])` | #36 | `todo-09-tan-pi-half` |
| 1 pow de la norme écrit une fois (`pow_standard()`) | #37 | `todo-01-pow-norm-half` |
| 5 `-ffinite-math-only` | #39 | `todo-05-finite-math-only` |
| 11 lecteur et denormals-are-zero | #40 | `todo-11-lexer-subnormal` |
| 10 `hausdorff()`, `nb_fp_numbers()` | #41 | `todo-10-hausdorff-nb-fp` |
| 18 lecture lente sous locale à virgule | #42 | `todo-18-reader-locale-speed` |
| 13 point écrit sous une forme que le lecteur refuse | #43 | `todo-13-point-interval-text` |
| 42 meson 0.53 et le Python du Store | #44 | `todo-42-meson-windows-python` |
| 43 `VERSION.txt` avec une marque d'ordre des octets | #45 | `todo-43-version-bom` |
| 15 `operator>>` sur une ligne vide | #46 | `todo-15-extract-empty-line` |
| 24 exceptions et indicateurs flottants | #47 | `todo-24-fp-exceptions` |

Commits faits directement sur `configure-clean` le 30 septembre, à votre demande : `examples/examples.md` ne dit plus que tous les
exemples du manuel sont justes (le programme de la vue d'ensemble n'avait pas le +1 de Goldstein-Price, 39) ; une étape de CI lance
`meson setup` sans Python dans le `PATH` (42) ; `VERSION.txt` est lu avec les mêmes règles par CMake, meson, autoconf et configure :
mêmes blancs, octet NUL refusé, UTF-16 nommé dans le message, CMake 3.14 à 3.16 compris (43) ; `gaol.pc.in` nomme `VERSION.txt` ;
`.claude/CLAUDE.md`.

La pull request #29 (`configure-clean` vers `MATH-CORE`) est toujours ouverte.

### Pull requests du 30 septembre

Les pull requests des corrections vont vers `configure-clean`.

| Point | Pull request | Branche | État |
| --- | --- | --- | --- |
| 24 (suite) : aucune opération ne lève FE_INVALID sur un opérande vide | #50 | `todo-24b-quiet-empty-operands` | fusionnée dans `configure-clean` |
| 6 (suite) : MinGW-w64 sur x86-64 refusé avant 12 ou lié à msvcrt, pour la vraie raison (`fma()` et `round()`) | #51 | `todo-06b-mingw-msvcrt-refused` | fusionnée dans `configure-clean` |

### Fusionnés le 30 septembre (#53 à #57)

Chaque branche a été relue par un relecteur indépendant, dont les points bloquants sont corrigés, poussée, puis fusionnée dans
`configure-clean`. La CI de `configure-clean` après ces fusions reste à vérifier : elle est rouge sur armhf depuis #50 (6 échecs
de `rounding_direction`, `operator&=` des intervalles FPU sur ARM 32 bits), correction en cours dans `fix-24b-armhf-fe-invalid`.

| Point | Branche (pull request) | Ce qui a été fait le 30 septembre |
| --- | --- | --- |
| 16 formats largeur et centre, et la décision du 13 | `todo-16-display-formats` (#57) | `configure-clean` fusionné ; remarques des relecteurs ; un intervalle ponctuel s'écrit `[a]` (et un zéro `[0]`) au lieu de `<a, a>`, par `operator<<` comme par `intervalToText` ; le lecteur accepte toujours `<a, b>` ; sous une locale à virgule, un point s'écrit avec ses deux bornes, que le lecteur refuse, au lieu de `[-2,5]` qu'il lisait [-2, 5] |
| 39 Goldstein-Price | `todo-39-goldstein-price` (#53) | exemple 16 au style du fichier, temps en ms, sources des valeurs (sympy, mpmath) |
| 40 petites erreurs | `todo-40-small-errors` (#55) | les trois commits de l'agent interrompu vérifiés et complétés ; `GAOL_NODISCARD` prend `[[nodiscard]]` avant C++17 avec GCC 7 et plus et Visual C++ 16.4 et plus ; `chi()` du vide écrit `nan` ; commentaire de `jail_parser.h` |
| 6 `fegetround()` lu dans MXCSR | `todo-06-fegetround-mxcsr` (#54) | `configure-clean` fusionné, relu ; **`cbrt.c` corrigé** : sous MinGW-w64 x64, `get_rounding_mode()` rendait `FE_UPWARD` = 0x800 au lieu de 0 à 3, et `nth_root(x, 3)` ne contenait pas la racine cubique en sept cas difficiles, avec toutes les toolchains MinGW acceptées (MinGW-Builds UCRT, MSYS2 UCRT64 et CLANG64) |
| 31 correctifs à proposer en amont | `todo-31-upstream-patches` (#56) | `3rd/README.md`, section « Changes to propose upstream » : six correctifs contre le master de CORE-MATH, vérifiés ; la glibc n'a aucun de ces défauts ; rien n'est envoyé |

`configure-clean`, avec #51, a été fusionné dans la branche 6 (conflit dans `doc/tests.md` résolu en gardant les deux ajouts ; ctest et
`core_math` passent).

### Travail inachevé (commit « WIP », non relu, poussé sans lancer la CI)

Reprises le 30 septembre au soir (fusion de `configure-clean`, fin du travail, relecture), avant leurs pull requests :

3 exactitude de `pow(x, y)` aux coins (`todo-03-pow-exact-corner`, sans conflit) · 4 flush-to-zero (`todo-04-ftz-daz` ; conflits dans
`doc/tests.md`, `doc/three-builds.md`, `doc/using.md`, `gaol.pc.in`, `manual/v5/gaol.tex` et `tests/rounding_direction.cpp`) · 8
`pow(x, n)` pour n grand (`todo-08-pow-large-n`, sans conflit) · 12 longues sommes (`todo-12-long-sums`, conflit dans
`tests/expressions.cpp`) · 17 vitesse de `operator<<` (`todo-17-output-speed`, fondé sur le 16, qu'il faudra fusionner d'abord :
`display_bounds()` y écrit maintenant `[a]` ; conflits dans `gaol/gaol_interval.cpp` et `tests/numbers.cpp`) · 36 l'arrondi vers le
haut dans la documentation (`todo-36-upward-rounding-effects`, conflit dans `doc/using.md`).

### Pas commencés

2 (se fait sur le 3), 19, 20, 21, 22, 23, 32, 34, 35, 37, 38, 41, les nouveaux 45, 46 et 47, et les gros points 25, 26, 27, 28, 30 ;
le 33 (temps de `doc/compare/performance.md`) est à faire en dernier, la machine ne faisant rien d'autre. Le découpage prévu des gros
points est dans `todo-notes/orchestration/points_w4.py`. Le point 34 (étiquette `v5.0.0` ou fusion de `MATH-CORE` dans `master`)
demande une décision de votre part, de même que l'envoi des correctifs du 31.

### Ce que la CI a montré

- **Debian 12 arm64** : GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` à `-O2` et `-O3` ; contournement fusionné (#34). La
  branche jetable `ci-debug-numbers-arm64` est encore sur le dépôt : à supprimer.
- **Visual C++, littéraux entiers** (point 1), **zéro écrit** (point 13), **lecture sous arrondi vers le haut** (point 15), **MSYS2,
  archive** (point 43) : voir les rapports de ces points.
- **Coût** : un push lance 111 jobs, environ 348 minutes de runner. Les workflows se lancent sur `push` (toutes les branches) **et** sur
  `pull_request` : chaque pull request les fait tourner deux fois. Ne lancer `push` que sur les branches principales diviserait la
  charge par deux (non fait).

### Décisions du 30 septembre

- **1** : `gaol::pow([0], y)` écrit `<0, 0>` : bon (il s'écrit maintenant `[0]`, voir 13).
- **5** : les tests de compilation n'existent que pour CMake : bon.
- **6** : le refus de MinGW-w64 avant 12 ne se lève pas. Sa raison écrite était fausse (`fegetround()` n'a pas d'état propre, il lit le
  mot de contrôle x87) : la vraie est le `fma()` de mingw-w64 (non correctement arrondi) et son `round()` (qui dépend du sens
  d'arrondi), que les mingw-w64 liés à msvcrt ont encore après 12. Décision : refuser sur x86-64 tout mingw-w64 avant 12 ou sans
  `_UCRT`, et écrire la vraie raison partout (#51). Sur x86 32 bits, les toolchains acceptées (mingw-w64 11) ont un `fma()` x87 qui
  n'est pas correctement arrondi (125 résultats sur 400 000 à un double près) mais passent tous les tests : **l'écart est accepté**, et
  les tests actuels sont gardés tels quels.
- **10** : pas de normalisation des zéros des opérations d'intervalles (une borne `-0` est acceptable).
- **11** : l'effet de denormals-are-zero hors du lecteur devient le point 45.
- **13** : `operator<<` et `intervalToText` écrivent `[a]` au lieu de `<a, a>` (branche du 16).
- **15** : la lecture par mot devient le point 46.
- **18** : l'analyse des chiffres reste quadratique : on ne fait rien.
- **24** : les opérations qui levaient encore FE_INVALID sur un opérande vide sont corrigées (#50, fusionnée).
- **31** : les correctifs pour CORE-MATH et la glibc sont écrits dans `3rd/README.md` (branche du 31).
- **39, 42, 43** : les cas restants sont corrigés directement dans `configure-clean`.

### Questions ouvertes

Les questions laissées ouvertes par chaque pull request (la sienne ou ses relectures) sont reportées ici dès qu'elles sont écrites,
avec le numéro de la pull request. Le relevé du 30 septembre reprend celles des pull requests #31 à #57 et des rapports de
`todo-notes/` qui restent ouvertes : celles que vos décisions ont tranchées, qu'un commit a réglées ou qui sont déjà un point de ce
fichier n'y sont plus. Le détail est dans `todo-notes/NN.md` et `todo-notes/2026-09-30/`.

- **1** :
  - Choix de conception : `pow_standard()` contient aussi le bloc « au-delà des int » et le pown « dans les int » sur x coupé à [0,
    +oo], que `gaol_pow_hybrid()` n'atteint jamais puisqu'il prend `[n]` avant. Le TODO les laissait dans `gaol_ieee1788::pow`, qui
    aurait alors dû refaire la coupe de x et le cas x = {0}. (#37)
  - Dans `gaol_pow_hybrid()`, la garde de l'exposant `[+oo]`/`[-oo]` est inatteignable : `interval(±oo)` est vide, donc pris d'abord
    par `is_empty()`. Le cas `at_upper == 1.0` du bloc au-delà des int est redondant, `pow(1, n)` de CORE-MATH valant exactement 1.
    Et le `is_empty()` de `gaol_pow_hybrid()` refait celui de `pow_standard()` ; un relecteur le juge nécessaire avant `J.left()`,
    l'autre non. Retirer ces vérifications compléterait « vérifié une fois » et reprendrait une part des +2 à 3 % (2 à 3 ns sur 100)
    de `gaol::pow` à exposant non entier, mesurés avec GCC 9.4 seulement. (#37)
  - `pown` aux grandes puissances n'était pas identique bit à bit entre SSE2 et FPU. Borne haute de `pown([-2, 3], 100)` :
    `0x1.69194f299cdddp+158` en FPU, `0x1.69194f299cddbp+158` en SSE2 ; 1 762 lignes du différentiel, mesuré à `a2ca992` avec GCC
    9.4. `doc/differences.md` ne le promet pas pour `pown`. Ce n'est pas reproduit sur `configure-clean` avec GCC 13.3 (SSE2 et FPU,
    avec et sans `GAOL_FMA`) : `0x1.69194f299cddbp+158` partout. À revérifier avec GCC 9.4. Le point 8 ne couvre que les puissances
    hors de la plage des doubles. (#37)
  - Le chemin `exp(y*log(x))` de `pow_standard()` (borne infinie, ou base partant de 0 avec y ≤ 0) peut être large de centaines de
    doubles. `gaol_ieee1788::pow([-oo, 2^-1000], [-oo, -1])` vaut `[0x1.ffffffffffd97p+999, +oo]`, soit 617 doubles sous 2^1000, qui
    est la borne la plus serrée. Ni le point 2 ni le point 3 ne couvrent ce cas. (#37)
  - `doc/tests.md` (test `ieee1788`) et `tests/ieee1788.cpp` (en-tête, commentaire de `pow_on_boxes()`) disent que les bornes sont «
    bit for bit » celles d'avant, alors que `==` ne distingue pas −0 de +0. Ils racontent aussi l'histoire (« when each had its own
    copy of the pow », « as on 70 500 more boxes », un différentiel resté hors de l'arbre). Mieux vaut dire ce que sont les
    littéraux : relevés sur l'ancien code et vérifiés avec mpmath à 500 bits. (#37)
  - Le commentaire d'en-tête de `pow_standard()` raconte l'histoire (« This was the second half of gaol_pow_hybrid(), which
    gaol_ieee1788::pow went through… ») au lieu de dire ce que fait la fonction. (#37)
  - Le Doxygen de `gaol_pow_hybrid()` (`gaol/gaol_interval.h`) dit que la partie standard est `exp(J log(I))`. Or les coins de `pow`
    de CORE-MATH servent aux bornes finies. C'était déjà faux avant #37. (#37)
  - `tests/ieee1788.cpp` utilise maintenant des littéraux flottants hexadécimaux : il ne compile plus en C++11 ou C++14 strict. Les
    trois builds compilent les tests en C++17, mais les commentaires de `tests/CMakeLists.txt`, `tests/Makefile.am` et
    `tests/meson.build` justifient C++17 par « les littéraux de `elementary_values.h` » seulement. Parler des tests en général.
    (#37)
  - Les scripts de différentiel et de mutation du point 1 (`diffpow.cpp`, `run_mutations.py`, `mutate2.py`, `checkrows.py`,
    `gen_table.py`) sont restés dans l'espace de travail de l'agent : ils ne sont ni dans l'arbre ni dans `todo-notes/`. Ils
    serviraient à régénérer la table de `pow_on_boxes()` après le point 3. (#37)
- **3** :
  - Le point 3 se fait maintenant dans `pow_standard()` (`gaol/gaol_interval.cpp`). Il change la borne basse attendue de la boîte
    `{[0.1, 2], [-2, 0.5]}` de `pow_on_boxes()` (`0x1.fffffffffffffp-3` devient `0x1p-2`), et rend fausse la phrase « where the
    corners below are one double wide » de `pow_standard()`. Le texte du point 3 ne le dit pas. (#37)
  - Correctifs de plateforme (Visual C++ et racine carrée de Windows 32 bits a8afd8b, 959b14b ; armhf 82367f0, 75a4f17) : les garder
    dans la branche du point 3, ou les sortir dans une PR à fusionner d'abord ? (#63)
  - Racine carrée de SSE2 seulement sur Windows 32 bits : l'étendre à tout x86 32 bits ? À -O0, GCC appelle sans doute aussi celle
    de la bibliothèque C sur Debian i386 (non vérifié), et les jobs i386 de la CI sont en Release seulement. (#63)
  - MinGW-w64 x86 en Release prenait la racine du runtime C (au plus près) dans les phases précises froides d'acos, asin et asinpi
    jusqu'à 959b14b ; aucun test n'a montré de borne fausse. Chercher des cas durs ? (#63)
  - Envoyer le patch 7 (`exact_pow()`, `3rd/README.md`) à CORE-MATH ? (#63)
  - Bornes infinies : `pow([4, +oo], 0.5)` vaut toujours `[2 - 2^-52, +oo]` ; étendre les coins avec le point 2 ? (#63)
  - Sous DAZ, `pow([0.5], [2^-1074])` vaut `[1, 1]` depuis le point 1 : relève du point 4 (#62) ? (#63)
  - Le commit WIP `d353335` entre dans l'historique, sauf fusion squash ; de même `62e72c8` (point 8, #61) et le WIP du point 4
    (#62). (#61, #62, #63)
  - #61 et #63 modifient `doc/accuracy.md` et le manuel : la seconde fusionnée aura un conflit à résoudre. (#63)
- **4** :
  - Le moins unaire ne fait pas de vérification : il échange les bornes stockées, que les modes ne changent pas ; documenté. Lui
    ajouter une sonde, au prix d'une addition sur une opération très courante, pour le seul effet d'effacer les modes ? (#62)
  - Coût : `x * y`, `sqrt` et `pow(x, 3)` +0,6 à 0,7 ns (Xeon Cascade Lake, Clang 18 ; autant avec une sonde faite de 2^-60, donc la
    seconde addition). Processeurs de la CI non mesurés : comparer le tableau de `gaol_performance` de la CI à celui de
    `configure-clean`. (#62)
  - `gaol.pc` porte `-mno-daz-ftz` : un programme lié par Clang, ou par GCC 12.0 à 12.3, avec le `gaol.pc` d'un GAOL compilé par
    GCC 13 s'arrête sur l'option. La garder (documenté) ou la réserver à `gaol::gaol` ? (#62)
  - Fonctions sans vérification (point 45) : constructeur, texte, relations, `&`, `|`, `max`, `min`, `abs`, `sign`, `floor`,
    `ceil`, `integer`, `invabs_rel`, `mig`, `mag`, `midpoint`, `split` lisent un sous-normal comme 0 sous DAZ/FZ ; le résultat de
    `|` (en ligne) dépend du compilateur du programme ([0, 0] avec GCC 13 -O2, juste avec Clang 18). (#62)
  - Visual C++ n'a ni barrière ni `rnd_reread()` et compte sur `/fp:strict` ; le test différentiel le vérifie dans les jobs
    Windows de la CI : à surveiller. (#62)
- **5** :
  - Avec GCC, `-Ofast` n'est pas refusé dès que `-fno-fast-math` est sur la ligne de commande, avant ou après (GCC 9.4 :
    `-fno-fast-math -Ofast` laisse `__FAST_MATH__` indéfini et `__FINITE_MATH_ONLY__` à 0), et le résultat est juste.
    `doc/three-builds.md` (tableau, lignes 137-138) et `doc/using.md` disent pourtant `-Ofast` refusé. Faut-il le préciser à côté du
    paragraphe sur `-fno-fast-math` ? (#39)
  - Aucune macro ne montre ces options, qui sont donc seulement documentées et non refusées : `-funsafe-math-optimizations` et
    `-ffast-math -fno-finite-math-only` (GCC), `-fno-honor-nans` seul (Clang, qui fausse l'ensemble vide comme
    `-ffinite-math-only`), et sans doute `#pragma GCC optimize`, `__attribute__((optimize))` et `#pragma clang fp` (non testés).
    Idée non essayée : une vérification à l'exécution dans l'objet statique que contient chaque fichier incluant GAOL
    (`gaol/gaol_init_cleanup.h`), compilé avec les options de ce fichier. (#39)
  - GCC 9.4 avec `-funsafe-math-optimizations` compile la sonde `1.0 + tiny == 1.0` de `round_upward_if_needed()` en `tiny == 0.0`,
    bien que `tiny` soit `volatile`. La sonde sous-normale du point 4 subit le même sort (vu dans l'assembleur). Ranger la somme
    dans un double `volatile` avant de comparer garde l'addition (coût non mesuré). Les tests compilés avec cette option échouent :
    `rounding_direction` 2 sur 16 772 (sur `width()`), `arithmetic` 93 773 sur 1 240 155 (les références sont compilées avec
    l'option elles aussi). Faut-il traiter cela avec le point 4 ? (#39)
  - Les bornes de `tests/refused_options.cpp` sont des constantes. Si on passe le refus (en-tête sans `#error`, avec
    `-ffinite-math-only`), Clang 18 à -O2 calcule l'intersection à la compilation et le programme rend 0, donc juste ; GCC à -O3
    rend 1. Avec des doubles `volatile`, comme dans `repro.cpp`, le défaut se verrait avec les deux compilateurs. Non bloquant : les
    tests ne vérifient que le message du refus. (#39)
  - La phrase « `([1, 2] & [3, 4]).is_empty()` est faux (GCC 9.4, Clang 18) » figure dans `gaol/gaol_config.h`,
    `doc/three-builds.md`, `doc/tests.md` et `tests/refused_options.cpp`. Elle n'est vraie qu'en code optimisé (à -O0, GCC donne le
    bon résultat) et pour des bornes que le compilateur ne calcule pas à la compilation. Faut-il écrire « en code optimisé, pour des
    bornes inconnues à la compilation » ? (#39)
  - Il n'y a pas de témoin positif : `tests/refused_options.cpp` n'est compilé qu'avec les options refusées. Sa phrase « compilé
    sans elles, ce programme est juste et rend 0 » n'est vérifiée qu'à la main, et son corps peut se dégrader sans que rien ne le
    voie. Options : un troisième test sans option (environ 1 s de plus dans `make test`), ou retirer la phrase. (#39)
  - Deux phrases sont ambiguës : dans `doc/three-builds.md`, « the code including GAOL's headers is refused when the option follows
    it » (ligne 145), et dans `doc/using.md`, « where the compilation stops » (ligne 33). Le « it » peut désigner `-fno-fast-math`.
    Proposition : « quand `-ffast-math` ou `-ffinite-math-only` vient après `-fno-fast-math` ». (#39)
  - Le `#error` de `__FAST_MATH__` (`gaol/gaol_config.h:203`) ne donne aucun remède, alors que celui de `__FINITE_MATH_ONLY__` dit
    de mettre `-fno-fast-math` après l'option. (#39)
  - `.github/audit` (`compare.py:25`, `make_probe.py:15`) compare `__FAST_MATH__` mais pas `__FINITE_MATH_ONLY__`. Cela reste sans
    effet tant que l'option `-fno-fast-math` elle-même est comparée. (#39)
  - Le tableau des cibles de `doc/building.md` (ligne 280, « The unit tests of `tests/` ») donne le même `make test` pour les trois
    builds, sans dire que celui de CMake lance aussi les tests de compilation. Une phrase suffirait ; elle est facultative puisque
    `doc/tests.md` et `doc/three-builds.md` le disent déjà. (#39)
  - Le n° 4 de l'annexe B de `examples/examples.md` (lignes 1071-1072) dit que `tests/fp_strict` enregistre des cas avec
    `PASS_REGULAR_EXPRESSION`. C'est faux : il fait `try_compile()` et `message(FATAL_ERROR)` à la configuration, pour Visual C++
    seulement, et ne vérifie que `/fp:precise` et le modèle par défaut. (#39)
- **Points 5, 24** :
  - Le commentaire du refus de `-ffinite-math-only` dans `gaol/gaol_config.h` dit encore que `is_empty()` lit l'ensemble vide comme
    `!(left() <= right())`, que le compilateur replie. Depuis #47, `is_empty()` vaut `!std::islessequal(left(), right())` : le
    commentaire est à mettre à jour (et l'affirmation que `([1, 2] & [3, 4]).is_empty()` est faux à revérifier avec la comparaison
    silencieuse). (#39, #47)
- **6** :
  - x86 32 bits avec SSE2 : la lecture de MXCSR de `core_math_port.h` est limitée à x86-64 (`__x86_64__ || _M_X64`), GAOL réglant
    les deux unités sur i386. L'étendre (`__i386__ && __SSE2_MATH__`, MSVC x86 /arch:SSE2) en défense de plus, non testable ici ?
    (#54)
  - Le `#warning The floating point rounding constants have an unknown value` de `cbrt.c`, `rsqrt.c` et `asinpi.c` s'affiche
    toujours avec mingw-w64 (texte amont gardé) ; le correctif 6 de 3rd/README.md le supprime en amont. (#54)
  - 3rd/README.md, « How the changes are checked » : le premier tiret dit que chaque fonction dont le fichier a changé donne les
    mêmes bits que l'amont sur 40 millions d'arguments ; `cbrt.c` a changé et diffère par construction avec mingw-w64 x64. Dire que
    le changement 6 est vérifié par tests/core_math.cpp dans les jobs MinGW/MSYS2 x64 (210 échecs sans lui sous wine). (#54)
  - Commentaire de `round_upward_if_needed()` : « it computes none of GAOL's doubles » est faux au pied de la lettre avec mingw-w64
    x64, dont `ldexp()` est le `fscale` x87 (appelé par gaol_interval.cpp et exp2m1.c, pour des résultats exacts). Écrire « none of
    GAOL's doubles but exact ones ». (#54)
  - `gaol/core_math_port.h` : « as the get_rounding_mode() of cbrt.c, rsqrt.c and asinpi.c does » vaut pour GCC et Clang
    (`__x86_64__`), pas pour Visual C++ x64, où ces fichiers appellent le `fegetround()` générique (devenu la lecture de MXCSR).
    Même effet ; précision de texte. (#54)
  - `examples/examples.md` décrit encore le défaut de `pow` sous-normal comme ouvert : ligne 3 du tableau 5.1 (« GAOL's own test
    lists that state … but no subnormal pow »), non marquée Fixed, et annexe B n° 3. (#54)
  - Les vérifications `cbrt_hard_cases()` de tests/core_math.cpp n'échouent qu'avec mingw-w64 sur x64 et n'ont tourné que sous wine
    (mingw-w64 11) ; pour MSYS2 le défaut est déduit des en-têtes. À confirmer par les jobs MinGW-w64 x64 et MSYS2 UCRT64/CLANG64.
    (#54)
- **6b** :
  - `GAOL_RND_MINGW_FENV_ONLY` (`gaol/gaol_fpu_fenv.h` l. 163-166 : x86, mingw-w64 avant 12, sens d'arrondi par `fesetround()`) ne
    paraît plus nécessaire : avec les registres écrits directement, le mingw-w64 11 i686 d'Ubuntu passe tout ctest sous wine32. Le
    garder (prudent, c'est ce que teste la CI) ou le supprimer (plus rapide). (#51)
  - mingw-w64 ARM64 lié à `msvcrt.dll`, ou 11 avec l'UCRT, est accepté alors que son `round()` est celui de mingw-w64, calculé en
    doubles ; Clang 18 pour aarch64 compile `round()` en `frinta` et `fma()` en `fmadd` même à -O0, donc ni GAOL ni CORE-MATH ne les
    appellent, et `tests/core_math.cpp` attraperait une chaîne qui le ferait. Refuser ARM64 avant 12 ou sans `_UCRT`, comme sur
    x86-64, ou laisser. (#51)
  - Le refus de mingw-w64 ARM avant 11 (`gaol_config.h` l. 305-306) est un reste de l'ancienne condition `!__x86_64__ && < 11`,
    visant le x86 32 bits : rien de testé ne montre une borne fausse sur ARM64 avant 11. Le garder (prudent, c'est le cas) ou le
    lever. (#51)
  - Le refus du x86 32 bits se fait par version (avant 11) alors que la preuve vient d'une seule chaîne : WinLibs GCC 11.2
    (mingw-w64 9), dont le `fma()` est compilé sans optimisation. Un autre mingw-w64 i686 avant 11 compilé avec optimisation aurait
    le `fma()` x87 du 11, accepté ; le message dit « as the mingw-w64 9 of WinLibs » pour cela. Garder le refus par version, ou le
    restreindre. (#51)
  - Un programme qui définit lui-même `_UCRT` (ou `__MSVCRT_VERSION__=0x1400`) tout en se liant à `msvcrt.dll` n'est pas refusé :
    aucune macro ne le montre, seul `tests/core_math.cpp` échoue. Accepter la limite (c'est écrit dans `gaol_config.h`), ou autre
    chose. (#51)
  - Un instantané git de mingw-w64 qui se dit 12 mais date d'avant le déplacement de `math/fma.c` et `math/round.c` dans
    `src_msvcrt_common` passerait avec les en-têtes UCRT tout en liant ceux de libmingwex (théorique, date du déplacement non
    vérifiée) ; le test de `tests/core_math.cpp` l'attraperait. (#51)
  - Le `fma()` de mingw-w64 (quatre produits de moitiés, quatre arrondis) et son `round()` (dépend du sens d'arrondi, faux en
    0x1.fffffffffffffp-2) sont des défauts de mingw-w64, encore là pour msvcrt dans les versions 13 et 14 : les signaler à mingw-w64
    ? Rien n'est envoyé, et `3rd/README.md` (point 31) ne traite que CORE-MATH. (#51)
  - Le `fma()` de `ucrtbase.dll` n'a été vérifié que sur les runners de la CI, qui ont FMA3 : son chemin logiciel, sur un processeur
    sans FMA3, ne l'a jamais été (le texte dit maintenant « which pass the tests »). Le test de `tests/core_math.cpp` ne le verrait
    que sur une telle machine. (#51)
- **7** :
  - Le résultat de GAOL 4 pour `atanh([1])` n'a pas été mesuré : sa variante libm ne compile pas, et ni CRLibm ni APMathlib ne sont
    installés. La puce proposée pour `doc/differences.md` dit seulement que GAOL v5 donnait `[DBL_MAX, +oo]` pour `atanh([1])` et
    `atanh([1, 5])`, et que `atanh([-1])` était déjà vide. La garder, l'adapter ou la retirer. (#31)
  - Dans le manuel, la phrase sur le domaine ]−1, 1[ de `atanh` (entrée `atanh`) doit-elle porter `\newinvfive` ? Pour l'instant,
    seul le paragraphe suivant, sur les fonctions de CORE-MATH, la porte. (#31)
  - `doc/compare/special_cases.md` ne contient que `atanh([-1, 1])` et `atanh([2, 3])` (cas 124 et 125). Ajouter `atanh([1])` et
    `atanh([-1])` à `doc/compare/code/cases.py` obligerait à relancer les cinq bibliothèques. Non fait. (#31)
  - Les tests de `atanh_rel` se limitent à `TEST_EMPTY` et à des `TEST_EQ` à bornes finies, parce que `hausdorff(x, x)` valait +oo
    pour une borne infinie. Ce n'est plus le cas depuis le point 10 : un `TEST_EQ` à borne infinie (par exemple `atanh_rel([0.5, 1],
    [0, +oo])`) peut maintenant être ajouté (facultatif). (#31)
  - Aucune vérification n'échoue quand on retire la clause `J.right() == -1.0` de `atanh()`. `atanh([-1])`, `atanh([-5, -1])` et
    `atanh([-oo, -1])` sont vides par le constructeur, `interval(-oo, -oo)` étant vide. La clause ne serait protégée que si cette
    règle du constructeur changeait. (#31)
- **Points 7, 9, 14** :
  - `examples/examples.md` compte encore ouverts les numéros 5 (atanh, #31), 8 (tan, #36) et 15 (`what()`, #32). Pour le 8, l'annexe
    B nomme le drapeau `narrower_than_pi = (w <= pi_dn)`, appliqué sous forme repliée. Pour le 15 : ligne 15 du tableau 5.2, item 15
    de l'annexe B, et item 9 de la priorité 3. Les phrases qui comptent les numéros appliqués (« 1, 9, 11, 13, 14 and 18 ») sont
    aussi à mettre à jour. C'est laissé à la pull request de synthèse, mais la « Reprise » de TODO.md ne cite pas ce fichier. (#31,
    #32, #36)
- **8** :
  - Une borne nulle (`pow([0, b], n)`, `pow([a, 0], n)`) envoie les deux bornes aux produits arrondis : la borne supérieure dépasse
    la plus serrée pour 88 % des b de [0,5, 2] (n = 3 à 10), jusqu'à 10 doubles. `ipow_exact_dn(0)` pourrait rendre 0 exactement
    (une ligne). Plus généralement, une borne hors de la plage suffit pour que les deux bornes prennent les produits arrondis
    (`pow([0, 1.0000001], 2^32 - 1)` a une borne haute à des milliards de doubles). À changer ? (#61)
  - La garantie passe de n 2^-104 à 5 n log2(n) 2^-104 (borne prouvée (3s + 2m) n 2^-104, mesurée environ 1,8 n log2(n) 2^-104) :
    relire la formulation. (#61)
  - L'ancien code SSE2 par le bit de poids fort (`MSB_position()`, `reverse_bits()`) reste sous `#if 0` : le supprimer ? (#61)
  - Le signe d'une borne nulle diffère toujours entre SSE2 et FPU (`sqr([-2, 3])` : `[-0, 9]` ou `[0, 9]`) ; documenté, laissé
    selon la décision de ne pas normaliser les zéros. (#61)
  - Formulation de `doc/accuracy.md` et du manuel : « les deux bornes quand l'une est 0 ou sous 2^-968 » est approximative pour une
    puissance paire d'un intervalle contenant 0, dont la plus petite borne n'est jamais élevée (`pow([-2^-400, 2], 4)` prend les
    produits exacts). (#61)
- **9** :
  - Le drapeau `narrower_than_pi` est supprimé, et le test du haut de `tan()` devient `!(w <= pi_dn)`, au lieu de corriger le
    drapeau en `(w <= pi_dn)` comme l'écrivait le TODO. Même comportement, moins de code. Le garder à la lettre est trivial si vous
    le préférez. (#36)
  - Un intervalle sans pôle dont la largeur exacte est strictement entre `pi_dn` et π donne encore `[-oo, +oo]` (`w` s'arrondit à
    `pi_up`). Le rendre serré demanderait de comparer `r - l` à π en double-double. On n'en connaît aucun, ni sur les 600 premiers
    pôles (3 à 20 doubles autour de chacun), ni sur 120 000 pôles en relecture : il faudrait que les distances des deux bornes à
    leurs pôles fassent ensemble moins de π − `pi_dn` ≈ 1,22·10^-16. Garder « above π̲ and below π » dans la documentation, ou dire
    que le cas est théorique. (#36)
  - Le commentaire de `tan()` au-dessus de `const double w` dit « the test was `w < pi_dn` », alors que l'ancien code testait `!(w <
    pi_up)`, puis `w < pi_dn` pour le drapeau. Il dit aussi « though none holds a pole », qui se lit comme si aucun intervalle de
    cette largeur n'avait de pôle (`[0, pi_dn]` contient π/2). Enfin, l'entrée `tan` du manuel dit `[-oo, +oo]` « quand I contient
    un pôle », sans les largeurs entre π̲ et π, que seul le tableau d'exactitude mentionne. (#36)
  - `cos_or_sin()` garde ses tests `w < pi_dn` et `w < 2.0*pi_dn`. Selon l'auteur, `narrow_halves` et la platitude du cosinus près
    d'un extremum donnent de toute façon le résultat le plus serré. Ce n'est pas démontré, seulement mesuré : 29 400 intervalles de
    largeur voisine de π, 0 mauvais résultat pour sin et cos. (#36)
- **10** :
  - `hausdorff()` sur un intervalle à borne infinie coûte maintenant environ 14 ns au lieu d'environ 4,5 ns (test du sens d'arrondi
    et arithmétique). Une sortie anticipée est possible : rendre +oo quand une borne n'est infinie que dans un des deux intervalles,
    et n'entrer dans l'arrondi que pour les paires finies. L'auteur a gardé la formule unique, plus simple. (#41)
  - `doc/differences.md` (ligne 288) a déjà une puce sur `hausdorff()` (arrondi vers le haut). Le texte sur les bornes infinies et
    sur `nb_fp_numbers()` avec −0 sera-t-il une puce à part, ou fusionné avec elle, dans la pull request de synthèse ? (#41)
  - L'entrée `hausdorff` du manuel (`gaol.tex`, lignes 3021-3034) porte deux `\newinvfive` de suite, après « rounded upward » et
    après le paragraphe sur les bornes infinies. On pourrait fondre ce paragraphe dans la première phrase pour n'en garder qu'une.
    (#41)
- **11** :
  - Dans le manuel (`gaol.tex`, paragraphe « Numbers », lignes 3418-3422), la phrase sur flush-to-zero et denormals-are-zero porte
    `\newinvfive` sans dire ce que faisait GAOL 4 (`strtod()` en arrondi dirigé), qui n'a pas été testé sous DAZ. Retirer la marque,
    ou tester GAOL 4 (`GAOL_V1`) ? (#40)
  - Le manuel écrit `\code{-Ofast}` à la ligne 3420, alors qu'il écrit partout ailleurs les options de compilation avec
    `\option{...}` (30 usages, dont les lignes 345 et 348). (#40)
  - Dans `tests/numbers.cpp`, `#if GAOL_TESTS_HAVE_MXCSR` (lignes 176, 238 et 320) teste une macro indéfinie hors x86, ce qui
    avertit sous `-Wundef`. `#ifdef` serait plus propre. (#40)
  - Sous Rosetta (jobs macOS 14 et 26 x86_64), on ne sait pas si le test DAZ de `numbers` s'exécute ou se saute avec son message.
    ctest n'affiche pas la sortie d'un test réussi. Les deux issues sont sûres, mais c'est à vérifier une fois, par exemple avec
    `ctest -V -R numbers`. (#40)
- **13** :
  - `[a]` suppose que `bound_to_text()` arrondit exactement vers l'extérieur, de sorte que deux textes égaux soient le double
    lui-même. C'est vérifié sur 11 788 textes, mais pas avec le drapeau `std::hexfloat`, où c'est la bibliothèque C qui écrit les
    chiffres, dans le sens d'arrondi courant. (#43)
- **14** :
  - Sans explication, `what()` renvoie le texte fixe `gaol_exception`. Un autre texte (`GAOL exception`, ou le nom de la classe par
    `typeid`) demanderait une ligne dans `gaol/gaol_exceptions.cpp` et une dans `tests/expressions.cpp`. (#32)
  - `operator<<` d'une exception écrit `file, line n: exception thrown: explication`. La forme plus courte `file, line n:
    explication` est possible. (#32)
  - `doc/using.md`, `doc/tests.md` et le manuel disent sans réserve que GAOL 4 donnait `std::exception`. C'est le texte de libstdc++
    et de libc++ ; le `what()` de `std::exception` sous Visual C++ est `Unknown exception`. Reformulation facultative. (#32)
  - `what()` renvoie `explanation_.c_str()`. Une explication qui contient un NUL est coupée à cet endroit, et une qui commence par
    NUL donne un texte vide, contre le « never empty » du commentaire. GAOL ne lève jamais une telle explication : c'est une
    question de formulation. (#32)
  - Les constructeurs qui prennent une explication `const char*` (`input_format_error`, `unavailable_feature_error`,
    `invalid_action_error`) passent un pointeur nul à `std::string`. C'est un comportement indéfini : libstdc++ lève
    `std::logic_error` « basic_string::_M_construct null not valid » pendant la construction, y compris par `gaol_ERROR(excep,
    NULL)`. Aucun appel de GAOL ne le fait. (#32)
  - Le manuel ne documente que les constructeurs de `gaol_exception` à `const char* e`. Les classes prennent aussi une `const
    string&`. (#32)
  - La macro `\defmethod` de `manual/v5/manual.cls` peut laisser l'en-tête d'une entrée seul en bas de page : c'est le cas de
    l'entrée `what()`, aux pages 80-81 de la mise en page de #32. À revoir quand le PDF sera régénéré. (#32)
  - Dans `doc/tests.md`, paragraphe du test `expressions`, « The reading of » reste seul sur une ligne courte : il faut refaire les
    retours à la ligne. (#32)
  - `operator<<` d'une exception appelle `explanation()` deux fois, et copie donc la chaîne deux fois. Sans conséquence. (#32)
- **15** :
  - Avec `exceptions(eofbit)` seul, un intervalle lu en fin d'entrée lève depuis `std::ws` avec l'état `eofbit` seul, là où un
    double reçoit `eofbit | failbit` ; et une dernière ligne sans fin de ligne est perdue (`getline` lève avant l'analyse, déjà vrai
    avant). Accepter (c'est documenté : `doc/tests.md` ne revendique l'équivalence avec double que pour rien, `failbit` et `badbit |
    failbit`), ou ajouter le code. (#46)
  - Le commentaire de `operator>>` (`gaol/gaol_interval.cpp` l. 541, « std::ws is no extraction: it constructs no sentry ») et
    `doc/tests.md` l. 194 (« std::ws alone does none of this ») présentent comme un fait général ce qui est vrai de libstdc++ 9 et
    10 ; C++11 fait de `ws` une entrée non formatée qui construit une sentinelle, et libc++ le fait. Écrire « le std::ws de
    libstdc++ », pour que personne ne retire la sentinelle au nom de la norme. (#46)
  - L'exemple du manuel `cout << d << ' ' << x << ' ' << (in >> x ? "more" : "end");` (`gaol.tex` l. 3342) lit et écrit `x` dans la
    même expression ; il n'affiche `1.5 [1, 2] end` que parce que la lecture qui échoue laisse `x` inchangé. En faire deux
    instructions. (#46)
  - L'entrée `operator>>` du manuel (`gaol.tex` l. 3311-3313) enchaîne dans une phrase « n'est pas une ligne à lire, avec ou sans
    `std::noskipws` » et « un intervalle ne s'écrit pas sur plusieurs lignes » : en faire deux phrases. (#46)
  - `doc/tests.md` l. 184-185, « Blank lines have to be skipped, as the blanks before a number are, with or without `std::noskipws`
    », se lit comme si un nombre sautait les blancs sous `noskipws`, ce qui est faux. Proposé : « blank lines have to be skipped,
    with or without std::noskipws, which stops the blanks before a number but not the reading of a line ». (#46)
  - `stream_without_buffer()` (`tests/numbers.cpp`, appelée en dernier dans `main()`) : en cas de régression le programme meurt par
    SIGSEGV, et comme stdout est tamponné, aucun des échecs précédents ne s'affiche. Ajouter `std::fflush(stdout)` avant l'appel.
    (#46)
  - `std::ws` saute `\v` et `\f` en tête de ligne, alors que le lexeur ne les compte pas comme blancs (règle `SPACE` = `[ \t\r\n]+`)
    et les refuse en fin de ligne : asymétrie sans conséquence pour la lecture ; aligner le lexeur, ou laisser. (#46)
  - Des textes disent encore qu'une ligne vide fait lever `input_format_error` : `examples/examples.md` l. 62-63 (« blank lines,
    which operator>> refuses »), l. 457-459 et la ligne 14 du tableau 5.2 (l. 634), et la puce « operator>> ends with the input » de
    `doc/differences.md` (l. 402-403, « a blank one included »). À corriger dans la pull request de synthèse. (#46)
  - Le commentaire Doxygen de `operator>>` (`gaol/gaol_interval.h` l. 790-800) ne dit pas, contrairement au manuel (l. 3318),
    qu'avec les exceptions désactivées une ligne refusée écrit un message sur `cerr` et arrête le programme (facultatif). (#46)
- **16** :
  - `intervalToText` a toujours 16 chiffres ; suivre `interval::precision()` tiendrait en une ligne. (#57)
  - Le format hexa écrit toujours un point `[a, a]`, bit à bit, avec les signes des zéros. (#43, #57)
  - Sous `std::showpos`, le rayon s'écrit `+2 (+/- +1)` ; sous une locale qui groupe les chiffres, le milieu est groupé et le rayon
    non. (#57)
  - `bound_to_text()` sous denormals-are-zero compare à 0.0 : à 1 chiffre, [22 × 5e-324] s'écrit `[1e-322]` (point 45 du TODO).
    (#57)
  - Point nul : `[0]` quels que soient les signes des bornes (choisi : un seul texte pour {0}, le même en SSE2 et en FPU), ou `[-0]`
    quand les deux bornes sont -0 (une ligne). Les deux se relisent {0}. (#57)
  - Formats width et center : le milieu s'écrit `-0` quand `midpoint()` vaut -0 ([-2^-1073, 2^-1074], [-2^-1074, 0] donnent `-0 (+/-
    4.94e-324)`), alors que le manuel montre 0 pour [-0, 0] et que le format bounds écrit maintenant `[0]`. (#57)
  - `#include <sstream>` ne sert plus dans `gaol/gaol_ieee1788.h` depuis que `intervalToText` est dans libgaol ; gardé, des
    programmes pouvant compter dessus. Le retirer ? (#57)
  - Le format width sous une locale à virgule n'est pas testé ; il écrirait `0,2 (+/- 0,1000000000000001)`. (#57)
  - `tests/rounding_direction.cpp` tire deux `random.uniform()` dans les arguments d'un même `std::make_pair` : l'ordre d'évaluation
    non spécifié donne d'autres opérandes selon le compilateur (corrigé par #57 pour `Random` et `numbers.cpp`, pas ici). (#57)
  - Locale à virgule : la garde de `display_bounds()` teste le texte (`left.find(',')`), pas `numpunct::decimal_point()` :
    `interval(4)` reste `[4]`, -2.5 s'écrit `[-2,5, -2,5]` (refusé) ; un zéro s'écrit avec le texte de +0 deux fois (`[0,000,
    0,000]`). Garder ? (#57)
  - Le format agreeing écrit encore les zéros avec leurs signes : `~[-0., 0.]` pour `interval::zero()` en SSE2, `-0.000000000000000`
    pour `interval(-0.0)`, contre la règle `[0]` du format bounds. (#57)
  - Défaut du format agreeing : [1, 10] s'écrit `1~[., 0.]`. `I.right() > 10*I.left()` est faux à l'égalité, et le préfixe commun
    `1` coupe `1.000000000000000` et `10.00000000000000`. (#57)
  - Sous une locale à virgule, `gaol_tests::hex()` suit la locale globale et écrit `0x1,4p+2` dans les messages d'échec. Cosmétique,
    antérieur à #57. (#57)
  - On ne sait pas si l'UCRT écrit un sous-normal sous DAZ comme sans : sinon `subnormal_output()` (tests/numbers.cpp) imprime son
    message et saute au lieu de vérifier. (#57)
  - Formats width et center documentés comme des affichages. L'alternative gardée en réserve : un rayon qui absorbe l'arrondi des
    chiffres du milieu, sûr mais `0.1 (+/- 5.6e-18)` pour le point 0.1 et `3.5 (+/- 0.049)` pour [3.452, 3.453] à 2 chiffres. En
    faire un format à part ? (#57)
  - CI de #57 : `numbers` dépassait 300 s dans les quatre jobs Visual Studio Debug x86 et x64 ; `d78af72` tire 30 intervalles au
    lieu de 300 sans `NDEBUG`. À confirmer par la CI. (#57)
  - `examples/examples.md` décrit encore le point 13 avec `<a, b>` / `<a, a>` (l. 62, ligne 12 du tableau 5.1 non marquée Fixed,
    annexe B n° 12 l. 1131) : noter que c'est devenu `[a]`. (#57)
  - `examples/examples.md` décrit encore la revue n° 21 (formats width et center, intervalToText) comme ouverte : ligne 21 du
    tableau (l. 641, non marquée Fixed) et annexe B (l. 1187) ; l. 566 dit encore que le format width est décrit « midpoint and
    width ». (#57)
  - Le runtime C Debug de Visual C++ déclenche sa propre assertion (« unexpected input value; log10 failed », `cfout.cpp`) quand
    il écrit un sous-normal sous denormals-are-zero : `numbers` bloquait sur sa boîte de dialogue. Un programme Debug qui a mis DAZ
    et écrit un intervalle à borne sous-normale la reçoit aussi, par `operator<<`, et le runtime écrit alors « 0 » pour la
    borne (la sortie n'est plus un encadrement, relue « [0] »). GAOL doit-il retirer DAZ de MXCSR le temps
    d'écrire ses bornes ? À décider avec le point 45. (#58)
- **18** :
  - `gaol_read_uncertain()` fait ses sommes avec `gaol_decimal_add_sub()`, qui insère en tête d'une `std::string`, ce qui est
    quadratique aussi. 20 000 chiffres prennent 0,07 s contre 0,045 s sous la forme simple, et 100 000 chiffres 1,3 s contre 0,5 s.
    La décision 18 vise-t-elle aussi la forme incertaine ? (#42)
  - Dans `tests/numbers.cpp`, le message d'échec du test de temps est construit avec `std::to_string` pendant que la locale à
    virgule est active : il affiche « 2,112549 s ». On peut construire le message avant les `setlocale`, ou dans un flux de la
    locale C. (#42)
  - Le commentaire de `gaol_number::beyond` (`gaol_interval_lexer.lpp:216`) laisse croire qu'il est positionné dès que v est sous le
    plus petit double positif ou au-dessus du plus grand. En fait il ne l'est que loin au-delà (10^311 et plus, ou sous 10^-330). La
    ligne fait aussi 119 colonnes : mettre le commentaire au-dessus du membre. (#42)
  - Le commentaire de `gaol_enclose_number()` (`gaol_interval_lexer.lpp`, lignes 370-372) dit que la lecture sous une locale à
    virgule prend le temps de la locale C. Mesuré : un rapport de 1,0 à 1,2. Écrire « à peu près le temps ». (#42)
- **24** :
  - FE_INVALID levée sur des opérandes non vides : `0 × oo` dans l'`operator*=` des intervalles SSE2 (`[0]*[1, +oo]`, `[0,
    +oo]*[0]`, et `pow([1], [1, +oo])` qui l'appelle), et le `pow` de CORE-MATH pour un exposant de magnitude extrême dans tous les
    builds (`pow([1, 2], [4.9e-324])`, `[1e-300]`, `[1e300, 1e308]`). Documenté dans `doc/using.md` et le manuel (« This list is not
    exhaustive »), non corrigé et sans test : le corriger, ou le laisser documenté. (#47, #50)
  - Avec `GAOL_PRESERVE_ROUNDING` et les intervalles SSE2, `GAOL_RND_ENTER_SSE()` écrit MXCSR avec `GAOL_SSE_MASK | _MM_ROUND_UP` et
    `GAOL_RND_LEAVE_SSE()` ne rend que les bits d'arrondi : `+`, `-`, `*`, `/`, `sqr()`, `inverse()`, `%`, `div_rel` et les formes à
    opérande double masquent de nouveau les exceptions du programme (MXCSR 0x1d80 avant, 0x1f80 après) et effacent ses indicateurs.
    Le build « preserve » ne devrait changer que les bits d'arrondi (proche du point 27). (#47)
  - `doc/using.md` (fin de « The floating-point exceptions ») et le manuel (`gaol.tex` l. 1078) listent `+`, `-`, `*`, `/`, `sqr()`
    et `inverse()` comme les opérations qui masquent les exceptions sous `GAOL_PRESERVE_ROUNDING` : il manque `%`, `div_rel` et les
    formes `+= d`, `-= d`, `*= d`, `/= d`, `%= d`. Les ajouter, ou écrire « entre autres » ; dire aussi qu'elles effacent les
    indicateurs. (#47)
  - « after an operation of GAOL, `fetestexcept(FE_INEXACT)` is raised whatever the result » (`doc/using.md` l. 384, `gaol.tex` l.
    1064) est trop fort : `-X`, `abs`, `&`, `|`, `floor` et `max(X, Y)` ne lèvent pas FE_INEXACT, et sous `GAOL_PRESERVE_ROUNDING`
    un `X+Y` exact laisse tous les indicateurs à zéro. Écrire « after most operations », comme la puce au-dessus. (#47)
  - `interval(double)` et `interval(double, double)` comparent leurs arguments par des comparaisons signalantes : `interval(NAN)`,
    `interval(NAN, 1)`, `x += NAN`, `set_contains(NAN)` lèvent FE_INVALID (on peut défendre le piège, puisque le NaN vient du
    programme). Les garder, ou rendre silencieuse la première comparaison du constructeur, ce qui rendrait aussi `floor`, `ceil` et
    `integer` gratuits (voir 24b). (#47, #50)
  - `gaol/gaol_intervalf.h` appelle `std::islessequal` (l. 168) sans inclure `<cmath>` (il n'inclut que `<iosfwd>`, `<new>`,
    `<limits>`) : l'en-tête ne compile plus seul avec `-DGAOL_FLOAT_INTERVALS=1`. Ajouter `#include <cmath>`, ou retirer ce
    changement des intervalles de flottants. (#47)
  - `doc/accuracy.md` garde une ligne courte (« hold on every architecture and with », l. 60) après l'ajout de la condition sur les
    exceptions : re-justifier le paragraphe. (#47)
  - Dans `tests/rounding_direction.cpp`, la structure `EmptySet` (l. 230) porte aussi le tableau `nonempty_sets` (l. 248) : un nom
    neutre (`SampleInterval`) serait plus lisible. (#47)
  - Manuel, section 3.3 « The floating-point exceptions » (`gaol.tex` l. 995-1080), toute nouvelle en v5 : `\newinvfive` marque
    l'introduction et les puces, pas les derniers paragraphes (indicateurs, initialisation, `GAOL_PRESERVE_ROUNDING`). (#47)
- **24b** :
  - La CI de #50 est rouge sur armhf : 6 échecs sur 58 « raises no invalid-operation flag » (`x & empty`,
    `gaol_ieee1788::intersection(x, empty)`, `nth_root_rel`, `asinh_rel`, `atanh_rel`, `invabs_rel`). Cause : l'if-conversion de
    GCC 12 à 14 rend signalante (`vcmpe`) la comparaison des bornes d'`operator&=`. Corrigé par #60, ouverte, après trois
    relectures. (#50, #60)
  - POWER9 (`-mcpu=power9`, défaut d'Ubuntu ppc64el) : `x &= empty` lève FE_INVALID à -O2 avec GCC 13 (`xscmpgedp`), déjà sur la
    base (code `#else` de #50) ; la CI ppc64le (Debian, base POWER8) ne le voit pas. Prendre aussi le code ARM sous `_ARCH_PWR9` ?
    Point à part. (#60)
  - Les choix du programme sur `is_empty()` ou une relation d'un intervalle vide lèvent FE_INVALID sur ARM 32 bits et POWER9 (GCC
    rend la comparaison signalante en l'if-convertissant) : durcir `is_empty()` sur ces cibles, ou signaler le chemin à GCC (bug
    52258 ouvert) ? (#60)
  - Le vectoriseur sur AArch64 et avec les intervalles FPU x86-64 : `set_le`, `set_strictly_contains`, `less`… lèvent FE_INVALID
    dans les boucles du programme. Non documenté. (#60)
  - Les trois vérifications de choix de `rounding_direction` (`is_empty()` d'une intersection à opérande gauche vide) sont fragiles :
    elles passent parce que GCC enchaîne le choix avec le `is_empty()` d'`operator&=` ; un futur GCC peut les faire échouer sans
    défaut de GAOL. Elles sont déjà sautées sous `__OPTIMIZE_SIZE__`. Les garder comme garde-fous ? (#60)
  - Une seule forme d'`operator&=` pour tous les processeurs (`std::isunordered(lb_, I.lb_)` puis des comparaisons simples) : plus
    rapide avec GCC sur x86-64, mitigée avec clang-18. Non retenue, x86 devant rester inchangé. (#60)
  - `floor`, `ceil` et `integer` coûtent +0,17 ns sur des opérandes non vides (1,52 → 1,69 ns avec clang-18, 1,43 → 1,60 avec GCC ;
    +8 à +17 % selon la relecture) à cause d'un `std::isunordered` ajouté avant le constructeur. Pour les rendre gratuits : (a)
    rendre silencieuse la première comparaison de `interval(l, r)`, mais `interval(NaN, x)` devient silencieux aussi ; (b)
    construire le résultat sans le constructeur (fonctions amies, `_mm_set_pd` ou `lb_`/`rb_`), `integer` ne gardant que
    `std::islessequal(ceil(l), floor(r))`. (#50)
  - Avec GCC 13, `x &= y` est de 13 à 29 % plus lent dans une boucle de débit (SSE2 1,82 → 2,05-2,33 ns, FPU 1,53 → 1,74-1,98 ns) :
    `ucomisd`, `cmov` et un branchement remplacent `vcmplesd`/`vblendvpd`, SSE2 n'ayant pas de `<=` ordonné silencieux (`_CMP_LE_OQ`
    est AVX) ; `set_leq` passe de 0,97 à 1,04-1,08 ns. Clang 18 est inchangé. Le texte de la PR #50 annonce encore un gain (6,4 →
    2,5 ns) : accepter, ou chercher une forme sans branchement. (#50)
  - Visual C++ : `std::islessequal()` et ses sœurs de l'UCRT passent sans doute par `_fpcomp()`/`_dpcomp()`, un appel là où GCC et
    Clang émettent un `ucomisd` ; l'`&=` FPU, que compile Visual C++, en fait maintenant quatre. `gaol_performance` ne mesure ni
    `&=` ni les relations. Mesurer, et au besoin un assistant GAOL avec `_mm_ucomile_sd` (pour `is_empty()` aussi). (#50)
  - Les intervalles de flottants, réservés au développement (`gaol_intervalf.h`, `gaol_interval2f.h`), gardent des comparaisons
    signalantes dans `&=` et les relations : les aligner sur `gaol::interval`, ou les laisser. (#50)
- **29** :
  - macOS et Windows lancent sans doute déjà la partie « locale à virgule » de `numbers` (`fr_FR.UTF-8`, `French_France.1252`), mais
    rien ne le montre. Lancer `sh .github/scripts/comma-locale.sh check build` après leurs tests le prouverait, sans rien changer au
    script sur macOS. (#35)
  - Conteneurs (`containers.yml`). Debian demande `apt-get install locales` puis une ligne dans `/etc/locale.gen` ou `localedef`,
    son `locale-gen` ne prenant pas de nom (vérifié sur 2.36-9+deb12u9 et 2.41-12+deb13u4). Alpine (musl) et les images manylinux
    n'ont pas de locale à virgule à générer. (#35)
  - `build-systems.yml` : les jobs Ubuntu d'autotools et de meson lancent aussi `numbers`. Leurs journaux sont `tests/numbers.log`,
    `build-meson/meson-logs/testlog.txt` et `numbers.log` : le contrôle demanderait un autre chemin de journal. (#35)
  - Variante écartée : une variable d'environnement (`GAOL_TESTS_REQUIRE_COMMA_LOCALE`) lue par `tests/numbers.cpp`, qui ferait
    échouer le test lui-même sur toutes les plateformes sans lire le journal de ctest. Elle avait été écartée parce que le point 18
    éditait ce code. Le point 18 est fusionné depuis : à reprendre si l'on veut le contrôle partout. (#35)
  - Si le contrôle reste réservé à Linux, faut-il garder un point 29 réduit dans `TODO.md` ? Texte proposé : « Le test sous une
    locale à virgule n'est vérifié que dans `linux.yml` : macOS, Windows, les conteneurs (le `locale-gen` de Debian ne prend pas de
    nom : `/etc/locale.gen` ou `localedef`) et `build-systems.yml` ne génèrent ni ne vérifient rien. Correction : `sh
    .github/scripts/comma-locale.sh check <build>` après `make test` là où la locale existe. » (#35)
  - `tests/fetch_content` et `tests/find_package` ne donnent aucun `TIMEOUT` à `numbers` : les 300 s ne sont fixées que dans
    `tests/CMakeLists.txt`. Un blocage y durerait jusqu'à la limite de 60 minutes du job. (#35)
  - D'après la note de l'image Ubuntu 26.04, la dépréciation des images Ubuntu 22 a commencé le 17 septembre, et elles ne seront
    plus prises en charge d'ici le 17 avril. Les jobs 22.04 de `linux.yml` disparaîtront (x86_64 GCC et Clang, arm64 GCC, arm64
    Clang 14 refusé, `cmake-3-14`, `gcc-9`) : que prendre à leur place ? (#35)
- **31** :
  - Envoyer les six correctifs à CORE-MATH demande votre accord : merge request sur gitlab.inria.fr (compte nécessaire) ou `git
    format-patch` à core-math@inria.fr ; qui envoie, avec ou sans `Signed-off-by`. (#56)
  - `sinpi.c` et `log1p.c` embarqués gardent `~0ul>>12`, sans effet mesuré : même changement `/* GAOL */` que sinh, cosh, tanh, ou
    attendre l'import du correctif amont ? (#56)
  - asinpi : le décalage sur 128 bits (correctif 2, et le portage de GAOL) déborde quand `asinpi_acc()` est appelé directement en
    ±(1 − 2^-53) vers le bas et vers zéro (ss = 76) ; `cr_asinpi` n'y passe jamais. La borne de ss de la preuve est à montrer aux
    auteurs (écrit dans le README). (#56)
  - Le master de CORE-MATH échoue à son propre `./check.sh --worst --rndd pow` : underflow parasite pour x = -0x1.10a688680a753p-93,
    y = 11 (argument ajouté par 8c2bc708, que 708e86ef devait corriger). Le signaler avec les correctifs ? (#56)
  - Correctif 6 proposé à CORE-MATH sous sa forme large (lire le champ d'arrondi de MXCSR quelles que soient les `FE_*`, dans cbrt,
    rsqrt, asinpi), plutôt que la petite (`mode = fegetround();` dans cbrt.c et `|| defined(_WIN32)` pour clang-cl), citée dans le
    README. À valider avant l'envoi. (#56)
  - `binary80/pow/powl.c` a la même chaîne de branches (rend des `FE_*`) : non examiné, GAOL ne s'en servant pas ; l'ajouter au
    correctif 6 ? (#56)
  - Les `~0ul` et `1ul<<52` de `binary80/atan2/atan2l.c` et `binary128/expm1/expm1q.c` (formats que GAOL n'utilise pas) n'ont pas
    été examinés. (#56)
  - 3rd/README.md après #54 et #56 : « Four of the changes above … (2 to 5) » ne compte pas le changement 6 (cbrt), qui est aussi un
    correctif ; l'item 6 dit que `rsqrt.c` et `asinpi.c` « are right », ce que le correctif 6 nuance (clang-cl, Cygwin) ; les
    statuts des correctifs 5 et 6 ne disent pas ce que fait la copie de GAOL. (#54, #56)
- **36** :
  - Point 36 : ajouter à sa documentation les mesures de TwoSum et TwoProd en arrondi dirigé (erreur inexacte dans 0,9 % et 3,8 %
    des cas, toujours trop petite ; exacte avec `fma`). (#49 (issue))
- **39** :
  - `examples/03_dependency_problem.cpp` et `06_global_optimization.cpp` écrivent le maximum de Goldstein-Price comme un double
    (1015690.2717980589, 2,97e-11 sous le vrai maximum) : leur « l'enveloppe contient l'image » teste un intervalle un peu plus
    étroit que l'image. Sans effet ici ; `textToInterval` comme dans 16 arrondirait vers l'extérieur. (#53)
  - `examples/16_Goldstein_Price.cpp` écrit le maximum `1015690.2717980589082988423`, 1,2e-20 sous la vraie valeur ; le lecteur
    arrondit vers l'extérieur (borne M + 8,7e-11), donc `range` contient l'image. Écrire `…0829884232` rendrait le texte lui-même
    majorant. Cosmétique. (#53)
  - L'exemple 16 garde dans `main()` le style de GAOL 4 : `x(-2,2)`, `y(-2,2)`, `z(0.1)`, `interval(0.,0.)` au lieu de
    `interval(0.0)`. (#53)
  - `examples/examples.md` §1.1 : « Their outputs are the same on every build » oublie le temps qu'imprime 16 ; « All 16 examples
    compile without a warning (-Wall -Wextra) » n'est vrai qu'avec les en-têtes de GAOL en -isystem (avertissements dans 04 à 11, 15
    et 16, surtout -Wdeprecated-copy). (#53)
  - `examples/CMakeLists.txt` : dans « a timing loop that checks its enclosure too, which the autotools and meson builds of GAOL
    also compile », « which » semble renvoyer à l'enveloppe. (#53)
  - Le manuel donne un avertissement de makeindex (entrées en conflit pour « canonical interval »), antérieur au point 39. (#53)
  - `examples/examples.md` §3 décrit le programme relu (sans le +1) ; depuis #53 le manuel a le +1 et la recommandation 16 dit «
    applied ». Dire que c'est corrigé ? (commit direct)
- **40** :
  - `examples/examples.md` §3 dit encore « three formatting slips » (29 booléens et un nan, corrigés par #55) et donne comme ouverts
    `chi([0,0]) = 0`, les 400 bits et `[[nodiscard]]` C++17 seulement, corrigés par #55. (#55)
  - `chi([-oo,+oo])` vaut 1, alors que chi vaut -1 pour tout autre intervalle symétrique autour de 0 : comportement de GAOL 4 et du
    manuel, gardé. (#55)
  - La doc Doxygen de `parse_interval()` (gaol/gaol_parser.h) ne liste que les formats de GAOL 4 : pas de `<a, b>`, pas
    d'expressions, `[l, f]` pour `[l, r]`, `\emp inf`. (#55)
  - Les tests de compilation `refused_*` du point 5 prennent `CMAKE_BINARY_DIR` et `CMAKE_SOURCE_DIR` comme répertoires d'inclusion
    (et `--build "${CMAKE_BINARY_DIR}"`) : faux quand GAOL est un sous-projet avec `WITH_TESTS`. Les tests nodiscard prennent
    `PROJECT_*`. (#55)
  - Un test `nodiscard_discard_*` reste en échec une fois son objet compilé avec succès (en-tête restauré avec son ancienne date) :
    il faut effacer l'objet. Seulement en développement ; correctif possible par `FIXTURES_SETUP` ou `cmake -E rm -f`, non essayé
    sous les générateurs Visual Studio. (#55)
  - Le programme qui compare les 88 sorties du manuel au programme (`run_examples.py`) est hors du dépôt : le garder avec le point
    39 ou dans `manual/` ? (#55)
  - `tests/gaol_tests.h` ne cite pas la vérification à 500 bits (mpmath) des boîtes de `pow` de tests/ieee1788.cpp, bornes
    enregistrées et non calculées par un script. (#55)
  - Visual C++ 2017 15.8 et 15.9 ne sont pas sur Compiler Explorer : la garde `_MSC_VER >= 1924` leur donne `_Check_return_` quoi
    que dise `__has_cpp_attribute`, non vérifié. (#55)
- **42** :
  - Que `WindowsApps` contienne un alias `python.exe` en plus de `python3.exe` n'a pas été testé sous Windows : l'affirmation vient
    de la connaissance des alias d'exécution de Windows 10 et 11. Elle est écrite dans le commentaire de `meson.build`, dans
    `doc/building.md` et dans le manuel. (#44)
- **43** :
  - Aucune étape Windows native (meson avec Visual C++, sous `pwsh`) ne lance `.github/scripts/version-file.sh` : le `sh` de Git for
    Windows passerait avant le `link` de Visual C++ dans le `PATH` ; seul le job meson MSYS2 exécute le script. À dire si une étape
    `pwsh` est voulue. (#45)
  - Les commandes Python de `meson.build` (l. 32, 53, 60) ouvrent `VERSION.txt` sans le fermer (`open(sys.argv[-1]).read()`) : sans
    effet sous CPython ; un `with` au besoin. (#45)
- **44** :
  - Avec les générateurs Makefile, `package_source` ne relance pas CMake après un changement de `VERSION.txt` (Ninja le fait).
    L'archive porte alors le nom de l'ancienne version et contient le nouveau `VERSION.txt` (observé : `gaol-5.0.0.tar.gz` avec
    `VERSION.txt` à 5.0.2). C'est documenté. Faut-il une vérification au moment de l'archive ? Elle demanderait
    `CPACK_PRE_BUILD_SCRIPTS` (CMake 3.19, alors que le minimum est 3.14) ou une dépendance de `package_source` sur une
    reconfiguration. (#33)
  - Le contrôle est un test CTest, `cpack_stale_configure`, qui configure des copies de l'arbre : de 1,3 à 4,4 s sous Linux, jusqu'à
    52,3 s sous QEMU ppc64le. Il n'est enregistré que sur un Unix qui construit pour lui-même, donc ni sous Windows ni sous MSYS2.
    L'autre choix serait une étape de workflow, à la façon de `linux.yml` qui lit `configure.log`. (#33)
  - `cpack_stale_configure` ne vérifie pas que l'arbre réel est cohérent (`configure` généré pour la version de `VERSION.txt`). Le
    job autotools de `build-systems.yml` le fait déjà. (#33)
  - Le test ne passe au configure imbriqué que `CMAKE_C_COMPILER` et `CMAKE_CXX_COMPILER`. Un parent dont le compilateur porte un
    argument (`CC="env gcc"`, donc `CMAKE_CXX_COMPILER_ARG1` non vide), ou un Ninja hors du `PATH`, configure bien mais fait échouer
    le test (« CMake did not configure the copy »). Correction : ne pas enregistrer le test quand `CMAKE_*_COMPILER_ARG1` est non
    vide, ou passer les compilateurs par `cmake -E env CC=... CXX=...`. (#33)
  - `cpack_stale_configure` a besoin de `sh` et de liens symboliques, et il échoue (`FATAL_ERROR`) quand l'un manque. La règle du
    dépôt est pourtant de vérifier à l'exécution ce qu'un test exige de la plate-forme, et de le dire ignoré. Aucun job de la CI
    n'est concerné. (#33)
  - Le commentaire de `tests/cpack_stale_configure.cmake` dit « removes what it made in this directory, and nothing else ». Or
    `file(REMOVE_RECURSE "${GAOL_WORK_DIR}")` retire tout le répertoire, avec pour seule garde le suffixe `/cpack_stale_configure`.
    La suppression est sûre ; c'est le commentaire qui est inexact. (#33)
  - `doc/tests.md` dit « CMake build » sans expliquer pourquoi autotools et meson n'ont pas `cpack_stale_configure` : seul CMake
    fait l'archive, et le job autotools compare déjà `configure --version` à `VERSION.txt`. (#33)
- **45** :
  - Le deuxième tiret du point 45 est à mettre à jour. `display_bounds()` compare maintenant les bornes par leurs bits (commit
    `6ad025a`, #57), si bien que sous DAZ [0, 5e-324] n'est plus écrit comme un point. De ce tiret ne reste que le chemin `x == 0.0`
    de `bound_to_text()`, que le quatrième tiret décrit déjà. (#40, #57)
- **47** :
  - Refuser clang-cl sans `/fp:strict` ? Il définit `_M_FP_STRICT` à partir de Clang 16 : `gaol_config.h` pourrait tester
    `defined(_MSC_VER) && (!defined(__clang__) || __clang_major__ >= 16)`, et `tests/fp_strict` vérifier clang-cl aussi. Plus
    urgent maintenant que clang-cl est dans la CI. (#59)
  - Cygwin x64 reste faux (les `FE_*` de newlib valent 0 à 3, sans `_WIN32`) : prendre le correctif 6 en entier (la table), ou
    refuser Cygwin dans `gaol_config.h` ? Cygwin n'est pas dans la CI. (#59)
  - Citer clang-cl x64 dans le manuel (Prerequisites) et `doc/three-builds.md` une fois la CI verte ? Ajouter le clang-cl de VS
    2026, et clang-cl x86 et arm64 (non concernés par le correctif) ? (#59)
  - Chaque build clang-cl affiche 11 avertissements `/Zc:strictStrings-` : réserver l'option à Visual C++ ? (#59)
- **49** :
  - `GAOL_INFINITY` avec clang-cl : le corriger dans sa propre pull request, par `__builtin_huge_val()` ou
    `std::numeric_limits<double>::infinity()` ? (#59)
- **Hors liste** :
  - La branche jetable `ci-debug-numbers-arm64` (sorties de débogage et workflow de variantes du diagnostic de GCC 12 sur aarch64)
    est toujours sur le dépôt : à supprimer. (#34)
  - Le `#pragma GCC optimize("no-inline-functions")` de `tests/numbers.cpp` n'est gardé que pour `__GNUC__ == 12 && __aarch64__`,
    seul cas observé (Debian 12, GCC 12.2, à `-O2` et `-O3`). D'autres versions ou d'autres architectures peuvent être touchées sans
    que la CI le montre ; GCC 13 (Ubuntu 24.04 arm64) et GCC 14 (Debian 13 arm64) passent. (#34)
  - GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` : la variable de boucle `for (int p : precisions)` est lue à une mauvaise
    adresse, d'où `"1e--1"`. Le défaut n'est pas signalé à GCC, faute de cas minimal. (#34)
  - La pull request #38, brouillon de Copilot vers `master` pour l'échec de Visual Studio 2026 x86 Release du point 1, est fermée
    sans fusion. Elle laisse la branche `copilot/fix-visual-studio-2026-x86-release-job` sur le dépôt : à supprimer. La « Reprise »
    de TODO.md ne la cite pas. (#38)
  - Clang 18 avertit dans le parser généré par bison 3.5.1 (`gaol/gaol_interval_parser.cpp:1538`) : `gaol_nerrs` est affecté mais
    jamais utilisé (`-Wunused-but-set-variable`). Chaque point l'a relevé. Faut-il le faire taire ou le laisser ? (#39, #40, #41,
    #42, #43)
  - L'en-tête de `TODO.md` dit « Ses numéros 1, 5, 6, 8, 9, 10, 11, 13, 14, 15 et 18 sont corrigés ». Les numéros de la revue que
    #39, #43 et #41 ont corrigés et qui sont fusionnés n'y figurent pas : 4 (`-ffinite-math-only`, #39), 12 (point écrit `<a, b>`,
    #43), 19 et 20 (`hausdorff()` et `nb_fp_numbers()`, #41) ; ni ceux de #53 à #57 (3 : `pow` sous-normal, #54 ; 21 : formats
    largeur et centre, #57). (#39, #41, #43)
  - `examples/examples.md` décrit encore l'ancien comportement : lignes 61-62 (« apart from point intervals written `<a, b>` »),
    section 2.7 à la ligne 464 (« One point still breaks the round trip »), numéros 4, 12, 19 et 20 du tableau de la section 5 sans
    la marque **Fixed**, et « Numbers 1, 9, 11, 13, 14 and 18 are fixed » à la ligne 607. Les n° 4, 12, 19 et 20 de l'annexe B sont
    aussi à marquer appliqués. Ce travail a été laissé à la pull request de synthèse. (#39, #41, #43)
  - Le manuel a un `Overfull \hbox` de 22,4 pt dans le paragraphe du paquet `libgaol-dev_version_arch.deb` (`gaol.tex`, lignes
    511-515, anciennement 481-485). Le défaut est antérieur à ces points. (#40, #44)
  - Mise en forme de `doc/tests.md` : des lignes de plus de 100 colonnes (24, 52, 233, 235, 243), alors que les voisines font
    environ 80, et une ligne orpheline (216 : « With flush-to-zero, denormals-are-zero or both set in MXCSR »). (#39, #41, #42)
  - Dans `tests/input_output.cpp`, `test_input()` a des assertions mortes (relevé au point 13). Le `try` attrape
    `input_format_error` et se contente de l'afficher. La ligne `<-inf,-inf>` lève une erreur (`inf` se lit [dmax, +oo], donc les
    deux bornes diffèrent), si bien que les assertions suivantes ne s'exécutent jamais : `[-inf, -inf]`, `[-inf]`, `inf`, `[inf]`,
    `[inf,inf]`, `<inf,inf>`. Correction : retirer le `try/catch`, et corriger ou retirer les deux cas `<inf,...>`. (#43)
  - Dans le format `agreeing`, la condition `I.right() > 10*I.left()` est vraie pour tout intervalle négatif, pour tout intervalle
    contenant 0 et pour [0, x] (relevé au point 13). Ces intervalles sont donc toujours écrits dans le format des bornes, jamais
    avec leurs chiffres communs. Seuls les intervalles positifs avec r ≤ 10 l sont écrits en `agreeing`. (#43)
  - `cancel_minus`/`cancel_plus` sont faux avec la bibliothèque Release compilée par GCC (`-O3`) : dans `difference_at_least()`
    (`gaol/gaol_interval.cpp` l. 2309), GCC déplace les termes d'erreur de `two_sum` après `GAOL_RND_NEAREST_LEAVE()`, donc calculés
    vers le haut. `cancel_minus([0.5], [4.9e-324, 1e-300])` donne [0x1.fffffffffffffp-2, 0.5] au lieu de [-oo, +oo],
    `cancel_minus([DBL_MAX], [0.5])` donne [-oo, +oo] au lieu de [pred(DBL_MAX), DBL_MAX], et `cancel_minus([DBL_MAX], [1])` lève
    FE_INVALID ; Debug et Clang sont justes. Correction proposée : `GAOL_RND_KEEP` sur `s1`, `e1`, `s2`, `e2`, un test, et le
    commentaire de `GAOL_RND_NEAREST_ENTER` (`gaol/gaol_fpu.h`) à corriger ; à ajouter comme point du TODO. (#47, #50)
  - La branche jetable `ci-debug-mingw-numbers` (diagnostic de #48) est encore sur le dépôt ; la liste des branches à supprimer de
    TODO.md ne la cite pas. (#48)
  - `numbers` a dépassé son délai (Timeout) sur « macOS 15 x86_64 GCC Debug ASan+UBSan » dans un des deux runs de #50 (job
    110043631810), et passé dans l'autre (job 110043346835) : test trop long en Debug avec sanitizers sur ce runner, à surveiller ou
    à alléger comme pour Visual C++ (d78af72). (#50)
  - Messages d'autoconf contraints par m4 : le texte cité écrit `?` pour tout caractère autre que chiffres, points et blancs (les
    octets en hexadécimal restent exacts ; une citation exacte demanderait des quadrigraphes), et les indices n'ont pas de virgule,
    dans les quatre lecteurs pour qu'ils restent pareils. (commit direct)
  - Le manuel ne donne pas le cas résiduel de WindowsApps (profil dont le répertoire diffère de `USERPROFILE`) que doc/building.md
    donne ; son « turn off the aliases » le couvre. (commit direct)
  - L'indice « the file is UTF-16 » est donné pour tout fichier commençant par FF FE ou FE FF, y compris UTF-32LE et FF FE suivi
    d'ASCII. Heuristique jugée sans danger, gardée. (commit direct)
  - L'étape de CI « VERSION.txt read by autoconf, as by configure » coûte environ 10 s sur les quatre entrées Linux du job
    autotools, dont les deux à sens d'arrondi préservé ; la limiter à `matrix.cfg.configure == ''` ? (commit direct)
  - Les descriptions de #50, #51 et #53 à #57 finissent par « 🤖 Generated with Claude Code » et le lien de la session Claude, alors
    que `.claude/CLAUDE.md` interdit tout ce qui crédite Claude (règle écrite pour les commits). Retirer ces lignes des
    descriptions, et étendre la règle aux pull requests ? (#50, #51, #53, #54, #55, #56, #57)
  - La pull request #52 (branche de session vers `master`, `doc/math-libraries.md`, fichier déjà identique dans `configure-clean`)
    est fermée sans fusion ; sa branche `claude/todo-md-contents-xfervz` reste sur le dépôt : à supprimer. Ni TODO.md ni la liste de
    ménage de #49 ne la citent. (#52)
  - Avant la réécriture de `TODO.md` (a2ca992), la PR #29 laissait cinq choix des trois builds « à confirmer » : 8, la référence
    HTML de Doxygen supprimée ; 9, meson sans `enable-debug` ni `enable-optimize` (le type de build les remplace) ; 10, la CI ne
    lance que `make test`, pas les exemples (#53 le rappelle : aucun job ne compile les exemples) ; 11, `make test` n'écarte les
    exemples qu'à partir de CMake 3.17 et meson 0.57 ; 12, le premier `make perf` déplace les colonnes des autres bibliothèques
    (`results.csv`). S'y ajoutait le point 7, passer GAOL v5 sous licence MIT. Le TODO actuel ne les reprend pas : acceptés,
    abandonnés, ou à reporter ? (#29)

### Reprise

État au 2 octobre :

- **#58** et **#59** (point 47) sont fusionnées dans `configure-clean` (61c7503). Les deux jobs clang-cl x64 restent rouges par le
  point 49 (`interval()`, `operator/`, `pow(x, n)`).
- **#60** (`fix-24b-armhf-fe-invalid` → `configure-clean`, f4881f6) : `operator&=` reconnaît l'opérande vide avant de comparer
  les bornes, sur ARM 32 bits seulement. Trois relectures indépendantes, la dernière sans point bloquant ; 61 vérifications, 0 échec
  sous qemu avec GCC 12.4, 13.3 et 14.2, 6 échecs sur la base. Ouverte le 2 octobre.
- **#61** (`todo-08-pow-large-n` → `configure-clean`, point 8) : approuvée par une relecture indépendante sans point bloquant ; ses
  remarques de formulation reprises (341d2ae). Contient la branche de #60 pour que les jobs armhf passent. Ouverte le 2 octobre.
- **#62** (`todo-04-ftz-daz` → `configure-clean`, point 4) : deux relectures indépendantes, les quatre points bloquants de la
  première corrigés, les remarques de la seconde reprises (e7b9f18, 709486d). Contient la branche de #60. Ouverte le 2 octobre.
- **#63** (`todo-03-pow-exact-corner` → `configure-clean`, point 3) : trois relectures indépendantes ; la dernière, sans point
  bloquant, vérifie `pow_is_double()` contre son propre oracle sur 488 402 couples ; ses deux remarques reprises (5b50af1).
  Contient la branche de #60. Ouverte le 2 octobre.
- **Points 12, 17, 36** : `configure-clean` fusionné dans 12 et 17 ; le travail reste « WIP » (12 : dernier commit WIP 553e649 ;
  17 : fusion 3b9311c ; 36 : inchangé). À terminer, relire, puis pull requests.
- Corriger le point 48, décider du point 49, fusionner #29 (`configure-clean` vers `MATH-CORE`).

Ensuite : commencer les points non commencés ; écrire la pull request de synthèse (retirer les points faits de ce fichier,
consigner les changements dans `ChangeLog` et `doc/differences.md`, régénérer une seule fois `manual/v5/gaol.pdf`) ; mesurer enfin
le point 33 sur une machine au repos. Le 2 attend le 3. Supprimer les branches fusionnées encore présentes : `todo-01`, `05`, `10`,
`11`, `13`, `15`, `16`, `18`, `24`, `24b`, `06`, `06b`, `29`, `31`, `39`, `40`, `42`, `43` et `ci-debug-numbers-arm64`.

## Code

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

45. **Le denormals-are-zero fausse encore des bornes hors du lecteur** (suite
    du point 11). Le point 11 a rendu le lecteur de nombres indépendant de DAZ
    (comparaisons sur les bits), mais trois entrées comparent encore des
    doubles sous-normaux comme des doubles, et aucune ne passe par la sonde de
    `round_upward_if_needed()` que le point 4 prévoit de renforcer :
    - l'action de `gaol_interval_parser.ypp` qui vérifie le littéral `<a, b>`
      (`l.left() != r.right()`) : sous DAZ, `<1e-310, 1e-309>` et
      `<0x1p-1074, 0x1p-1073>` sont acceptés au lieu d'être refusés ;
    - `operator<<` : sous DAZ, [0, 5e-324] est écrit comme un point, les deux
      bornes se comparant égales, et une borne sous-normale passe par le chemin
      de la bibliothèque C de `bound_to_text()` (`x == 0.0`), dont
      l'encadrement dépend alors de l'arrondi de la bibliothèque C ;
    - le constructeur `interval(l, r)` : des bornes sous-normales dans le
      mauvais ordre ([0x1p-1073, 0x1p-1074]) ne donnent pas l'ensemble vide,
      `l <= r` étant aplati ;
    - `bound_to_text()` compare avec `0.0` : sous DAZ, une borne sous-normale
      est écrite au plus proche au lieu d'être arrondie vers l'extérieur ; à 16
      chiffres le texte relu contient encore l'intervalle, mais à un chiffre
      [22·2^-1074] s'écrit `[1e-322]`, relu [20·2^-1074, 21·2^-1074].
    Correction : les rendre indépendantes du mode, comme le lecteur (comparer
    les bits), ou leur faire exécuter la sonde ; régénérer le parser commité
    (bison 3.5.1) pour le premier. Le même mode existe sur AArch64 et ARM32
    (FPCR.FZ) : le test du point 11 y est sauté, faute de pouvoir écrire FPCR
    ici. Voir `todo-notes/11.md`.

47. **clang-cl sur x64 : `cbrt`, `rsqrt` et `asinpi` lisent mal le sens
    d'arrondi.** clang-cl définit `__x86_64__` et `_WIN32` mais pas
    `__WIN32__` : le `get_rounding_mode()` de `cbrt.c`, `rsqrt.c` et `asinpi.c`
    compare alors `RC>>3` (0x400, 0x800, 0xc00) aux valeurs `FE_*` de l'UCRT
    (0x100, 0x200, 0x300). Une émulation sur Linux donne 21 résultats de
    `cbrt` faux vers le haut et 21 vers le bas, 1 026 de `rsqrt` vers le haut
    et environ la moitié de ceux d'`asinpi` : les bornes de `rsqrt`, `asinpi`
    et `nth_root(x, 3)` ne contiendraient pas le résultat exact. clang-cl
    n'est pas dans la CI. Correction : un mot dans chaque fichier,
    `#if defined(__WIN32__) || defined(__WIN64__) || defined(_WIN32) /* GAOL */`
    (testé par émulation, sans effet sur les compilateurs de la CI), ou
    refuser clang-cl sur x64 dans `gaol_config.h`. Le correctif 6 de
    `3rd/README.md` (branche du 31) propose la même correction à CORE-MATH.
    Voir `todo-notes/06.md`.

48. **`cancel_minus` et `cancel_plus` sont faux avec la bibliothèque compilée par GCC à `-O3`.** Dans
    `difference_at_least()` (`gaol/gaol_interval.cpp`), GCC déplace les termes d'erreur de `two_sum` après
    `GAOL_RND_NEAREST_LEAVE()` : ils sont calculés vers le haut, où TwoSum n'est plus exact.
    `cancel_minus([0.5], [4.9e-324, 1e-300])` donne [0x1.fffffffffffffp-2, 0.5] au lieu de [-oo, +oo],
    `cancel_minus([DBL_MAX], [0.5])` donne [-oo, +oo] au lieu de [pred(DBL_MAX), DBL_MAX], et `cancel_minus([DBL_MAX], [1])`
    lève FE_INVALID ; en Debug et avec Clang les résultats sont justes. Correction proposée : `GAOL_RND_KEEP` sur `s1`, `e1`,
    `s2`, `e2`, un test de non-régression, et le commentaire de `GAOL_RND_NEAREST_ENTER` (`gaol/gaol_fpu.h`) à corriger.
    Relevé par les relectures des points 24 et 24b (#47, #50).

49. **Compilé par clang-cl, `GAOL_INFINITY` n'est pas l'infini quand le programme arrondit vers le bas ou vers zéro.**
    `gaol/gaol_port.h` le définit comme `HUGE_VAL`, que l'UCRT (SDK 10.0.26100) écrit `((double)(float)1e+300)` ; sous
    `/fp:strict`, clang-cl fait cette conversion à l'exécution (`vcvtsd2ss`/`vcvtss2sd`), qui donne FLT_MAX vers le bas ou vers zéro
    et lève dépassement et inexact dans les quatre sens. `interval()` et `[1]/[-1, 1]` valent alors
    `[-0x1.fffffep+127, 0x1.fffffep+127]`, `pow(x, n)` donne `[-nan, nan]`, `interval(1e300)` est vide : 37 échecs de
    `rounding_direction` avec clang-cl 18 sous wine, aucun avec `__builtin_huge_val()`. Visual C++, GCC, Clang et MinGW ne sont pas
    concernés. Correction proposée : `GAOL_INFINITY` = `__builtin_huge_val()` sous `__GNUC__`/`__clang__`, ou
    `std::numeric_limits<double>::infinity()`. Les jobs clang-cl de #59 en sont le test. Trouvé par la relecture du point 47.

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

46. **`operator>>` ne relit pas `os << x << ' ' << y`** (suite du point 15).
    Il lit un intervalle par ligne : un intervalle contient des espaces
    (`[1, 2]`) et la grammaire admet des littéraux imbriqués et des
    expressions (`[[1, 2], 3]`, `sqrt(2)+1`, `1 -2`), si bien que la fin d'un
    intervalle dans un flux est ambiguë. Écrire avec un saut de ligne
    (`os << x << '\n' << y`) se relit. Options : garder un intervalle par
    ligne (documenté, c'est ce que fait le point 15) ; n'accepter dans
    `operator>>` que les littéraux (`[..]`, `<..>`, `empty`, nombres), lus
    jusqu'au crochet fermant, en gardant la lecture par ligne pour le reste ;
    ou une fonction à part. Il faut d'abord décider de la syntaxe acceptée.
    Voir `todo-notes/15.md`.

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
    **Fait le 30 septembre** (branche `todo-31-upstream-patches`) :
    `3rd/README.md`, section « Changes to propose upstream », donne six
    correctifs contre le master de CORE-MATH (`b1a4badf`), vérifiés : les
    masques `~0ul` (aussi dans `sinpi.c`, `log1p.c` et `lgamma.c`), le
    décalage d'`asinpi`, le `__builtin_expect` de `rsqrt`, le
    `__builtin_roundeven` de `sin.c`, le `fegetround()` de `pow.h` (point 6)
    et le champ d'arrondi de MXCSR lu quelles que soient les valeurs `FE_*`
    dans `cbrt`, `rsqrt`, `asinpi` (MinGW-w64, clang-cl, Cygwin) ; les
    décalages de `cospi` sont déjà corrigés en amont. La glibc n'a aucun de
    ces défauts : rien à lui envoyer. Reste l'envoi à CORE-MATH, qui attend
    votre accord.

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
