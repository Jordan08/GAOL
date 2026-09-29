## Point n° 5 du `TODO.md`

Le point (revue n° 4 de `examples/examples.md`) demande que `-ffinite-math-only` soit refusé à la compilation, comme l'est `-ffast-math`, par un `#error` sur `__FINITE_MATH_ONLY__` dans `gaol/gaol_config.h`, à côté de celui sur `__FAST_MATH__`, avec une ligne dans les options refusées de `doc/three-builds.md` et un test de compilation. Il signale aussi que `-funsafe-math-optimizations` et `-ffast-math -fno-finite-math-only`, qu'aucune macro ne révèle, laissent GCC réduire la sonde `1 + tiny == 1` en `tiny == 0` dans le code inline.

## Problème

Avec `-ffinite-math-only`, le compilateur suppose que les NaN et les infinis n'arrivent jamais, y compris dans les fonctions inline des en-têtes de GAOL v5, qui sont compilées avec les options du programme. L'ensemble vide a des bornes NaN : il n'est plus reconnu. Mesuré sur la base de cette branche (`a2ca992`, dont l'en-tête n'a pas de `#error` pour cette option) avec GCC 9.4 (`-std=c++11 -O2 -frounding-math -ffp-contract=off -msse2 -msse3 -mfma -ffinite-math-only`, contre une bibliothèque compilée avec les options de `gaol.pc` ; opérandes lus dans des `volatile double`, pour que le compilateur ne calcule pas tout à la compilation) :

```cpp
const interval a(1.0, 2.0), b(3.0, 4.0);
(a & b).is_empty();                     // faux, attendu : vrai
interval::emptyset().is_empty();        // faux, attendu : vrai
interval::emptyset() | a;               // [-nan, nan], attendu : [1, 2]
(interval::emptyset() + a).is_empty();  // faux, attendu : vrai
```

Avec Clang 18, l'intersection et la somme sont fausses de la même façon, et les deux autres lignes sont justes. Le `TODO.md` écrit que l'enveloppe de l'ensemble vide et de `[1, 2]` est vide : avec GCC, la mesure donne `[-nan, nan]`, un intervalle à bornes NaN que `is_empty()` ne reconnaît pas comme vide, alors que la valeur attendue est `[1, 2]`. Le même programme compilé sans l'option donne les valeurs attendues.

Sont touchés les programmes qui incluent les en-têtes de GAOL et sont compilés avec `-ffinite-math-only` sans `-ffast-math` (que `gaol_config.h` refusait déjà). Le `-fno-fast-math` de `gaol.pc` et de `gaol::gaol` ne désactive l'option que s'il vient après elle sur la ligne de commande (vérifié avec GCC et Clang) : placé avant elle, il laisse le code compiler et donner les mauvais résultats ci-dessus.

Le contrôle de `__FAST_MATH__` avait en outre un trou, trouvé ici : avec Clang, `-Ofast` ou `-ffast-math` suivi de `-frounding-math` laisse `__FAST_MATH__` indéfini, mais garde `__FINITE_MATH_ONLY__` à 1. Un tel code compilait sans message et donnait les mêmes ensembles vides faux.

## Cause

`interval::is_empty()` (`gaol/gaol_interval.h`) est `!(left() <= right())`, la négation étant là pour traiter les NaN, bornes de l'ensemble vide. Avec `-ffinite-math-only`, le compilateur tient les NaN pour impossibles et réécrit ce test en `left() > right()`, faux quand une borne est NaN (vérifié à l'assembleur avec GCC 9.4 et Clang 18) : l'ensemble vide n'est plus reconnu comme vide, et les opérations sur lui rendent des intervalles à bornes NaN que `is_empty()` ne dit pas vides.

`gaol/gaol_config.h` refusait `-ffast-math` par `__FAST_MATH__`, mais cette macro n'est définie ni par `-ffinite-math-only` seul, ni par Clang avec `-Ofast` suivi de `-frounding-math`. GCC et Clang définissent en revanche `__FINITE_MATH_ONLY__` (0 ou 1) dans tous les cas ; Visual C++ n'a pas de macro de ce genre (son `/fp:fast` est refusé par `_M_FP_FAST`, inchangé).

