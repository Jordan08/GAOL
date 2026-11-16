# À faire

Ce qui reste à faire sur GAOL v5 au commit `9e36147` de `dev` (la fusion de #379
le 16 novembre à 20 h 39 UTC, la dernière pull request de la tâche PX, après les
registres de la tâche PY, `1307cf9`), et avant au commit `41fa78a` de `dev` (la
fusion de #378 le 16 novembre à 17 h 45 UTC, la dernière pull request de la
tâche PY, après les registres de la tâche PX, `9e257be`, et ceux de PY.12,
`7205e5c`), et avant au commit `a42d016` de `dev` (la fusion de #377 le 16
novembre à 7 h 23 UTC, la dernière des trois pull requests de PY.12, #375 à
#377, fusionnées de 7 h 19 à 7 h 23 UTC par le mainteneur, après celle de #353 à
7 h 16 UTC, la dernière des six pull requests de la tâche PX, #351 à #355 et
#372, fusionnées de 13 h 27 à 22 h 15 UTC le 15 novembre et à 7 h 16 UTC le 16,
et les registres de la tâche PY, `1bfca77`), et avant au commit `1efa1c4` de
`dev` (la fusion de #368 le 15 novembre à 21 h 58 UTC, la dernière des quatorze
pull requests de la tâche PY, #356 à #368 et #370, fusionnées de 13 h 31 à 21 h
58 UTC par le mainteneur, après celle de #352, PX.2, et les registres de la
tâche PW, `40c328a`), et avant au commit `ebf2efd` de `dev` (la fusion de #349
le 14 novembre à 22 h 18 UTC, la dernière des treize pull requests de la tâche
PW, #337 à #349, fusionnées de 21 h 01 à 22 h 18 UTC après les registres de
#336, `8780a1c`), et avant au commit `89cd896` de `dev` (la fusion de #336 le 14
novembre 2026 à 18 h 28 UTC, hors des points : les jobs de la CI regroupés, 207
en 59, et la correction de `x += d` et des opérations voisines avec `-flto`,
après les registres de la tâche PV, `e3acf0a` ; `configure-clean` est à
`83e2f69`, la fusion de #321, sans les notes de travail), et avant au commit
`773d6a1` de `dev` (la fusion de #335 le 14 novembre à 14 h 53 UTC, la tâche PV,
en une seule pull request, après les registres de la tâche PU, `079687e`), et
avant au commit `998e61c` de `dev` (la fusion de #334 le 13 novembre à 22 h 09
UTC, la dernière des douze pull requests de la tâche PU, #323 à #334, fusionnées
depuis 16 h 43 UTC après les registres de ses décisions, `07dca51`, et ceux de
#319, #321 et #322, `03f3783`), et avant au commit `f7046b9` de `dev` (la fusion
de #322 à 10 h 53 UTC, après les registres de la relecture du 13 novembre et la
fusion de #319, `45efbd1`), et avant au commit `9d59e0c` de `dev` et de
`configure-clean` (les décisions de PJ.3, après `0ae1840` et la fusion de #318,
`f006421`, la tâche PJ.7, le 12 novembre 2026 à 19 h 56 UTC, après celle de
#317, la suite de #316, à 17 h 31 UTC, après celle de #316, la tâche PJ.2, à 15
h 30 UTC, après celle de #315, la tâche PJ.1, à 15 h 20 UTC, le commit direct
`7745279`, la fusion de #314, hors des points, à 12 h 42 UTC, celle de #313,
hors des points, à 12 h 31 UTC, le commit direct `7528253`, la fusion de #312,
hors des points, à 7 h 53 UTC, le commit direct `f27c6c2`, celle de #309, hors
des points, le 11 novembre à 22 h 16 UTC, le commit direct `be73e24` et celles
de #311, de #310 et de #308, hors des points, de #307 et de #306, hors des
points, de #305, hors des points, de #302, la tâche PP, de #304 et de #303, les
registres de la tâche PT, de #301, la tâche PT, de #300, hors des points, de
#298, de #299, de #297, de #296 et de #295, de #294, le manuel dans le style de
la thèse, et de #292, de #293, #291, #290 et #289), en tâches PZ (les constats
de la relecture complète du 13 novembre), PH et PJ (PA à PG, PI, PL, PP, PQ, PS,
PT, PU, PV, PW, PX et PY sont faites, PK et PM sont closes sans code : voir
`TODO_DONE.md`). Ce fichier et l'issue #179 ont le même contenu. Ce qui est fait
et décidé est dans `TODO_DONE.md` (l'issue #177 n'est plus mise à jour), ce qui
est reporté dans `TODO_AFTER.md` (issue #178). Les anciennes issues #49 et #142
sont fermées, et `TODO_mistral.md` est supprimé, comme `TODO_fin.md` : ses
articles sont répartis dans les trois registres, dont les tâches PS (faite par
#297), PT (faite par #301 : `TODO_DONE.md` 3.25) et PP (faite par #302 :
`TODO_DONE.md` 2.3 et 7.25).

Les deux défauts bloquants de la relecture du 13 novembre sont corrigés :
`acos_rel()`, `asin_rel()` et `atan_rel()` rendaient parfois le vide alors que
l'intervalle contient des solutions (PU.1, #323), et deux threads qui
affichaient la même expression `-x` plantaient (PU.2, #324) ; une borne est
peut-être fausse encore pour un programme qui met lui-même `FPCR.AH` sur arm64
(issue #198 : `x / y` des intervalles NEON, cause non trouvée). Rien n'est à
envoyer à un autre projet ni à publier sans l'accord du mainteneur. Aucune pull
request n'est ouverte. #379 (le reste de PX.3 : les bornes qui sont des doubles
dans les `TEST_HULL`, avec `TEST_HULL_EXACT_LO`, `TEST_HULL_EXACT_HI` et
`TEST_SEQ`, et les 57 `TEST_EQ` qui restaient, la macro `TEST_EQ` retirée) est
fusionnée le 16 novembre à 20 h 39 UTC, ses registres faits (`TODO_DONE.md` 2.3,
3.46, 4 et 7.45) : la tâche PX est faite. #373 (hors des points : le diagnostic
du contrôle de `pow(x, [2])` de `tests/arithmetic.cpp`, qui a échoué une fois
sous Rosetta sur #358) est dans `dev` par le commit de fusion `cc81735`, que
GitHub a fait le 15 novembre à 22 h 50 UTC sans marquer la pull request
fusionnée ; le mainteneur l'a fermée le 16 novembre à 7 h 15 UTC (`TODO_DONE.md`
2.4). #351 à #355 et #372 (la tâche PX : les tests sans dents, le défaut de GCC
12 sur aarch64 que la CI de #353 a trouvé, et `==` et `!=` entre intervalles)
sont fusionnées, de 13 h 27 à 22 h 15 UTC le 15 novembre et #353 le 16 à 7 h 16
UTC, leurs registres faits (`TODO_DONE.md` 2.3, 3.45, 4 et 7.43), alors que ceux
de #350 (une suite de PW.6, fusionnée le 15 novembre à 7 h 37 UTC) ne le sont
pas. #356 à #368 et #370 (la tâche PY : les textes faux hors du manuel, le code
mort, le nom de `GAOL_ERROR` et les avertissements de la relecture du 13
novembre, une pull request par point, plus #368 et #370 pour les suites de PY.12
et de PY.11) sont fusionnées dans `dev` le 15 novembre, de 13 h 31 à 21 h 58
UTC, par le mainteneur ; #369 (les références des exemples 01, 02 et 04 par
`textToInterval`) est fermée sans fusion à 21 h 09 UTC (« non on ne veut pas de
TextToInterval dans les exemples »). #375 à #378 (la suite de PY.12 : les
conversions inutiles de `gaol/` et de `tests/`, les boucles à compteur flottant
de `gaol_interval_pow.cpp`, puis celles de `tests/`, une pull request par
famille) sont fusionnées le 16 novembre, #375 à #377 de 7 h 19 à 7 h 23 UTC et
#378 à 17 h 45 UTC. Leurs registres sont faits (`TODO_DONE.md` 2.3, 2.4, 3.43,
3.44, 4, 7.41, 7.42 et 7.44), la tâche PY est faite, l'issue #371 est fermée, et
ce qu'elles laissent au manuel, à `doc/differences.md` et au `ChangeLog` est
dans PZ (PZ.2, PZ.5, PZ.6 et PZ.9). #337 à #349 (la tâche PW : les builds et la
CI de la relecture du 13 novembre, une pull request par point, plus #349 pour
les versions Mach-O de la dylib) sont fusionnées dans `dev` le 14 novembre, de
21 h 01 à 22 h 18 UTC, par le mainteneur, avant la fin de leur CI (aucun run ne
s'est terminé en échec) ; leurs registres sont faits (`TODO_DONE.md` 2.3, 3.42,
4 et 7.40), et ce qu'elles laissent au manuel, à `doc/differences.md` et au
`ChangeLog` est dans PZ (PZ.5, PZ.7 et PZ.8). La CI de `dev` à `ebf2efd`
tournait au moment des registres. #336 (hors des points : les 207 jobs de la CI
regroupés en 59, un job de plus pour l'IPO sur le chemin SSE2, et la correction
de `x += d` et des opérations voisines avec `-flto`) est fusionnée dans `dev` le
14 novembre à 18 h 28 UTC par le mainteneur ; ses registres sont faits
(`TODO_DONE.md` 2.3, 3.41 et 7.39), et ce qu'elle laissait aux workflows est
fait par la tâche PW (#339, #340, #345, #347 et #348) ; PZ.2 reste au manuel.
#335 (la tâche PV : le lecteur et les expressions, six points en une seule pull
request sur la demande du mainteneur, et quatre commits qu'il a demandés
ensuite) est fusionnée dans `dev` le 14 novembre à 14 h 53 UTC par le mainteneur
; ses registres sont faits (`TODO_DONE.md` 2.3, 3.40, 4 et 7.38), et ce qu'elle
laisse au manuel, à `doc/differences.md` et au `ChangeLog` est dans PZ (PZ.3 et
PZ.5 à PZ.7). Les douze pull requests de la tâche PU, une par point, sont
fusionnées dans `dev` le 13 novembre par le mainteneur : #323 (PU.1) à 16 h 43
UTC, #324 (PU.2) à 16 h 46, #325 (PU.4) à 17 h 03, #326 (PU.6) à 17 h 09, #327
(PU.7) à 21 h 09, #329 (PU.9) à 21 h 26, #328 (PU.8) à 21 h 29, #331 (PU.5) à 21
h 38, #332 (PU.10) à 21 h 46, #330 (PU.3) à 21 h 54, #333 (PU.11) à 22 h 04 et
#334 (PU.12) à 22 h 09 ; leurs registres sont faits (`TODO_DONE.md` 2.3, 3.39, 4
et 7.37), et ce qu'elles laissent au manuel, à `doc/differences.md` et au
`ChangeLog` est dans PZ (PZ.5 et PZ.7). #319 (les renvois aux registres récrits,
la même archive des sources) et #322 (la CI sur un push vers `dev`) sont
fusionnées le 13 novembre dans `dev`, la branche créée de `configure-clean` le
12 novembre et la base du développement, et #321 dans `configure-clean`, la
branche des versions, qui n'a plus les notes de travail : ce fichier n'est que
dans `dev` (process.md §1 et §13). #295 (PS.2), #296 (PM), #297 (la tâche PS),
#299 (PQ.2), #298 (PQ.1), #300 (hors des points : la recette de compilation
croisée GCC 12.2 pour aarch64 de `todo-notes/gcc12-aarch64/`) et #301 (la tâche
PT) sont fusionnées le 8 novembre, à 17 h 21, 17 h 29, 19 h 38, 20 h 31, 20 h
44, 21 h 38 et 23 h 09 UTC, #302 (la tâche PP : le cas N1 M de `div_rel()` en
SSE2) le 9 novembre à 22 h 03 UTC, #305 (hors des points : six `friend` de
`interval` retirés) le 10 novembre à 8 h 52 UTC, #306 (hors des points :
`gaol_interval_core.h` allégé, ses aides dans `gaol_interval_detail.h`) à 17 h
55 UTC et #307 (hors des points : les en-têtes de tous les fichiers sur un même
modèle) à 21 h 04 UTC, puis, le 11 novembre, #308 (hors des points :
`gaol_interval_core.cpp` découpé par domaine) à 21 h 05 UTC, #310 (du mainteneur
: `.gitignore`) à 21 h 08 UTC, #311 (hors des points : l'en-tête `<cstdint>`,
que #310 avait remplacé par un `<stdint>` qui n'existe pas) à 21 h 36 UTC et
#309 (hors des points : `make dist` dans les trois builds, le workflow Release
des paquets) à 22 h 16 UTC, puis, le 12 novembre, #312 (hors des points :
`MATH-CORE` retirée de la CI, `numbers` à 600 s dans le job macOS 26 arm64 LLVM
Clang Debug ASan+UBSan et à 900 s dans celui de macOS 15 x86_64 GCC,
`compile_commands.json` hors de l'archive de CMake), fusionnée à 7 h 53 UTC par
la session, sur la demande explicite du mainteneur, puis #313 (hors des points :
les textes qui citent macOS 14, PJ.6, et un commentaire de `CMakeLists.txt`,
PJ.7) à 12 h 31 UTC et #314 (hors des points : les textes de #279, #281, #85,
#306, #308, #309 et #312 dans `doc/differences.md` et le `ChangeLog`, trois
passages du manuel avec son PDF, PJ.7) à 12 h 42 UTC, par le mainteneur ; leurs
registres sont faits, puis #315 (PJ.1 : le rapport de couverture, 95,1 % des
lignes de `gaol/`) à 15 h 20 UTC et #316 (PJ.2 : les temps remesurés, le manuel
et son PDF) à 15 h 30 UTC, puis #317 (la suite de #316, PJ.2 : le tableau de
Visual C++ et de clang-cl par une expérience de CI, `x.midpoint()`) à 17 h 31
UTC, par le mainteneur, alors que leur CI attendait des runners de GitHub, puis
#318 (PJ.7 : les restes de la synthèse, quatre entrées de CI au `ChangeLog`) à
19 h 56 UTC, par le mainteneur, alors qu'un job de macOS attendait encore un
runner ; leurs registres sont faits ici (`TODO_DONE.md` 2.3, 3.34, 3.35, 7.31 et
7.32).

