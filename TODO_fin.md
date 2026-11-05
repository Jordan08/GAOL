# Ce que l'on a pu oublier

Le résultat de la relecture du 4 novembre 2026, au commit `1e0f368` de
`configure-clean` (la tâche PK close sans code) : toutes les pull requests (de
#15 à #290 ; les autres numéros sont des issues) avec leurs commentaires et
leurs revues, les messages des 1964 commits de `configure-clean`, les marques du
code (`TODO`, `FIXME`, les renvois « point X of TODO.md »), les issues ouvertes
et fermées, `todo-notes/synthese.md` et les trois registres. Ce fichier donne ce
qui n'est ni fait, ni décidé, ni dans `TODO.md`, ni dans `TODO_AFTER.md`. Ce que
`TODO_AFTER.md` donnait encore à faire et qui est fait est passé dans
`TODO_DONE.md` (3.19) ; le classement de tout ce qui est fait est dans
`STEP.md`.

**Aucun de ces oublis ne donne une borne fausse.** Ce sont des questions restées
sans réponse, des textes faux ou périmés, des mesures jamais faites, des limites
que rien n'inscrit, et des registres qui se contredisent. Rien n'est à envoyer à
un autre projet sans l'accord du mainteneur.

## Comment lire ce fichier

- Chaque article dit ce qui est resté, d'où il vient (la pull request, le commit
  ou le registre), ce qu'en dit le code au commit `1e0f368` (`fichier:ligne`),
  et une destination proposée : une tâche de `TODO.md` (PH, PJ ou PM), une
  section de `TODO_AFTER.md`, ou une ligne de `TODO_DONE.md` quand c'est fait ou
  décidé sans être noté.
- Les numéros de ligne des registres changent à chaque mise à jour : ils sont
  donnés par section et par citation.
- La section 1 est faite de questions au mainteneur : rien ne s'y fait avant sa
  réponse. Les sections 2 à 6 peuvent se faire sans décision nouvelle, sauf
  quand l'article dit « à décider ». La section 9 n'est qu'un rappel du travail
  en cours.
- *Décidé le 5 novembre* : ce fichier est gardé comme la mémoire de la
  relecture. Ses articles ne deviennent pas des tâches ; un article traité le
  dit (« fait » ou « décidé »), sans sortir du fichier. Il n'a pas d'issue.

## 1. Les questions restées sans réponse