## Correction

**`gaol/gaol_config.h`.** Un bloc `#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__` avec un `#error` (« GAOL cannot be compiled with -ffinite-math-only (which -ffast-math and -Ofast turn on) ... ») est ajouté entre celui de `__FAST_MATH__` et celui de `_M_FP_FAST`. Le message nomme l'option et le remède (`-fno-fast-math`, mis après elle). Le contrôle refuse aussi les cas où `__FAST_MATH__` est muet (Clang avec `-frounding-math`, ci-dessus). Un long commentaire donne la raison, les macros, l'ordre de `-fno-fast-math` et la liste de ce qu'aucune macro ne montre. Visual C++ n'est pas touché.

**`tests/refused_options.cpp` (nouveau), `tests/CMakeLists.txt`.** Voir Tests.

**Documentation.** `doc/three-builds.md` : une ligne `-ffinite-math-only` dans le tableau « Compilers and options refused » (« the last three » devient « the last four » dans la phrase qui le précède), un paragraphe sur la place de `-fno-fast-math` et sur les tests, et la liste de ce qu'aucune macro ne montre. `doc/using.md` : la liste de ce que l'en-tête refuse, et la place de `-fno-fast-math`. `doc/tests.md` : l'entrée des deux tests, qui dit qu'ils sont propres à CMake. `doc/continuous-integration.md` : la liste des refus que la CI vérifie. `manual/v5/gaol.tex` : un point de la liste « Compilers and options refused » et le paragraphe sur ce qu'aucune macro ne montre, la phrase de « Compiling a program using GAOL », une phrase après l'exemple `-ffast-math` de « The flags of interval arithmetic ». `CMakeLists.txt` : un commentaire (vers la ligne 264) qui cite `-ffinite-math-only`.

**Choix.**

- Un seul contrôle, sur `__FINITE_MATH_ONLY__` : il couvre `-ffinite-math-only`, que `-ffast-math` et `-Ofast` activent, et le cas de Clang que `__FAST_MATH__` laissait passer. Le test de `__FAST_MATH__` reste, avec son message.
- Ce qu'aucune macro ne montre n'est pas refusé, faute de macro pour le voir, mais écrit dans `gaol_config.h`, `doc/three-builds.md` et le manuel, chaque affirmation étant vérifiée avec GCC 9.4 et Clang 18 :
  - `-funsafe-math-optimizations`, et `-ffast-math -fno-finite-math-only`, avec GCC : le compilateur réécrit la sonde `1.0 + tiny == 1.0` de `round_upward_if_needed()` (`gaol/gaol_fpu.h`) en `tiny == 0.0`, si bien qu'une opération ne remet pas le sens d'arrondi vers le haut après que le code utilisant GAOL l'a laissé au plus proche, et que `width()` est sous la largeur exacte. Clang 18 ne la réécrit pas ;
  - `-fno-honor-nans` seul, avec Clang (trouvé ici, absent de la revue) : il fait à l'ensemble vide ce que fait `-ffinite-math-only`, `__FINITE_MATH_ONLY__` ne valant 1 qu'avec `-fno-honor-infinities` aussi.
- Le `TODO.md` propose un test de compilation « comme dans `tests/fp_strict` », et l'annexe B de `examples/examples.md` propose un test à `PASS_REGULAR_EXPRESSION`, « as `tests/fp_strict` does for other options ». Ce n'est pas ce que fait `tests/fp_strict` : c'est un projet CMake à part, exécuté par les seuls jobs Visual Studio, qui vérifie à la configuration (`try_compile()` et `message(FATAL_ERROR)`) que `/fp:strict` compile et que `/fp:precise` et le modèle par défaut sont refusés, sans aucun `PASS_REGULAR_EXPRESSION`. Les deux nouveaux tests sont donc des tests `ctest` de `tests/CMakeLists.txt`, qui tournent dans le `make test` des jobs CMake avec GCC ou Clang (détail plus bas).

