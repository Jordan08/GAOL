# À faire

Ce qui reste à faire sur GAOL v5 au commit `2ceeb51` de `configure-clean` (la
fusion de #209), le 28 octobre 2026, en tâches PG à PJ (PA à PF sont faites, PG
est entamée : voir `TODO_DONE.md`). Ce fichier et l'issue #179 ont le même
contenu. Ce qui est fait et décidé est dans `TODO_DONE.md` (issue #177), ce qui
est reporté dans `TODO_AFTER.md` (issue #178). Les anciennes issues #49 et #142
sont fermées, et `TODO_mistral.md` est supprimé.

Aucune borne fausse n'est connue, sauf peut-être pour un programme qui met
lui-même `FPCR.AH` sur arm64 (issue #198 : `x / y` des intervalles NEON, cause
non trouvée). Rien n'est à envoyer à un autre projet ni à publier sans l'accord
du mainteneur. La seule pull request ouverte est #216 (le tableau des lignes
longues sur ARM 64 bits, à la fin de `performance.md`, branche `perf-doc-rerun`).

## Comment lire ce fichier

- Une tâche (PG à PJ) regroupe des points qui touchent le même code, les mêmes
  fichiers ou les mêmes builds de validation, pour les traiter dans la même
  session ; chaque point reste une pull request à part (process.md : un point,
  une branche), sauf quand la tâche dit qu'une seule suffit. Les tâches sont
  dans l'ordre conseillé. Critères : la sûreté des bornes d'abord ; ce qui coûte
  plus cher en attendant (une décision qui bloque) ; ce qui tue un programme ou
  rompt une promesse écrite avant ce qui ne fait que ralentir ; le petit et sans
  dépendance avant le gros ; la version (PI, PJ) en dernier.
- PG et PH ne dépendent pas l'une de l'autre, sauf quand le texte le dit (PI
  décrit le travail de PA à PH, PJ suit PI et les études de PG).
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
| PG | Les études de vitesse | 105, 116, 106 et six petits | M |
| PH | Les signalements à d'autres projets | #125 à #138, #175 | accord |
| PI | La pull request de synthèse et les textes de la version | Y | L |
| PJ | Les mesures et la publication de v5.0.0 | 32, 33, 34, 71 | M |

## PG. Les études de vitesse [M]

Des mesures d'abord, toutes avant v5.0.0 (*décidé le 19 octobre* ; sauf l'étude
113, reportée après la version le 22 octobre : `TODO_AFTER.md`). Ordre : l'écart
mesuré le plus grand sur les builds par défaut, puis les builds avec
`GAOL_PRESERVE_ROUNDING`, puis ce qui est un gain à porter. Les mesures de la CI
ne se comparent pas d'un passage à l'autre : le banc A/B de `performance.yml`
(#160, #166) compare dans le même job la base d'une pull request et sa tête.

1. **105 (AH)** [M] : `-mbranches-within-32B-boundaries` pour les builds de
   GAOL, *décidé le 14 octobre* à la fin du point AA, sur le Xeon E3-1220 v6 et
   sur l'i7-1185G7. L'erratum des sauts des processeurs Skylake à Comet Lake y
   déplace une opération jusqu'à 1 ns selon le seul placement du code (#122 : `x *
   y`, au code inchangé, passe de 4,96 à 4,69 ns ; `[1] / p`, de 7,0 à 8,1 ns
   quand seul `sqr()` change). À mesurer : le gain sur le Xeon, le coût sur
   l'i7, qui n'a pas l'erratum, et ce que les trois builds peuvent donner aux
   compilateurs qui prennent l'option. #191 l'a montré encore : le banc A/B y
   donnait `x + y` 1,46 puis 1,92 ns pour un `add_bounds()` identique octet pour
   octet, dont seule l'adresse changeait (alignée sur 64 octets, puis décalée
   de 16), et `-falign-functions=64` donné aux deux builds ramenait tous les
   rapports entre 0,98 et 1,03.
2. **116 (AI)** [S] : des écarts de temps sans cause trouvée, relevés par des
   pull requests : `cos(x)` 6 ns plus lent et `sin(x)` 2,4 ns plus rapide dans
   `gaol_performance` après #100 (Clang 18, i7-1185G7), alors que le code de
   `cos()` ne diffère que par l'ordre de deux branches ; `asinh_rel` et
   `nth_root_rel` plus lents de 16 % et 13 % avec Clang 18 après #117, avec
   moins d'instructions (311 contre 330 et 367 contre 399, comptées par
   valgrind) ; `nth_root(x, -2)` à 22,9 ns au lieu de 18,2 avec
   `GAOL_PRESERVE_ROUNDING` (205 instructions contre 266). `perf` n'est pas
   permis sur cette machine. Les mesures d'alors sont faites avec Clang 18 : les
   refaire d'abord avec Clang 21, le compilateur des mesures depuis le 13
   octobre, avant de chercher.
3. **106 (AI), le reste** [M] : le (a), NEON pour Visual C++ arm64, est fait
   par #192, qui l'a donné aussi à clang-cl, à MinGW-w64 et à MSYS2 (à la
   demande du mainteneur : NEON avec tous les compilateurs de l'arm64, SSE2 avec
   tous ceux de l'x86, le FPU là où l'on n'a pas le choix), avec SSE2 pour
   Visual C++ et clang-cl sur x86 32 bits (#193) et le build universel de macOS
   (#194). Restent : (b) mesurer les noyaux de #157 sur POWER, s390x et RISC-V
   (sur POWER8, s390x et RISC-V, GCC met des branches sur les comparaisons de
   produits : le noyau y est plus long que l'ancien code et y perd peut-être
   sans rien gagner) : il faut du matériel, les temps sous qemu ne sont pas ceux
   d'un processeur ; (c) les opérations avec un double (`x * 2.0`) n'ont ces
   noyaux sur aucune cible (#124) ; (d) l'effet de `__vectorcall` (les noyaux
   `add_bounds()` et les autres de Visual C++ et clang-cl x64, #148) n'est pas
   séparé dans les temps de la CI : #193 l'a étendu à x86 32 bits, où le banc
   A/B ne montre aucune différence au-delà de 5 % sur ses rapports FPU/SSE2, et
   le mainteneur l'y a gardé. À rapprocher de `doc/simd.md` (#154).
4. **Le seuil de la liste** [S] (#160) : le seuil de la liste des lignes
   écartées par `compare-performance.py` (un facteur 2 au moins et au-delà de la
   dispersion) : les mesures donnent des rapports jusqu'à 1,17 pour du code
   identique ; 1,5 était possible. *Décidé le 24 octobre* : 1,5 (une ligne de
   l'outil et de son test).
5. **112 (AI)** : rien à faire pour les fonctions élémentaires avec
   `GAOL_PRESERVE_ROUNDING` (deux écritures de MXCSR, jusqu'à 27 % pour `exp`) :
   la seule solution, un mode de CORE-MATH où le sens est un paramètre,
   modifierait `3rd/math-core`, ce que le mainteneur a refusé le 22 octobre
   (Q15c) ; voir `TODO_AFTER.md`.
6. **La lecture d'un nombre sous-normal** [S] (#182) : `1e-310` et les nombres
   au-dessous de 2^-1022 ne sont plus donnés à `strtod()`, qui lève
   `FE_UNDERFLOW` pour eux : le lexeur part de 0 et les place par des pas
   doublés puis la bissection (une centaine de comparaisons exactes). Leur
   lecture prend de 3 à 10 fois plus de temps qu'avant (`1e-310` : 1 019 ns puis
   8 700 à 12 800, `1e-320` : 1 001 puis 2 250 à 3 270, mesurés une fois sur une
   machine occupée) ; celle des nombres normaux ne change pas. Un point de
   départ plus proche, calculé des premiers chiffres, la ramènerait à quelques
   comparaisons, au prix de code de plus dans le lexeur. *À décider* : le faire,
   ou laisser tel quel (la description de #182 propose de le faire si quelqu'un
   lit des sous-normaux en texte).
7. **La cause du gain faible du noyau NEON de `pown`** [S] (#188) : le noyau à
   deux voies de `pown` pour NEON (`vfmaq_f64`, fusionné) ne gagne que 1,03 à
   1,07 sur `pow(x, 3)` avec un Apple M2 (1,30 pour les intervalles autour de 0
   dans `tests/tools/pow/bench.cpp`), 1,23 avec GCC et 0,90 avec Clang sur le
   Neoverse des jobs arm64 de la CI, au lieu du 1,5 attendu ; les rangées que
   #188 ne touche pas y bougent de 5 à 13 %. Une hypothèse, non vérifiée : les
   deux voies scalaires s'exécutaient déjà côte à côte, la chaîne de produits
   dépendants restant la même. *À décider* : chercher (compter les cycles,
   essayer de couper la chaîne en démarrant la boucle des carrés plus tôt), ou
   laisser tel quel.
8. **Le quotient des intervalles NEON avec clang-cl** [S] (#192, #195) : sur
   les Neoverse N2 des runners arm64 de GitHub, `x / y` est plus lent en NEON
   qu'avec le FPU avec clang-cl (5,36 ns contre 4,38, la même mesure sur deux
   exécutions du banc A/B), alors qu'avec Visual C++ NEON gagne (5,40 contre
   10,08) et que `x * y` et `sqrt(x)` gagnent avec clang-cl aussi (5,19 contre
   5,71, 5,65 contre 11,02). Le mainteneur a demandé de garder NEON et de
   chercher la cause. L'essai (#195, fermée sans fusion : deux divisions
   scalaires à la place de la division vectorielle) est plus lent partout, de
   5,36 à 6,17 ns avec clang-cl, de 5,40 à 6,30 avec Visual C++ et de 2,38 à
   2,76 sur l'Apple M2 : la perte n'est pas dans la division mais dans le reste
   du noyau. Une hypothèse, non mesurée (`perf` n'est pas permis ici) : le
   nombre d'instructions vectorielles du noyau de `div_bounds()` face aux deux
   pipelines vectoriels du N2. Les chiffres sont dans `doc/simd.md`. *À
   décider* : compter les instructions de ce noyau dans l'assembleur de
   clang-cl et en chercher un plus court, ou laisser tel quel.
9. **`x / d` dans le banc A/B** [S] (#191) : les deux `vdivsd {ru-sae}`
   scalaires de #191 font passer `x / d` de 3,62 à 2,76 ns (meilleur temps, d >
   0, i7-1185G7), mais le banc A/B de `performance.yml` n'a aucune ligne `x /
   d` : il ne verra ni ce gain ni une perte. *À décider*, trois questions de
   #191 sans réponse : (a) ajouter cette ligne à `tests/performance.cpp` (les
   lignes de la table sont décrites dans `doc/tests.md` et choisies par
   `compare-performance.py`) ; (b) les divisions du dispatch d'un diviseur qui
   contient 0 et du `1/h` des puissances négatives sont converties sans gain
   mesuré (rapports de 0,92 à 1,04) : les garder pour la cohérence, ou les
   remettre en forme 512 bits ; (c) `div_bounds()` répète à la main ce que fait
   `er_div2()` : les rapprocher, ce que #191 n'a pas fait pour ne pas déplacer
   de code (l'effet de placement du 105). Le gain de `x / d` n'est mesuré que
   sur un Tiger Lake (`TODO_AFTER.md`, 4.6).
10. **`set_rounding()` et `set_rounding_and_flush_modes()` sans écrire pour
   rien** [S] (#210) : `GAOL_RND_NEAREST_LEAVE()` et `GAOL_RND_RESTORE()`
   passent par `fesetround()` de la glibc sur x86-64, qui écrit l'unité x87 et
   MXCSR comme avant #210, parce que le cas courant y change la direction (de au
   plus proche à vers le haut) et que l'économie ne serait que celle d'un
   `fldcw`. #210 recommandait de ne pas le faire. *À décider* : le laisser tel
   quel, ou l'étendre.

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
   résultat observable.
2. **#126 GCC** (la comparaison silencieuse rendue signalante par
   l'if-conversion sur ARM 32 bits et POWER9 et par la vectorisation, et le
   miscompilage de GCC 12.2 sur aarch64 dans `tests/numbers.cpp` ; le bogue
   52258 est de la même famille), **#127 mingw-w64** (`fma()` et `round()` de
   msvcrt, `fesetexceptflag()` en 32 bits, le `fma()` x87 de mingw-w64 11),
   **#129 ITF1788** (huit cas dont le résultat attendu n'est pas le plus serré),
   **#130 Boost.Interval** (`rounded_transc_opp` donne la borne supérieure pour
   l'inférieure ; issue seule ou avec une pull request, selon la politique de
   Boost) et **#175 Cygwin** (le `fma()` mal arrondi dans les quatre sens, le
   `nextafter()` qui calcule le débordement dans le sens d'arrondi du programme,
   CPack qui ne met pas le lien `README` dans l'archive, cause non trouvée ;
   mesures de CI et brouillon anglais dans l'issue, à rejouer avant l'envoi).
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

## PI. La pull request de synthèse et les textes de la version (Y) [L]

Ensemble parce que ce sont les textes de la version (`ChangeLog`,
`doc/differences.md`, `examples/examples.md`, `manual/v5/`), à écrire une fois
PA à PH faites ou décidées (PA à PE le sont), et avant les mesures de PJ.

1. **La pull request de synthèse** : `ChangeLog` (une entrée du 27 septembre,
   touchée le 4 octobre) et `doc/differences.md` (28 septembre, puis des commits
   directs des 16 et 18 octobre) ne reprennent pas le travail du 29 septembre au
   24 octobre, hors quelques commits directs. Les textes proposés sont, pour les
   points faits du 29 septembre au 2 octobre (#31 à #63), dans
   `todo-notes/synthese.md` (section 1, dont onze sont marqués « In
   `configure-clean`: no », certains périmés), et, pour le reste, dans les
   descriptions de pull requests qui ont une section « Pour `doc/differences.md`
   et `ChangeLog` » : #71, #76, #77, #83 à #97, #99, #100, #105, #108, #111,
   #113, #114, #117 à #124, #139, #140, #141, #144 à #151, #155 à #158 et #160 à
   #164 (#123, #139, #155 et #160 n'ont rien à y mettre), et à lire aussi, #165
   à #174, #176, #180 à #184 (celle de #176 dit : rien), #191 à #194 et #196 et
   #197 (qui disent : rien), #199 (l'installation des en-têtes dans
   `include/gaol/core/`), #200 (les options des trois builds : voir 2), #201
   (les drapeaux C de CORE-MATH, déjà dans `doc/differences.md`), #202 à #204
   (qui disent : rien) et #205 (les jobs qui manquent : voir 2). Les points K,
   L et `make distclean` (#72 à #75) n'ont pas cette section : à écrire d'après
   leurs descriptions. À écrire d'après les commits, sans pull request : les
   changements du 4 octobre (`nb_fp_numbers()` rend un `unsigned long long`,
   `gaol_parameters.h` supprimé) et ceux du 15 au 18 octobre, qui changent
   l'interface installée : les en-têtes du parser et des expressions dans
   `include/gaol/parser/` (`gaol_expression.h`…), `gaol_interval.h` scindé en
   `gaol_interval_core.h` et `gaol_interval.h`, les intervalles de floats dans
   `gaol/float/`, `gaol_assert.h`, `gaol_profile.h` (la classe `timepiece`),
   `gaol_exact.h`, `gaol_flags.h`, `gaol/dllmain.cpp` et `operator""_iv`
   (`gaol_literals.h`) supprimés, `f_negate`, `f_negate_simple`, `gaol_opposite`
   et `reset_fpu_cw` supprimés (a6786af), la lecture en bases 16, 8 et 2
   (91dcce2) ; vérifier ce que `doc/differences.md` en dit déjà. Le point G n'a
   pas de texte dans la description de #91 (écrite avant la fin du travail) : à
   écrire d'après c7915a7, ccf694a et `doc/three-builds.md`. Les chiffres de
   `todo-notes/synthese.md` sont à harmoniser (l'ancien 18 : « 2.5 s down to
   0.03 s » contre 0,02 s), et `doc/differences.md` contredit déjà #71 (la
   locale du flux pour le format `bounds`) : c'est dans ses notes.
2. **Ce que les descriptions demandent de corriger** dans `doc/differences.md`
   et `ChangeLog` : celle de #146 dit que la ligne 333 (« the intervals of ARM »
   sont FPU) n'est plus vraie sur arm64, et celle de #148 que la même ligne («
   of Visual C++ and of 32-bit Windows ») ne vaut plus pour x64 ; celle de #150
   ajoute que `GAOL_PRESERVE_ROUNDING` coûte 22 ns au lieu de 48 sur ARM 32 bits
   avec GCC, et le défaut de `width()` et `bisect()` sur arm64 ; celle de #156
   ajoute (vers les lignes 159 et 272) la lecture directe du sens d'arrondi sur
   x86 32 bits (`fnstcw`, `stmxcsr`) et la remise de chaque unité dans son sens
   avec Visual C++ pour x86 ; celle de #158 dit que la puce « The n-th roots are
   proved with integer powers » doit parler de « jusqu'à 2 doubles » (4 pour
   `rootn(x, q)` avec q < 0) et non d'« un double », et du chemin des x sous
   2^-1020 — la même phrase « tightest bounds, or one double beyond » est dans
   la description de `nth_root` et le tableau des précisions de
   `manual/v5/gaol.tex` ; celle de #157 ajoute les noyaux sans branche des
   intervalles FPU (`x * y`, `x / y`, `x % y`, `sqr`, et `x - y` au temps de `x +
   y`), le signe d'une borne nulle d'un produit, non spécifié avec la FPU, et
   les indicateurs `FE_UNDERFLOW` et `FE_OVERFLOW` d'un coin du produit ; celle
   de #161 donne les deux entrées de `ChangeLog` du défaut de CMake et des
   descriptions d'options ; celle de #162 que `pow(x, 2)` est `sqr(x)`, au signe
   d'un zéro et aux indicateurs de `sqr` près ; celle de #163 que GCC pour
   Windows x86-64 reçoit `-mfma -mprefer-vector-width=128` et que GAOL y lit et
   écrit lui-même les registres de contrôle avec mingw-w64 13 et plus ; celle de
   #164 que le format `agreeing` écrit un intervalle ponctuel comme un nombre
   nu, que la lecture d'un long nombre prend un temps quadratique en ses
   chiffres, qu'une espace est nécessaire entre un signe et un nombre incertain,
   le piège de `namespace gaol { }`, et que `gaol_performance` est compilé en
   C++14 ; celle de #143 que la phrase de `doc/differences.md` sur « les macros
   que lisent les sources de GAOL » reste à corriger ici ; celles de #192 à
   #194 que la même ligne 333 ne vaut plus pour aucun compilateur de l'arm64
   (NEON avec Visual C++, clang-cl, MinGW-w64 et MSYS2 comme avec GCC et Clang),
   ni pour x86 32 bits avec Visual C++ et clang-cl (SSE2, leurs bornes rangées
   en deux `double` d'alignement 8), ni pour un build universel de macOS (NEON
   pour arm64, SSE2 pour x86_64), et que les opérations NEON de Visual C++
   effacent FZ et FIZ de FPCR, ce que ses intervalles FPU ne faisaient pas ;
   celle de #196 que `doc/differences.md` (ligne 790) cite encore
   `interval(sl, sr)`, qui n'existe plus ; celle de #199 que
   `gaol_interval_core.h`, `gaol_interval_fpu.h`, `gaol_interval_neon.h` et
   `gaol_interval_sse.h` sont installés dans `include/gaol/core/` et non plus
   dans `include/gaol/`, sans en-tête de renvoi (*décidé le 25 octobre*) ; celle
   de #200 que les trois builds ont les mêmes options (`GAOL_PRESERVE_ROUNDING`,
   `GAOL_SIMD`, `GAOL_COVERAGE`, `WITH_TESTS`, `WITH_EXAMPLES`,
   `BUILD_SHARED_LIBS`, le type de build, le code indépendant de la position et
   `CMAKE_OSX_ARCHITECTURES`), que la bibliothèque statique est le défaut des
   trois (configure et meson construisaient les deux), que `GAOL_ARM64` et
   `--enable-optimize` n'existent plus, que `GAOL_SIMD=OFF` vaut aussi pour
   NEON, que le chemin AVX-512 suit `GAOL_PRESERVE_ROUNDING`, que les
   instructions FMA et l'entier 128 bits sont décidés par des tests de la
   configuration ; ces passages de `doc/differences.md` et de `ChangeLog` sont
   corrigés par #201 (fusionnée le 26 octobre, *décidé le 26
   octobre*), à relire ici seulement ; celle de #205 que Clang avec le pilote
   de GCC pour la cible de Visual C++ est pris en charge par les trois builds
   (C++14, `unistd.h` de `cmake/msvc`, bibliothèque statique), que le
   générateur d'Xcode sans `CMAKE_OSX_ARCHITECTURES` construit pour
   l'architecture de la machine (la liste `arm64;x86_64` donne le build
   universel), que `.github/scripts/tests.sh` compile les tests avec les
   Cflags du `gaol.pc` installé, et que MSYS2 MINGW32 (mingw-w64 12 ou plus,
   `msvcrt.dll`) est refusé sans `-mfma`, comme msvcrt sur x86-64 (*décidé le
   27 octobre*).
   *Décidé le 3 octobre*
   : la puce de `atanh` de `doc/differences.md` reste, en disant que GAOL 4
   n'est pas mesuré, les puces de `hausdorff()` sont fusionnées.
3. **`examples/examples.md`** : marquer **Fixed** les n° 2, 3, 4, 5, 7, 8, 10,
   12, 15, 16, 19, 20, 21 et 22 de la section 5 et de l'annexe B, le n° 10 comme
   corrigé par la suppression des constructeurs à partir de chaînes (#30), et
   par #147 pour `parse_interval(nullptr)` (et par le 128, #180, pour
   `textToInterval(nullptr)`) ; reprendre ce qu'il dit des points corrigés
   (la ligne vide du n° 14 l'est par #114 ; les phrases de #114 sur la locale à
   virgule et l'intervalle ponctuel sont déjà corrigées).
4. **Le manuel** : une seule régénération de `manual/v5/gaol.pdf` (la dernière
   date du 28 septembre ; ne jamais le commiter dans une branche de point),
   après avoir corrigé dans `gaol.tex` les entrées d'index en conflit de «
   canonical interval » et l'`Overfull \hbox` connu (le seul : les lignes 511 à
   515 de `gaol.tex` le 14 octobre), et en vérifiant que l'entrée de `what()` ne
   laisse pas son en-tête seul en bas de page (macro `\defmethod`). *Décidé le 3
   octobre*, pour les points 7 et 10 : un seul `\newinvfive` dans l'entrée
   `hausdorff` du manuel, et aucun sur la phrase du domaine de `atanh`. Le
   manuel se compile avec `pdflatex`.

## PJ. Les mesures et la publication de v5.0.0 (Y) [M, dans cet ordre]

En dernier : elle suppose les corrections faites et les études de PG faites
(*décidé le 19 octobre*).

1. **32** : le rapport de couverture ([coverage/README.md](coverage/README.md)
   et `coverage/coverage.html`), refait le 2026-09-27 à `605728e` (87,2 % des
   lignes de `gaol/`), ne correspond plus aux sources : le parser est dans
   `gaol/parser/`, `gaol_interval.cpp` est scindé en `gaol_interval_core.cpp` et
   `gaol_interval.cpp`, et `gaol_exact.c`, `s_nextafter.c`, `gaol_profile.cpp`
   sont supprimés ; `coverage.html` cite encore `sysdeps/gaol_exact_c99.h` et
   `s_nextafter.c`, et `coverage/README.md` a été corrigé à la main (88,0 %). Le
   job « Coverage » de `linux.yml` le refait à chaque push, mais pas la copie du
   dépôt. À refaire au commit de la version (`-DGAOL_COVERAGE=ON`, cible
   `coverage`, gcovr) et à enregistrer ; *décidé le 15 octobre* : pas avant.
   Lancer aussi une fois, sous ASan + UBSan, le test `numbers` en entier (trop
   long pour la relecture du 14 octobre : 158 s pour `arithmetic` seul) ;
   `GAOL_NUMBERS_TIMEOUT` n'est pas relevé pour les builds avec sanitizers
   (*décidé le 14 octobre*).
2. **33** : les temps de
   [doc/compare/performance.md](doc/compare/performance.md), remesurés le 17
   octobre (`febd2b1`, `a055160`), sa colonne « rounding direction preserved »
   le 18 (`7fd5c09`), avec Clang 21 sur l'i7-1185G7 ; les études de PG peuvent
   encore les changer. À remesurer en dernier, sur le commit propre de la
   version, la machine ne faisant rien d'autre (`doc/compare/code/run_bench.sh`,
   ou `make perf`), puis reprendre le tableau de `doc/compare/README.md` :
   `performance.md` cite encore l'amont de CORE-MATH `6b84457`, et l'import du
   27 octobre (`0b2ae6d1`) a réécrit `sin`, `cos` et d'autres fonctions, dont
   les temps peuvent changer (*décidé le 27 octobre* : dans PJ). Les temps
   pris sur une machine non au repos (#158, les racines d'un x
   sous-normal) sont à refaire ici si on les cite. Refaire aussi, au même
   commit, les colonnes de Visual C++ et de clang-cl pour x86 et pour arm64 de
   `doc/using.md` (« The FPU intervals ») et de `doc/three-builds.md` : elles
   ont été mesurées avec les intervalles FPU, avant #192 et #193 (une phrase le
   dit), et ces compilateurs ont maintenant SSE2 et NEON. *Décidé le 25
   octobre* (#192, #193) : à la fin.
   Refaire aussi, au même commit, le tableau des lignes longues sur un
   processeur ARM de 64 bits, à la fin de `performance.md`
   (`doc/compare/code/run_lines_arm64.sh` sur un Mac M2, branche
   `perf-doc-rerun`) : #216 le propose, avec une première mesure du 28 octobre
   (*décidé le 28 octobre*).
   Refaire enfin, au même commit, la mesure de la règle du chemin AVX-512 de
   `doc/simd.md` (il suit `GAOL_PRESERVE_ROUNDING`, seuil d'un gain médian de 5
   %, mesuré le 26 octobre sur le seul i7-1185G7, dont l'AVX-512 est à
   demi-largeur), sur une machine à AVX-512 pleine largeur si le mainteneur en
   donne l'accès (un Xeon, un Zen 4) :
   `.github/scripts/performance-ab.sh --runs 9 --lines`, quatre builds de la
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
   : fait.
4. **71 et #128** : annoncer GAOL v5.0.0 à Frédéric Goualard, l'auteur de GAOL,
   une fois l'étiquette posée : pas d'annonce publique, et pas de GitHub
   Release, l'étiquette suffisant (*décidé le 3 octobre*). Le message est écrit,
   en brouillon dans Gmail (objet « GAOL v5.0.0 ») ; il suppose que l'étiquette
   existe. À faire avant l'envoi : vérifier l'adresse (celle du brouillon vient
   d'un échange de 2014 et d'une liste de 2020 : elle peut avoir changé) ; le
   second paragraphe (les défauts de GAOL 4) dépasse l'annonce décidée le 3
   octobre, mais *décidé le 24 octobre* : le garder tel quel.
