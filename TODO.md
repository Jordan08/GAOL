# À faire

Ce qui reste à faire sur GAOL v5 au commit `634e3e1` de `configure-clean` (le
dernier commit du code : l'amitié de `gaol_pown()` retirée le 8 novembre, après
la fusion de #294, le manuel dans le style de la thèse, et après #292, #293,
#291, #290 et #289), le 8 novembre 2026, en tâches PH, PJ, PP, PQ, PS et PT
(PA à PG, PI et PL sont faites, PK et PM sont closes sans code : voir
`TODO_DONE.md`). Ce fichier et l'issue #179 ont le même contenu. Ce qui est
fait et décidé est dans `TODO_DONE.md` (l'issue #177 n'est plus mise à jour),
ce qui est reporté dans `TODO_AFTER.md` (issue #178). Les anciennes issues #49
et #142 sont fermées, et `TODO_mistral.md` est supprimé, comme `TODO_fin.md` :
ses articles sont répartis dans les trois registres, dont les tâches PS et PT
(`TODO_DONE.md` 3.25).

Aucune borne fausse n'est connue, sauf peut-être pour un programme qui met
lui-même `FPCR.AH` sur arm64 (issue #198 : `x / y` des intervalles NEON, cause
non trouvée). Rien n'est à envoyer à un autre projet ni à publier sans l'accord
du mainteneur. Trois pull requests sont ouvertes : #297 (la tâche PS, ouverte
par une autre session), #298 (PQ.1, `centered_form`) et #299 (PQ.2, le test de
`hexfloat`). #295 (le texte de `doc/simd.md` et du manuel sur les puissances
entières de NEON, PS.2) est fusionnée à 17 h 21 UTC (`5bd52a6`) : ses registres
restent à faire.

## Comment lire ce fichier

- Une tâche (PH, PJ, PP, PQ, PS, PT) regroupe des points qui touchent le
  même code, les mêmes fichiers ou les mêmes builds de validation, pour les
  traiter dans la même session ; chaque point reste une pull request à part
  (process.md : un point, une branche), sauf quand la tâche dit qu'une seule
  suffit. Les tâches sont dans l'ordre conseillé. Critères : la sûreté des
  bornes d'abord ; ce qui coûte plus cher en attendant (une décision qui
  bloque) ; ce qui tue un programme ou rompt une promesse écrite avant ce qui ne
  fait que ralentir ; le petit et sans dépendance avant le gros ; la version
  (PJ) en dernier. La lettre PR n'est pas une tâche : le sigle dit déjà « pull
  request ».
- PH, PP, PQ, PS et PT ne dépendent d'aucune autre tâche, sauf quand le
  texte le dit (PJ suit PI et les études de PG, qui sont faites ; le point 1 de
  PQ, PS et PT précèdent l'étiquette de PJ.3, et PS précède les mesures de
  PJ.2, qui touchent aussi `doc/simd.md`).
- Chaque point garde sa lettre et son ancien numéro, par exemple « 79 (AB) » :
  ce sont ceux des pull requests, des issues, des commentaires du code et de
  `TODO_DONE.md` (section 4 pour les lettres, section 5 pour les numéros faits).
  Tailles, estimées d'après les descriptions : S, moins d'une demi-journée ; M,
  un à deux jours ; L, plusieurs jours ou incertain.
- « Décidé le 22 octobre (Q13) » marque une décision du mainteneur, dans
  `TODO_DONE.md` (3.5 pour le 22 octobre, 3.6 pour le 23) : applique-la telle
  quelle. « À décider » marque une question sans réponse : pose-la au mainteneur
  avant d'écrire du code.
- La procédure est dans `process.md` ; son §9 dit comment tenir ces trois
  registres à jour après une fusion.

## Les tâches restantes

| Tâche | Contenu | Points | Taille |
| --- | --- | --- | --- |
| PH | Les signalements à d'autres projets | #125 à #138, #175 | accord |
| PJ | Les mesures et la publication de v5.0.0 | 32 à 34, 71, manuel, CI | M |
| PP | Le FIXME de `div_rel` : le cas N1 M en SSE2 | un point | M |
| PQ | `centered_form` pour `midrad`, et le test de `<a, a>` en `hexfloat` | deux points | S |
| PS | Les textes faux ou périmés du code et de la documentation | huit points | S |
| PT | Les relevés et les vérifications jamais faits | deux points | S |

## PH. Les signalements à d'autres projets [attend l'accord du mainteneur]

Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à
refaire avant l'envoi et ses décisions ; les issues « À communiquer… » ne sont
pas touchées par ce ménage. **Rien n'est envoyé**, et rien ne l'est sans
l'accord explicite du mainteneur. Ordre : ce qui est prêt et qui a une valeur
pour GAOL, puis ce qui demande une réplique à écrire, puis ce qui ne demande
probablement rien.