**Ce qui n'est pas changé.** Les options de `gaol.pc` et de `gaol::gaol` (`-fno-fast-math` reste, il protège quand il vient après l'option) ; les builds autotools et meson, qui partagent l'en-tête et n'ont pas de test équivalent (question 1) ; le message de l'`#error` de `__FAST_MATH__` ; `doc/building.md`, qui ne dit pas que les deux tests sont propres à CMake (`doc/tests.md` le dit) ; `TODO.md`, `ChangeLog` et `doc/differences.md` (voir Suivi).

## Tests

`tests/refused_options.cpp` (nouveau) est un programme qui inclut `<gaol/gaol>` et calcule `(interval(1.0, 2.0) & interval(3.0, 4.0)).is_empty()` : compilé normalement, il retourne 0 ; compilé avec `-ffinite-math-only` quand l'en-tête ne le refuse pas, il retourne 1 avec GCC 9.4 (les deux vérifiés à la main ; avec Clang 18, il retourne 0 dans les deux cas : il ne montre le défaut qu'avec GCC). Il n'est jamais lancé par les tests : ce sont les compilations qui sont vérifiées.

`tests/CMakeLists.txt` : la fonction `gaol_refused_test(name flag text)` déclare une bibliothèque `OBJECT` `gaol_<name>` (`EXCLUDE_FROM_ALL`) faite de `refused_options.cpp`, et un test `ctest` qui lance `cmake --build <build> --config $<CONFIG> --target gaol_<name>`. Elle est appelée, si le compilateur est GCC ou Clang et non Visual C++ ni `clang-cl`, pour :

- `refused_finite_math_only`, avec `-ffinite-math-only` : la sortie de la compilation doit contenir `GAOL cannot be compiled with -ffinite-math-only` ;
- `refused_fast_math`, avec `-ffast-math` : la sortie doit contenir `GAOL cannot be compiled with -ffast-math`. Ce second test couvre le contrôle de `__FAST_MATH__`, qui n'en avait pas.

Choix du test :

- Le test réussit si le message de l'en-tête est dans la sortie (`PASS_REGULAR_EXPRESSION`), et non sur le code de sortie : une compilation échoue pour bien des raisons. Le motif est la phrase du message, non le nom de l'option, que Ninja écrit dans la ligne de commande.
- La compilation est faite par un `cmake --build` imbriqué, et non par une commande de compilateur écrite à la main, pour que le compilateur et les options de la plate-forme soient ceux des fichiers de GAOL : le job macOS x86_64 sous Rosetta compte sur CMake pour passer `-arch`.
- L'option doit venir après les flags d'arithmétique d'intervalles, dont le `-fno-fast-math` l'annulerait sinon. `gaol::gaol` n'est donc pas lié : CMake met les options d'une bibliothèque liée après celles de la cible (vérifié dans `flags.make`). Les flags qu'il porte (`GAOL_INTERVAL_FLAGS`, `GAOL_FMA_FLAGS`) sont repris explicitement, puis l'option.
- Les tests portent l'étiquette `unit` et un `RESOURCE_LOCK gaol_refused_options` : une seule compilation à la fois dans le répertoire de build, quoi que dise `ctest -j`.
- Limites : reformuler le message de l'en-tête oblige à changer les tests ; un compilateur qui couperait les lignes de ses diagnostics (`-fmessage-length`) pourrait scinder la phrase (les jobs MinGW « refused » de `windows.yml` cherchent déjà un message de la même façon). Chaque test dure de 0,2 à 2,8 s en natif dans la CI, et jusqu'à 8,5 s sous qemu (ppc64le).

Dans les builds locaux, `ctest` liste 47 tests au lieu de 45 sur la base `a2ca992`, et `make test` (étiquette `unit`) 31 au lieu de 29.

Les tests détectent bien le défaut. Dans le build SSE2 (GCC 9.4), les tests étant conservés, `ctest -R refused --output-on-failure` donne :

