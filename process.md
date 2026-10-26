# Procédure pour traiter un point du TODO.md de GAOL v5

Ce fichier s'adresse à un agent IA qui reprend le travail sur GAOL v5. Il décrit
la procédure suivie, point après point, depuis fin septembre 2026 : ce qu'exige
le mainteneur, comment on valide, teste, commite, pousse, fait passer la CI et
propose une pull request, et comment on tient les registres à jour une fois la
pull request fusionnée. Lis-le en entier avant de toucher au dépôt.

Ce fichier décrit la procédure ; l'état du travail (ce qui est fait, ce qui est
en cours, ce qui reste) est au §12. Dernière mise à jour : 27 octobre 2026, au
commit `f625bbc` de `configure-clean` (la fusion de #201).

## 1. Le contexte

- **GAOL v5** (version 5.0.0) est une bibliothèque C++ d'arithmétique
  d'intervalles. Elle est issue de GAOL 4 (Frédéric Goualard) et utilise
  CORE-MATH (`3rd/math-core`) pour les fonctions élémentaires correctement
  arrondies. Le dépôt s'appelle « GAOL v5 », jamais « ce fork ». mathlib n'existe
  plus : ne la mentionne jamais.
- **Le mainteneur** est Jordan Ninin, compte GitHub `Jordan08`, dépôt
  `Jordan08/GAOL`. Il écrit en français. Réponds-lui en français, de façon
  factuelle et concise.
- **Les branches** :
  - `configure-clean` : la branche de développement, et la base de toutes les
    pull requests.
  - `MATH-CORE` et `master` : elles ne recevront `configure-clean` qu'à la
    publication de v5.0.0 (point Y : `configure-clean` → `MATH-CORE` →
    `master`, puis l'étiquette `v5.0.0`).
  - `todo-status`, l'ancienne branche des rapports et de l'outillage, a été
    supprimée le 4 octobre : ce qui en sert encore est dans `todo-notes/`.
- **Les arbres de travail** : `/home/jninin/Documents/WORK/DEV/GAOL/GAOL_V9/GAOL`
  et `/home/jninin/Documents/WORK/DEV/GAOL/GAOL_V8`, deux clones du même dépôt,
  qui se valent : travaille dans l'un ou dans l'autre (décidé le 12 octobre).
  Le mainteneur et d'autres agents y travaillent aussi. Des fichiers peuvent y
  être modifiés sans être à toi ; c'est souvent le cas de `TODO.md`, que le
  mainteneur enrichit lui-même. Avant de créer ta branche, regarde l'état de
  l'arbre (`git status --short`, `git branch --show-current`) : s'il porte le
  travail non commité de quelqu'un d'autre, prends l'autre arbre, ou
  demande.

## 2. Les sources de vérité

| Quoi | Où |
| --- | --- |
| Le reste à faire, en tâches PA à PJ qui regroupent des points (lettre et ancien numéro), avec leurs décisions | `TODO.md` de `configure-clean`, et l'issue #179 (même texte) |
| Ce qui est fait et décidé : pull requests fusionnées, décisions par date, points faits, mesures | `TODO_DONE.md` de `configure-clean`, et l'issue #177 (même texte) |
| Ce qui est reporté : après la version, indéfiniment, en attente de matériel | `TODO_AFTER.md` de `configure-clean`, et l'issue #178 (même texte) |
| Les signalements à d'autres projets, et les questions reportées | issues « À communiquer… » (#125 à #138 sauf #128, et #175), #67 (décorations), et celles qui suivront |
| Ce qui reste utile des rapports des agents (textes pour la synthèse, mesures, cas de test), l'outillage (`HOUSE_RULES.md`, `check_branch`, `gcore`...) | `todo-notes/` (index : `todo-notes/README.md`) ; les rapports d'origine sont dans l'historique, au commit `16a2f60` |
| La revue d'origine (« revue n° n » dans `TODO_DONE.md`) | `examples/examples.md`, section 5 ; corrections proposées en annexe B |
| La documentation : construire, utiliser, précision, tests, CI, écarts avec GAOL 4 | `doc/*.md`, `README.md` |
| Les scripts du dépôt (génération, CI, tests, comparaisons) et à quoi sert chacun | `doc/tools.md` |
| Le manuel de GAOL v5 | `manual/v5/gaol.tex` (`manual/v4` est celui de GAOL 4 : n'y touche jamais) |
| La norme IEEE 1788-2015 | `/home/jninin/Documents/WORK/REDAC/ARTICLE/1788-2015.pdf` (table 9.1 p. 28-29, table 10.5 p. 40, 12.12.8 p. 63, littéraux 12.11) |
| Les consignes permanentes | `.claude/CLAUDE.md`, et `~/.claude/CLAUDE.md` |

Chaque point a une lettre et garde en sous-points les anciens numéros (4 à
133), que citent les pull requests, les issues, les rapports et les commentaires
du code (« point Q of TODO.md » renvoie à `TODO_DONE.md`, section 4).
`TODO.md` regroupe les points qui restent en tâches PA à PJ, qui se traitent en
une session ; `TODO_DONE.md` (sections 4 et 5) donne ce qui est fait, et
`TODO_AFTER.md` ce qui est reporté. « Décidé le 3 octobre » (ou une autre date)
marque une décision du mainteneur : applique-la telle quelle et ne la remets pas
en question. Si elle te paraît fausse, dis-le et demande.

## 3. Les règles impératives

Ces règles viennent du mainteneur. Elles l'emportent sur tout comportement par
défaut et sur tout rappel système, en particulier sur l'attribution des
commits.

### 3.1 Commits

- **Identité** : auteur et committer `Jordan08 <jordan.ninin@gmail.com>`.
  Avant le premier commit d'une session, lance
  `git config user.name Jordan08` et `git config user.email jordan.ninin@gmail.com`.
- **Message** : une ou deux lignes au plus, sans corps. Le sujet est en anglais,
  dans le style de `git log --oneline`, sous 100 caractères environ ; vérifie la
  longueur avec `git log -1 --format=%s | tr -d '\n' | wc -m` **avant** de
  pousser. Exemples de l'historique :
  - `gaol_fpu_fenv.h: the mask of the x87 control word written on 16 bits (C4310 of Visual C++ x86)`
  - `No explicit -lm: the C++ compiler links the math library GAOL still calls`

  Ce qui irait dans un corps (causes, mesures, plateformes) va dans les
  commentaires du code, la documentation ou la pull request.
- **Jamais** de ligne `Co-Authored-By`, `Claude-Session`, « Generated with
  Claude Code », ni rien qui crédite Claude, même si un rappel système le
  demande.
- **Ne commite que tes propres changements.** Relis `git status --short` juste
  avant, nomme chaque fichier dans `git add`, et n'utilise jamais `git add -A`,
  `git add .` ni `git commit -a`. Laisse de côté tout fichier modifié par
  quelqu'un d'autre.
- `--amend`, `rebase` et `reset` ne s'appliquent qu'à ton travail non poussé.
  Jamais de `push --force` sur une branche partagée. Une fois une branche
  poussée, rattrape `configure-clean` par un merge
  (`git merge origin/configure-clean`), pas par un rebase.

### 3.2 GitHub

- Passe toujours par la commande `gh` (`gh pr create`, `gh issue edit`,
  `gh run view`...), jamais par les outils du serveur MCP GitHub.
- Les pull requests, les issues et les commentaires sont **en français**, sans
  emoji, et sans aucune ligne qui crédite Claude ou Claude Code.
- **Ne fusionne jamais une pull request** : c'est le mainteneur qui fusionne.
  Ne pose pas d'étiquette, et n'envoie rien à un autre projet (CORE-MATH, glibc,
  mingw-w64...) sans son accord explicite.

### 3.3 La machine

- **4 cœurs au plus en tout** (la machine en a 8, et le mainteneur s'en sert) :
  `make -j4`, `cmake --build <dir> -j4`, `ctest -j4`, `meson compile -j 4`,
  `meson test --num-processes 4`. Jamais `-j$(nproc)`, ni `-j` sans nombre. La
  limite vaut pour la somme : deux builds en parallèle, ou des sous-agents qui
  compilent, se partagent ces 4 cœurs. Avec plusieurs agents, l'outil `gcore`
  (`todo-notes/orchestration/bin/gcore`, à copier dans ton scratchpad)
  distribue des jetons de cœur.
- Les répertoires de build et les fichiers temporaires vont **hors de l'arbre**,
  dans ton scratchpad, jamais dans `/tmp` directement ni dans le dépôt. Les
  chemins `/tmp/claude-1001/...` des anciennes sessions sont éphémères : ne
  compte pas dessus.
- Pas de `sudo`.

### 3.4 Branches et arbre de travail

- Travaille **en local, dans `GAOL_V8` ou `GAOL_V9/GAOL` (section 1), sur une
  nouvelle branche** créée depuis `origin/configure-clean` à jour. Jamais de
  worktree : le mainteneur l'a demandé le 3 octobre (« met toi en local dans la
  branche sur laquelle tu travailles »).
- Noms de branche : `todo-<lettre>-<sujet>` pour un point du TODO
  (`todo-j-headers-werror`, `todo-l-ci-workflows`) ; un nom descriptif pour une
  autre tâche (`autotools-distclean`, `no-explicit-libm`).
- Une seule exception : on commite **directement sur `configure-clean`**
  - la mise à jour de `TODO.md`, `TODO_DONE.md` et `TODO_AFTER.md` après une
    fusion, avec celle de la date et du commit de l'en-tête de `process.md` et
    de son §12 (section 9) ;
  - les corrections que le mainteneur a explicitement autorisées là, comme les
    avertissements trouvés par les jobs `-Werror` et `/WX` ou un délai de la
    CI.

  Tout le reste passe par une branche et une pull request.

## 4. Les exigences de robustesse du code

Ce sont les principes du mainteneur pour GAOL v5. Une modification qui en viole
un est refusée, quel que soit son gain.

1. **La sûreté d'abord.** Toute borne doit contenir le résultat exact. Ne
   propose jamais d'option, de drapeau ni de chemin de code qui puisse casser
   un encadrement, même pour de la vitesse. La vitesse ne vient que de ce qui
   garde toutes les bornes sûres. C'est pourquoi `gaol/gaol_config.h` et les
   builds refusent `-ffast-math`, `-ffinite-math-only`, Visual C++ et clang-cl
   sans `/fp:strict`, les doubles calculés sur l'unité x87, et certains
   MinGW-w64 et Clang (voir `doc/three-builds.md`).
2. **IEEE 1788-2015** pour les cas particuliers : 0, infinis, bords de domaine,
   ensemble vide. Lis les pages utiles de la norme. Les intervalles construits
   avec une borne infinie suivent les règles d'IBEX (`interval(+oo)` est vide,
   `interval(a, b)` avec `a > b` est vide). `pow(I, J)` est volontairement
   hybride : `pown` pour un exposant entier dégénéré, le `pow` de la norme
   sinon.
3. **Le sens d'arrondi.** GAOL calcule en arrondi vers le haut.
   `GAOL_PRESERVE_ROUNDING` restaure celui du programme. Tout chemin qui change
   l'arrondi doit le rétablir, y compris quand une exception est levée (point
   H).
4. **Les exceptions flottantes.** Aucune opération de l'interface ne doit lever
   `FE_INVALID` sur un opérande vide (#50), même une fois vectorisée par le
   compilateur (point D). Ces cas se testent dans un processus fils sous
   `feenableexcept(FE_INVALID)`.
5. **La portabilité.** La CI couvre :
   - Linux x86_64 et arm64 ; Debian i386, armhf, s390x (gros-boutiste),
     ppc64le, riscv64 ; Alpine (musl) ;
   - macOS arm64 et x86_64 ;
   - Windows : Visual C++ x86, x64 et arm64, clang-cl, MinGW-w64, MSYS2.

   Écris donc du code portable, et raisonne sur ces cibles : `size_t` sur
   32 bits en x86, `long` sur 32 bits sous Windows, comparaisons signalantes sur
   ARM 32 bits et POWER9, absence de `__builtin_roundeven()` avec GCC 9, etc.
6. **Les trois builds restent pareils.** CMake, autotools et meson doivent
   donner la même bibliothèque, les mêmes en-têtes installés, les mêmes tests et
   les mêmes options (celles de l'utilisateur sont les mêmes dans les trois, et
   le reste se décide par des tests de la configuration : `doc/building.md`,
   section « The options »). Si tu ajoutes ou retires une source, un en-tête, un
   test ou une option, mets à jour les trois :
   - `CMakeLists.txt` ;
   - `Makefile.am`, avec son `Makefile.in` régénéré (section 6.3) ;
   - `meson.build` ;
   - les listes d'en-têtes installés.

   `.github/audit` vérifie dans la CI que les trois builds donnent la même
   configuration.
7. **L'interface.**
   - `interval(double)` est `explicit` depuis le 4 octobre, comme
     `expression(double)` et `expression(const interval&)` ;
     `interval(double, double)` ne l'est pas, et un entier y devient un double,
     inexact au-delà de 2^53 (point D.21). N'ajoute pas de nouvelle conversion
     implicite.
   - Les fonctions `*_dn()` et `*_up()` sur les doubles sont privées
     (`gaol_double_op.h` n'est pas installé).
   - Les intervalles de floats restent désactivés (`GAOL_FLOAT_INTERVALS`).
   - Les macros des en-têtes publics commencent par `GAOL_`, et chaque en-tête
     installé compile seul en C++11 avec `-Wall -Wextra -Wold-style-cast
     -Wundef -Werror` (`.github/scripts/headers.sh`) : une macro lue par
     `#if` y est toujours définie (à 0 ou 1 pour les options et les macros
     de la cible ; point Z).
8. **CORE-MATH** (`3rd/math-core`) reste aussi proche que possible de l'amont.
   Un changement indispensable y est marqué `/* GAOL */` et décrit dans
   `3rd/README.md`, comme correctif à proposer en amont.
9. **Les avertissements.** La bibliothèque, CORE-MATH compris, les tests et les
   exemples compilent sans avertissement :
   - `-Wall -Wextra -Werror` avec GCC, Clang et AppleClang ;
   - `/W4 /WX` avec Visual C++ x64 et x86.
10. **Les textes.**
    - Le code, les commentaires et la documentation sont en anglais, dans le
      style des fichiers voisins. Commente le **pourquoi** quand il n'est pas
      évident : c'est là que vont les explications qu'un message de commit ne
      porte pas. Ce qui est nouveau par rapport à GAOL 4 est marqué
      « (GAOL v5) » dans les commentaires et `doc/*.md`, et `\newinvfive` dans
      le manuel.
    - Dans les tests, la documentation et les exemples, un intervalle aux bornes
      exactes se construit avec les constructeurs numériques
      (`interval(1.0, 2.0)`, `interval(2.0)`, `interval::emptyset()`,
      `interval::universe()`). `textToInterval()` ne sert que lorsque le texte
      apporte quelque chose : un décimal comme `"0.1"`, une expression, ou un
      test du lecteur.
    - Tout programme complet de la documentation se termine par
      `gaol::cleanup();`, après le dernier usage de GAOL et avant `return`.
      `gaol::init()` est automatique : ne le montre pas.
11. Ne vérifie pas et ne rapporte pas les effets d'un changement sur IBEX ou
    Codac. Les comparaisons de performance se font avec Clang 21 (depuis le 13
    octobre), sans `GAOL_PRESERVE_ROUNDING`, avec en plus, pour GAOL v5, une
    colonne avec la direction préservée (`doc/compare/performance.md`).

## 5. Les tests

- **Tout bug corrigé reçoit un test de non-régression qui a des dents.** Annule
  seulement la correction en gardant le test, reconstruis, et vérifie que le
  test **échoue**. Rétablis ensuite la correction et vérifie qu'il passe. Note
  la commande et le résultat : ils iront dans la pull request (« Vérifié :
  avec les sources de `configure-clean`, ce test échoue »). Un test qui passe
  avec le bug ne protège rien.
- **Les valeurs de référence** se calculent indépendamment du code testé :
  arithmétique rationnelle exacte, entiers, mpmath à 2000 bits (scripts
  `tests/*_values.py`), arrondi dirigé explicite, ou CORE-MATH appelé dans les
  deux sens.
- **Pas de test fragile.** Ce qu'un test exige de la plateforme (x86, glibc,
  une locale) se vérifie à l'exécution, et le test dit qu'il passe le cas. Les
  tests doivent aussi passer sous une locale à virgule décimale (`fr_FR.UTF-8`,
  installée sur la machine).
- Préfère ajouter des vérifications à un programme existant de `tests/` plutôt
  que d'en créer un nouveau. Un nouveau test s'inscrit dans les trois builds :
  `tests/CMakeLists.txt`, `tests/Makefile.am` (et `Makefile.in`),
  `tests/meson.build`. `doc/tests.md` décrit ce que vérifie chaque programme :
  mets-le à jour.
- `numbers` est le test le plus long. Son délai est de 300 s
  (`GAOL_NUMBERS_TIMEOUT`), et 600 s dans le job macOS 15 x86_64 GCC avec les
  sanitizers.

## 6. Valider en local

### 6.1 Les outils de la machine

| Outil | Version, ou emplacement |
| --- | --- |
| GCC | 15.2 (`gcc`, `g++`) |
| Clang | 18, 20 et 21 (`clang-18`, `clang++-18`, `clang++-20`, `clang++-21`) |
| CMake | 4.2 (`cmake`) |
| autoconf, automake, libtool | 2.72, 1.18.1, 2.5.4 |
| bison, flex | 3.8.2, 2.6.4 (les versions des fichiers générés commités) |
| meson, ninja | 1.10, 1.13 |
| Autres | `pdflatex` (TeX Live 2025), `gcovr` 7.2, python 3.14 avec mpmath 1.3 |

Ce sont les outils d'Ubuntu 26.04, tous dans le `PATH` (constaté le 11 octobre
2026). Vérifie donc les versions au début du travail, et écris « Non vérifiable
ici » ce que la machine ne permet pas. La machine d'Ubuntu 26.04 n'a plus GCC 9,
que couvre le job `gcc-9` de `linux.yml`, ni meson 0.53.2, la plus ancienne
version que le build meson prend en charge, que couvre le job `meson-oldest` de
`build-systems.yml` (#110). Sur une machine sous Ubuntu 24.04 (GCC 13.3, CMake
3.28, python 3.12), qui n'a ni autoconf, ni automake, ni libtool, ni meson, ni
ninja, construis autoconf 2.72 (celui qui a généré le `configure` du dépôt)
depuis le tarball de ftp.gnu.org (sha256 vérifié), et installe meson et ninja
dans un environnement virtuel.

Le processeur (i7-1185G7) a AVX-512 et FMA. Il ne voit donc jamais les chemins
sans FMA : la CI les couvre.

### 6.2 La matrice minimale

Construis toujours hors de l'arbre. Le plus sûr est de copier les fichiers
suivis dans ton scratchpad, ce qui fonctionne aussi pour autotools, qui
construit dans les sources :

```sh
N=<scratchpad>/check; rm -rf $N && mkdir -p $N/src
git ls-files -co --exclude-standard | tar -c -T - | tar -x -C $N/src
```

1. **CMake, trois variantes**, avec tests et exemples, et `ctest -j4` à 100 % :

   ```sh
   cmake -S . -B $N/sse   -DWITH_TESTS=ON -DWITH_EXAMPLES=ON                 # GCC, SSE2
   cmake -S . -B $N/fpu   -DWITH_TESTS=ON -DWITH_EXAMPLES=ON -DGAOL_SIMD=OFF # GCC, FPU
   CC=clang-18 CXX=clang++-18 cmake -S . -B $N/clang -DWITH_TESTS=ON -DWITH_EXAMPLES=ON
   cmake --build $N/sse -j4 && (cd $N/sse && ctest -j4 --output-on-failure)
   ```

   `intervalf` et `interval2f` sont « Skipped » par construction.
   `-DGAOL_SIMD=OFF` donne les intervalles FPU, sur x86 comme sur arm64, où
   les intervalles NEON sont le choix par défaut (`GAOL_ARM64` n'est plus une
   option). Les autres réglages (FMA, chemin AVX-512, entier 128 bits, débogage)
   sont décidés par la configuration ; `-DGAOL_FMA=OFF` et les autres réglages
   des développeurs sont dans `doc/building.md`.
2. **Avertissements** : une compilation avec
   `-DCMAKE_CXX_FLAGS="-Wall -Wextra" -DCMAKE_C_FLAGS="-Wall -Wextra"` et
   `-DCMAKE_COMPILE_WARNING_AS_ERROR=ON`, avec GCC et avec Clang. Ce réglage ne
   s'applique qu'aux cibles, donc aucun test de configuration de CMake ne change
   de résultat.
3. **Debug avec les sanitizers**, dès que le changement touche à la mémoire, au
   parser ou à un comportement indéfini possible : `-DCMAKE_BUILD_TYPE=Debug`
   et `-fsanitize=address,undefined -fno-omit-frame-pointer` dans les drapeaux C
   et C++ et ceux de l'édition de liens.
4. **autotools** :

   ```sh
   ./configure --prefix=$N/ac-inst --with-tests --with-examples
   make -j4 && make install && make -j4 check
   ```

   Puis `make distclean`, qui doit rendre l'arbre tel que git l'a.
5. **meson** :

   ```sh
   meson setup $N/mb --prefix=$N/m-inst --libdir=lib -Dwith-tests=true -Dwith-examples=true
   meson compile -C $N/mb -j 4 && meson install -C $N/mb && meson test -C $N/mb --num-processes 4
   ```

6. **Le GAOL installé** : `sh .github/scripts/tests.sh <prefix> shared|static`
   reconstruit les tests avec le GAOL installé, et `pkg-config --cflags --libs
   gaol` doit suffire pour lier un programme. Fais-le dès que le changement
   touche à l'installation, à l'édition de liens ou à `gaol.pc`.
7. **Le manuel**, s'il a changé : copie `manual/v5` dans ton scratchpad et
   remplis `gaol_version.tex` à partir de `gaol_version.tex.in`, avec les
   valeurs de `configure.ac` (`GAOL_V5_EDITION`, `GAOL_V5_UPDATED`) et la
   version. Lance ensuite deux fois `pdflatex -interaction=nonstopmode
   -halt-on-error gaol.tex`. Il faut zéro erreur, et pas de nouvel « Overfull ».
   Ne commite jamais `manual/v5/gaol.pdf`.

Ce que tu ne peux pas tester ici (Windows, macOS, ARM, gros-boutiste, sans FMA),
écris-le dans la pull request sous « Non vérifiable ici ».

### 6.3 Les fichiers générés

- **Un `Makefile.in`** : `automake <dir>/Makefile` avec automake 1.18.1. Le diff
  de `Makefile.in` ne doit contenir que l'effet de ton changement.
- **`configure`** : quand seul `configure.ac` change, `autoconf`, qui prend
  l'`aclocal.m4` du dépôt, puis `rm -f configure~`. Sous Ubuntu 26.04,
  `autoreconf --force --install` réécrirait aussi `INSTALL`, `config.guess`,
  `config.sub`, `ltmain.sh` et `m4/libtool.m4`, ces deux derniers avec le
  libtool de Debian (2.5.4-9), dont les correctifs entreraient dans
  `configure`. Le diff de `configure` ne doit contenir que l'effet de ton
  changement.
- **Le lexer et le parser** : `sh gaol/parser/regenerate_parser.sh`, avec bison
  3.8.2 et flex 2.6.4.
- Les fichiers du dépôt sont en LF. Seul `manual/v4/relation-cos.tex` est en
  CRLF, et `manual/v4` ne se touche pas.

## 7. La procédure, pas à pas

1. **Comprendre le point.** Lis-le dans `TODO.md` (sa tâche PA à PJ) avec ses
   sous-points, ses décisions (celles de `TODO_DONE.md` aussi) et les issues
   qu'il cite. Lis aussi ce qui le concerne dans
   `todo-notes/synthese.md` (et au besoin le rapport d'origine, au commit
   `16a2f60`), la revue d'`examples/examples.md` s'il la cite, et le code
   concerné. Reproduis le défaut avant de le corriger. S'il est déjà corrigé ou
   ne se reproduit pas, dis-le avec la preuve, sans forcer de changement.
2. **Poser les questions qui reviennent au mainteneur avant d'écrire du code.**
   C'est le cas d'un nom d'API publique, d'un compromis que ni le TODO, ni ses
   décisions, ni la section 4 ne tranchent, ou d'un changement d'interface ou
   de compatibilité. Pose-les avec l'outil de questions :
   - 2 à 4 options, celle que tu recommandes en premier, marquée
     « (Recommandé) » ;
   - le plus souvent, une option « Reporter (créer une issue) ».

   Une question reportée devient une issue GitHub en français (`gh issue create`),
   citée dans `TODO.md`. Quand le TODO offre des alternatives (« ou... ») que les
   principes tranchent, choisis, et explique ton choix dans la pull request.
3. **Créer la branche** dans l'arbre choisi :

   ```sh
   git fetch origin && git status --short     # repère ce qui n'est pas à toi
   git switch --no-track -c todo-<lettre>-<sujet> origin/configure-clean
   ```

   `--no-track` : sans lui, la branche suit `origin/configure-clean`, et avec
   la synchronisation de VS Code des commits de branche sont arrivés
   directement dans `configure-clean` (c7915a7 et ccf694a, le 6 octobre).

4. **Corriger, au plus juste.** Fais ce que le point demande, avec son test et
   la documentation de ce qui change : `doc/*.md`, les commentaires Doxygen des
   en-têtes, `manual/v5/gaol.tex`. Pas de nettoyage hors sujet, ni de
   reformatage du code que tu ne changes pas. Un bug remarqué en chemin qui
   n'est pas ton point va dans les questions ouvertes, pas dans ta branche.
5. **Ne touche pas aux registres dans la branche** : `TODO.md`, `TODO_DONE.md`,
   `TODO_AFTER.md`, `ChangeLog`, `doc/differences.md`, `manual/v5/gaol.pdf`, et
   jamais `manual/v4`. Le texte que tu y aurais mis va dans la section « Pour
   `doc/differences.md` et `ChangeLog` » de la pull request, que reprendra la
   pull request de synthèse (tâche PI).
6. **Tester avec des dents** (section 5), puis **valider** (section 6).
7. **Faire relire.** Un relecteur indépendant relit la branche : un sous-agent
   sans ton contexte, à qui tu donnes ce fichier, le point, la base et le nom
   de la branche. Il cherche :
   - les bugs et les affirmations fausses, dans le code, les commentaires et la
     documentation ;
   - les cas oubliés, les plateformes de la CI ;
   - les trois builds ;
   - les tests sans dents ;
   - les fichiers générés, qu'il régénère pour vérifier qu'ils sont identiques.

   Corrige ses points bloquants et vérifie-les. La pull request dit ce qu'il a
   trouvé et vérifié.
8. **Contrôler et commiter.** Lance `check_branch <branche>` depuis l'arbre
   (`todo-notes/orchestration/bin/check_branch`, à copier dans ton
   scratchpad). Il vérifie les messages (deux lignes au plus, aucune mention
   de Claude), l'absence des registres, du PDF et des produits de build, et les
   espaces. Commite par fichiers nommés (section 3.1). Plusieurs commits
   courts valent mieux qu'un fourre-tout.
9. **Pousser et ouvrir la pull request.** Un push sur une branche autre que
   `master`, `MATH-CORE` ou `configure-clean` ne lance **aucune** CI. Elle part
   à l'ouverture de la pull request, ou par `workflow_dispatch`
   (`gh workflow run <fichier>.yml --ref <branche>`).

   ```sh
   git fetch origin && git merge origin/configure-clean   # si la base a avancé
   git push -u origin <branche>
   gh pr create --base configure-clean --head <branche> --title "<titre>" --body-file <fichier>
   ```

   La forme du texte est donnée à la section 8. Le jeton de `gh` peut ne
   permettre ni de créer une pull request ni de lancer un workflow (HTTP 403,
   « Resource not accessible by personal access token », le 12 octobre) :
   pousse alors la branche, et donne au mainteneur le titre et le texte de
   chaque pull request, une par une ; il les ouvre lui-même.
10. **Surveiller la CI et corriger** (section 10) jusqu'à ce que tout soit vert.
    Chaque correction est un nouveau commit sur la branche, puis un push.
11. **Rendre compte** au mainteneur, en français :
    - le lien de la pull request ;
    - ce qui a été vérifié, et ce qui ne l'a pas été ;
    - l'état réel de la CI : ne la dis jamais verte avant de l'avoir vue ;
    - les questions ouvertes, posées avec l'outil de questions.

    Le mainteneur demande souvent « est-ce que tu as des questions ? » à la fin
    d'un point : prépare-les.
12. **Après la fusion par le mainteneur**, mets à jour `TODO.md`,
    `TODO_DONE.md` et leurs issues #179 et #177 (section 9) et supprime ta
    branche locale (`git branch -d`).

## 8. Le texte de la pull request

C'est un texte en français correct, avec ses accents et la typographie
française (espace avant `;`, `:`, `!` et `?`, guillemets « »). Le ton est
factuel, sans emoji, sans aucune ligne qui crédite Claude. Les identifiants, les
fichiers, les options et les commandes restent en `code`. Un relecteur qui n'a
pas suivi le travail doit pouvoir comprendre et relire le changement à partir
de ce seul texte, et rien n'y est inventé : chaque chiffre vient d'une mesure.

- **Titre** : une ligne, environ 90 caractères au plus, sans point final. Par
  exemple : `Point J : gaol::sin(0.5) compile, en-têtes publics nettoyés et
  macros préfixées GAOL_, -Wall -Wextra -Werror dans la CI`.
- **Corps**, dans cet ordre ; omets une section seulement si elle n'a rien à
  dire :

```markdown
Point X de `TODO.md` : <ce que demande le point> (anciens n, m).

## <Sous-partie 1, par ancien numéro ou par thème>
Problème (exemple d'entrée et de sortie fausse), cause, correction (fichiers et
fonctions, choix faits et alternatives écartées), tests, et comment on a montré
qu'ils échouent sans la correction.

## Pour `doc/differences.md` et `ChangeLog` (pull request de synthèse)
Les puces à reporter dans ces registres : ce qui change pour l'utilisateur.

## Relecture
Ce que le relecteur indépendant a trouvé (corrigé) et ce qu'il a vérifié.

## Vérifié en local
Builds, compilateurs et nombre de tests réussis : CMake SSE2, FPU et Clang 18,
-Wall -Wextra -Werror, autotools, meson, manuel.
**Non vérifiable ici** : ce que seule la CI couvre.

## Dépendances
« Aucune. », ou les pull requests à fusionner d'abord, et pourquoi.

## Questions ouvertes
Les décisions laissées au mainteneur, et ce qui a été remarqué sans être corrigé.
```

Si une pull request dépend d'une autre, dis-le explicitement dans le texte.

## 9. Après la fusion : les registres `TODO.md`, `TODO_DONE.md`, `TODO_AFTER.md`

Trois registres, chacun un fichier de `configure-clean` et une issue au même
texte :

| Registre | Fichier | Issue |
| --- | --- | --- |
| Le reste à faire, en tâches PA à PJ | `TODO.md` | #179 |
| Le fait et le décidé | `TODO_DONE.md` | #177 |
| Le reporté | `TODO_AFTER.md` | #178 |

Les fichiers se mettent à jour **directement sur `configure-clean`**, et les
issues par `gh`, une fois les pull requests fusionnées par le mainteneur.
Fais-les ensemble, et regarde d'abord les derniers commits de
`origin/configure-clean` après un `git fetch origin` : une autre session peut
les avoir faites (§12).

**Les fichiers.** D'abord, `git diff TODO.md TODO_DONE.md TODO_AFTER.md` : si le
mainteneur y a des changements non commités, ne les écrase pas et ne les
commite pas. Fais seulement ta mise à jour, ou demande-lui. Ensuite :

- `TODO.md` : l'en-tête « au commit `xxxxxxx` de `configure-clean` » prend le
  commit courant, celui de la fusion ; retire le point fait de sa tâche et des
  phrases qui le citent (une tâche entièrement faite sort du fichier et du
  tableau « Les tâches restantes ») ; un point fait en partie (106 et 107 par
  #146) reste dans sa tâche, avec « fait par #NN pour ... » ; les « Questions
  ouvertes » de la pull request, et ce qu'elle a remarqué sans le corriger, vont
  dans la tâche où elles se traitent, en « À décider » quand le mainteneur n'a
  pas répondu, ou dans `TODO_AFTER.md` quand c'est reporté ; « Aucune pull
  request n'est ouverte » devient la liste de celles qui le sont ;
- `TODO_DONE.md` : ajoute la pull request à la section 2.3 (ce qu'elle fait,
  avec ses chiffres), ses décisions à la section 3.4 (« Décidé pendant le point
  X (<date>), fait par #NN »), le point fait à la section 4 ou 5, la ligne du
  jour à la section 7 et, à la section 8, les branches supprimées ; mets à jour
  l'état de la section 1 ;
- `TODO_AFTER.md` : seulement si quelque chose est reporté, écarté ou repris ;
- `process.md` : la date et le commit de l'en-tête, et l'état du §12 (pull
  requests ouvertes, faits, à faire).

Garde des lignes de 80 colonnes au plus. Commite seulement les fichiers que tu
as changés, en les nommant (`git add TODO.md TODO_DONE.md`), avec un sujet comme
`TODO files: point X done by #NN`, puis pousse. Ce push annule la CI en cours du
même workflow sur `configure-clean` (§10) : si tu as besoin de la CI du commit
de fusion, attends qu'elle finisse.

**Les issues.** Chaque issue a le même texte que son fichier, un paragraphe ou
une puce par ligne : joins chaque ligne de continuation à la précédente par une
espace, sans toucher aux titres ni aux lignes de tableau (les fichiers n'ont pas
de bloc de code). Sans ligne de crédit :

```sh
cat > join.py <<'EOF'
import re, sys
MARK = re.compile(r'^\s*([-*+]|\d+[.)])\s')
OTHER = re.compile(r'^(#{1,6} |\s*\|)')
out, prev = [], None
for l in open(sys.argv[1]).read().rstrip('\n').split('\n'):
    kind = 'other' if (not l.strip() or OTHER.match(l)) else (
        'item' if MARK.match(l) else 'text')
    if kind == 'text' and prev in ('item', 'text'):
        out[-1] += ' ' + l.strip()
    else:
        out.append(l.rstrip())
    prev = kind
print('\n'.join(out))
EOF
python3 join.py TODO.md > issue.md
gh issue edit 179 --body-file issue.md
gh issue view 179 --json body --jq .body | diff - issue.md   # seul le saut de ligne final diffère
```

Pour `TODO_DONE.md` c'est l'issue #177, pour `TODO_AFTER.md` l'issue #178.

Si `gh issue edit` ou `gh pr edit` échoue avec « Projects (classic) is being
deprecated » (constaté avec `gh pr edit`), passe par l'API REST :
`gh api -X PATCH repos/Jordan08/GAOL/issues/179 -F body=@issue.md`
(`pulls/<n>` pour une pull request).

Les anciennes issues #49 et #142, fermées le 24 octobre 2026, ne se mettent
plus à jour : leur contenu est repris dans `TODO_DONE.md` et `TODO.md`.

## 10. La CI

- **Les workflows** (`.github/workflows/`, décrits dans
  `doc/continuous-integration.md`) :
  - `linux.yml` : GCC et Clang, sanitizers, CMake 3.14, GCC 9, GAOL inclus
    par FetchContent, jobs `warnings` en `-Werror` ;
  - `containers.yml` : Debian, manylinux, Alpine, qemu ;
  - `macos.yml` : dont un job AppleClang en `-Werror` et les sanitizers ;
  - `windows.yml` : Visual Studio 2022 et 2026, clang-cl, MinGW-w64, MSYS2, et
    deux jobs `/W4 /WX` ;
  - `build-systems.yml` : autotools, meson, audit des trois builds,
    `make distclean` ;
  - `manual.yml` : ne tourne que si `manual/` change ;
  - `performance.yml` : ne tourne que sur les pull requests qui touchent le
    code, les `CMakeLists.txt` ou les outils de mesure ; construit et mesure
    la base de la pull request et son commit de fusion dans le même job, et
    écrit un tableau dans le résumé du job (aucun temps ne fait échouer un
    job).
- **Le déclenchement** : sur chaque pull request (`manual.yml` et
  `performance.yml` seulement si leurs chemins changent), sur un push vers
  `master`, `MATH-CORE` ou `configure-clean` (sauf `performance.yml`, qui ne
  tourne que sur les pull requests), et par `workflow_dispatch`. La
  `concurrency` annule le run en cours d'un même workflow sur la même
  référence : un nouveau push sur `configure-clean` annule la CI du commit
  précédent. Ne pousse pas un commit de registre pendant qu'une CI dont tu as
  besoin tourne, ou sache que la suivante la remplace.
- **Suivre** :

  ```sh
  gh pr checks <n>                         # ou :
  gh run list --commit <sha complet> --json name,status,conclusion
  gh run view <run> --json jobs --jq '.jobs[] | select(.conclusion=="failure") | "\(.name) \(.databaseId)"'
  gh api --allow-escape-sequences repos/Jordan08/GAOL/actions/jobs/<job>/logs > job.log
  ```

  `gh run list --commit` exige le SHA complet. Pour attendre, une boucle en
  arrière-plan qui interroge toutes les deux minutes suffit.
- **Avant de conclure à une régression**, compare avec les runs précédents du
  même job. Le runner macOS Intel, par exemple, a fait varier `numbers` de 146 à
  plus de 400 s sur le même code. Un dépassement de délai sur un runner lent se
  relance (`gh run rerun <run> --failed`). S'il revient, relève le délai de ce
  job : c'est une décision du mainteneur. Un test qui échoue vraiment se
  corrige ; on ne l'affaiblit jamais pour faire passer la CI.
- **Particularités connues** :
  - avec `/WX`, MSBuild arrête les projets qui dépendent d'une cible en erreur ;
    un même problème peut donc demander plusieurs tours de CI ;
  - avec Visual C++, `/W4` passe par les variables d'environnement `CFLAGS` et
    `CXXFLAGS` (CMP0092 est NEW) ;
  - `-Werror` et `/WX` ne s'appliquent qu'aux cibles
    (`CMAKE_COMPILE_WARNING_AS_ERROR`) ;
  - les jobs Debug de Visual C++ écrivent les assertions du runtime sur stderr
    au lieu d'ouvrir une boîte de dialogue (`tests/gaol_tests.h`).

## 11. Pièges déjà rencontrés

- Le compte-rendu d'un agent, ou une note, ne prouve rien : vérifie dans le code,
  le diff ou les logs avant d'affirmer quelque chose au mainteneur.
- La sortie de `TODO.md` dans la conversation ne suffit pas pour éditer :
  relis-le juste avant, car le mainteneur le modifie souvent.
- Un sujet de commit trop long ne se corrige plus une fois poussé sur
  `configure-clean` (on ne force pas cette branche) : mesure-le avant.
- `make distclean` doit rendre l'arbre tel que git l'a, y compris sans laisser
  de répertoire `.deps` vide. La CI le vérifie.
- `pkg-config --libs gaol` ne donne pas `-lm` : g++ et clang++ lient eux-mêmes
  la bibliothèque mathématique, que GAOL appelle encore (`<fenv.h>`, `sqrt`,
  `floor`, `fma` sans FMA matériel...). Ne rajoute pas `-lm`.
- Les en-têtes internes (`gaol_interval_parser.h`, `gaol_init_cleanup.h`...)
  ne sont pas installés, et `gaol/gaol` n'inclut pas
  `gaol_expression.h` : un programme qui construit des expressions l'inclut
  lui-même.
- Les constantes `double` de `gaol_port.h` ne font plus partie de l'interface.
  Il reste `detail::pi_dn`, `pi_up`, `half_pi_dn` et `half_pi_up` ; le code
  utilisateur emploie `interval::pi()`, `interval::half_pi()` et
  `interval::two_pi()`.
- Des branches locales et des stashs du mainteneur existent :
  - ne supprime une branche que si elle est entièrement contenue dans
    `origin/configure-clean` (`git branch -d` le vérifie) ;
  - ne touche pas à une branche non fusionnée sans son accord (par exemple
    `fix-path-core-math`, qui est la seule copie de son travail) ;
  - ne lance pas `git stash` sur ses fichiers.
- Le compilateur fusionne les comparaisons de doubles de part et d'autre d'un
  changement de mode (`GAOL_RND_ENTER()` et `GAOL_RND_LEAVE()` avec
  `GAOL_PRESERVE_ROUNDING`, flush-to-zero ou denormals-are-zero du programme qui
  reviennent) : un `asm volatile` ne les ordonne pas avec les opérations sur les
  registres, et une comparaison refaite après `LEAVE` peut reprendre la valeur
  calculée dans la fenêtre (`[c, r]` de deux sous-normaux sortait vide avec GCC
  15 dans `bisect(ratio)`). Compare par les bits (`gaol_detail::bound_less()`,
  `bound_key()`...) ou fais tout, la construction du résultat comprise, avant
  `LEAVE`.
- GCC 12.2 sur aarch64 (le job Debian 12 arm64) lit de mauvaises valeurs dans
  une boucle sur un tableau local de structures (`-O2` et `-O3` ; `-O0` et `-O1`
  donnent les bonnes) : écris un appel de fonction par cas. C'est le même défaut
  que celui de `tests/numbers.cpp`, qui dépasse son délai de 300 s quand le code
  autour des appels change.
- Sans matériel ARM : une compilation croisée avec GCC 12.2 dans une image
  Docker locale (`debian:bookworm`, `g++-aarch64-linux-gnu`, `qemu-user`, au
  plus 4 cœurs avec `--cpus`), les tests lancés par `qemu-aarch64 -L
  /usr/aarch64-linux-gnu`. QEMU 7.2 (Debian 12) et 8.2 (Ubuntu 24.04) ne
  retiennent pas `FPCR.AH` ; QEMU 10.0 (Debian 13), avec `-cpu max`, le retient.
  Les binaires de Debian 12 tournent sous le QEMU de Debian 13. Les images
  créées pour un essai se suppriment ensuite (`docker rmi`), celles des sessions
  précédentes (`gaol-bookworm-cross`, `gaol-i386-build`) restent.
- Pour montrer qu'un test échoue sans la correction, ne remets que les fichiers
  de la correction, par leur nom (`git checkout origin/configure-clean --
  gaol/f.cpp`), jamais `git checkout origin/configure-clean -- .`, qui efface
  aussi les modifications non commitées du test ; garde la correction en patch
  pour la rétablir.
- Un changement de signature d'un en-tête installé touche beaucoup de lignes :
  cherche les usages internes avec le compilateur (marque la fonction
  `[[deprecated]]` provisoirement et compile avec chaque configuration : SSE2,
  SSE2 avec AVX-512, NEON en compilation croisée), non par `grep`, et recommence
  après chaque fusion de `configure-clean`.
- Le test d'allocations de `tests/expressions.cpp` remplace `operator new` et
  `operator delete` du programme. Quand la bibliothèque C++ est dans une DLL
  (Windows : libstdc++ de MSYS2 et de MinGW-w64, libc++ de CLANG64), il ne voit
  pas les allocations qu'elle fait pour un `std::string`, dont le destructeur en
  ligne appelle pourtant le `operator delete` du programme : un `std::string` de
  plus de 15 caractères (22 avec libc++) dans le code de la lecture fait
  échouer « a reading frees all it allocated » (quatre jobs de #149). Un
  `std::vector` est sans danger.
- Une écriture du registre de contrôle par `asm volatile` (avec « memory » dans
  les clobbers) est ordonnée avec les accès à la mémoire et avec les autres
  `asm`, non avec les opérations sur des valeurs en registres (bogue 34678 de
  GCC) : une fonction en ligne qui lit ses bornes avant `GAOL_RND_ENTER()`
  calcule dans le sens du programme (#150). La règle est dans le commentaire
  de `gaol/gaol_fpu.h` : lire les bornes après la vérification, et passer un
  `double` pris par valeur par `rnd_reread()`. Un test doit appeler la
  fonction sur un intervalle construit dans la même fonction, non sur un
  intervalle relu en mémoire, qui cache le défaut.
- Un script qui édite un long fichier avec `s.index("# un commentaire")` peut
  tomber sur une occurrence plus haute et dupliquer des centaines de lignes :
  `meson.build` est passé de 775 à 1097 lignes dans #192, et la copie, que
  meson exécutait en dernier, aurait laissé clang-cl et MinGW-w64 en FPU sans
  qu'aucun job de la CI le voie. Contrôle `wc -l` et `git diff --stat` après
  toute édition par script.
- Visual C++ (`cl`) pour ARM64 (#192) : `/volatile:iso` y est le réglage par
  défaut, et un `volatile` n'y est pas ordonné avec le reste, d'où
  `_ReadWriteBarrier()` avant et après `_WriteStatusReg()`
  (`gaol/gaol_fpu_fenv.h`) ; `float64x2_t`, `uint64x2_t` et `int64x2_t` y sont
  un seul type, `__n128` (non revérifié ici). Et `/O2
  /fp:strict` réécrit `(-.5)*l + .5*r` en `.5*r - .5*l` : `mid()` passe les
  moitiés négatives par `rnd_keep()`, dans toutes les sortes d'intervalles.
- Sur le runner macOS, `arch -x86_64 ctest` échoue quand le `ctest` (Homebrew)
  n'est que pour arm64 : lance les programmes de test un à un (#194). Et sous
  `bash -e`, un `! grep ...` au milieu d'une étape ne la fait pas échouer (seule
  la dernière commande donne son statut) : écris un `if` explicite.

## 12. Où en est le travail (27 octobre 2026)

Le reste à faire est dans `TODO.md` (tâches PE à PJ) et l'issue #179 ; ce
qui est fait et décidé (pull requests fusionnées, décisions, mesures) est dans
`TODO_DONE.md` et l'issue #177 ; ce qui est reporté est dans `TODO_AFTER.md` et
l'issue #178. L'état qui suit est daté : les commandes ci-dessous, les trois
fichiers et les issues font foi. Avant de commencer un point, regarde les pull
requests ouvertes et les branches distantes : une autre session peut déjà le
traiter (le 19 octobre, la même mise à jour de `TODO.md` a été faite deux fois).

```sh
gh api 'repos/Jordan08/GAOL/pulls?state=open' --jq '.[] | "#\(.number) \(.head.ref) \(.title)"'
git ls-remote --heads origin
```

- **Faits** : tout ce que dit `TODO_DONE.md` (sections 4 et 5), jusqu'à #176,
  #180 à #184 et #186 à #190 (les tâches PA, PB et PC), #196 et #197 (la tâche
  PD), et #191 à #194 (de la tâche PG : la division du chemin AVX-512, puis NEON
  avec tous les compilateurs de l'arm64, SSE2 avec tous ceux de l'x86, le FPU en
  dernier recours) ; #199 et #200 (hors des tâches : les sources des intervalles
  dans `gaol/core/`, puis les options des trois builds : les mêmes options pour
  l'utilisateur, tout le reste décidé par la configuration) et #201 (les mêmes
  drapeaux C pour CORE-MATH, comparés par l'audit) ; l'audit du 13 octobre a
  vérifié dans `configure-clean` ce qu'annonçaient alors les points A, C à H et
  J à V.
- **En cours** : deux pull requests ouvertes au 27 octobre, #202 (le point 4 de
  PE : les remarques d'actionlint et de shellcheck, branche
  `todo-pe-tool-remarks`) et #203 (le point 6 : la relecture des commits d'après
  #163, branche `todo-pe-review-fixes`) ; la branche `todo-pe-mingw32` (le point
  3 : les jobs qui manquent) attend sa relecture et sa pull request ;
  `configure-clean` est au commit de la fusion de #201, et ses branches sur
  GitHub sont `configure-clean`, `MATH-CORE`, `master`, `perf-doc-rerun` (les
  scripts des mesures reportées, hors des points) et celles de ces trois
  travaux. L'issue #198 (`x / y` des intervalles NEON avec `FPCR.AH`) est
  ouverte, sans tâche dans `TODO.md` ; #185 est fermée, le masquage des
  exceptions de `pow` est gardé.
- **À faire**, dans l'ordre de `TODO.md` : le reste des trois builds et de la CI
  (PE : le reste du 95 avec le commentaire du bogue 54412, la pull request du
  point 3), x86 32 bits et Windows (PF), les études de vitesse, toutes avant
  v5.0.0 (PG, entamée), les signalements à d'autres projets (PH, avec l'accord
  du mainteneur), puis la version (PI, PJ). Ce qui est reporté (le 113, le site,
  les traits pour YalAA et VNODE-LP, les décorations, ce qui attend du matériel)
  est dans `TODO_AFTER.md`. Les dépendances entre points sont écrites dans
  chaque tâche. Le mainteneur donne les tâches ou les points un par un (« traite
  la tâche PE », « traite le point 129 ») : ne commence pas le suivant sans lui.
