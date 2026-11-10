# À faire

Ce qui reste à faire sur GAOL v5 au commit `99229cc` de `configure-clean` (la
fusion de #306, hors des points, le 10 novembre 2026 à 17 h 55 UTC, après
celles de #305, hors des points, de #302, la tâche PP, de #304 et de #303, les
registres de la tâche PT, de #301, la tâche PT, de #300, hors des points, de
#298, de #299, de #297, de #296 et de #295, de #294, le manuel dans le style de
la thèse, et de #292, de #293, #291, #290 et #289), en tâches PH et PJ (PA à
PG, PI, PL, PP, PQ, PS et PT sont faites, PK et PM sont close sans code : voir
`TODO_DONE.md`). Ce fichier et l'issue #179 ont le même
contenu. Ce qui est fait et décidé est dans `TODO_DONE.md` (l'issue #177 n'est
plus mise à jour), ce qui est reporté dans `TODO_AFTER.md` (issue #178). Les
anciennes issues #49 et #142 sont fermées, et `TODO_mistral.md` est supprimé,
comme `TODO_fin.md` : ses articles sont répartis dans les trois registres, dont
les tâches PS (faite par #297), PT (faite par #301 : `TODO_DONE.md` 3.25) et PP
(faite par #302 : `TODO_DONE.md` 2.3 et 7.25).

Aucune borne fausse n'est connue, sauf peut-être pour un programme qui met
lui-même `FPCR.AH` sur arm64 (issue #198 : `x / y` des intervalles NEON, cause
non trouvée). Rien n'est à envoyer à un autre projet ni à publier sans l'accord
du mainteneur. Une seule pull request est ouverte : #307 (hors des points : les
en-têtes des fichiers, un même modèle partout, sans adresses électroniques ;
ouverte le 10 novembre à 17 h 52 UTC). #295 (PS.2), #296 (PM), #297 (la tâche
PS), #299 (PQ.2), #298 (PQ.1), #300 (hors des points : la recette de compilation
croisée GCC 12.2 pour aarch64 de `todo-notes/gcc12-aarch64/`) et #301 (la tâche
PT) sont fusionnées le 8 novembre, à 17 h 21, 17 h 29, 19 h 38, 20 h 31,
20 h 44, 21 h 38 et 23 h 09 UTC, et #302 (la tâche PP : le cas N1 M de
`div_rel()` en SSE2) le 9 novembre à 22 h 03 UTC, #305 (hors des points : six
`friend` de `interval` retirés) le 10 novembre à 8 h 52 UTC et #306 (hors des
points : `gaol_interval_core.h` allégé, ses aides dans `gaol_interval_detail.h`)
à 17 h 55 UTC ; leurs registres sont faits.

## Comment lire ce fichier

- Une tâche (PH, PJ) regroupe des points qui touchent le
  même code, les mêmes fichiers ou les mêmes builds de validation, pour les
  traiter dans la même session ; chaque point reste une pull request à part
  (process.md : un point, une branche), sauf quand la tâche dit qu'une seule
  suffit. Les tâches sont dans l'ordre conseillé. Critères : la sûreté des
  bornes d'abord ; ce qui coûte plus cher en attendant (une décision qui
  bloque) ; ce qui tue un programme ou rompt une promesse écrite avant ce qui ne
  fait que ralentir ; le petit et sans dépendance avant le gros ; la version
  (PJ) en dernier. La lettre PR n'est pas une tâche : le sigle dit déjà « pull
  request ».
- PH ne dépend d'aucune autre tâche, sauf quand le texte le dit (PJ suit PI,
  PS, PQ, PP, PT et les études de PG, qui sont faites).
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
   ligne `gaol5p`, `results_lines_arm64.csv` en a 30). Le cas N1 M de
   `div_rel()` en SSE2 (#302) n'a aucune ligne du banc : sa mesure (avec Clang
   18, de 12,97 ns avant à 11,01 après ; avec GCC 13.3, de 11,76 à 11,59 ; sur
   une machine virtuelle) a été refaite sur l'i7-1185G7 le 9 novembre par un
   petit programme à part (`TODO_DONE.md` 7.25) : aucun gain mesurable (la
   branche contre la base, de -3,9 % avec Clang 18 à +2,5 % avec Clang 21, dans
   le bruit), et les 15 % de Clang 18 ne se reproduisent pas. *À décider* : le
   cas P1 M de
   `div_rel()` (`gaol/core/gaol_interval_sse.cpp`) calcule deux `_mm_div_sd` de
   même numérateur, que l'on pourrait ramener à une division vectorielle comme
   #302 l'a fait pour N1 M ; non mesuré, non demandé.
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
   834, « `x * d` et `x *= d` ») y ajoute `d * x`. Restent aussi trois
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
   Le texte « Pour `doc/differences.md` et `ChangeLog` » de #306 (fusionnée le
   10 novembre) n'est pas reporté non plus : `doc/differences.md`, lignes 1966
   et 1967, dit que les trois builds installent quatre en-têtes dans
   `include/gaol/core/` (`gaol_interval_core.h`, `gaol_interval_fpu.h`,
   `gaol_interval_neon.h` et `gaol_interval_sse.h`) : ils en installent cinq,
   avec `gaol_interval_detail.h` (les aides de `gaol_detail` que
   `gaol_interval_core.h` inclut ; il compile seul), et le reste de ce
   paragraphe est exact ; au `ChangeLog`, `gaol/core/gaol_interval_detail.h`,
   installé, reçoit de `gaol_interval_core.h` les aides de `gaol_detail` qui ne
   lisent aucun `interval`, et `x + y`, `x - y`, `x * y` et `x / y` sont définis
   dans le fragment de chaque type d'intervalles, sans changement pour
   l'utilisateur de l'interface.
   *Décidé le 8 novembre* : quatre lignes de plus au `ChangeLog`, une pour
   chaque pull request limitée à la CI, #34, #35, #48 et #74 (leurs textes sont
   dans `todo-notes/synthese.md`, l'article `29.md` pour #35) ; le `ChangeLog`
   décrit la bibliothèque, non les corrections des pages de `doc/` (la seconde
   puce de #140 n'y entre donc pas : `TODO_DONE.md` 3.24). [S]