- le bloc `#error` sur `__FINITE_MATH_ONLY__` retiré de `gaol/gaol_config.h` : `refused_finite_math_only ***Failed  Required regular expression not found.` (`Regex=[GAOL cannot be compiled with -ffinite-math-only]`, l'objet est compilé), `refused_fast_math` passe : `50% tests passed, 1 tests failed out of 2` ;
- le bloc `#error` sur `__FAST_MATH__` (déjà dans la base) retiré à la place : `refused_fast_math ***Failed` (motif absent), `refused_finite_math_only` passe ;
- l'en-tête rétabli (comparé à la copie sauvegardée) : `100% tests passed, 0 tests failed out of 2`.

Sans le `#error`, `refused_options.cpp` compile avec `-ffinite-math-only` : c'est ce que le premier test signale.

## Validation

Compilations locales sur le code de la branche, avec un seul cœur (`-j1`) :

| Build | Résultat de `ctest -j1` |
|---|---|
| SSE2, GCC 9.4, Release | 47 tests sur 47 réussis, dont 2 ignorés |
| FPU (`-DGAOL_SIMD=OFF`), GCC 9.4 | 47 sur 47, dont 2 ignorés |
| Clang 18.1.8 | 47 sur 47, dont 2 ignorés |

`intervalf` et `interval2f` sont ignorés par conception, comme sur la base (les intervalles de flottants sont désactivés) ; `ctest` les compte parmi les réussis. `cmake --build <build> --target test`, l'invocation de la CI (un `make` dans un `make`), dans le build SSE2 : 31 sur 31. Avec le générateur Ninja : `ctest -R refused` réussit, et `cmake --build --target test` (un `ninja` dans un `ninja`, comme les jobs MSYS2) réussit les deux tests (les autres n'étaient pas construits). Après la dernière modification de `tests/CMakeLists.txt`, `ctest -R refused` a été relancé dans les trois builds : 2 sur 2 chacun.

En-tête : compilé (`-fsyntax-only -Wall -Wextra -Wundef -pedantic`, flags de `gaol.pc`) en C++11, 14, 17 et 2a avec GCC 9.4, et de C++11 à 2b avec Clang 18, sans avertissement de `gaol_config.h`. Options, sur les deux compilateurs, avec les flags de `gaol.pc` d'abord sauf indication contraire :

- refusés : `-ffinite-math-only`, `-ffast-math` et, avec Clang, `-Ofast` ; avec Clang aussi, `-Ofast` ou `-ffast-math` suivi de `-frounding-math`, sans `-fno-fast-math` ;
- `-ffinite-math-only` ou `-ffast-math` placé avant `-fno-fast-math` : compile, et le résultat est juste ;
- `-ffast-math -fno-finite-math-only`, `-funsafe-math-optimizations` et, avec Clang, `-fno-honor-nans` : compilent, faute de macro.

Manuel : `manual/build-pdf.sh` (pdflatex, bibtex, makeindex) dans un répertoire hors de l'arbre, 124 pages, aucune référence indéfinie ; le PDF n'est pas commité. Le contrôle de la branche (messages de commit, fichiers parasites, espaces) ne signale rien. Le seul avertissement de compilation est celui de Clang 18 sur une variable inutilisée du parseur généré (`gaol/gaol_interval_parser.cpp:1538`), qui n'est pas de cette branche.

