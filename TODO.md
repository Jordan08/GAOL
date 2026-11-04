# À faire

Ce qui reste à faire sur GAOL v5 au commit `221c8d4` de `configure-clean` (la
fusion de la branche `std-exceptions`, après #285), le 4 novembre 2026, en
tâches PH, PJ et PK (PA à PG, PI et PL sont faites : voir `TODO_DONE.md`). Ce
fichier et l'issue #179 ont le même contenu. Ce qui est fait et décidé est dans
`TODO_DONE.md` (l'issue #177 n'est plus mise à jour), ce qui est reporté dans
`TODO_AFTER.md` (issue #178). Les anciennes issues #49 et #142 sont fermées, et
`TODO_mistral.md` est supprimé.

Aucune borne fausse n'est connue, sauf peut-être pour un programme qui met
lui-même `FPCR.AH` sur arm64 (issue #198 : `x / y` des intervalles NEON, cause
non trouvée). Rien n'est à envoyer à un autre projet ni à publier sans l'accord
du mainteneur. La pull request #287 (`pown-rel`, hors des points : `pownRev`
pour p ≤ 0 par `nth_root_rel`) est ouverte.

## Comment lire ce fichier

- Une tâche (PH, PJ, PK) regroupe des points qui touchent le même code, les
  mêmes fichiers ou les mêmes builds de validation, pour les traiter dans la
  même session ; chaque point reste une pull request à part (process.md : un
  point, une branche), sauf quand la tâche dit qu'une seule suffit. Les tâches
  sont dans l'ordre conseillé. Critères : la sûreté des bornes d'abord ; ce qui
  coûte plus cher en attendant (une décision qui bloque) ; ce qui tue un
  programme ou rompt une promesse écrite avant ce qui ne fait que ralentir ; le
  petit et sans dépendance avant le gros ; la version (PJ) en dernier.
- PH et PK ne dépendent d'aucune autre tâche, sauf quand le texte le dit (PJ
  suit PI et les études de PG, qui sont faites).
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
| PK | Des `FE_INVALID` avec Clang et le FPU | un point | à décider |

