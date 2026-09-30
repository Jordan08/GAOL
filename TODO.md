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

### Pull requests ouvertes vers `MATH-CORE` (30 septembre)

Faites à partir de `configure-clean`, elles **dépendent de #29** et montrent ses commits tant que #29 n'est pas fusionnée : à fusionner
après #29.

| Point | Pull request | Branche |
| --- | --- | --- |
| 24 (suite) : aucune opération ne lève FE_INVALID sur un opérande vide | #50 | `todo-24b-quiet-empty-operands` |
| 6 (suite) : MinGW-w64 sur x86-64 refusé avant 12 ou lié à msvcrt, pour la vraie raison (`fma()` et `round()`) | #51 | `todo-06b-mingw-msvcrt-refused` |

### Poussé et relu, sans pull request

Chaque branche a été relue par un relecteur indépendant, dont les points bloquants sont corrigés, puis poussée. Aucune n'a de conflit
avec `configure-clean` tel qu'il est maintenant.

| Point | Branche | Ce qui a été fait le 30 septembre |
| --- | --- | --- |
| 16 formats largeur et centre, et la décision du 13 | `todo-16-display-formats` | `configure-clean` fusionné ; remarques des relecteurs ; un intervalle ponctuel s'écrit `[a]` (et un zéro `[0]`) au lieu de `<a, a>`, par `operator<<` comme par `intervalToText` ; le lecteur accepte toujours `<a, b>` ; sous une locale à virgule, un point s'écrit avec ses deux bornes, que le lecteur refuse, au lieu de `[-2,5]` qu'il lisait [-2, 5] |
| 39 Goldstein-Price | `todo-39-goldstein-price` | exemple 16 au style du fichier, temps en ms, sources des valeurs (sympy, mpmath) |
| 40 petites erreurs | `todo-40-small-errors` | les trois commits de l'agent interrompu vérifiés et complétés ; `GAOL_NODISCARD` prend `[[nodiscard]]` avant C++17 avec GCC 7 et plus et Visual C++ 16.4 et plus ; `chi()` du vide écrit `nan` ; commentaire de `jail_parser.h` |
| 6 `fegetround()` lu dans MXCSR | `todo-06-fegetround-mxcsr` | `configure-clean` fusionné, relu ; **`cbrt.c` corrigé** : sous MinGW-w64 x64, `get_rounding_mode()` rendait `FE_UPWARD` = 0x800 au lieu de 0 à 3, et `nth_root(x, 3)` ne contenait pas la racine cubique en sept cas difficiles, avec toutes les toolchains MinGW acceptées (MinGW-Builds UCRT, MSYS2 UCRT64 et CLANG64) |
| 31 correctifs à proposer en amont | `todo-31-upstream-patches` | `3rd/README.md`, section « Changes to propose upstream » : six correctifs contre le master de CORE-MATH, vérifiés ; la glibc n'a aucun de ces défauts ; rien n'est envoyé |

Les branches 6 et 6b se recoupent dans `doc/tests.md` (ajouts dans la même liste) : quand l'une est fusionnée, l'autre demande une
fusion de `configure-clean`.

### Travail inachevé (commit « WIP », non relu, poussé sans lancer la CI)

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
- **24** : les opérations qui levaient encore FE_INVALID sur un opérande vide sont corrigées (#50).
- **31** : les correctifs pour CORE-MATH et la glibc sont écrits dans `3rd/README.md` (branche du 31).
- **39, 42, 43** : les cas restants sont corrigés directement dans `configure-clean`.

### Questions ouvertes

Le détail est dans `todo-notes/NN.md`.

- **16** : `intervalToText` écrit toujours 16 chiffres ; suivre `interval::precision()` est un changement d'une ligne. Sous `showpos`, le
  rayon porte aussi un signe ; sous une locale qui groupe les chiffres, le centre est groupé et le rayon non. Le format hexa écrit
  toujours un point `[a, a]`.
- **24b** (#50) : `floor`, `ceil` et `integer` coûtent +0,17 ns sur des opérandes non vides ; les rendre gratuits demande de rendre
  silencieuse la première comparaison du constructeur, ou des fonctions amies. Visual C++ pourrait appeler une fonction pour
  `std::islessequal`.
- **6b** (#51) : garder ou supprimer `GAOL_RND_MINGW_FENV_ONLY`, qui ne paraît plus nécessaire ; mingw-w64 ARM64 avant 12.
- **31** : envoyer les correctifs à CORE-MATH (merge request sur gitlab.inria.fr, ou `git format-patch` à core-math@inria.fr) demande
  votre accord ; `sinpi.c` et `log1p.c` embarqués gardent `~0ul>>12`, sans effet mesuré.
- **9** (fusionné) : un intervalle sans pôle dont la largeur exacte est entre `pi_dn` et π donne encore `[-oo, +oo]`.
- **24** (fusionné) : `GAOL_PRESERVE_ROUNDING` masque de nouveau les exceptions dans les opérations SSE2.

### Reprise

Ouvrir, si vous le voulez, les pull requests des branches 16, 39, 40, 6 et 31 vers `configure-clean` ; fusionner #29, puis #50 et #51.
Reprendre les branches inachevées (3, 4, 8, 12, 17, 36 : relire, terminer, lancer la CI), en fusionnant d'abord `configure-clean` dans
celles qui ont un conflit (fusion, jamais de réécriture d'une branche poussée) ; commencer les points non commencés ; écrire la pull
request de synthèse (retirer les points faits de ce fichier, consigner les changements dans `ChangeLog` et `doc/differences.md`,
régénérer une seule fois `manual/v5/gaol.pdf`) ; mesurer enfin le point 33 sur une machine au repos. Le 17 attend le 16, le 2 attend le 3.
Supprimer les branches fusionnées encore présentes : `todo-01`, `05`, `10`, `11`, `13`, `15`, `18`, `24`, `29`, `42`, `43` et
`ci-debug-numbers-arm64`.

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