## Comment lire ce fichier

- Une tâche (PZ, PH, PJ) regroupe des points qui touchent le même code, les
  mêmes fichiers ou les mêmes builds de validation, pour les traiter dans la
  même session ; chaque point reste une pull request à part (process.md : un
  point, une branche), sauf quand la tâche dit qu'une seule suffit. Les tâches
  sont dans l'ordre conseillé. Critères : la sûreté des bornes d'abord ; ce qui
  coûte plus cher en attendant (une décision qui bloque) ; ce qui tue un
  programme ou rompt une promesse écrite avant ce qui ne fait que ralentir ; le
  petit et sans dépendance avant le gros ; la version (PJ) en dernier. La lettre
  PR n'est pas une tâche : le sigle dit déjà « pull request ».
- PH ne dépend d'aucune autre tâche, sauf quand le texte le dit (PJ suit PI, PS,
  PQ, PP, PT, PU, PV, PW, PX et les études de PG, qui sont faites, et PZ : la
  version en dernier, les corrections faites, *décidé le 19 octobre*). PZ suit
  PU à PY, dont les corrections se reportent dans le manuel et les registres.
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
| PZ | Le manuel, `differences.md` et le `ChangeLog` | PZ.1 à PZ.10 | M |
| PH | Les signalements à d'autres projets | #125 à #138, #175 | accord |
| PJ | La publication de v5.0.0 (`master`, étiquette, annonce) | 34, 71 | S |

