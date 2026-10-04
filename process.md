# Procédure pour traiter un point du TODO.md de GAOL v5

Ce fichier s'adresse à un agent IA qui reprend le travail sur GAOL v5. Il décrit
la procédure suivie, point après point, depuis fin septembre 2026 : ce qu'exige
le mainteneur, comment on valide, teste, commite, pousse, fait passer la CI et
propose une pull request, et comment on tient les registres à jour une fois la
pull request fusionnée. Lis-le en entier avant de toucher au dépôt.

État au 4 octobre 2026 : `configure-clean` est au commit `dc036c6`, et toute sa
CI est verte.

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
  - `todo-status` : les rapports et l'outillage de l'orchestration
    (`todo-notes/`).
- **L'arbre de travail** : `/home/jninin/Documents/WORK/DEV/GAOL/GAOL_V8`. Le
  mainteneur et d'autres agents y travaillent aussi. Des fichiers peuvent y être
  modifiés sans être à toi ; c'est souvent le cas de `TODO.md`, que le
  mainteneur enrichit lui-même.

## 2. Les sources de vérité

| Quoi | Où |
| --- | --- |
| Les points à faire, regroupés par lettres (A à Z), avec leurs décisions | `TODO.md` de `configure-clean` |
| Le suivi d'ensemble (fait, en cours, reste à faire) | issue #49 |
| Les questions reportées | issues #64 à #70, et celles qui suivront |
| Les rapports des agents, l'outillage (`HOUSE_RULES.md`, `check_branch`, `gcore`...) | branche `todo-status`, `todo-notes/` |
| La revue d'origine (« revue n° n » dans `TODO.md`) | `examples/examples.md`, section 5 ; corrections proposées en annexe B |
| La documentation : construire, utiliser, précision, tests, CI, écarts avec GAOL 4 | `doc/*.md`, `README.md` |
| Le manuel de GAOL v5 | `manual/v5/gaol.tex` (`manual/v4` est celui de GAOL 4 : n'y touche jamais) |
| La norme IEEE 1788-2015 | `/home/jninin/Documents/WORK/REDAC/ARTICLE/1788-2015.pdf` (table 9.1 p. 28-29, table 10.5 p. 40, 12.12.8 p. 63, littéraux 12.11) |
| Les consignes permanentes | `.claude/CLAUDE.md`, et `~/.claude/CLAUDE.md` |

Chaque point de `TODO.md` a une lettre. Il garde en sous-points les anciens
numéros (1 à 72), que citent les pull requests, les issues et les rapports. La
table à la fin de `TODO.md` donne la lettre de chaque ancien numéro, et un
numéro absent de la table est un point fait. « Décidé le 3 octobre » (ou une
autre date) marque une décision du mainteneur : applique-la telle quelle et ne
la remets pas en question. Si elle te paraît fausse, dis-le et demande.

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
  de `todo-status` (`todo-notes/orchestration/bin/gcore`) distribue des jetons
  de cœur.
- Les répertoires de build et les fichiers temporaires vont **hors de l'arbre**,
  dans ton scratchpad, jamais dans `/tmp` directement ni dans le dépôt. Les
  chemins `/tmp/claude-1001/...` des anciennes sessions sont éphémères : ne
  compte pas dessus.
- Pas de `sudo`.

### 3.4 Branches et arbre de travail

- Travaille **en local, dans l'arbre principal, sur une nouvelle branche** créée
  depuis `origin/configure-clean` à jour. Jamais de worktree : le mainteneur l'a
  demandé le 3 octobre (« met toi en local dans la branche sur laquelle tu
  travailles »).
- Noms de branche : `todo-<lettre>-<sujet>` pour un point du TODO
  (`todo-j-headers-werror`, `todo-l-ci-workflows`) ; un nom descriptif pour une
  autre tâche (`autotools-distclean`, `no-explicit-libm`).
- Une seule exception : on commite **directement sur `configure-clean`**
  - la mise à jour de `TODO.md` après une fusion (section 9) ;
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
   les mêmes options. Si tu ajoutes ou retires une source, un en-tête, un test
   ou une option, mets à jour les trois :
   - `CMakeLists.txt` ;
   - `Makefile.am`, avec son `Makefile.in` régénéré (section 6.3) ;
   - `meson.build` ;
   - les listes d'en-têtes installés.

   `.github/audit` vérifie dans la CI que les trois builds donnent la même
   configuration.
7. **L'interface.**
   - Les constructeurs d'`interval` sont `explicit` ; ne réintroduis pas de
     conversion implicite.
   - Les fonctions `*_dn()` et `*_up()` sur les doubles sont privées
     (`gaol_double_op.h` n'est pas installé).
   - Les intervalles de floats restent désactivés (`GAOL_FLOAT_INTERVALS`).
   - Les macros des en-têtes publics commencent par `GAOL_`, et chaque en-tête
     installé compile seul en C++11 avec `-Wall -Wextra -Werror`
     (`.github/scripts/headers.sh`).
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
    Codac. Les comparaisons de performance se font avec clang-18, sans
    `GAOL_PRESERVE_ROUNDING`.

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
| GCC | 9.4 (`gcc`, `g++`) |
| Clang | 18 (`clang-18`, `clang++-18`) |
| CMake | 4.4 (`~/.local/bin/cmake`) |
| autoconf, automake, libtool | 2.72, 1.18.1, 2.5.4 (`~/Logiciel/autotools/bin`) |
| bison, flex | 3.5.1, 2.6.4 (les versions des fichiers générés commités) |
| meson, ninja | meson 0.53.2 (le système, la plus ancienne prise en charge), ninja 1.10 |
| Autres | `pdflatex`, `gcovr`, python3.8 avec mpmath |

Une version récente de meson s'installe dans ton scratchpad
(`pip install --target`), puis se lance avec
`PYTHONPATH=<dir> python3 -m mesonbuild.mesonmain`.

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
- **`configure`** : `AUTOHEADER=true autoreconf --force --install`, puis
  `git checkout -- INSTALL config.guess config.sub` et `rm -f configure~`.
- **Le lexer et le parser** : `sh gaol/regenerate_parser.sh`, avec bison 3.5.1
  et flex 2.6.4.
- Les fichiers du dépôt sont en LF. Seul `manual/v4/relation-cos.tex` est en
  CRLF, et `manual/v4` ne se touche pas.

## 7. La procédure, pas à pas

1. **Comprendre le point.** Lis-le dans `TODO.md` avec ses sous-points, ses
   décisions et les issues qu'il cite. Lis aussi les rapports qui le concernent
   dans `todo-notes/` (`git show origin/todo-status:todo-notes/...`), la revue
   d'`examples/examples.md` s'il la cite, et le code concerné. Reproduis le
   défaut avant de le corriger. S'il est déjà corrigé ou ne se reproduit pas,
   dis-le avec la preuve, sans forcer de changement.
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
3. **Créer la branche** dans l'arbre principal :

   ```sh
   git fetch origin && git status --short     # repère ce qui n'est pas à toi
   git switch -c todo-<lettre>-<sujet> origin/configure-clean
   ```

4. **Corriger, au plus juste.** Fais ce que le point demande, avec son test et
   la documentation de ce qui change : `doc/*.md`, les commentaires Doxygen des
   en-têtes, `manual/v5/gaol.tex`. Pas de nettoyage hors sujet, ni de
   reformatage du code que tu ne changes pas. Un bug remarqué en chemin qui
   n'est pas ton point va dans les questions ouvertes, pas dans ta branche.
5. **Ne touche pas aux registres dans la branche** : `TODO.md`, `ChangeLog`,
   `doc/differences.md`, `manual/v5/gaol.pdf`, et jamais `manual/v4`. Le texte
   que tu y aurais mis va dans la section « Pour `doc/differences.md` et
   `ChangeLog` » de la pull request, que reprendra la pull request de synthèse
   (point Y).
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
8. **Contrôler et commiter.** Lance `check_branch <branche>`
   (`todo-notes/orchestration/bin/check_branch` de `todo-status`, à copier dans
   ton scratchpad). Il vérifie les messages (deux lignes au plus, aucune mention
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

   La forme du texte est donnée à la section 8.
10. **Surveiller la CI et corriger** (section 10) jusqu'à ce que tout soit vert.
    Chaque correction est un nouveau commit sur la branche, puis un push.
11. **Rendre compte** au mainteneur, en français :
    - le lien de la pull request ;
    - ce qui a été vérifié, et ce qui ne l'a pas été ;
    - l'état réel de la CI : ne la dis jamais verte avant de l'avoir vue ;
    - les questions ouvertes, posées avec l'outil de questions.

    Le mainteneur demande souvent « est-ce que tu as des questions ? » à la fin
    d'un point : prépare-les.
12. **Après la fusion par le mainteneur**, mets à jour `TODO.md` et #49
    (section 9) et supprime ta branche locale (`git branch -d`).

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

## 9. Après la fusion : `TODO.md` et l'issue #49

Ces deux mises à jour se font **directement sur `configure-clean`**, une fois
les pull requests fusionnées par le mainteneur.

**`TODO.md`.** D'abord, `git diff TODO.md` : si le mainteneur y a des
changements non commités, ne les écrase pas et ne les commite pas. Fais
seulement ta mise à jour, ou demande-lui. Ensuite :

- l'en-tête « au commit `xxxxxxx` de `configure-clean` » prend le commit
  courant, celui de la fusion ;
- le paragraphe « Fait depuis : ... » gagne « le point X (anciens n, m), par
  #NN », et les décisions prises en fin de point ;
- retire la section du point fait, et sa ligne de « En cours » s'il y en a une ;
- retire ses anciens numéros de la table finale ;
- mets à jour « Ménage » : branches supprimées, lignes de crédit.

Garde des lignes de 80 colonnes au plus. Commite seul `TODO.md`, avec un sujet
comme `TODO.md: point X done by #NN`, puis pousse.

**L'issue #49.** Elle se met à jour par un fichier, sans ligne de crédit :

```sh
gh issue view 49 --json body --jq .body > old.md
# modifier new.md :
#  - « État au <date> (commit <sha> de configure-clean) » ;
#  - ajouter les pull requests à « Fusionné depuis le 30 septembre » ;
#  - cocher le point en section 4 avec « fait par #NN » ;
#  - mettre à jour « Ménage »
gh issue edit 49 --body-file new.md
gh issue view 49 --json body --jq .body | diff - new.md   # seul le saut de ligne final diffère
```

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
  - `manual.yml` : ne tourne que si `manual/` change.
- **Le déclenchement** : sur chaque pull request, sur un push vers `master`,
  `MATH-CORE` ou `configure-clean`, et par `workflow_dispatch`. La
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
- Les en-têtes internes (`gaol_interval_parser.h`, `gaol_exact.h`,
  `sysdeps/`...) ne sont pas installés, et `gaol/gaol` n'inclut pas
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

## 12. Où en est le travail (4 octobre 2026)

- **Faits** : les points C, J, K et L ; `make distclean` (#75) ; les constantes
  de `gaol_port.h` retirées (#77) ; plus de `-lm` explicite (#78) ; les jobs
  AppleClang et Visual C++ x64 et x86 en `-Werror` et `/WX`.
- **Inachevés**, en branches « WIP » :
  - E, `todo-12-long-sums` ;
  - H, `todo-36-upward-rounding-effects`, en conflit avec `configure-clean`
    dans `doc/using.md`.
- **À faire** : A, B, D, F, G, I, M à X, puis Y (la publication de v5.0.0).
  La section « Ordre proposé pour les tâches restantes », à la fin de
  `TODO.md`, donne l'ordre suivi et ses raisons ; les dépendances entre points
  sont écrites dans chaque point. Le mainteneur donne les points un par un
  (« traite le point J ») : ne commence pas le suivant sans lui.
