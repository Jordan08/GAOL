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
2026-09-28 : ce que les agents ont trouvé et qui n'est pas corrigé (le 44 l'est
depuis).

## Avancement au 2026-09-29

Les points ont été repris un par un, chacun dans sa branche `todo-NN-…` faite à partir de `configure-clean`, validé en local
(builds SSE2, FPU et Clang 18 selon le point, test de non-régression dont on a montré qu'il échoue sans la correction), relu par un
ou deux relecteurs indépendants, poussé pour lancer la CI, puis proposé en pull request vers `configure-clean`, avec un texte en
français. Le travail a été arrêté avant la fin : cette section dit où chaque point en est. Les points 7, 9, 14, 29 et 44, fusionnés,
ne sont plus dans la liste ci-dessous. Les rapports détaillés de chaque point (résumé, tests, validation, questions ouvertes, ce
qu'il faudra consigner dans `ChangeLog` et `doc/differences.md`, remarques des relecteurs) sont dans [todo-notes/](todo-notes/), le
brouillon du texte de la pull request du point 5 dans `todo-notes/pr-05-brouillon.md`, et les scripts et consignes de la méthode
dans [todo-notes/orchestration/](todo-notes/orchestration/) (à supprimer si vous n'en voulez pas).

### Fusionné dans `configure-clean`

| Point | Pull request | Branche |
| --- | --- | --- |
| 7 `atanh([1, x])` | #31 | `todo-07-atanh-domain` |
| 14 `what()` des exceptions | #32 | `todo-14-exception-what` |
| 44 archive CPack et `configure` en retard | #33 | `todo-44-cpack-stale-configure` |
| hors liste : le job Debian 12 arm64 (voir plus bas) | #34 | `fix-numbers-gcc12-aarch64` |
| 29 locale à virgule dans la CI | #35 | `todo-29-ci-comma-locale` |
| 9 `tan([-M_PI_2, M_PI_2])` | #36 | `todo-09-tan-pi-half` |

### Poussé et relu, sans pull request

Chaque branche a été relue (aucun point bloquant à la fin) et poussée ; les branches marquées « fusionnée » ont reçu
`configure-clean` (fusion, non réécriture) pour lever un conflit avec les points déjà fusionnés. La CI de ces branches était en
file d'attente ou en cours à l'arrêt : il faut la regarder (`gh run list --branch <branche>`) ; aucun échec autre que le job Debian 12
arm64 (corrigé par #34) n'avait été vu, sauf pour le point 13 (voir plus bas). Le texte français des pull requests n'est écrit
que pour le point 5.

| Point | Branche | Remarque |
| --- | --- | --- |
| 1 pow de la norme écrit une fois | `todo-01-pow-norm-half` | fonction `pow_standard()` ; seul changement de résultat : le signe d'un zéro. Les points 2 et 3 se font par-dessus. |
| 5 `-ffinite-math-only` | `todo-05-finite-math-only` | fusionnée ; deux tests de compilation, CMake seul |
| 10 `hausdorff()`, `nb_fp_numbers()` | `todo-10-hausdorff-nb-fp` | |
| 11 lecteur et denormals-are-zero | `todo-11-lexer-subnormal` | fusionnée ; lexer régénéré (flex 2.6.4) |
| 13 point écrit sous une forme que le lecteur refuse | `todo-13-point-interval-text` | correctif ajouté après un échec sous Visual C++ (voir plus bas) |
| 15 `operator>>` sur une ligne vide | `todo-15-extract-empty-line` | |
| 18 lecture lente sous locale à virgule | `todo-18-reader-locale-speed` | **dépend du 11** (fondée sur sa branche) ; fusionnée |
| 24 exceptions et indicateurs flottants | `todo-24-fp-exceptions` | |
| 42 meson 0.53 et le Python du Store | `todo-42-meson-windows-python` | fusionnée ; correction par la documentation |
| 43 `VERSION.txt` avec une marque d'ordre des octets | `todo-43-version-bom` | fusionnée ; test `version_file` et `.github/scripts/version-file.sh` |

### Poussé maintenant, relu ou non

| Point | Branche | Remarque |
| --- | --- | --- |
| 16 formats largeur et centre | `todo-16-display-formats` | relu et approuvé ; **dépend du 13** (fusionné avec sa branche) |
| 39 Goldstein-Price | `todo-39-goldstein-price` | relu et approuvé |
| 6 `fegetround()` lu dans MXCSR | `todo-06-fegetround-mxcsr` | fait et validé par son auteur, **pas relu** |
| 40 petites erreurs | `todo-40-small-errors` | trois commits, l'agent a été interrompu avant son rapport : **à valider** |

### Travail inachevé (commit « WIP », non relu, poussé sans lancer la CI)

3 exactitude de `pow(x, y)` aux coins (`todo-03-pow-exact-corner`, sur le 1) · 4 flush-to-zero (`todo-04-ftz-daz`, trois builds
modifiés) · 8 `pow(x, n)` pour n grand (`todo-08-pow-large-n`) · 12 longues sommes (`todo-12-long-sums`, conflit avec
`configure-clean` dans `tests/expressions.cpp`) · 17 vitesse de `operator<<` (`todo-17-output-speed`, sur le 16, conflit avec le 13
dans `gaol/gaol_interval.cpp`) · 36 l'arrondi vers le haut dans la documentation (`todo-36-upward-rounding-effects`).

### Pas commencés

2 (se fait sur le 3), 19, 20, 21, 22, 23, 31, 32, 34, 35, 37, 38, 41, et les gros points 25, 26, 27, 28, 30 ; le 33 (temps de
`doc/compare/performance.md`) est à faire en dernier, la machine ne faisant rien d'autre. Le découpage prévu des gros points est dans
`todo-notes/orchestration/points_w4.py` : 25 en onze sous-fonctions (mulRevToPair, inflate, bisect, milieu-rayon, hull et intersect,
encadrement de la largeur, littéral `_iv`, erf et erfc, réciproques de atan2, de pow, de max/min/sign/floor) réunies dans une seule
branche et une seule pull request ; 26 un indicateur de sortie de domaine (pas de décorations) ; 27 une étude chiffrée de l'arrondi
porté par l'instruction (AVX-512), en option seulement ; 28 trois en-têtes optionnels ; 30 trois bancs d'essai. Les points 31
(propositions à CORE-MATH et à la glibc : préparer, ne rien envoyer sans votre accord) et 34 (étiquette `v5.0.0` ou fusion de
`MATH-CORE` dans `master` : c'est à vous de le décider, la branche ne fait qu'écrire l'étiquette dans les recettes) demandent une
décision de votre part.

### Recouvrements entre branches

Conflits de fusion textuels trouvés à l'arrêt (`git merge-tree`), sans conséquence sur le fond : ils se règlent en gardant les deux
côtés, presque toujours des ajouts dans la même liste. Les fichiers qui reviennent : `doc/tests.md`, `tests/CMakeLists.txt`,
`tests/rounding_direction.cpp`, `tests/numbers.cpp`, `tests/expressions.cpp`, `manual/v5/gaol.tex`.

- 4 × 5 (`doc/three-builds.md`, `manual/v5/gaol.tex`), 4 × 6 et 6 × 24 (`doc/tests.md`, `tests/rounding_direction.cpp`), 4 × 24
  (`doc/tests.md`, `doc/using.md`, `manual/v5/gaol.tex`, `tests/rounding_direction.cpp`), 4 × 36 et 24 × 36 (`doc/using.md`).
- 5 × 43 (`doc/tests.md`, `tests/CMakeLists.txt`), 42 × 43 (`meson.build`).
- 12 × 5, 11, 18, 42, 43 (`tests/expressions.cpp`) : le 12 n'a pas reçu `configure-clean`, où le point 14 a ajouté des vérifications au
  même endroit.
- 13 × 17 et 16 × 17 (`gaol/gaol_interval.cpp`), 15 × 17, 16 × 18 et 17 × 18 (`tests/numbers.cpp`).

Quand une pull request est fusionnée, celles qui la recoupent demandent une fusion de `configure-clean` dans leur branche.

### Ce que la CI a montré

- **Debian 12 arm64** : le job « Debian 12 Bookworm arm64 » de « Linux containers » était rouge sur `configure-clean` depuis
  `907fe05` : GCC 12.2 sur aarch64 compile mal `tests/numbers.cpp` à `-O2` et `-O3` (la variable de boucle d'un tableau local est lue à
  une mauvaise adresse, selon les décisions d'inlining ; ni AddressSanitizer ni UBSan ne signalent rien sur x86-64). Contournement
  (#34) : `#pragma GCC optimize("no-inline-functions")` pour GCC 12 sur aarch64. Le diagnostic est dans la pull request. La branche
  jetable `ci-debug-numbers-arm64`, qui l'a permis, est encore sur le dépôt : à supprimer.
- **Visual C++** : les nouveaux tests du point 13 ont trouvé, sous Visual C++ (x86 et x64), que le runtime de Windows écrit 0
  « arrondi vers le haut » comme `0.1` : `[0, 0]` sortait `<0.0, 0.1>` et `[-1, 0]` avait pour borne haute `0.1`.
  `bound_to_text()` écrit maintenant un zéro à l'arrondi au plus proche (dernier commit de `todo-13-point-interval-text`).
- **Coût** : un push lance 111 jobs, environ 348 minutes de runner (Linux 25 jobs, macOS 11, Windows 27, Autotools et meson 30,
  conteneurs 17, Manual 1), soit trois push à l'heure au mieux avec 20 jobs simultanés. Les workflows se lancent sur `push` (toutes les
  branches) **et** sur `pull_request` : chaque pull request les fait tourner deux fois. Ne lancer `push` que sur les branches
  principales diviserait la charge par deux (non fait).

### Décisions et questions ouvertes

Le détail est dans `todo-notes/NN.md` ; ici les choix à confirmer.

- **1** : le bloc « exposants entiers au-delà des int » est dans `pow_standard()` et non dans `gaol_ieee1788::pow` ; `gaol::pow([0], y)`
  s'écrit `<0, 0>` et non `<-0, 0>` en SSE2 (`interval(0.0)` au lieu de `interval::zero()`).
- **5** : les deux tests de compilation n'existent que pour CMake (comme `tests/fp_strict`) ; un script POSIX enregistré dans autotools et
  meson les donnerait aux trois builds. Clang laissait `__FAST_MATH__` indéfini pour `-Ofast` ou `-ffast-math` suivis de
  `-frounding-math` : le `#error` sur `__FINITE_MATH_ONLY__` le refuse. `-fno-honor-nans` seul avec Clang casse aussi l'ensemble vide et
  ne se détecte pas (documenté).
- **6** : le refus de MinGW-w64 avant la version 12 sur x64 reposait sur un `fegetround()` propre à MinGW ; il n'a plus de raison sur
  x64 (MXCSR est lu directement) : à lever ou non, la CI devant montrer que ces compilateurs passent. i386 avec `__SSE2_MATH__` est
  laissé de côté (rien ici ne teste le 32 bits).
- **9** (fusionné) : le drapeau `narrower_than_pi` a disparu, le test unique est `w <= pi_dn` ; un intervalle sans pôle dont la largeur
  exacte est entre `pi_dn` et π donne encore `[-oo, +oo]`. L'annexe B de `examples/examples.md` et la revue n° 8 parlent du drapeau.
- **10** : `hausdorff()` sur des bornes infinies est un peu plus lent (14 ns au lieu de 4,5 ns) ; une sortie anticipée est possible.
  `[1, 2] - 1` a `-0` pour borne inférieure : faut-il normaliser les zéros des opérations d'intervalles ?
- **11** : le même effet de denormals-are-zero existe hors du lecteur : l'action de `gaol_interval_parser.ypp` compare des bornes
  sous-normales comme des doubles, et `operator<<` écrit `[0, 5e-324]` sous la forme `<0, 4.95e-324>` sous DAZ. Non corrigés.
- **13** : un zéro de l'un ou l'autre signe garde les chevrons (`<-0, 0>` pour `interval::zero()`), sinon ce serait `[-0, 0]` :
  à confirmer. `<a, a>` n'est pas un littéral d'IEEE 1788-2015 (un point y est `[m]`) : à traiter avec le point 16.
- **15** : la lecture par mot n'est pas faite (un intervalle contient des espaces, la grammaire admet des expressions) ; options : un
  intervalle par ligne (fait), seulement les littéraux jusqu'au crochet fermant, ou une fonction à part.
- **18** : l'analyse des chiffres reste quadratique (20 000 chiffres : 0,02 s ; 100 000 : 0,55 s) ; neuf chiffres par passe la
  rendraient neuf fois plus rapide.
- **24** : beaucoup d'opérations lèvent encore FE_INVALID sur les bornes NaN d'un opérande vide (liste dans `todo-notes/24.md`) ; seule
  `is_empty()` est corrigée, le reste est documenté (les exceptions matérielles doivent être désactivées).
- **39** : l'exemple 16 garde le style de GAOL 4 et la première ligne `<-0, 0>` ; `examples/examples.md` dit encore que les exemples du
  manuel « sont justes ».
- **42** : correction par la documentation plutôt que par l'ordre `python`/`python3` (changer l'ordre fait perdre à meson son repli sur
  son propre Python) ; un pas de CI qui lance meson sans Python dans le `PATH` n'est pas fait.
- **43** : les règles de lecture diffèrent pour les blancs exotiques (autoconf accepte `5. 0.0` à la génération, `strip()` de meson
  enlève aussi les espaces Unicode) ; un fichier en UTF-16 est refusé avec ses octets, non décodé.

### Reprise

Fusionner les pull requests dans l'ordre qui vous convient, puis pour chaque branche restante : lui fusionner `configure-clean` si
elle a un conflit, vérifier la CI, ouvrir la pull request. Ensuite : relire les branches non relues (6, 40) et terminer les six
inachevées ; commencer les points non commencés ; écrire la pull request de synthèse (retirer les points faits de ce fichier, consigner
les changements dans `ChangeLog` et `doc/differences.md`, régénérer une seule fois `manual/v5/gaol.pdf` : aucune branche n'y touche, pour
éviter des conflits binaires) ; mesurer enfin le point 33 sur une machine au repos.

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