Non testé localement : Visual C++, macOS, MinGW et MSYS2, les conteneurs (i386, armhf, s390x, etc.), le générateur Xcode (que la CI n'utilise pas) et CMake 3.14 (seules des fonctions de CMake 3.0 à 3.14 sont employées ; la CI a un job en CMake 3.14.7).

Intégration continue : exécutions de la branche `todo-05-finite-math-only` au commit `a6dc120`, toutes terminées :

- Linux : https://github.com/Jordan08/GAOL/actions/runs/36528210616 (succès)
- macOS : https://github.com/Jordan08/GAOL/actions/runs/36528210676 (succès)
- Windows : https://github.com/Jordan08/GAOL/actions/runs/36528210619 (succès)
- Autotools and meson : https://github.com/Jordan08/GAOL/actions/runs/36528210684 (succès)
- Manual : https://github.com/Jordan08/GAOL/actions/runs/36528210643 (succès)
- Linux containers : https://github.com/Jordan08/GAOL/actions/runs/36528210669 (16 jobs sur 17 réussis ; seul le job « Debian 12 Bookworm arm64 » échoue, comme sur `configure-clean` au commit `a2ca992`, exécution https://github.com/Jordan08/GAOL/actions/runs/36489260950 ; cet échec est sans rapport avec cette pull request)

{{ARM64}}

Les journaux de ces exécutions montrent les deux tests, `refused_finite_math_only` et `refused_fast_math`, à `Passed` dans tous les jobs qui construisent GAOL avec GCC ou Clang et lancent ses tests :

- Linux : 23 jobs sur 25 (GCC et Clang, x86_64 et arm64, Ubuntu 22.04, 24.04 et 26.04, avec ASan et UBSan, en bibliothèque partagée, avec le sens d'arrondi conservé, avec CMake 3.14.7, avec GCC 9 sans `__builtin_roundeven`, avec l'entier de 128 bits émulé, et la couverture). Les deux autres ne les lancent pas : « Ubuntu 22.04 arm64 Clang 14, refused », dont la configuration est refusée exprès, et « Ubuntu 24.04 x86_64 GCC, GAOL built by FetchContent », dont `make test` ne lance que les 6 tests de `tests/fetch_content`, le projet qui construit GAOL pour lui-même par FetchContent.
- macOS : les 11 jobs (macOS 14, 15 et 26 ; arm64, x86_64 et x86_64 sous Rosetta ; AppleClang, LLVM Clang et GCC), `make test` : 30 tests.
- Windows : 10 jobs sur 27, tous à `Passed` : MinGW-w64 GCC (12.2.0 et 13.2.0 x86 Release ; 14.2.0 x64 et x86 Release ; 15.2.0 x64 et x86, Debug et Release) et MSYS2 (UCRT64 GCC et CLANG64 Clang, x64), `make test` : 30 tests. Les 4 jobs MinGW « refused » (11.2.0 x64 et x86, 12.2.0 x64, 13.2.0 x64) font échouer la compilation exprès et ne lancent pas de tests. Les 13 jobs Visual C++ (2022 et 2026) ne lancent pas les deux tests, qui n'existent pas avec Visual C++ : `make test` (`RUN_TESTS`) y lance 28 tests, tous réussis.
- Linux containers : 16 jobs sur 17, sur Debian 12 et 13 (amd64, arm64, armhf et i386 ; s390x, ppc64le et riscv64 sous qemu), Alpine 3.22 (musl) et manylinux. Le job « Debian 13 Trixie armhf, Clang refused » s'arrête à la configuration, exprès. Dans « Debian 12 Bookworm arm64 », les deux tests passent ; l'échec est celui du test `numbers` (« Subprocess aborted »), qui échoue de la même façon sur `configure-clean` au commit `a2ca992`.

Les jobs de l'exécution « Autotools and meson » n'ont pas ces tests (CMake seul, question 1) ; ils passent, et l'exécution « Manual » compile le manuel modifié.

## Effets

- Comportement : un code qui inclut les en-têtes de GAOL et qui est compilé avec `-ffinite-math-only` ne compile plus, avec un message qui nomme l'option et le remède (`-fno-fast-math` après elle). Il compilait avant et donnait des résultats faux (l'ensemble vide n'était plus reconnu). Avec Clang, `-Ofast` ou `-ffast-math` suivi de `-frounding-math` est refusé aussi. Rien ne change pour le code compilé avec les flags de `gaol.pc` ou de `gaol::gaol` quand `-fno-fast-math` vient après l'option de l'utilisateur (il la désactive, avec GCC et Clang), ni pour Visual C++, ni pour les builds autotools et meson de GAOL lui-même.
- Avec GCC, `-Ofast` placé après les flags de `gaol.pc` compile encore, avec `__FINITE_MATH_ONLY__` à 0 et l'ensemble vide reconnu : GCC laisse un `-fno-fast-math` explicite l'emporter sur le `-ffast-math` que `-Ofast` implique, où qu'il soit sur la ligne. Dire que `-Ofast` est refusé n'est donc vrai qu'en l'absence de `-fno-fast-math` sur la ligne de commande.
- API et ABI : aucun changement.
- Fichiers installés : aucun ajouté ni retiré ; `gaol/gaol_config.h`, installé, a un `#error` et un commentaire de plus.
- Tests : deux de plus, `refused_finite_math_only` et `refused_fast_math` ; dans les builds locaux, `ctest` compte 47 tests (45 sur `a2ca992`) et `make test` 31 (29 sur `a2ca992`).
- Documentation : `doc/three-builds.md`, `doc/using.md`, `doc/tests.md`, `doc/continuous-integration.md` et `manual/v5/gaol.tex`.

## Dépendances et recouvrements

Aucune dépendance.

{{OVERLAPS}}

## Suivi

`TODO.md`, `ChangeLog` et `doc/differences.md` ne sont pas modifiés ici : la pull request de synthèse {{PR:bookkeeping}} retire le point 5 du TODO et consigne le changement. L'en-tête de `TODO.md` liste les numéros de la revue déjà corrigés (1, 6, 9, 10, 11, 13, 14 et 18) : le n° 4 de la revue peut s'y ajouter, corrigé dans la mesure où une macro le permet (les options qu'aucune macro ne montre sont documentées, non refusées).

Le manuel (`manual/v5/gaol.tex`) est modifié ici ; le PDF commité n'est pas régénéré dans cette pull request : il l'est une seule fois dans la pull request de synthèse, pour éviter les conflits sur un fichier binaire entre pull requests.

## Questions ouvertes

1. Les builds autotools et meson n'ont pas d'équivalent des deux tests de compilation (CMake seul, comme `tests/fp_strict`, que seuls les jobs Visual Studio lancent ; l'en-tête est le même pour les trois builds). `doc/tests.md` le dit. Pour les rendre semblables : un script POSIX `tests/refused_options.sh` qui prend la commande du compilateur, déclaré dans `tests/Makefile.am` (`Makefile.in` régénéré par automake 1.18.1) et dans `tests/meson.build` (`test()` avec `find_program('sh')`), ou une étape de `.github/scripts/tests.sh`. Non fait, car l'emploi de `sh` sous Windows et les plates-formes qu'on ne peut pas tester ici restent à décider.
2. Ce qu'aucune macro ne montre (`-funsafe-math-optimizations`, `-ffast-math -fno-finite-math-only` avec GCC, `-fno-honor-nans` seul avec Clang) n'est pas refusé, seulement documenté. Il en va probablement de même des réglages par fonction ou par région (`#pragma GCC optimize`, `__attribute__((optimize))`, `#pragma clang fp`), non essayés. Une idée, non essayée : un autocontrôle écrit en ligne dans l'en-tête, à côté de l'objet statique `_gaol_initializer` de `gaol/gaol_common.h` que chaque fichier incluant GAOL contient ; compilé avec les options de ce fichier, il les verrait (le constructeur de `gaol_initializer`, défini dans `gaol/gaol_init_cleanup.cpp`, tourne avec celles de la bibliothèque).
3. Rapport avec le point n° 4 du `TODO.md` (FTZ et DAZ) : avec GCC 9.4 et `-funsafe-math-optimizations`, la sonde sous-normale que ce point propose (`1.0 + (subnormal + 0.0) == 1.0`) est aussi compilée en `subnormal == 0.0` (vérifié dans l'assembleur), comme la sonde actuelle. Stocker la somme dans un `volatile double` avant de comparer garde l'addition (`addsd` puis `ucomisd`) et pourrait renforcer les deux sondes ; coût non mesuré. Mais `-funsafe-math-optimizations` casse plus que la sonde : avec GCC 9.4, des tests compilés avec lui échouent (`rounding_direction` : 2 vérifications sur 16772, sur `width()` ; `arithmetic` : 93773 sur 1240155, où les calculs de référence des tests sont compilés avec l'option eux aussi, si bien que ce qui est cassé dans le code inline de GAOL au-delà de `width()` n'est pas établi).
4. L'annexe B de `examples/examples.md` propose pour `-ffinite-math-only` un test à `PASS_REGULAR_EXPRESSION`, « as `tests/fp_strict` does for other options » : `tests/fp_strict` n'en a aucun (voir Correction). La phrase est à corriger dans la pull request de synthèse, si le mainteneur le veut.
5. Remarqué, non corrigé : l'`#error` de `__FAST_MATH__` ne nomme aucun remède ; `.github/audit` (`make_probe.py`, `compare.py`) compare `__FAST_MATH__` parmi les macros du compilateur mais pas `__FINITE_MATH_ONLY__` (sans conséquence : le flag `-fno-fast-math` est comparé).