## PZ. Le manuel et les registres [M]

Les tâches PU à PZ viennent de la relecture complète du 13 novembre : 31
relecteurs indépendants sur l'archive de `9d59e0c`, après des contrôles
automatiques qui passent tous (builds, fichiers générés, PDF, exemples du
manuel), chaque constat revérifié par la session ; le mainteneur en a fait des
tâches le même jour (`TODO_DONE.md` 3.37), dont la tâche PU, faite par #323 à
#334 (`TODO_DONE.md` 2.3 et 3.39), la tâche PV, faite par #335 en une seule pull
request (`TODO_DONE.md` 2.3 et 3.40), la tâche PW, faite par #337 à #349
(`TODO_DONE.md` 2.3 et 3.42), la tâche PY, faite par #356 à #368, #370 et #375 à
#378 (`TODO_DONE.md` 2.3, 3.43, 3.44 et 4) et la tâche PX, faite par #351 à
#355, #372 et #379 (`TODO_DONE.md` 2.3, 3.45, 3.46 et 4). Le rapport, les
rapports des relecteurs et les programmes de preuve sont hors du dépôt, dans
`/home/jninin/Documents/WORK/DEV/GAOL/GAOL_review_2026-11-13/` (`rapport.md`).
Les numéros de ligne sont ceux de `9d59e0c` : au début d'un point, le relire
contre l'arbre, que PU à PY et #336 ont changé. « Démontré » : un programme ou
un calcul le montre ; « plausible » : raisonné, non montré.