Les brouillons sont à remettre à jour avant tout envoi (relecture du 4
novembre, `TODO_DONE.md` 3.25) : des chemins des issues ouvertes ont changé
avec #199 et #283 (`gaol/gaol_ldexp.h` est `gaol/math/gaol_ldexp.h`, dans les
issues #125, #131 et #132 ; `gaol/core_math_port.h` est
`gaol/math/gaol_core_math_port.h`, dans #127 et #131 ;
`gaol/gaol_interval_core.cpp` est `gaol/core/gaol_interval_core.cpp`, dans #131,
#132 et #133 ; `gaol/gaol_interval_neon.cpp` est
`gaol/core/gaol_interval_neon.cpp`, dans l'issue #198), et l'issue #125 donne
encore le correctif 3 « à envoyer » (l'amont l'a fait, `0d8b885a`) et le
correctif 8 « à écrire » (#144 l'a écrit, `3rd/README.md`).

1. **#125 CORE-MATH** : six correctifs de `3rd/README.md` à refaire ou à envoyer
   sur le master du jour, `0b2ae6d1` (importé le 27 octobre). L'amont a fait
   lui-même le 3 (`rsqrt.c`, `0d8b885a`) et le 4 (`sin.c`, `19e85418`) ; le 1
   (les masques de `long`) ne s'applique plus, `cos.c` ayant été réécrit et les
   lignes de `cosh.c` ayant bougé (ceux de `sinh.c` et de `cosh.c` restent),
   donc à refaire ; les 2 et 5 à 8 s'appliquent tels quels (le 8, `exp_1()`, est
   fait dans la copie de GAOL par #144). Des défauts sans correctif :
   `atan2l.c`, la borne de `ss` de `asinpi_acc()`, `return` d'une expression
   `void` dans `erfc.c`, `erfc` et `__builtin_ldexp()` ; qui envoie et comment
   (merge request sur gitlab.inria.fr ou `git format-patch` à
   core-math@inria.fr). Les deux défauts de `cr_pow()` que GAOL contournait
   (*décidé le 24 octobre*, #184), la comparaison `<` d'un NaN (invalidité) et
   le carré `rh * rh` qui déborde, sont corrigés par `c01f08c5` (5 octobre),
   dans l'import : la pull request #213 (fusionnée le 28 octobre) a retiré les
   contournements, sauf la puissance minuscule de |y| ≥ 2^1013, qu'un défaut de
   plus de `cr_pow()` impose encore (`is_exact()` calcule `y * e`, qui déborde à
   partir de |y e| = 2^1024 quand x est une puissance de 2 : `FE_OVERFLOW` pour
   une puissance qui s'annule) ; #213 a gardé le raccourci (*décidé le 28
   octobre*), qui sera retiré si l'amont corrige. Ce défaut est à proposer
   aussi : la réplique et le correctif d'une ligne (`y <= 1074 &&` devant
   `-1074 <= y * e`, car e ≠ 0 dans ce cas) sont écrits et vérifiés le 28
   octobre sur 50 896 000 appels dans les quatre arrondis (mêmes résultats,
   mêmes indicateurs sauf `FE_OVERFLOW`, levé exactement quand 2^(e y) déborde,
   contre 4 551 008 indicateurs en trop avant) ; le brouillon anglais, avec la
   réplique, est dans #125 (point 9, ajouté le 28 octobre à la demande du
   mainteneur), pas encore dans `3rd/README.md` (correctif 9). Un autre, que
   GAOL contourne sans toucher aux sources, est à proposer aussi : `cr_atan()`
   calcule `x + x` pour un NaN avant le test qui le garde, ce qui lève
   `FE_OVERFLOW` à partir de |x| = 2^1023 avec Visual C++ (`/fp:strict`), que
   `atan_lo()` et `atan_hi()` évitent en rendant les bornes de pi/2 à partir de
   |x| = 2^54 ; un import qui le corrigerait permettrait de retirer ce
   contournement. Question de #212 (fusionnée le 28 octobre) sur des arguments
   difficiles à arrondir de `cos.wc` (le fichier des pires cas de l'amont,
   absent de l'arbre) : *décidé le 28 octobre*, non, les 45 arguments de
   `cos_accurate_path()` suffisent ; elle ne voit pas les fautes de moins de
   2^-70 environ en relatif, et les deux mutants qui survivent ne changent aucun
   résultat observable. Depuis #283, `3rd/math-core/src` ne garde que
   `binary64` : ce qui vise `atan2l.c` et `powl.c` de `binary80` ou `expm1q.c`
   de `binary128` se refait contre un clone de l'amont.
2. **#126 GCC** (la comparaison silencieuse rendue signalante par
   l'if-conversion sur ARM 32 bits et POWER9 et par la vectorisation, et le
   miscompilage de GCC 12.2 sur aarch64 dans `tests/numbers.cpp` ; le bogue
   52258 est de la même famille ; le brouillon a aussi, depuis le 5 novembre
   (rien n'est envoyé), le défaut de GCC 14 que #279 a trouvé : `quiet_less(r,
   l) || r == l` fusionné en un seul `r <= l`, qui signale, sur x86-64, AArch64
   et RISC-V, avec sa réplique à écrire), **#127 mingw-w64** (`fma()` et
   `round()` de msvcrt, `fesetexceptflag()` en 32 bits, le `fma()` x87 de
   mingw-w64 11 ; deux triplets où il se trompe, qui ne sont ni dans le
   brouillon ni ailleurs dans le dépôt que `todo-notes/synthese.md`, lignes 1346
   et suivantes, sont à y ajouter),
   **#129 ITF1788** (huit cas dont le résultat attendu n'est pas le plus serré,
   et, depuis le rapport du 4 novembre, deux cas de `textToInterval` où ITF1788
   attend un intervalle pour un littéral `[l, u]` dont l est au-dessus de u, que
   GAOL rend vide selon IEEE 1788-2015, 12.12.7 ; *décidé le 4 novembre* :
   ajoutés au brouillon de #129, en anglais, pour une issue à part ; *à
   décider* depuis #290 : l'ambiguïté de la Table 10.7 que `overlap` relève, les
   spécifications d'ensembles de `meets` et de `metBy` étant vraies aussi pour
   un point d'intervalle sur une borne de l'autre, avec celles de `finishedBy`
   et de `finishes`, par exemple [2] et [1, 2], à ajouter au brouillon de #129
   ou à envoyer au groupe de la norme, si le mainteneur le veut : il n'a pas
   demandé de signalement), **#130 Boost.Interval** (`rounded_transc_opp` donne
   la borne supérieure pour l'inférieure ; issue seule ou avec une pull request,
   selon la politique de Boost) et **#175 Cygwin** (le `fma()` mal arrondi dans
   les quatre sens, le `nextafter()` qui calcule le débordement dans le sens
   d'arrondi du programme, CPack qui ne met pas le lien `README` dans l'archive,
   cause non trouvée ; mesures de CI et brouillon anglais dans l'issue, à
   rejouer avant l'envoi).
3. **#131 Microsoft** (`ldexp()` de l'UCRT, `NAN` et `INFINITY` sous
   `/fp:strict`, `sqrt()` en 32 bits, runtime de Debug, `nextafter()`, le
   `fma()` d'`ucrtbase.dll` jamais vérifié), **#132 Apple** (`ldexp()` et
   `nextafter()` de macOS ; vérifié pendant la session du 20 au 21 octobre, un
   commentaire posté dans l'issue), **#133 LLVM/Clang** (ARM 32 bits, `fcsel`
   sur arm64, `-ffast-math` et `-frounding-math` ; Clang 11 à 23.1 et le trunk
   émettent encore « overriding currently unsupported rounding mode on this
   target » pour ARMv7 à flottants matériels, GCC ARM 10.5 à 16.1 est correct,
   vérifié pendant la même session ; le refus de Clang par GAOL est donc
   intact), **#135 musl** (la conversion décimale de `printf`) et **#136 les
   auteurs du benchmark de Tang et al.** (fuite de mémoire, bornes infinies,
   entrées de FPBench, constantes décimales, README) : chaque point attend une
   réplique d'un fichier, vérifiée sur la plateforme, et un tri entre défaut,
   limite connue et comportement voulu.
4. **#134 gmpy2** (2.2.1 ; probablement rien à envoyer si 2.3.2 les a corrigés),
   **#137 libieeep1788** (probablement rien : les six cas des réciproques sont
   dans #129, comme les deux littéraux de `textToInterval`, qui viennent aussi
   de ses tests) et **#138 la glibc** (probablement rien : relancer avec la
   glibc 2.44) : vérifier, puis fermer.
5. **#128 Frédéric Goualard** : l'annonce de v5.0.0, après l'étiquette (tâche
   PJ, point 71) ; l'issue est fermée depuis le 18 octobre et le brouillon est
   dans Gmail.

## PJ. Les mesures et la publication de v5.0.0 (Y) [M, dans cet ordre]

En dernier : elle suppose les corrections faites et les études de PG faites
(*décidé le 19 octobre*).

1. **32** : le rapport de couverture ([coverage/README.md](coverage/README.md)
   et `coverage/coverage.html`) a été refait le 4 novembre à `2313e07` (94,3 %
   des 5 466 lignes de `gaol/`, contre 87,2 % le 2026-09-27 à `605728e`), à la
   demande du mainteneur, avec la tâche PL (*décidé le 4 novembre*) : l'ancien
   citait `gaol_exceptions.*` et des fichiers disparus. Le job « Coverage » de
   `linux.yml` le refait à chaque push, mais pas la copie du dépôt (94,9 % de
   6 329 lignes dans celui de #288, où `compoundm1` est entrée). À refaire une
   dernière fois au commit de la version (`-DGAOL_COVERAGE=ON`, cible
   `coverage`, gcovr) et à enregistrer ; *décidé le 15 octobre* : pas avant,
   sauf ce relevé du 4 novembre. Le test `numbers` y dépasse les 300 s de CTest
   quand la machine est chargée (`ctest --timeout` ne les change pas) : le
   relevé du 4 novembre a lancé `ctest -E '^numbers$'`, puis `gaol_test_numbers`
   à la main. Lancer aussi une fois, sous ASan + UBSan, le test `numbers` en
   entier (trop long pour la relecture du 14 octobre : 158 s pour `arithmetic`
   seul) : fait en local le 4 novembre, en Debug, 9 tests sur 9 ; à relancer au
   commit de la version si `numbers` ou la bibliothèque ont changé depuis ;
   `GAOL_NUMBERS_TIMEOUT` n'était pas relevé pour les builds avec sanitizers
   (*décidé le 14 octobre*) ; il vaut 400 s dans les quatre jobs de `linux.yml`
   qui en ont, depuis #281 (*décidé le 3 novembre* : `numbers` a dépassé 300 s
   une fois, dans le job Clang x86_64).
2. **33** : les temps de
   [doc/compare/performance.md](doc/compare/performance.md), remesurés le 17
   octobre (`febd2b1`, `a055160`), sa colonne « rounding direction preserved »
   le 18 (`7fd5c09`), avec Clang 21 sur l'i7-1185G7 ; les études de PG, faites,
   les ont changés. *Décidé le 30 octobre* : le tableau de Clang 21 de
   `doc/compare/performance.md` et de `process.md` §4.11 se remesure ici, et
   seulement ici (sur l'i7-1185G7 avec Clang 21, la base d'avant #232 avait
   `line of 15 +` à 22,4 ns et la base avec elle à 13,2 ; `x &= y`, que #234
   ralentit avec Clang 21 (rapport 0,92, accepté le 29 octobre) ; `max` et
   `min`, `x * y`, `inverse()`, `interval(l, r)` et `x <= y` ont changé de noyau
   ou de disposition depuis). À remesurer en dernier, sur le commit propre de la
   version, la machine ne faisant rien d'autre (`doc/compare/code/run_bench.sh`,
   ou `make perf`), puis reprendre le tableau de `doc/compare/README.md` :
   `performance.md` cite encore l'amont de CORE-MATH `6b84457`, et l'import du
   27 octobre (`0b2ae6d1`) a réécrit `sin`, `cos` et d'autres fonctions, dont
   les temps peuvent changer (*décidé le 27 octobre* : dans PJ). Les temps pris
   sur une machine non au repos (#158, les racines d'un x sous-normal) sont à
   refaire ici si on les cite. Refaire aussi, au même commit, les colonnes de
   Visual C++ et de clang-cl pour x86 et pour arm64 de `doc/using.md` (« The FPU
   intervals ») et de `doc/three-builds.md` : elles ont été mesurées avec les
   intervalles FPU, avant #192 et #193 (une phrase le dit), et ces compilateurs
   ont maintenant SSE2 et NEON. *Décidé le 25 octobre* (#192, #193) : à la fin.
   Refaire aussi, au même commit, le tableau des lignes longues sur un
   processeur ARM de 64 bits, à la fin de `performance.md`
   (`doc/compare/code/run_lines_arm64.sh` sur un Mac M2 sur secteur, sans mode
   économie d'énergie, au repos) : #216 l'a ajouté avec une première mesure du
   28 octobre (*décidé le 28 octobre*). Refaire enfin, au même commit, la mesure
   de la règle du chemin AVX-512 de `doc/simd.md` (il suit
   `GAOL_PRESERVE_ROUNDING`, seuil d'un gain médian de 5 %, mesuré le 26 octobre
   sur le seul i7-1185G7, dont l'AVX-512 est à demi-largeur ; *remesuré le 7
   novembre par #294* sur le même processeur, Clang 21.1.8, médiane de 9
   passes : la règle tient, 0,83 sans préservation et 2,87 avec, et
   `doc/simd.md`, `doc/using.md`, `CMakeLists.txt` et les chapitres 4 et 12 du
   manuel portent les chiffres du 7 novembre), sur une machine à AVX-512 pleine
   largeur si le mainteneur en donne l'accès (un Xeon, un Zen 4) :
   `.github/scripts/performance-ab.sh --runs 9 --lines`, quatre builds de la
   même source ; corriger la règle et `doc/simd.md` si le résultat diffère (et
   le manuel, PJ.5). *Décidé le 26 octobre* : avec les mesures de PJ, pas
   avant ; reste donc la machine à pleine largeur. Au même commit, les
   chiffres que la synthèse de PI a gardés en les renvoyant à PJ (#264, #266,
   #267 et #280) sont à remesurer ou à vérifier dans `doc/differences.md` et
   le `ChangeLog` : ceux de #150, #218, #210, #209 et #62 (le bloc « The
   rounding direction and the control registers »), ceux de NEON de #146, qui
   viennent de trois sources à remesurer ensemble (#146, `doc/three-builds.md`
   et `doc/compare/performance.md`), et ceux de l'armhf sans
   `GAOL_PRESERVE_ROUNDING` (`doc/using.md`), que seul le job « Rounding
   direction preserved, armhf » de `containers.yml` mesure. Lancer enfin
   `doc/compare/code/run_lines.sh` en entier sur l'i7-1185G7 : sa colonne
   `gaol5p` (#216) n'a jamais tourné sur x86 (`results_lines.csv` n'a aucune
   ligne `gaol5p`, `results_lines_arm64.csv` en a 30).
3. **34** : les recettes FetchContent récupèrent GAOL 4 : `doc/using.md`, le
   manuel et `tests/fetch_content` prennent la branche `master` de
   `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par
   défaut. `master` est GAOL 4.3.2, avec lequel les programmes de la
   documentation ne compilent pas, et il n'y a pas d'étiquette `v5.0.0`. *Décidé
   le 3 octobre* : une fois `configure-clean` fini, fusionner `configure-clean`
   dans `MATH-CORE` (c'est un ancêtre de `configure-clean` : une avance rapide),
   puis `MATH-CORE` dans `master` (qui a deux commits de plus, `577c850` et
   `406d850`, ceux de `.claude/CLAUDE.md` que `configure-clean` a par `68fe887`
   et `195e4a2` : une vraie fusion, sans différence de contenu), puis étiqueter
   `v5.0.0` ; les recettes et le test gardent `master` jusqu'à l'étiquette, puis
   la citent. *Vérifié le 8 novembre* par l'API de GitHub (lecture seule) :
   `master`, la branche par défaut, n'a pas de protection classique, mais le jeu
   de règles « master verif » (18 octobre, actif, sans contournement, même pour
   le mainteneur) interdit de la supprimer et d'y pousser autre chose qu'une
   avance rapide, et exige une pull request (0 approbation, fusion, squash et
   rebase permis) ; aucun contrôle d'état n'est exigé, donc aucun job de macOS
   14 ; rien ne vise `MATH-CORE`, `configure-clean` ni les étiquettes (les
   quatre `v4.*` existent). La fusion de `MATH-CORE` dans `master` passe donc
   par une pull request, non par un push direct. Après v5.0.0, le développement
   continue dans `configure-clean`, et `master` ne reçoit que les versions. Les
   commits à ligne de crédit (123) restent (*décidé le 19 octobre*). La licence
   est dite (LGPL pour GAOL, MIT pour CORE-MATH) dans la section « Licences » de
   `README.md` et dans le manuel : fait. Pour #281, les cas de
   `tests/test_ibex_codac.cpp` viennent de tests d'IBEX et de Codac sous LGPL v3
   (leur `COPYING.LESSER`), alors que le `README.md` dit GAOL sous LGPL v2
   (`COPYING.LIB`) ; l'en-tête du fichier donne les copyrights et les deux
   licences sans conclure (*décidé le 2 novembre* : l'en-tête seul). *Décidé le
   5 novembre* : le mainteneur écrit aux auteurs d'IBEX et de Codac (Gilles
   Chabert, Simon Rohou) pour leur accord, et l'en-tête seul reste en attendant.
   *À décider* à leur réponse : une ligne dans la section « Licences » du
   `README.md`, voire un `COPYING.LESSER`.
4. **71 et #128** : annoncer GAOL v5.0.0 à Frédéric Goualard, l'auteur de GAOL,
   une fois l'étiquette posée : pas d'annonce publique, et pas de GitHub
   Release, l'étiquette suffisant (*décidé le 3 octobre*). Le message est écrit,
   en brouillon dans Gmail (objet « GAOL v5.0.0 ») ; il suppose que l'étiquette
   existe. À faire avant l'envoi : vérifier l'adresse (celle du brouillon vient
   d'un échange de 2014 et d'une liste de 2020 : elle peut avoir changé) ; le
   second paragraphe (les défauts de GAOL 4) dépasse l'annonce décidée le 3
   octobre, mais *décidé le 24 octobre* : le garder tel quel.
5. **Le manuel** (l'ancien point 4 de PI) : *fait par #294* (fusionnée le 7
   novembre). Le manuel est réécrit dans la mise en page et le style de la
   thèse du mainteneur (`manual/v5/manual_gaolv5.tex` et un fichier par chapitre
   dans `manual/v5/chapters/`, à la place de `gaol.tex`), relu deux fois par des
   relecteurs indépendants, et son PDF, `manual/Manual_GAOLv5.pdf` (385 pages, 0
   erreur de TeX, les 7 « Overfull \hbox » de moins de 4,1 pt qui restent), est
   commité : le mainteneur a supprimé l'ancien `manual/v5/gaol.pdf` (154
   pages) et `manual/v4/gaol.pdf`, et la cible `pdf` (`-DWITH_DOC=ON`,
   `make -C manual pdf` ou `meson compile pdf`) écrit le PDF dans `manual/`.
   *Décidé le 7 novembre* : la pull request du manuel (une branche `manual*`) le
   régénère dans son dernier commit, `check_branch` le lui permet et
   `process.md` (§6.2 point 7, §7 point 5) le dit ; les autres branches ne le
   commitent jamais. La date
   d'édition (`GAOL_V5_UPDATED` de `configure.ac`, de `configure` et de
   `manual/meson.build`) reste le 6 novembre 2026 (*décidé le 7 novembre*). Le
   PDF est à jour avec sa source : à refaire seulement si les mesures de ce
   point changent le manuel (dans la pull request qui les change, comme #294),
   la date d'édition étant alors à remettre à jour. *Décidé le 3 octobre*, pour
   les points 7 et 10 : un seul `\newinvfive` dans l'entrée `hausdorff` du
   manuel, et aucun sur la phrase du domaine de `atanh`.
6. **La CI de macOS 27** (#276, fusionnée le 30 octobre) : le job `macOS 27
   arm64` remplace `macOS 14` sur l'étiquette `xcode-27` (le mainteneur,
   `57650a4` du 1er novembre) et passe le 2 novembre ; celui de x86_64 sous
   Rosetta est retiré (« not available now », l'étiquette `macos-27` n'a pas
   d'exécuteur). Les textes qui citent macOS 14 comme testé
   (`manual/v5/chapters/02_installation.tex` ligne 473, `doc/three-builds.md`
   ligne 76, `CMakeLists.txt` ligne 512, `gaol/math/gaol_core_math_port.h`
   ligne 453, `configure.ac` et `meson.build`) sont à revoir avec la version si
   macOS 14 ne doit plus y figurer ; `doc/continuous-integration.md` ligne 54
   (« macOS: 14, 15 and 26 », alors que `macos.yml` a 15, 26 et 27) est à
   corriger de même.
7. **Les restes de la synthèse de #283 à #288, et le texte de #293** : la pull
   request #289 a fait la synthèse (`doc/differences.md`, `ChangeLog`,
   `todo-notes/synthese.md`, fusionnée le 5 novembre) ; #290 a écrit son texte
   dans sa propre pull request. Le texte « Pour `doc/differences.md` et
   `ChangeLog` » de #293 (`d * x`, fusionnée le 5 novembre) n'est pas reporté :
   `d * x` (un double fois un intervalle) est calculé comme `x * d`, et `n * x`
   d'un entier comme `x * n`, les mêmes bornes, bit pour bit, le signe d'une
   borne nulle compris, avec toutes les sortes d'intervalles ; GAOL 4 calculait
   le produit des intervalles `interval(d)` et `x`, le même ensemble avec
   d'autres signes de zéro (`[-0, +0]` au lieu de `[+0, -0]` pour x = `[+0, -0]`
   et un d positif, avec les intervalles SSE2) ; dans `doc/differences.md`, la
   phrase de l'exception du chemin AVX-512 au « bit pour bit » (ligne
   834, « `x * d` and `x *= d` ») y ajoute `d * x`. Restent aussi trois
   détails : le commentaire de `CMakeLists.txt` (ligne 1441) cite
   `core_math_log_dint.h`, qui n'existe pas ; la liste de `doc/differences.md`
   (lignes 2387 et 2388) des fichiers que l'archive des sources laisse de côté
   ne nomme pas `STEP.md`, que `CMakeLists.txt` exclut depuis #291 ; dans
   `todo-notes/synthese.md`, les notes « Présence : absents » et les renvois «
   change 6 » et « change 7 » de `3rd/README.md` (lignes 447 et 1073, périmés
   depuis #111) sont périmés, et des points y sont faits sans que le texte le
   dise (`run_examples.py` devenu `manual/check_examples.py`, le commentaire
   hexadécimal de la ligne 1233, le `#include <cmath>` de la ligne 1278, la
   remarque de la ligne 1611). Trois textes manquent aussi à
   `doc/differences.md` et au `ChangeLog` (relecture du 4 novembre) : celui de
   #279, dont la pull request n'a pas de section « Pour `doc/differences.md` et
   `ChangeLog` » (`GAOL_RELATION_BOUNDS_FIRST`, `doc/building.md` ligne 211, et
   les relations qui testent les bornes avant les ensembles vides, alors que
   les deux documents ont les réglages voisins) ; celui de #281, dont l'entrée
   proposée pour le `ChangeLog` (`tests/test_ibex_codac.cpp`,
   `tests/tools/ibex_codac/`, 3 905 vérifications) n'y est pas ; et celui de
   #85 : `auto f = &gaol_ieee1788::rootn;` ne compile plus (les surcharges pour
   tout type entier), `pown` non plus depuis #84, et `doc/differences.md`
   (ligne 2065) ne le dit que pour `textToInterval` et `exactToInterval`.
   *Décidé le 8 novembre* : quatre lignes de plus au `ChangeLog`, une pour
   chaque pull request limitée à la CI, #34, #35, #48 et #74 (leurs textes sont
   dans `todo-notes/synthese.md`, l'article `29.md` pour #35) ; le `ChangeLog`
   décrit la bibliothèque, non les corrections des pages de `doc/` (la seconde
   puce de #140 n'y entre donc pas : `TODO_DONE.md` 3.24). [S]

## PP. Le FIXME de `div_rel` : le cas N1 M en SSE2 [M]

Demandée par le mainteneur le 8 novembre (réponse à la relecture du 4 novembre,
`TODO_DONE.md` 3.24 : « créer une tâche PP pour faire le FIXME: replace with SSE
code », et laisser les deux marques pour le moment). Dans `div_rel()` de
`gaol/core/gaol_interval_sse.cpp` (le `mulRev` d'IEEE 1788-2015 : le quotient de
K par J, coupé par I), le cas de K strictement négatif (N1) et de J qui contient
0 dans son intérieur (M) est le seul du fichier en code scalaire, à la ligne 191
(« FIXME: replace with SSE code ») : deux intervalles, `tmp(-oo, K.right() /
J.right())` et `tmp2(-((-K.right()) / J.left()), +oo)`, puis `(I & tmp) | (I &
tmp2)`. Les autres cas sont déjà en SSE2 (`_mm_div_sd()` et `_mm_div_pd()`) ;
NEON et le FPU appellent `div_rel_upward()` de `gaol_interval_core.cpp` et
n'ont pas ce cas.

À tenir : les mêmes bornes, bit pour bit, que le code scalaire, la borne
inférieure de `tmp2` étant arrondie vers le bas alors que le sens est vers le
haut (le commentaire du code) ; aucune exception de plus (le commentaire du
point D.24, plus haut : `_mm_div_sd()` divise la moitié basse seule, car la
moitié jetée par `_mm_div_pd()` divisait une borne infinie par une infinie ou
une finie par zéro) ; les sens d'arrondi et `GAOL_PRESERVE_ROUNDING`, les grands
et les petits opérandes étant déjà pris par `div_rel_upward()` plus haut. Les
tests qui appellent `div_rel` sont dans `tests/reverse.cpp`,
`reverse_mappings.cpp`, `rounding_direction.cpp`, `other_functions.cpp`,
`misc.cpp`, `elementary.cpp` et `test_ibex_codac.cpp` : le nouveau code a un
test qui échoue sur un mutant de chaque opérande (`process.md` §5). Aucune ligne
du banc (`tests/performance.cpp`) n'a `div_rel` : mesurer d'abord si le cas vaut
le code, par un petit programme à part, comme pour la racine cubique
(`TODO_DONE.md` 7.20). La marque « FIXME » de la ligne 191 se retire avec le
code ; l'autre (« TODO: check existence of stdint.h in configure.in », ligne 25)
reste (`TODO_DONE.md` 3.24).

## PQ. Deux petits changements décidés le 8 novembre [S]

Demandée par le mainteneur le 8 novembre (réponses à la relecture du 4
novembre : `TODO_DONE.md` 3.24). Deux points indépendants, chacun sa branche et
sa pull request : #298 (le point 1, ouverte le 8 novembre, avec `centeredForm`
en fonction libre, *confirmé le 8 novembre*) et #299 (le point 2 : le test passe
avec libstdc++, qui ignore la précision de `hexfloat`, donc rien à corriger
ici ; *décidé le 8 novembre* : si la CI le fait échouer sous une autre
bibliothèque, GAOL écrit lui-même le hexfloat arrondi vers l'extérieur, dans la
même pull request).