1. **`d * x` (#171, 24 octobre).** « `d * x` a des bits non documentés : à
   documenter, ou à aligner sur `x * d` ? », sans réponse. `d * x` est
   `interval(d) *= I` (`gaol/core/gaol_interval_core.h:2264-2267`) ;
   `doc/tests.md:422` dit que son test le laisse de côté, « its bits being
   undocumented », et `doc/using.md:1270-1273` ne parle que de `x * d`. La
   description de #171 dit que `d * x` diffère aussi entre SSE2 et AVX-512 pour
   d < 0, son relecteur dit le contraire : à vérifier d'abord. Destination : la
   réponse dans `TODO_DONE.md`, puis une phrase de `doc/using.md`, ou une limite
   dans `TODO_AFTER.md` §5. *Décidé le 5 novembre* : aligner `d * x` sur
   `x * d`, les mêmes bits sur tous les chemins, avec un test de ces bits : un
   point à part, qui change le code (`TODO_DONE.md` 3.20). Commencé le 5
   novembre, à sa demande, dans une pull request à part ; fait par #293,
   fusionnée le 5 novembre (`TODO_DONE.md` 2.3).
2. **GCC pour Windows 32 bits sans FMA (#172, 24 octobre).** « Un processeur
   sans FMA ne fait plus tourner la bibliothèque de GCC pour Windows 32 bits :
   le laisser ainsi ? », sans réponse écrite. Le comportement est documenté
   (`doc/building.md:140`, `doc/differences.md:2260`). Destination : une ligne
   dans `TODO_DONE.md` 3.4, comme la question de #205 sur mingw-w64 11 l'a été
   dans `TODO_AFTER.md` 3.5. *Décidé le 5 novembre* : laisser ainsi.
3. **Les tests à plusieurs threads (`0048c79`, 23 septembre).** Ils sont en
   commentaire : `tests/ieee1788.cpp:52-57` et `:1113`,
   `tests/expressions.cpp:132-139`, `:2048` et `:2126`,
   `tests/CMakeLists.txt:300-306` ; `doc/tests.md:892` et `:1239-1241` le
   disent. Mais `doc/using.md:1040-1043` affirme que plusieurs threads écrivent
   des intervalles et lisent des chaînes sans risque (« checked with
   ThreadSanitizer »), ce qu'aucun test ne vérifie plus. Aucun registre n'en
   parle. À décider : rétablir un test sous ThreadSanitizer (`TODO_AFTER.md`
   §1), inscrire la limite (§5), ou retirer le code en commentaire et la phrase.
   *Décidé le 5 novembre* : inscrire la limite ; fait (`TODO_AFTER.md` 5.4).
4. **Le nom décoré de `float64x2_t` (#146).** #146 dit que GCC et Clang décorent
   ce nom autrement, si bien qu'une bibliothèque construite par l'un ne se
   lierait pas avec un programme compilé par l'autre (`TODO_DONE.md` §6, #146) :
   ni décision, ni ligne dans `doc/`. Les noyaux exportés
   `gaol_detail::add_bounds(float64x2_t, float64x2_t)` et les autres
   (`gaol/core/gaol_interval_neon.h:327-331`) sont appelés par les opérateurs en
   ligne. Le constat est à vérifier (les deux compilateurs suivent l'AAPCS64 ;
   rien n'a été compilé pour cette relecture). Destination : selon le résultat,
   une limite dans `doc/using.md` et `TODO_AFTER.md` §5, ou « sans objet » dans
   `TODO_DONE.md` §6. *Décidé le 5 novembre* : le vérifier par `nm` ; fait le
   même jour, avec GCC 13.3 pour aarch64 et Clang 18 : les noyaux exportés
   prennent leurs vecteurs par valeur, et leurs noms sont les mêmes
   (`13__Float64x2_t`) ; une référence `const float64x2_t&` est décorée
   autrement par les deux, mais n'apparaît que dans des fonctions en ligne :
   sans objet (`TODO_DONE.md` 3.20).
5. **Le défaut de GCC 14 trouvé par #279.** GCC 14 (x86-64, AArch64, RISC-V)
   fusionne `quiet_less(r, l) || r == l` en un seul `r <= l` qui signale. GAOL
   le contourne (`gaol/core/gaol_interval_core.h:1264-1272`), mais il n'est ni
   dans l'issue #126 (GCC), ni dans la tâche PH. Destination : PH.2, un défaut
   de plus dans le brouillon de #126 (rien n'est envoyé sans accord). *Décidé le
   5 novembre* : l'ajouter au brouillon de #126 ; fait le même jour, sans rien
   envoyer.
6. **Les règles de protection de `master` (#276).** « À changer dans les règles
   de protection de branche s'il y en a » : `master` est protégée sur GitHub, et
   aucun registre n'en parle. Destination : PJ.3, vérifier avant la fusion de
   `MATH-CORE` dans `master` que ces règles n'exigent pas un contrôle qui
   n'existe plus (les jobs de macOS 14) ; non vérifiable ici, les règles ne se
   lisent pas avec l'accès de cette session. Le mainteneur s'en occupe (réponse
   du 5 novembre).

## 2. Des textes faux ou périmés dans le code et la documentation

Pour la version (PJ), avec les textes de PJ.6 et PJ.7 ; aucun ne change le code
compilé.

1. **Les en-têtes des intervalles SSE2 et FPU (#234).**
   `gaol/core/gaol_interval_sse.h:21` et `:28` (le texte et le message du
   `#error`), de même que `gaol/core/gaol_interval_fpu.h:21` et `:28`, disent
   que le fichier « shall be included by gaol_interval_core.cpp only » ; c'est
   `gaol/core/gaol_interval_core.h:1078` et `:1082` qui les incluent. Le texte
   de NEON est juste. Remarqué par la relecture de #234, non repris.
2. **`doc/simd.md` (#219, #225).** Les lignes 334 à 343 gardent l'hypothèse que
   #219 a réfutée (« The two scalar lanes ran side by side already, perhaps…
   (not checked) »), et les lignes 413 à 426 présentent comme actuel le quotient
   NEON plus lent qu'avec le FPU sous clang-cl, que #225 a corrigé ; le fichier
   n'a pas changé depuis le 26 octobre (`d9b179f`). Le commentaire de tête de
   `gaol/core/gaol_interval_core.cpp` (ligne 34) cite encore les puissances
   entières de NEON parmi ce qui pourrait gagner. Destination : PJ.2, qui touche
   déjà `doc/simd.md`.
3. **`doc/continuous-integration.md:54` (#276).** « macOS: 14, 15 and 26 » ;
   `.github/workflows/macos.yml:48-53` a 15, 26 et 27. Ce fichier manque à la
   liste des textes qui citent macOS 14 de PJ.6.
4. **`doc/three-builds.md:319-320` (#173, #205).** « A Debug build of Cygwin is
   not in the job » : faux depuis #205 (le job « Cygwin GCC x64 Release and
   Debug » de `windows.yml`).
5. **`doc/using.md:1574` (#210).** Le titre « The control registers of 32-bit
   x86 processors » couvre aussi x86-64 (ligne 1613) : une remarque de la
   relecture de #210, non reprise.
6. **`doc/tests.md:199-204` (#117, question 6).** La puce d'`elementary` décrit
   des tests de `exp2`, `exp10`, `log2`, `log10` et de `nth_root(I, q)` pour q <
   0, qui sont dans `tests/core_math.cpp` (lignes 647-659, 1178-1185 et
   1216-1225), pas dans `tests/elementary.cpp`.
7. **Deux renvois faux.** `CMakeLists.txt:1212` cite « point PG.105 of
   TODO.md », qui n'a jamais existé (c'est PG.1, le 105, `TODO_AFTER.md` 3.6) ;
   `tests/ieee1788.cpp:1119` cite « (TODO.md, point 7) », un numéro d'avant le 3
   octobre (le 7 est aujourd'hui `atanh`).
8. **Le délai de `numbers` sous macOS (`dc036c6`, 4 octobre).**
   `tests/CMakeLists.txt:136-137` dit que le job macOS x86_64 avec ASan et UBSan
   donne 400 s à `numbers` ; `.github/workflows/macos.yml:424-425` lui en donne
   600 depuis ce commit direct (`process.md` §5 le dit, `TODO_DONE.md` non).
9. **Deux marques de GAOL 4 dans le code compilé.**
   `gaol/core/gaol_interval_sse.cpp:25` (« TODO: check existence of stdint.h in
   configure.in ») et `:191` (« FIXME: replace with SSE code », dans
   `div_rel`) : à retirer ou à expliquer.
10. **L'exemple 15 sans exceptions (#97, décidé le 6 octobre).** La phrase
    d'`examples/examples.md` sur l'exemple 15 construit sans exceptions,
    renvoyée « à la pull request de synthèse », manque
    (`examples/examples.md:304-312`) ; elle n'est que dans l'en-tête de
    `examples/15_text_and_ieee1788.cpp:14-17`.
11. **Le job MSYS2 garde `ls | grep` (#202).** `TODO_DONE.md` (2.3, #202) dit
    que la boucle remplace `ls | grep` ;
    `.github/workflows/build-systems.yml:628` en garde un. Mineur.
12. **`TODO_fin.md` et `STEP.md` dans l'archive des sources** (cette branche).
    `CPACK_SOURCE_IGNORE_FILES` (`CMakeLists.txt:1574-1575`) n'exclut que
    `TODO.md`, `TODO_AFTER.md`, `TODO_DONE.md`, `process.md` et `todo-notes/`
    (#79) : les deux nouveaux fichiers iraient dans `gaol-<version>.tar.gz`.
    Destination : PJ, les ajouter à la liste, à son commentaire
    (`CMakeLists.txt:1554`), à `doc/building.md:264` et à
    `doc/differences.md:2313`, ou ranger les deux fichiers dans `todo-notes/`.
    *Décidé le 5 novembre* : les exclure ; fait par #291 (`CMakeLists.txt`,
    `doc/building.md`), la ligne de `doc/differences.md` attendant la synthèse.

## 3. Des textes à reporter dans `doc/differences.md` et le `ChangeLog`

Pour la pull request de synthèse (PJ.7, après #289), ou pour #289 elle-même si
elle n'est pas encore fusionnée.

1. **#279 : l'ordre des tests des relations.** La pull request n'a pas de
   section « Pour `doc/differences.md` et `ChangeLog` » ;
   `GAOL_RELATION_BOUNDS_FIRST` (`doc/building.md:208`) et les relations qui
   testent les bornes avant les ensembles vides ne sont dans aucun des deux
   documents, qui ont les réglages voisins.
2. **#281 : les cas d'IBEX et de Codac.** L'entrée proposée pour le `ChangeLog`
   (`tests/test_ibex_codac.cpp`, `tests/tools/ibex_codac/` : 3 905
   vérifications) n'y est pas, ni dans PJ.7, ni dans #289.
3. **#35 : la locale à virgule dans la CI.** La ligne proposée
   (`todo-notes/synthese.md:332`) manque au `ChangeLog`, comme #34, #48 et #74,
   eux aussi limités à la CI. À décider : une ligne, ou une règle écrite dans
   `TODO_DONE.md` (les changements de la seule CI restent hors du `ChangeLog`).
4. **#85 : l'adresse de `rootn` et de `pown`.**
   `auto f = &gaol_ieee1788::rootn;` ne compile plus (les surcharges pour tout
   type entier), `pown` non plus depuis #84 ; `doc/differences.md:2004-2006` ne
   le dit que pour `textToInterval`.
5. **#271 : `midrad` et `mid_rad`.** La puce de `interval::midrad(m, r)` suit
   celle de `mid_rad()` avec un nom presque identique
   (`doc/differences.md:1477-1485`) ; la remarque de #271 n'a pas de réponse.
6. **#140 : le `ChangeLog` abrégé.** `ChangeLog:1041-1046` reprend la première
   puce de #140 en abrégé, et la seconde (les textes que le code contredisait :
   `doc/accuracy.md`, `doc/tests.md`, `doc/building.md`...) n'y est pas ;
   peut-être voulu par la synthèse (à confirmer).

## 4. Des mesures et des vérifications jamais faites

1. **Les chiffres de `doc/differences.md` et du `ChangeLog` « à remesurer par
   PJ ».** #264 (les chiffres de #150, #218, #210, #209 et #62), #266 (ceux de
   NEON de #146), #267 et #280 (armhf sans `GAOL_PRESERVE_ROUNDING`) les
   renvoient à PJ, mais PJ.2 ne cite ni ces deux documents ni ces sources.
   Destination : une phrase de PJ.2.
2. **Le banc A/B de #219 sur le Neoverse N2 et avec GCC**, « à lire dans le
   résumé de ses jobs », n'est noté nulle part (`TODO_DONE.md` 2.3 ne donne que
   le M2).
3. **Le micro-banc de `x op= d` de #250** (x vide, d NaN), demandé à la session
   MACarm4 : son résultat n'est nulle part, et `tests/performance.cpp` n'a pas
   de ligne `x op= d`. Destination : une ligne dans `TODO_DONE.md` 2.3 (#250),
   fait ou non fait.
4. **La phase rapide de `compoundm1` (#288)** ne change que le temps : aucun
   test ne la voit, et `tests/performance.cpp` n'a pas de ligne `compoundm1`.
   Destination : une ligne du banc (PJ.2), ou la limite dans l'entrée de #288.
5. **`round_ties_to_even()` hors ligne depuis #283**
   (`gaol/core/gaol_interval_core.cpp:6686`) : le coût de l'appel n'est mesuré
   par aucun banc. Mineur.
6. **`/GS` de Visual C++ (#230).** Le cookie de sécurité de `/GS` pourrait faire
   le même tort que le canari de GCC sur le chemin rapide ;
   `GAOL_RND_OUT_OF_LINE` est vide pour Visual C++ (`gaol/gaol_fpu.h:138-144`,
   « no measure showed this »), et #230 ne l'a pas étendu « sans mesure ».
   Destination : `TODO_AFTER.md` (à mesurer : le désassemblage d'`add_bounds()`
   de `cl` x64).
7. **Ce que #200 n'a pas vérifié.** Une machine sans FMA ni AVX-512 pour les
   chemins que la configuration décide, et `make coverage` et
   `ninja gaol-coverage` sous macOS et Windows. Destination : `TODO_AFTER.md`
   §4.
8. **`<a, a>` avec `hexfloat` (#43, question 8).** La règle suppose que
   `bound_to_text()` arrondit exactement vers l'extérieur, ce que la
   bibliothèque C ne promet pas avec l'option `hexfloat` ; jamais vérifié.
   Code : `gaol/core/gaol_interval_core.cpp:1894-1896` ; le seul test
   (`tests/numbers.cpp:1521-1538`) porte sur le format centre. Destination : un
   test d'une ligne, ou une limite dans `TODO_AFTER.md` §5.
9. **Les avertissements `-Wconversion` de meson** sur
   `tests/rounding_direction.cpp` (`todo-notes/synthese.md:1482-1486`,
   `meson.build:335-339`) ne sont pas revérifiés : aucun job meson ne compile en
   `-Werror`.
10. **`run_lines.sh` avec `gaol5p` sur x86 (#216)** : lu, non exécuté
    (`results_lines.csv` n'a aucune ligne `gaol5p`) ; PJ.2 ne nomme pas
    `run_lines.sh`.
11. **Deux triplets de mingw-w64 11 (`todo-notes/synthese.md:1339-1346`).** Deux
    triplets où le `fma()` de mingw-w64 11 en x86 se trompe ne sont ni dans le
    brouillon de #127, ni dans `TODO_AFTER.md` 3.5. Destination : PH.2 (#127).

## 5. Des limites connues que rien n'inscrit

Des candidats pour `TODO_AFTER.md` §5 (une limite connue, documentée, sans
décision de la lever), ou pour « sans objet » dans `TODO_DONE.md`.

1. **`move_to_side()` reste quadratique en la précision (#71)** :
   `gaol/core/gaol_interval_core.cpp:1666` fait jusqu'à quatre appels à
   `compare_number_with_double()`.
2. **`pow(y, -1)` sur un état brut (#245).** Il échouait déjà sur la base pour
   la voie stockée `[-0, NaN]` ; noté en fait dans `TODO_DONE.md` 2.3 (#245),
   sans test ni décision (`tests/other_functions.cpp:538` ne teste pas `pow`,
   qui passe par `gaol_pown()`, `gaol/gaol_interval.h:214`). États bruts, hors
   de l'interface.
3. **766 drapeaux d'invalidité avec GCC 12 (#248)**, tous avec un NaN signalant,
   les mêmes dans la bibliothèque de base : non étudiés ; l'outil `maxdiff_neon`
   n'est pas dans le dépôt. États bruts.
4. **Un code mort possible dans `get_rounding()` (#218)** : la branche
   générique, le repli SSE sans `<fenv.h>` (`gaol/gaol_fpu.h:822-826`, et 841,
   856), où ne passe qu'un compilateur x86 autre que GCC, Clang et Visual C++
   (`gaol/gaol_fpu.h:180-186` et `:231-236`).
5. **meson ancien (#95)** : avant meson 0.63, GAOL en sous-projet prend la norme
   C++ du projet parent ; de meson 0.53 à 0.56 avec Visual C++, `enable-debug`
   ne donne pas `/Zi`. `doc/using.md:205-225` et `doc/building.md:204` n'en
   disent rien.
6. **`gaol_pown()` est encore amie d'`interval`** sans en avoir besoin (#117,
   question 5 ; `gaol/core/gaol_interval_core.h:879`).
7. **Une clause d'`atanh()` sans test (#92)** : retirer `J.right() == -1.0`
   (`gaol/core/gaol_interval_core.cpp:5797`) ne fait échouer aucun test ;
   `todo-notes/synthese.md:1191-1196` propose de la joindre à R.7, qui est fait.
8. **Les marques de GAOL 4 dans `gaol/float/`** : 12 `TODO` et `FIXME`
   (`gaol_interval2f.cpp:2815` et `:3014-3093`, `gaol_intervalf.cpp:257`,
   `:336-337` et `:436`) et `tests/interval2f.cpp:184` ; les intervalles de
   floats sont désactivés (`GAOL_FLOAT_INTERVALS`) et dits inachevés
   (`doc/differences.md:2290-2308`).
9. **Le générateur Xcode avec GAOL inclus par `add_subdirectory`** dans un
   projet parent universel (#205) : non vérifié.
10. **Les sections à part de Visual C++ arm64 (#262)** : « si l'un en montre, la
    même technique s'applique » aux autres noyaux
    (`gaol/core/gaol_interval_neon.cpp:134-144`) ; une condition de réouverture
    pour `TODO_AFTER.md` §3.
11. **Le temps du banc A/B (#166)** : « on peut retirer des entrées (les trois
    clang-cl, par exemple) si le temps pèse »
    (`.github/workflows/performance.yml:105-107`) ; une condition pour
    `TODO_AFTER.md` §3.
12. **LTO** : CORE-MATH est construit sans LTO par autotools (`-fno-lto`,
    `configure.ac:737-743`, `3rd/Makefile.am:50`) et par CMake
    (`INTERPROCEDURAL_OPTIMIZATION OFF`, `CMakeLists.txt:1259`), et
    `meson.build` n'en dit rien ; l'audit des trois builds ne compare pas ce
    réglage (#201). Aucun job avec `-flto` : *décidé le 23 octobre* (3.6).
13. **Les macros qui appellent les gabarits (#210)** : aucun test ne voit si
    elles le font, c'est une propriété de vitesse.
14. **`cos(x)` 6 ns plus lent (#100)**, de 55,6 à 61,6 ns, sans enquête ; sans
    doute le placement du code, traité depuis par #232. Destination : la
    remesure de PJ.2, ou « sans objet ».
15. **Des noms de MinGW qui couvrent Cygwin (#167)** : `mingw_gcc`
    (`meson.build:681`) et `gaol_mingw_gcc` (`configure.ac:640`). Cosmétique.

## 6. Des faits et des décisions que `TODO_DONE.md` ne note pas

1. **Décidé le 18 octobre (#140)** : pas de job de CI pour les blocs de code des
   fichiers Markdown (`README.md`, `examples/tutorial.md`). À ajouter à 3.4
   (« Décidé le 18 octobre »).
2. **Décidé le 19 octobre (#145)** : pas de `#error` dans
   `gaol/core_math_port.h` (aujourd'hui `gaol/math/gaol_core_math_port.h`), les
   trois builds suffisent. À ajouter à 3.4 (« Décidé pendant le point AE.90 »).
3. **Le commit de Copilot (#164)** : le commit de fusion `849384a`, qui résout
   un conflit à la demande du mainteneur, a pour auteur
   `copilot-swe-agent[bot]` ; c'est le seul commit de l'historique dont l'auteur
   n'est ni Jordan08 ni un auteur de GAOL 4, et le recensement des lignes de
   crédit (point 123, 19 octobre) ne le compte pas. À noter au point 123 ou en
   8.1 (laissé, comme les autres).
4. **#82** (fusionnée le 4 octobre : l'oracle de `strictPrecedes`, `pow` et
   `pown` vérifiés selon IEEE 1788) n'est nulle part dans `TODO_DONE.md`.
5. **#15 à #28 et #30** (du 18 au 28 septembre) manquent à 2.1, qui ne cite que
   #31 à #48 ; #15 à #24 n'y sont que par la plage des lignes de crédit. #48 a
   été fusionnée le 30 septembre à 7 h 24 UTC, pas avant. `STEP.md` (étapes 1,
   3, 9, 10, 12) les classe.
6. **Les commits directs non décrits en 7** : depuis le 1er octobre, 31 commits
   directs ne sont décrits nulle part et 6 le sont en partie : `aac4e05` (3
   octobre) ; ceux du 4 octobre sur `/W4` et les gardes d'inclusion (`cdbde99` à
   `0867109`), `dc036c6`, `ea8f36d` et `d1d3498`, et `8d9c90b` (les
   constructeurs `explicit`, que seul `doc/differences.md` décrit) ; `2d80cc4`
   (5 octobre) ; `2ac9d4e` (15 octobre) ; ceux du 16 octobre, de `3ffde94` à
   `6dd3321` (13, et 5 cités en partie en 3.4) ; `26bb495`, `cd90a58` et
   `eb4f45a` (18 octobre) ; `fa8bc14` (24 octobre) ; `57650a4` (1er novembre,
   cité seulement en 3.13). La section 7 commence au 15 octobre. `STEP.md` les
   classe avec les 271 commits de septembre.
7. **La question 4 de #87** (une variable `upward` qui cassait `-Werror` avec
   `GAOL_PRESERVE_ROUNDING`) a été corrigée par #118 (`19673169`) ; l'entrée de
   #118 en 2.3 ne le dit pas.
8. **Des fusions sans relecture indépendante** : #201, #202 et #204 (leur
   description l'offrait « si le mainteneur le veut »), et les quatre mutants
   `er_div` de #197 ; 2.3 ne le dit pas, alors qu'il le dit pour #176, #199 et
   #215.
9. **Le regroupement du 18 octobre** : #125 à #138 créées, #65, #66, #69, #80,
   #102 et #104 fermées ; ce n'est pas en 7.2, et #1 à #14, #102 et #104 ne sont
   jamais citées (leurs raisons sont dans leurs commentaires, leurs restes sans
   objet).
10. **Des points sans entrée en section 4** : les anciens 10 et 13, faits avant
    les lettres du 3 octobre (par #41, et par #43 puis #57) ; les lettres AH (le
    105, non adopté) et AP (128 à 133) ; et le point ZZ (#124), hors de la plage
    « A à AP » de « Comment lire ce fichier », que citent pourtant environ 26
    commentaires « point ZZ of TODO.md » (`gaol/gaol_fpu.h:375`,
    `gaol/core/gaol_interval_sse.cpp:363`, `doc/three-builds.md:448`...).
11. **Le chiffre de F6 (#241)** : 2.3 (#239) et 2.4 la disent « 0,94, neutre » ;
    le commentaire du mainteneur sur #241 dit « N2 Clang 0,96 (une perte) ».

## 7. Les incohérences des registres

Corrigées par #291 le 5 novembre (*décidé le 5 novembre* : dans #291 ;
`TODO_DONE.md` 3.20) dans `TODO_DONE.md`, `TODO_AFTER.md` et `process.md`,
sauf : en 9, les renvois de 3.4 à PG.6, PG.7 et PG.8 ; en 14, l'entrée de #63 ;
18 à 20, dans `TODO.md`, que `check_branch` refuse dans une branche (avec la
mise à jour qui suit la fusion) ; 21, dans `todo-notes/synthese.md`, que #289
change. La liste reste, comme mémoire.

1. **`TODO_DONE.md` §4, point AI** : « Restent les 105 (AH), 106 (b, c, d), 112
   et 116 » ; le 116 est fait par #222 et #224, le 105 n'est pas adopté (3.10),
   le reste du 112 est refusé (`TODO_AFTER.md` 3.2).
2. **`TODO_DONE.md` §4 AO et §5 (127)** : « celle de `doc/differences.md`
   reste » ; elle est faite par la synthèse de la tâche PI (`16a21ab`, 30
   octobre ; `doc/differences.md:2355`).
3. **`TODO_DONE.md` §4 F** : « syntaxe de la lecture par mot reportée (#64) » ;
   c'est décidé (22 octobre, Q4) et #64 est fermée.
4. **`TODO_DONE.md` §4 AE** est rangé parmi « Les points partiellement faits »
   alors que son texte dit « AE est faite en entier ».
5. **Les tâches citées** : « tâches PH à PJ » (§4, en tête), « tâches PF à PJ »
   (§6, en tête), « tâches PA à PJ » (`process.md` §2, §7 et §9) ; il reste PH,
   PJ et PM. §4 renvoie aussi à « `TODO.md`, anciens 77 à 103 », qui sont dans
   l'ancien `TODO.md` du commit `62dad3a`.
6. **`TODO_DONE.md` §5 (133)** : « Restent le Debug complet de Cygwin et
   autotools et meson sous Cygwin, non vérifiés » ; c'est fait par #205
   (`windows.yml:498` et `:549`, `build-systems.yml:651-652`). De même, §6
   (#163) dit Cygwin « sans job de CI », que #173 a ajouté.
7. **`TODO_DONE.md` §5, le titre « Décisions du mainteneur attendues
   (reportées) »** : les trois sont prises.
8. **`TODO_DONE.md` 3.2** (les décisions du 3 octobre, un compte rendu daté) :
   « 7 : atanh([1]) et atanh([-1]) ajoutés à special_cases.md avec le point 33 »
   est fait par #139 (`doc/compare/code/cases.py:703-704`), et « 8 : l'ancien
   code SSE2 reste sous `#if 0` » est dépassé (#122, `0f45a95`, l'a retiré) : au
   plus, un renvoi à ajouter.
9. **`TODO_DONE.md` 3.4** : les renvois à PG.6, PG.7 et PG.8 de `TODO.md` sont
   faits (#191 et #182, #219, #225), et PG.6 désigne deux points différents (la
   division du chemin AVX-512, #191, et la lecture d'un sous-normal, #182) ;
   « `--no-track` à la section 7.3 reste ouvert » est fait par #155 ; le renvoi
   « voir la section 6 (#151) » vise une entrée qui n'existe pas (la décision
   est en 3.4, tâche PB).
10. **`TODO_DONE.md` 3.11 et 8.1** : 3.11 dit que le mainteneur supprime
    lui-même sur GitHub `todo-pg-max-base` et `todo-pg-max-control`, 8.1
    qu'elles étaient locales et jamais poussées.
11. **`TODO_DONE.md` 3.14** : `coverage.html` « attend le relevé de la
    version » ; il est refait le 4 novembre (`2313e07`, 3.15).
12. **`TODO_DONE.md` §6** se dit « tranchées », mais #146 (le nom de
    `float64x2_t`) n'a pas de sort (voir 1.4) ; #149 n'y renvoie pas à sa
    décision (3.5, Q12 : garder le texte de tous les nombres, le coût étant
    connu et documenté) ; #160 n'a pas sa question 2 (le seuil de 1,5, fait par
    #217) ; et #163 range parmi ses décisions « pas d'entrée
    `GAOL_PRESERVE_ROUNDING` pour autotools et meson sous MSYS2 », qui se lit
    mieux comme un constat à confirmer.
13. **`TODO_DONE.md` §1** : la liste des branches sur GitHub omet
    `todo-pg-relations-order` (#279, fusionnée, `7b54dbe`) et
    `todo-pg-xle-exfirst` (#246, fermée, `f08a53b`), encore là, que `process.md`
    §12 et 8.1 disent « à supprimer » par le mainteneur.
14. **`TODO_DONE.md` 2.3** : l'entrée de #263 à #274 renvoie à « `TODO.md`,
    tâche PI », qui n'existe plus ; celle de #276 dit encore « le workflow macOS
    ne se termine pas », périmé depuis `57650a4` (1er novembre) ; celle de #288
    dit « 10 commits » (11 avec la fusion `68fd774`) et ne la décrit pas
    encore ; celle de #63 ne nomme pas les trois défauts de CORE-MATH corrigés.
15. **`TODO_DONE.md` 2.4** : #221 est datée du 28 octobre, fermée le 29 à 8 h 12
    UTC ; #246 y est décrite par « les ensembles vides d'abord », alors qu'elle
    testait `*this` vide, puis les bornes, puis `I` (son titre : « la
    comparaison des bornes avant le test du second intervalle ») ; les
    commentaires de fermeture de #254 à #261 disent leurs branches « gardées
    jusqu'à la fusion de #262 », 2.4 et 8.1 « gardées », sans terme ; #52 ne dit
    pas que son document est entré par `d6c8ac1`.
16. **`TODO_DONE.md` §6 (#162)** : « (#169, ouverte) » ; #169 est fusionnée
    depuis le 23 octobre.
17. **`TODO_DONE.md` 7.15** : « `TODO_AFTER.md` et l'issue #178 ne changent
    pas » ; #178 n'a pas suivi le `TODO_AFTER.md` du 3 novembre (`47c5c01`, voir
    8.1 ci-dessous).
18. **`TODO.md`** : PJ.1 et PJ.7 donnent la couverture de « 94,3 % des 5 466
    lignes », d'avant #288 (94,9 % de 6 329 lignes dans la CI de #288, sans
    régénérer `coverage/`) ; PJ.7 dit que « #288 y a écrit son texte » juste
    après `todo-notes/synthese.md`, que #288 n'a pas touché ; le commentaire de
    `CMakeLists.txt:1436` (`core_math_log_dint.h`) restera dans PJ.7 après #289,
    qui ne touche pas `CMakeLists.txt`. Fait le 5 novembre (`TODO_DONE.md` 3.21
    et 7.18) : PJ.1 donne aussi la couverture de #288 ; PJ.7, réécrit après
    #289, garde le commentaire de `CMakeLists.txt`.
19. **`TODO.md` PJ.1, le test `numbers` sous ASan et UBSan** : PJ.1 demande
    encore de le lancer une fois en entier, et la même phrase dit qu'il a passé
    en local le 4 novembre (Debug, 9 tests sur 9) : fait, ou à refaire au commit
    de la version ? Destination : la phrase de PJ.1. Fait le 5 novembre
    (`TODO_DONE.md` 3.21) : PJ.1 le dit fait le 4 novembre, à relancer au commit
    de la version si `numbers` ou la bibliothèque ont changé.
20. **`TODO.md` PJ.5, la vérification de `\defmethod`** : PJ.5 demande de
    vérifier que l'entrée de `what()` ne laisse pas son en-tête seul en bas de
    page : depuis #285 (`fb8c54e8`), le manuel n'a plus d'entrée `\defmethod`
    pour `what()` (`manual/v5/gaol.tex:6229` est de la prose). La clause est
    sans objet ; l'autre clause de PJ.5 tient (l'entrée `hausdorff` a encore
    deux `\newinvfive`, lignes 4529 et 4534, pour un seul décidé le 3 octobre).
    Destination : retirer la clause de PJ.5. Fait le 5 novembre (`TODO_DONE.md`
    3.21) : la clause est retirée de PJ.5.
21. **`todo-notes/synthese.md`** : son introduction (lignes 19 à 23) et ses
    notes « Présence : absents » sont périmées depuis #263 à #274 et #280 (#289
    corrige l'introduction) ; les lignes 447 et 1073 disent « change 6 » et
    « change 7 » de `3rd/README.md`, signalés périmés par #111 ; des points y
    sont faits sans le dire (`run_examples.py` devenu
    `manual/check_examples.py`, le commentaire hexadécimal de la ligne 1233, le
    `#include <cmath>` de la ligne 1278, la remarque de la ligne 1611).

## 8. Les issues GitHub

1. **#178** n'a pas suivi `TODO_AFTER.md` depuis le 3 novembre : son en-tête dit
   « au 31 octobre 2026 (commit `5aef739`) », et il n'a aucun des changements
   faits depuis. À mettre à jour avec le `TODO_AFTER.md` de cette branche, après
   sa fusion (`process.md` §9).
2. **#179** est à mettre à jour après `1e0f368` (la tâche PK) ; les titres de
   #178 (« au 31 octobre ») et de #179 (« PH à PK (au 31 octobre) ») sont
   périmés.
3. **#125 (CORE-MATH)** donne encore le correctif 3 « à envoyer », que l'amont a
   fait (`0d8b885a`), et le correctif 8 « à écrire », que #144 a écrit
   (`3rd/README.md:1557`).
4. **Des chemins disparus** depuis #199 et #283 dans les issues ouvertes :
   `gaol/gaol_ldexp.h` (#125, #131, #132), `gaol/core_math_port.h` (#127, #131),
   `gaol/gaol_interval_core.cpp` (#131, #132, #133),
   `gaol/gaol_interval_neon.cpp` (#198). À corriger avant l'envoi (PH).

## 9. En cours, pour mémoire

1. **#288 (fusionnée le 4 novembre)** n'est pas encore décrite dans
   `TODO_DONE.md` (2.3, ses décisions, la branche `compoundm1` en 8.1) :
   `TODO.md` et `process.md` §12 le disent. Sa fusion est arrivée avant la fin
   de la CI de son dernier commit (partie à 19 h 18, finie à 20 h 54 UTC :
   verte, sauf un job macOS annulé faute d'exécuteur, « macOS 15 universal
   (arm64 and x86_64), meson »). Fait : décrite le 5 novembre sur
   `configure-clean` (`6ed4441`, `TODO_DONE.md` 2.3 et 3.18).
2. **#289 (ouverte)** : après sa fusion, son entrée et ses décisions (« Dans
   #289 ») ; le job `Manual` de la CI reste à voir ; si #290 est fusionnée après
   elle, le PDF du manuel sera de nouveau plus vieux que `gaol.tex`.
3. **#290 (ouverte)** : son texte « Pour `doc/differences.md` et `ChangeLog` »
   demandera une synthèse de plus ; l'ambiguïté de la Table 10.7 qu'elle relève
   (les spécifications de `meets` et de `metBy` vraies aussi pour un point sur
   une borne) n'est à signaler à ITF1788 ou au groupe de la norme que si le
   mainteneur le veut (PH) ; le lecteur de texte ne connaît pas `overlap` (« à
   traiter à part si cela sert » : `TODO_AFTER.md`) ; son `operator<<` n'a pas
   été relu par un sous-agent.