Le manuel dans une pull request `manual*`, qui régénère le PDF dans son dernier
commit (process.md §6.2, point 7) et corrige `doc/differences.md` avec lui ; le
`ChangeLog` par la pull request de synthèse (process.md §7, point 5). Après PU à
PY, dont les corrections se reportent ici ; celles de PU, de PV, de PW, de PX et
de PY, faites, sont aux points 2 à 10.

1. **Les chiffres du chemin AVX-512 et de FMA** (important) :
   `02_installation.tex` (lignes 872 à 874 : « no other processor was
   measured ») et `04_rounding.tex` (lignes 526 à 528, 1629 et 1630 :
   « expected, not measured ») contre le Xeon du chapitre 12 (lignes 254 à 272),
   où le chemin perd plus que sur l'i7 sans l'option (0,73 et 0,69 contre
   0,83) ; `02_installation.tex` (lignes 843 à 846 et 1927) donne avec FMA
   « `cos` 2.5 times, `sin` 1.3 » et « up to 2.5 times », contre le tableau du
   même chapitre (lignes 1377 et 1378 : `cos` 1,44, `sin` 1,39, `log` 1,56,
   `pow(x, 3)` 3,1).
2. **Le chapitre 14** (important) : lignes 1458 à 1461, 1469 et 1470, cinq jobs
   `-Wall -Wextra -Werror` (sept : six configurations de `linux.yml`, les jobs
   `l7`, `l8` et `l9`, et une de `macos.yml`) ; lignes 1455 et 1456, deux jobs
   `GAOL_SSE2_UNALIGNED_STORAGE` (quatre configurations) ; lignes 1270 à 1279,
   `release.yml` manque parmi les exceptions des déclencheurs (et `manual.yml`
   dans
   `doc/continuous-integration.md`) ; lignes 87 et 88, 520 et 521 (45
   vérifications de `static_initialization` : 44) ; lignes 440, 478 et 697 (66,
   96 et 1 969 : 67, 97 et 2 071 depuis #358, PY.3), `15_migration.tex`, ligne
   712 (25 des 45 vérifications) et `11_accuracy.tex`, ligne 878 (1 969 cas,
   dont 1 263 avec points : 2 071 et 1 338).
3. **Le lecteur** : le chapitre 9 (lignes 285 à 288), comme
   `gaol/parser/gaol_parser.h` (lignes 97 à 99) : les seuils des bornes non
   départagées (au-dessous de 2^-1084 en base 16, et de 10^-331 en décimal,
   d'après `gaol/parser/gaol_interval_lexer.lpp`, lignes 276 à 279) ; et, depuis
   PV.6 (#335), un nombre au-delà des doubles (plus de 10^311, moins de 10^-331
   et leurs équivalents en bases 16, 8 et 2) est lu en temps linéaire, quand les
   passages suivants disent le temps quadratique sans exception :
   `doc/differences.md` (lignes 960 et 975), `03_using.tex` (ligne 1248),
   `09_io.tex` (lignes 192 et 765), `15_migration.tex` (lignes 1280 et 1696),
   `doc/using.md` (ligne 1087) le disant déjà avec l'exception. Les phrases
   « le lecteur connaît toutes les fonctions de GAOL » (chapitre 15, lignes 238,
   1378 et 1379, `doc/differences.md`, lignes 1761 et 1765 à 1770) sont vraies
   depuis PV.1, et le lecteur de la norme lit `cancelMinus`, `cancelPlus`,
   `intersection` et `convexHull` : rien n'y est à corriger pour ce point.
4. **Les autres passages du manuel** : `03_using.tex` (lignes 730 et 731 :
   l'exemple de PY.4) ; `07_arithmetic.tex` (lignes 620 et 621) et
   `11_accuracy.tex` (lignes 749 et 750) : les seuils d'`erfc` ;
   `07_arithmetic.tex` (lignes 103 à 105 : `hull` du vide n'est pas vide) ;
   `06_relations.tex` (lignes 157 à 169 : `set_contains(+oo)` est vrai pour `[1,
   +oo]`, quand `isMember` est faux) ; `05_creation.tex` (lignes 54 à 58 : deux
   constructeurs SIMD publics) ; `11_accuracy.tex` (lignes 1032 à 1038 : les
   chiffres d'`operator<<` ; lignes 1146 à 1149 : une borne finie de K) ;
   `02_installation.tex` (ligne 1653 : de 200 à 7000 bits ; lignes 1616 à 1618
   et 1932 : 1,2 à 1,5 fois) ; `01_interval_arithmetic.tex` (lignes 634 à 637 :
   `fabs` et `copysign`) ; `12_computing.tex` (ligne 1735 : GCC a tourné, sur le
   Xeon ; lignes 754 et 990 : PROFIL/BIAS et la racine carrée ; ligne 853 : la
   4.2.2) ; `abstract.tex` (lignes 27 à 29) et `introduction.tex` (lignes 67 à
   69) : « the double below it » est faux quand f(x) est un double ;
   `conclusion.tex` (lignes 39 à 44 : une phrase cassée, et un coût de 0,6 à 2
   fois, non « a few tens of percent ») ; *à décider* : le financement du
   post-doctorat de Frédéric Goualard, l'ERCIM (`copyright.tex`, lignes 30 à 33)
   ou le FNS suisse (`introduction.tex`, lignes 38 à 41), les deux venant du
   manuel de GAOL 4.
5. **`doc/differences.md` et le `ChangeLog`** : `interval::midrad(m, r)`
   (`differences.md`, lignes 1527 et 1531 ; `ChangeLog`, ligne 872) est
   `centered_form` depuis #298 (important) ; d'après le `ChangeLog` (lignes 1166
   à 1171), la cible `pdf` et `manual.yml` construiraient le manuel de GAOL 4 :
   ils ne construisent que celui de v5 (important ; `differences.md`, lignes
   2742 à 2758, le dit) ; `cos([2^60])` est montré un double plus large que ce
   que GAOL donne (`differences.md`, lignes 1390 et 1391 : `…6947p-1`) ;
   `check/non_arithmetic.cpp` et `check/trigonometric.cpp` cités au présent
   (lignes 1318 et 1319, 1475 à 1478 : `tests/`) ; dans le `ChangeLog`,
   `manual/v5/gaol.tex` (lignes 211, 721, 1157 et 1161) et des chemins
   d'avant #308 (`gaol_interval_{sse,fpu,neon,avx512}.cpp`, les fonctions de
   `gaol_interval_core.cpp`) ; des chiffres différents d'un registre à l'autre
   (NEON préservé : 27 ns contre 39 ou 37 ; sept produits FPU : 35,8 et 23,8
   contre 35,0 et 23,3 ; `max` et `min` « 3 to 4 times ») ; les corrections de
   PU à PY, en puces pour ces deux registres : pour PU, les sections « Pour
   `doc/differences.md` et `ChangeLog` » de #323 à #334, et `differences.md`
   (lignes 2344 à 2353 de `998e61c`), qui dit encore qu'aucune macro ne montre
   les options que #330 refuse avec GCC 12 et après. Pour PV, la section « Pour
   `doc/differences.md` et `ChangeLog` » de #335 : les noms de GAOL et de la
   norme que le lecteur lit, les nœuds d'expression qui ne se copient plus,
   `result()` qui lève l'erreur, `gaol_value()`, le texte de repli d'une borne,
   le nombre très long lu en temps linéaire et le test `bad_snprintf`. Pour PW,
   les sections « Pour `doc/differences.md` et `ChangeLog` » de #337 à #349 :
   `make check` qui passe avec `--disable-static` (#338), les drapeaux de la
   bibliothèque seule avec configure et meson (#341), l'audit des trois builds
   et ses cinq jeux d'options (#342), les 300 s de `make check` (#343),
   `-mno-daz-ftz` pour Clang 20 et plus (#344), `stamp-h.in` retiré (#345, que
   `differences.md` et le `ChangeLog` disaient déjà disparu) et les versions
   Mach-O (#349). Pour PY, les sections « Pour `doc/differences.md` et
   `ChangeLog` » de #365, #366 et #367 : `-fno-honor-infinities` seul avec
   Clang, comme `-fno-honor-nans` seul (`differences.md`, ligne 2349, § des
   options refusées) ; `gaol_fpu_msvc.h`, retiré (#366) des listes d'en-têtes
   installés (lignes 1980 et 2669) ; `GAOL_ERROR`, la macro de v5, avec
   `gaol_ERROR`, son alias (lignes 1136, 1145 et 1851) ; et `override` sur les
   fonctions virtuelles de `gaol_expression.h` et de `gaol_expr_eval.h` (#367 :
   une classe de l'utilisateur qui dérive d'`expr_eval` et redéfinit `visit()`
   sans `override` reçoit au plus `-Wsuggest-override`). Pour PX, la section «
   Pour `doc/differences.md` et `ChangeLog` » de #353 : avec GCC 12 pour une
   cible 64 bits ARM (Debian 12, GCC 12.2.0), les trois builds donnent
   `-fno-section-anchors` à GAOL, à CORE-MATH, aux tests, aux exemples et au
   code qui utilise GAOL (`gaol::gaol`, `gaol.pc`), parce que GCC 12.2 écrit
   `denorm_min()` sur 16 octets dans une liste de doubles avec `-frounding-math`
   et lit les constantes qui la suivent 8 octets trop tôt (issue #374) ; #372 a
   déjà corrigé la puce des relations de `differences.md` et la ligne de `gaol/`
   du `ChangeLog` (`==` et `!=`), à garder quand PZ les réécrit. Le `ChangeLog`
   garde
   ses entrées anciennes.
6. **Le PDF** : régénéré dans le dernier commit (l'index de celui de `9d59e0c`
   donne « accuracy, 208 », 207 dans le PDF reconstruit) ; le rapport de
   couverture et les mesures de PJ.2 sont à refaire, PU ayant changé le code de
   `gaol/` (#323, #324 et #326 à #332 ; PJ), et PV aussi (#335 : le lecteur,
   l'écriture d'une borne, l'évaluateur) ; PW n'a changé aucun code compilé de
   `gaol/` (un bloc mort de `gaol_interval_internal.h`, #346) ; PY a retiré
   du code que rien n'appelait (`gaol_core::nearest`, trois blocs `#if 0`,
   `finite_products`, la branche sans `<fenv.h>` : #366), ce qui change le
   chiffre de couverture de PJ.1, sans changer une borne. Le PDF de #335
   (391 pages, régénéré dans cette pull request à la demande du mainteneur, avec
   les noms des chapitres 9 et 10) a le dernier paragraphe du chapitre 10 seul
   sur une page de trois lignes, suivie d'une page blanche : le mainteneur l'a
   laissé ; à resserrer au besoin, avec le PDF de PZ.
7. **Les passages du manuel que laissent les tâches PU et PV** (important ;
   avant le PDF de PZ.6) : `02_installation.tex`, la section « Interprocedural
   optimization is not offered » (lignes 1612 à 1645 : « With GCC, moreover, it
   breaks the bounds », « which the three builds keep away from CORE-MATH
   alone »), comme
   `doc/three-builds.md` par #325 et #333 (GCC 13.3 et 15.2 passent avec la LTO,
   que deux configurations de `linux.yml` construisent, avec le chemin AVX-512
   et avec le chemin SSE2, #336) ; la liste « No macro of the compiler
   shows the following options » (lignes 243 à 268) et `15_migration.tex`
   (lignes 1602 à 1606), comme `doc/three-builds.md` par #330 ; l'arrêt de
   `init()` quand le sens réglé vers le haut n'arrondit pas vers le haut, sur
   Debian armel et sous valgrind, comme `doc/using.md` par #331 ; le
   constructeur `expression(const expr_node&)` `explicit` (#332), dans le
   chapitre des expressions s'il montre la construction d'un nœud, et dans la
   liste des changements de l'interface du chapitre 15. Pour PV (#335), s'ils
   les montrent : les nœuds d'expression qui ne se copient plus et
   `expr_eval::result()` qui lève l'erreur, dans le chapitre des expressions et
   dans la même liste, et les quatre noms de la norme (`cancelMinus`,
   `cancelPlus`, `intersection`, `convexHull`) que
   `gaol_ieee1788::textToInterval` lit, dans le chapitre 10 (fait) et dans cette
   liste.
8. **Les passages du manuel que laisse la tâche PW** (avant le PDF de PZ.6) :
   `02_installation.tex` (lignes 404 à 413 et 1162 à 1164) liste encore
   `-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, la
   visibilité cachée et `-Wall -Wconversion` comme les drapeaux de tous les
   builds, avec `-falign-functions=64` seul pour la bibliothèque, quand
   configure et meson ne les donnent plus qu'à la bibliothèque, comme CMake
   (#341, comme `doc/three-builds.md` et `doc/building.md`) ; la liste des
   compilateurs qui reçoivent `-mno-daz-ftz` (« Clang 20 and 21 », les versions
   essayées) quand `gaol::gaol` le donne à Clang 20 et plus (#344) ; et, s'il
   les décrit, les 300 s de `make check` de configure (#343) et les versions
   Mach-O de la dylib (#349).
9. **Les passages du manuel que laisse la tâche PY** (avant le PDF de PZ.6) :
   `12_computing.tex` (lignes 1581 à 1583) donne 6,3 ns pour `sqrt` en SSE2 et
   3,75 avec le chemin AVX-512 et le sens préservé, quand le tableau du même
   chapitre (lignes 953 et 1523) et `doc/simd.md` (#360) disent 7,35 et 4,36 ;
   `02_installation.tex` (lignes 102 et 584 à 585) dit « CMake 3.14 or later »,
   quand `cmake --install` (lignes 576, 747 et 1057, `03_using.tex`, ligne 361)
   date de CMake 3.15 : la 3.14 installe par `--target install` (#361, comme
   `INSTALL`, `README.md` et `doc/building.md`) ; `02_installation.tex` (lignes
   269 à 272), « `-fno-honor-nans` alone, with Clang » : `-fno-honor-infinities`
   seul, avec Clang, échappe aussi à `__FINITE_MATH_ONLY__` (#365, comme
   `doc/three-builds.md`) ; `03_using.tex` (ligne 157) cite `gaol_fpu_msvc.h`,
   retiré par #366 ; `13_numbers_errors.tex` (lignes 276 et 295) donne la macro
   `gaol_ERROR`, que #366 remplace par `GAOL_ERROR`, `gaol_ERROR` restant un
   alias (aussi dans la liste des changements du chapitre 15) ; et, s'il dit le
   chemin AVX-512 identique à SSE2 bit à bit, la réserve du signe d'un zéro de
   `x * d` (#360, `doc/accuracy.md`).

10. **Les passages du manuel que laisse la tâche PX** (avant le PDF de PZ.6) :
    #372 a corrigé le manuel pour `==` et `!=` (`03_using.tex`,
    `06_relations.tex`, `14_tests_ci.tex` et `15_migration.tex`) et régénéré le
    PDF, mais #353 n'a pas régénéré le sien : le texte de `03_using.tex` sur GCC
    12 pour 64 bits ARM (la remarque `rem:gcc12-denorm` :
    `-fno-section-anchors`, GCC 13.1 et 13.2 non essayés) n'est pas dans le PDF
    de `d9abe7d`, et celui de PZ.6 le reprend ; le tableau des programmes de
    `14_tests_ci.tex`, que #372 a complété de `interval_ieee_basic`, est à
    compter avec le reste du chapitre 14 (PZ.2).

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
   miscompilage de GCC 12.2 sur aarch64, dont #353 a trouvé la cause (avec
   `-frounding-math`, `denorm_min()` écrit sur 16 octets dans une liste de
   doubles, puis les constantes suivantes lues 8 octets trop tôt avec
   `-fsection-anchors` ; un cas minimal de 40 lignes et le texte anglais sont
   dans l'issue #374 ; GCC 11.4, 13.3 et 14.2 sont justes, 13.1 et 13.2 non
   essayés sur aarch64) ; le bogue
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

## PJ. La publication de v5.0.0 (Y) [S, dans cet ordre]

En dernier : elle suppose les corrections faites et les études de PG faites
(*décidé le 19 octobre*). Les points faits sont coupés d'ici et sont dans
`TODO_DONE.md` : le 32 (la couverture, PJ.1) par #315, le 33 (les temps, PJ.2)
par #316 et #317, le manuel (PJ.5) par #294, les textes qui citent macOS 14
(PJ.6) par #313 et les restes de la synthèse (PJ.7) par #313, #314 et #318 (2.3,
3.23, 3.33, 3.34 et 3.35). À refaire au commit de la version, si le code ou les
textes ont changé depuis : le rapport de couverture (`coverage/`, 5 minutes),
les mesures de PJ.2 (25 minutes, avec le Mac M2 sur secteur et un Xeon à
AVX-512 ; `sqrt` y est plus lent de 10 à 17 % qu'au 17 octobre, cause non
isolée) et le PDF du manuel, avec la date d'édition (dans la pull request qui
change les mesures ou les textes, comme #294). *Vérifié le 12 novembre* à
`0ae1840` : rien n'est à refaire, `gaol/` n'ayant changé depuis #315 et #316 que
d'un commentaire (#318) et les sources du manuel pas depuis son PDF
(`TODO_DONE.md` 3.36) ; à revérifier après PU à PZ, qui changent le code et les
textes : PU a changé celui de `gaol/` (PZ.6). *Décidé le 13 novembre* : pas de
fusion dans `master` tant que PU à PZ ne sont pas faites (« il y a encore des
TODO. Donc ne fusionne pas dans master maintenant ») ; la version passe d'abord
de `dev` à `configure-clean`, notes retirées (process.md §13), puis de
`configure-clean` à `master` (PJ.3).

3. **34** : les recettes FetchContent récupèrent GAOL 4 : `doc/using.md`, le
   manuel et `tests/fetch_content` prennent la branche `master` de
   `Jordan08/GAOL`, et le `git clone` de `doc/building.md` la branche par
   défaut. `master` est GAOL 4.3.2, avec lequel les programmes de la
   documentation ne compilent pas, et il n'y a pas d'étiquette `v5.0.0`. *Décidé
   le 12 novembre* (à la place du chemin du 3 octobre, par `MATH-CORE`, que
   le mainteneur a supprimée sur GitHub le 11 novembre) : une fois
   `configure-clean` fini, fusionner `configure-clean` dans `master` (qui a deux
   commits de plus, `577c850` et `406d850`, ceux de `.claude/CLAUDE.md` que
   `configure-clean` a par `68fe887` et `195e4a2` : une vraie fusion, sans
   différence de contenu), puis étiqueter `v5.0.0`. *Décidé le 12 novembre au
   soir* (`TODO_DONE.md` 3.36) : le mainteneur ouvre et fusionne lui-même cette
   pull request, la session ne l'ouvre pas ; une fois la fusion faite, et sur
   son mot seulement, la session pose sur le commit de fusion l'étiquette
   `v5.0.0`, annotée comme `v4.3.2`, qui lance `release.yml` : la GitHub
   Release, avec les notes que génère GitHub et les trois paquets (`.deb`,
   `.pkg` de macOS, `.exe` de Windows) ; aucune date ne change, et rien n'est
   gardé de l'ancien `master` (pas de branche `gaol-4`, les étiquettes `v4.*`
   suffisent). La fusion d'essai (`git merge-tree`, le 12 novembre à `0ae1840`)
   est sans conflit et donne l'arbre de `configure-clean`. *Décidé le 12
   novembre* : les recettes FetchContent (`doc/using.md`, le manuel,
   `tests/fetch_content`) et le `git clone` de `doc/building.md` visent `master`
   tout le temps, avant comme après l'étiquette, pour avoir la dernière version
   de GAOL ; elles ne citent donc jamais `v5.0.0`, et rien n'est à y changer
   à la publication (elles disent déjà `GIT_TAG master`). *Vérifié le 8
   novembre* par l'API de GitHub (lecture seule) : `master`, la branche par
   défaut, n'a pas de protection classique, mais le jeu
   de règles « master verif » (18 octobre, actif, sans contournement, même pour
   le mainteneur) interdit de la supprimer et d'y pousser autre chose qu'une
   avance rapide, et exige une pull request (0 approbation, fusion, squash et
   rebase permis) ; aucun contrôle d'état n'est exigé, donc aucun job de macOS
   14 ; rien ne vise `configure-clean` ni les étiquettes (les quatre `v4.*`
   existent). La fusion de `configure-clean` dans `master` passe donc par une
   pull request, non par un push direct. Après v5.0.0, le développement
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
   une fois l'étiquette posée : pas d'annonce publique (*décidé le 3 octobre*) ;
   la GitHub Release que `release.yml` fait à l'étiquette remplace le « pas de
   GitHub Release » du 3 octobre (*décidé le 12 novembre au soir* :
   `TODO_DONE.md` 3.36). Le message est écrit,
   en brouillon dans Gmail (objet « GAOL v5.0.0 ») ; il suppose que l'étiquette
   existe. À faire avant l'envoi : vérifier l'adresse (celle du brouillon vient
   d'un échange de 2014 et d'une liste de 2020 : elle peut avoir changé) ; le
   second paragraphe (les défauts de GAOL 4) dépasse l'annonce décidée le 3
   octobre, mais *décidé le 24 octobre* : le garder tel quel.