1. **`centered_form` à la place de `midrad`** (la remarque de #271), **avant
   l'étiquette de PJ.3** : après v5.0.0, un renommage casserait des programmes.
   Garder `midRad(x, m, r)` (`gaol_ieee1788`) et `mid_rad(m, r)` (`gaol`) ;
   renommer le constructeur `interval::midrad(m, r)`
   (`gaol/core/gaol_interval_core.h`, lignes 875 et 2752) en
   `interval::centered_form(m, r)` dans `gaol`, et le donner à `gaol_ieee1788`
   sous le nom `centeredForm(m, r)` : une fonction libre rendant un `interval`,
   comme les autres de ce fichier, qui n'a aujourd'hui aucun constructeur de ce
   genre (à confirmer au début de la tâche, la réponse du mainteneur disant «
   interval::centeredForm(m,r) dans gaol_ieee1788 »). `midrad` n'existe pas dans
   GAOL 4 (`master`) : aucun alias à garder. 42 lignes de 12 fichiers le citent
   : `gaol_interval_core.h` (2), `tests/other_functions.cpp` (13), `ChangeLog`
   (1), `doc/differences.md` (2, dont la puce qui suit celle de `mid_rad()`),
   `doc/tests.md` (1), `doc/using.md` (3), `examples/examples.md` (2),
   `examples/tutorial.md` (1) et les chapitres 3, 5, 14 et 15 du manuel (17).
   Le test de `centeredForm` rend les mêmes bornes que `centered_form`. Le PDF
   du manuel sera plus vieux que sa source : une pull request `manual*` le
   refait avant la version (PJ.5).

2. **Le test de `<a, a>` avec `hexfloat`** (#43, question 8) : la règle qui
   écrit `[a]` pour un intervalle réduit à un point (`display_bounds()`,
   `gaol/core/gaol_interval_core.cpp`, ligne 2033) suppose que `bound_to_text()`
   (ligne 1990) arrondit exactement vers l'extérieur, ce que la bibliothèque C
   ne promet pas avec l'option `hexfloat` ; le seul test (`tests/numbers.cpp`,
   lignes 1521 à 1538) porte sur le format centre. Un test d'une ligne : écrire
   `interval(a)` en `hexfloat` pour quelques `a` (dont des valeurs qui ne
   tiennent pas en peu de chiffres hexadécimaux), relire le texte, et vérifier
   qu'il contient `a`. S'il échoue, c'est un défaut à corriger dans la même pull
   request, ou à décider avec le mainteneur.

## PS. Les textes faux ou périmés du code et de la documentation [S]

Relevés par la relecture du 4 novembre (`TODO_DONE.md` 3.19), revérifiés dans
l'arbre le 8 novembre (les numéros de ligne sont ceux de ce jour) ; le
mainteneur en a fait une tâche le 8 novembre (`TODO_DONE.md` 3.25). Aucun ne
change le code compilé ni une borne. Une seule pull request suffit
(`todo-ps-stale-texts`), avant l'étiquette de PJ.3 ; au début, relire chaque
article contre l'arbre, car un autre travail a pu le régler.

1. **Les en-têtes des intervalles SSE2 et FPU** (#234) :
   `gaol/core/gaol_interval_sse.h` et `gaol/core/gaol_interval_fpu.h` disent
   (ligne 21, et ligne 28 dans le message du `#error`) qu'ils « shall be
   included by gaol_interval_core.cpp only » ; c'est
   `gaol/core/gaol_interval_core.h` qui les inclut (lignes 1134 à 1138). Le
   texte de NEON est juste. Remarqué par la relecture de #234, non repris.
2. **Le commentaire de tête de `gaol_interval_core.cpp`** (#219) : il cite
   encore « the integer powers with NEON » parmi ce qui pourrait gagner (ligne
   34), alors que `doc/simd.md` les dit faites, et #219 en a trouvé la cause. La
   phrase périmée de `doc/simd.md` (« perhaps… (not checked) ») et celle du
   chapitre 12 du manuel sont corrigées par #295 (*décidé le 8 novembre* :
   maintenant, avec le PDF du manuel refait) ; le paragraphe du quotient NEON
   est déjà réécrit par #294. Ce commentaire est du code, et le toucher lance
   `performance.yml` : il reste dans la pull request de PS.
3. **`doc/using.md`, ligne 1574** (#210) : le titre « The control registers of
   32-bit x86 processors » couvre aussi x86-64 (ligne 1651 : « An x86-64
   processor has the two units too ») : « of x86 processors » ; une remarque de
   la relecture de #210, non reprise. Voir si le manuel a le même titre.
4. **`doc/tests.md`, lignes 199 à 204** (#117, question 6) : la puce
   d'`elementary` décrit des tests de `exp2`, `exp10`, `log2`, `log10` et de
   `nth_root(I, q)` pour q < 0 qui sont dans `tests/core_math.cpp`, non dans
   `tests/elementary.cpp` : la mettre sous `core_math` (ligne 964).
5. **Deux renvois faux** : `CMakeLists.txt`, ligne 1217, cite « point PG.105 of
   TODO.md », qui n'a jamais existé (c'est PG.1, le 105 (AH), non adopté :
   `TODO_AFTER.md` 3.6) ; `tests/ieee1788.cpp`, ligne 1431, cite « (TODO.md,
   point 7) », un numéro d'avant le 3 octobre (le 7 est aujourd'hui `atanh`),
   dans le commentaire d'un test à plusieurs threads mis en commentaire
   (`TODO_AFTER.md` 5.4) : retirer le renvoi, la phrase se suffit.
6. **Le délai de `numbers` sous macOS** (`dc036c6`, 4 octobre) :
   `tests/CMakeLists.txt` (lignes 134 à 138) dit que le job macOS x86_64 avec
   ASan et UBSan donne 400 s à `numbers`, et `.github/workflows/macos.yml`
   (lignes 424 et 425) lui en donne 600 depuis ce commit direct (`process.md`
   §5 le dit) : corriger le commentaire.
7. **L'exemple 15 construit sans exceptions** (#97, *décidé le 6 octobre*) : la
   phrase d'`examples/examples.md` sur l'exemple 15 construit sans exceptions,
   renvoyée « à la pull request de synthèse », manque (lignes 304 à 312) ; elle
   n'est que dans l'en-tête de `examples/15_text_and_ieee1788.cpp` (lignes 12 à
   18).
8. **L'archive des sources** : `TODO_fin.md` étant supprimé, la liste
   `CPACK_SOURCE_IGNORE_FILES` (`CMakeLists.txt`, ligne 1580), son commentaire
   (ligne 1560) et la phrase de `doc/building.md` (ligne 267) ne le nomment
   plus ; `STEP.md` reste exclu, comme `process.md` et `todo-notes/`. Vérifier
   que `package_source` donne les mêmes fichiers (775 au dernier relevé, 3.20)
   ; la liste de `doc/differences.md` est dans PJ.7.

## PT. Les relevés et les vérifications jamais faits [S]

Deux mesures ou vérifications que des pull requests ont demandées sans que leur
résultat soit noté nulle part (relecture du 4 novembre, `TODO_DONE.md` 3.19 ;
tâche demandée le 8 novembre, 3.25). Une troisième, le micro-banc de `x op= d`
de #250, est faite : la session MACarm4 l'avait mesuré le 30 octobre, et son
résultat est dans l'entrée de #250 (`TODO_DONE.md` 2.3). Chacune est
indépendante. Aucune ne change le code, sauf si la première trouve des
avertissements à corriger.

1. **Les avertissements de meson dans `tests/rounding_direction.cpp`.** Le
   build meson y donnait 6 avertissements (`-Wconversion` et
   `-Wunused-function`), antérieurs à la branche de la revue 40
   (`todo-notes/synthese.md`, lignes 1493 à 1497) : `meson.build` (lignes 335 à
   340) ajoute `-Wconversion` à tout le projet, tests compris, alors que
   `CMakeLists.txt` (ligne 1203) ne le met qu'aux sources de la bibliothèque et
   que les jobs `-Werror` passent par CMake avec `-Wall -Wextra`. Non revérifié
   depuis, aucun job de meson ne compilant en `-Werror`. Construire avec meson
   (GCC et Clang, tests et exemples) et lire les avertissements de la
   bibliothèque, de CORE-MATH, des tests et des exemples : les corriger dans
   une pull request, ou les décider avec le mainteneur ; s'il n'y en a aucun, le
   dire dans `TODO_DONE.md`.
2. **Le banc A/B de #219 sur le Neoverse N2 et avec GCC**, « à lire dans le
   résumé de ses jobs » : `TODO_DONE.md` 2.3 (#219) ne donne que le M2. Relire
   le résumé des jobs `performance.yml` de la pull request (GitHub garde les
   journaux 90 jours par défaut, soit jusqu'à la fin de janvier 2027), noter les
   rapports du N2 dans l'entrée de #219 ; si la CI ne les a plus, écrire qu'ils
   sont perdus, la mesure de PJ.2 les refaisant avec la version.