## PH. Les signalements à d'autres projets [attend l'accord du mainteneur]

Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à
refaire avant l'envoi et ses décisions ; les issues « À communiquer… » ne sont
pas touchées par ce ménage. **Rien n'est envoyé**, et rien ne l'est sans
l'accord explicite du mainteneur. Ordre : ce qui est prêt et qui a une valeur
pour GAOL, puis ce qui demande une réplique à écrire, puis ce qui ne demande
probablement rien.

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
   52258 est de la même famille), **#127 mingw-w64** (`fma()` et `round()` de
   msvcrt, `fesetexceptflag()` en 32 bits, le `fma()` x87 de mingw-w64 11),
   **#129 ITF1788** (huit cas dont le résultat attendu n'est pas le plus serré,
   et, depuis le rapport du 4 novembre, deux cas de `textToInterval` où ITF1788
   attend un intervalle pour un littéral `[l, u]` dont l est au-dessus de u, que
   GAOL rend vide selon IEEE 1788-2015, 12.12.7 : *à décider*, les ajouter au
   brouillon de #129 ?), **#130 Boost.Interval** (`rounded_transc_opp` donne la
   borne supérieure pour l'inférieure ; issue seule ou avec une pull request,
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
   **#137 libieeep1788** (probablement rien : les six cas sont dans #129) et
   **#138 la glibc** (probablement rien : relancer avec la glibc 2.44) :
   vérifier, puis fermer.
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
   `linux.yml` le refait à chaque push, mais pas la copie du dépôt. À refaire
   une dernière fois au commit de la version (`-DGAOL_COVERAGE=ON`, cible
   `coverage`, gcovr) et à enregistrer ; *décidé le 15 octobre* : pas avant,
   sauf ce relevé du 4 novembre. Le test `numbers` y dépasse les 300 s de CTest
   quand la machine est chargée (`ctest --timeout` ne les change pas) : le
   relevé du 4 novembre a lancé `ctest -E '^numbers$'`, puis `gaol_test_numbers`
   à la main. Lancer aussi une fois, sous ASan + UBSan, le test `numbers` en
   entier (trop long pour la relecture du 14 octobre : 158 s pour `arithmetic`
   seul ; il a passé en local le 4 novembre, en Debug, 9 tests sur 9) ;
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
   sur le seul i7-1185G7, dont l'AVX-512 est à demi-largeur), sur une machine à
   AVX-512 pleine largeur si le mainteneur en donne l'accès (un Xeon, un Zen 4)
   : `.github/scripts/performance-ab.sh --runs 9 --lines`, quatre builds de la
   même source ; corriger la règle et `doc/simd.md` si le résultat diffère.
   *Décidé le 26 octobre* : avec les mesures de PJ, pas avant.
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
   la citent. Après v5.0.0, le développement continue dans `configure-clean`, et
   `master` ne reçoit que les versions. Les commits à ligne de crédit (123)
   restent (*décidé le 19 octobre*). La licence est dite (LGPL pour GAOL, MIT
   pour CORE-MATH) dans la section « Licences » de `README.md` et dans le manuel
   : fait. *À décider* (#281) : les cas de `tests/test_ibex_codac.cpp` viennent
   de tests d'IBEX et de Codac sous LGPL v3 (leur `COPYING.LESSER`), alors que
   le `README.md` dit GAOL sous LGPL v2 (`COPYING.LIB`) ; l'en-tête du fichier
   donne les copyrights et les deux licences sans conclure (*décidé le 2
   novembre* : l'en-tête seul). Est-ce compatible, faut-il une ligne dans la
   section « Licences » du `README.md`, voire un `COPYING.LESSER` ?
4. **71 et #128** : annoncer GAOL v5.0.0 à Frédéric Goualard, l'auteur de GAOL,
   une fois l'étiquette posée : pas d'annonce publique, et pas de GitHub
   Release, l'étiquette suffisant (*décidé le 3 octobre*). Le message est écrit,
   en brouillon dans Gmail (objet « GAOL v5.0.0 ») ; il suppose que l'étiquette
   existe. À faire avant l'envoi : vérifier l'adresse (celle du brouillon vient
   d'un échange de 2014 et d'une liste de 2020 : elle peut avoir changé) ; le
   second paragraphe (les défauts de GAOL 4) dépasse l'annonce décidée le 3
   octobre, mais *décidé le 24 octobre* : le garder tel quel.
5. **Le manuel** (l'ancien point 4 de PI) : une seule régénération de
   `manual/v5/gaol.pdf`, qu'on ne commite jamais dans une branche de point. Le
   PDF versionné est plus vieux que `gaol.tex` (son dernier commit est c02e15d)
   : il montre encore « GAOL 4.3.2 », « bison 3.5.1 », « one double beyond » et
   l'ancienne section 14.2 (#275). À faire après avoir corrigé dans `gaol.tex`
   les entrées d'index en conflit de « canonical interval » et l'`Overfull
   \hbox` connu (les lignes 511 à 515 de `gaol.tex` le 14 octobre), et en
   vérifiant que l'entrée de `what()` ne laisse pas son en-tête seul en bas de
   page (macro `\defmethod`). *Décidé le 3 octobre*, pour les points 7 et 10 :
   un seul `\newinvfive` dans l'entrée `hausdorff` du manuel, et aucun sur la
   phrase du domaine de `atanh`. Le manuel se compile avec `pdflatex`
   (`manual/build-pdf.sh` : 154 pages, 0 erreur le 30 octobre).
6. **La CI de macOS 27** (#276, fusionnée le 30 octobre) : le job `macOS 27
   arm64` remplace `macOS 14` sur l'étiquette `xcode-27` (le mainteneur,
   `57650a4` du 1er novembre) et passe le 2 novembre ; celui de x86_64 sous
   Rosetta est retiré (« not available now », l'étiquette `macos-27` n'a pas
   d'exécuteur). Les textes qui citent macOS 14 comme testé
   (`manual/v5/gaol.tex` ligne 277, `doc/three-builds.md` ligne 76,
   `CMakeLists.txt` ligne 506, `gaol/math/gaol_core_math_port.h` ligne 453,
   `configure.ac` et `meson.build`) sont à revoir avec la version si macOS 14
   ne doit plus y figurer.
7. **Les restes de #283, de #284 et de #285** (hors des points) : les textes
   « Pour `doc/differences.md` et `ChangeLog` » de ces pull requests ne sont pas
   reportés. Dans `doc/differences.md` : les anciens chemins de
   `gaol_double_op.h`, `gaol_u128.h`, `gaol_ldexp.h`, `gaol_roundeven.h` et
   `gaol_core_math.h` (13 lignes : 49, 84, 93, 242, 251, 309, 361, 1128, 1825,
   1858, 1859, 2400, 2424) et de `core_math_port.h` (122, 166, 1130), qui sont
   dans `gaol/math/` ; « The whole tree of CORE-MATH is vendored » (ligne 1126),
   l'arbre ne gardant que `binary64` ; `gaol_roundeven.h`, qui n'est plus
   installé (ligne 242), et `round_ties_to_even()`, qui n'est plus en ligne ;
   les sept opérateurs de `gaol_ieee1788`. Pour #285 (*décidé le 4 novembre* :
   le texte se prépare après la fusion, avec les registres), les exceptions de
   la bibliothèque standard : les 23 lignes qui nomment `gaol_exception`,
   `input_format_error`, `invalid_action_error` ou `unavailable_feature_error`
   (225, 1002, 1064, 1068, 1071, 1072, 1076, 1078, 1079, 1088, 1390, 1484, 1562,
   1587, 1613, 1632, 1664, 1710, 1727, 1728, 1950, 1980, 1988), dont la 1950
   (`gaol_exceptions.h` ne déclare plus `string` ni `exception`, et n'est plus
   un en-tête), le `what()` par nom de classe (1076 à 1079), le fichier nul
   (1088) et `operator<<`, et la ligne 1735 (`gaol_ERROR` sans exceptions) ; les
   lignes 77 et 78 disent « 87 % » de couverture (94,3 % le 4 novembre). Le
   texte : les classes d'exceptions de GAOL 4 disparaissent ; GAOL lance
   `std::invalid_argument` (une chaîne qui n'est pas un intervalle, un pointeur
   nul) et `std::domain_error` (un argument qu'une fonction ne prend pas :
   `nb_fp_numbers()`, `bisect()`, `pownRev()` pour p ≤ 0, `nth_root(8, 1.5)`
   dans une chaîne) ; un gestionnaire de `gaol_core::input_format_error` attrape
   `std::invalid_argument`, un de `gaol_core::invalid_action_error` attrape
   `std::domain_error`, et il n'y a plus de base commune à GAOL
   (`std::logic_error` attrape les deux), sans alias ; `what()` est
   `fichier:ligne: explication`, le fichier sans ses répertoires, où GAOL 4
   donnait `std::exception`, et `file()`, `line()`, `explanation()` et
   `operator<<` n'existent plus ; `gaol_ERROR(excep, msg)` prend une classe
   dérivée de `std::exception`, ce que le compilateur vérifie avec exceptions
   comme sans ; `gaol_ieee1788::pownRev()` pour p ≤ 0 lance `std::domain_error`,
   et sans exceptions activées écrit l'erreur et abandonne, où il lançait un
   `std::invalid_argument` même alors. Dans `ChangeLog` : une entrée pour ces
   déplacements (les lignes 100, 649, 794 et 1569 nomment les fichiers à leur
   place d'alors) ; pour #285, les 12 lignes qui nomment les anciennes classes
   (322, 785, 841, 868, 911, 937, 974 à 976, 980, 981, 999), dont les entrées
   sur `gaol_exceptions.{h,cpp}` (322, 974 et 980), deviennent historiques, et
   une entrée pour le changement. Restent aussi le commentaire de
   `CMakeLists.txt` (ligne 1435) qui cite `core_math_log_dint.h`, qui n'existe
   pas, et `todo-notes/synthese.md`, qui garde 11 lignes sur les anciennes
   classes (un registre, non touché par #285). *À décider* (#285) : un littéral
   `[l, u]` dont les bornes sont des résultats d'opérations (des quotients, la
   ligne 137 de `libieeep1788_class.itl`) passe où ITF1788 l'attend, parce que
   GAOL ne compare pas exactement ces bornes (commentaire de `gaol_literal()`) ;
   or, selon IEEE 1788-2015, 12.12.7, une implémentation qui ne peut pas décider
   doit rendre un intervalle qui contient l et u et signaler
   `PossiblyUndefinedOperation`, et GAOL ne signale aucune des exceptions du
   standard : le dire dans `doc/differences.md`, ou comparer exactement ?
   `doc/compare/itf1788.md` le dit pour l'instant. [S]

## PK. Des `FE_INVALID` avec Clang et les intervalles FPU [S, à décider]

Trouvé par la relecture de #279 (31 octobre), **déjà dans la base** : avec les
intervalles FPU (`GAOL_SIMD=OFF`, ou les cibles sans SSE2 ni NEON), Clang 18 et
21 à `-O3 -march=native` lèvent encore `FE_INVALID` dans des boucles de
relations (`certainly_le()`, `certainly_leq()`, `certainly_ge()`,
`certainly_geq()`), avant comme après #279 ; avec les intervalles SSE2, la base
en levait avec Clang sur des boucles à bornes constantes
(`x[i].certainly_le(interval(-7.5, c))`), ce que #279 ne fait plus. Le mécanisme
n'est pas étudié (peut-être le même repli qu'avec GCC 14 : une comparaison
silencieuse que l'optimiseur rend signalante). *À décider* : en faire un point
(48 boucles de relations comme pour #279, Clang 18, 20 et 21, `-O3` avec et sans
`-march=native`, les intervalles FPU), ou le reporter dans `TODO_AFTER.md`, le
FPU n'étant le défaut ni de x86-64 ni d'arm64.
