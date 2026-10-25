# À faire

Ce qui reste à faire sur GAOL v5 au commit `6eabe20` de `configure-clean`, le 25
octobre 2026, en tâches PD à PJ (PA, PB et PC sont faites : voir
`TODO_DONE.md`). Ce fichier et l'issue #179 ont le même contenu. Ce qui est fait
et décidé est dans `TODO_DONE.md` (issue #177), ce qui est reporté dans
`TODO_AFTER.md` (issue #178). Les anciennes issues #49 et #142 sont fermées, et
`TODO_mistral.md` est supprimé.

Aucune borne fausse n'est connue. Rien n'est à fusionner, à envoyer à un autre
projet ni à publier sans l'accord du mainteneur. Aucune pull request n'est
ouverte.

## Comment lire ce fichier

- Une tâche (PD à PJ) regroupe des points qui touchent le même code, les mêmes
  fichiers ou les mêmes builds de validation, pour les traiter dans la même
  session ; chaque point reste une pull request à part (process.md : un point,
  une branche), sauf quand la tâche dit qu'une seule suffit. Les tâches sont
  dans l'ordre conseillé. Critères : la sûreté des bornes d'abord ; ce qui coûte
  plus cher en attendant (une décision qui bloque) ; ce qui tue un programme ou
  rompt une promesse écrite avant ce qui ne fait que ralentir ; le petit et sans
  dépendance avant le gros ; la version (PI, PJ) en dernier.
- PD à PH ne dépendent pas les unes des autres, sauf quand le texte le dit (PI
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
| PD | `process.md`, les textes, les tests, le ménage | 130 et sept petits | S |
| PE | Les trois builds, la CI, les vérifications qui manquent | 95, 92 et quatre autres | M |
| PF | x86 32 bits, Windows et le registre de contrôle | 108, 109 et trois | M |
| PG | Les études de vitesse | 105, 116, 106 | M |
| PH | Les signalements à d'autres projets | #125 à #138, #175 | accord |
| PI | La pull request de synthèse et les textes de la version | Y | L |
| PJ | Les mesures et la publication de v5.0.0 | 32, 33, 34, 71 | M |

## PD. `process.md`, les textes, les tests, le ménage [S]

Ensemble parce que ce sont de petits textes et de petits tests, sans risque pour
les bornes, qui tiennent dans une ou deux pull requests.

1. **130 (AP)** [S] : `process.md` : une phrase plus courte à la place du
   paragraphe du §6.1 que #155 a retiré (l'arbre `GAOL_V9/GAOL`, les 19 et 20
   octobre, avait bien les outils qu'il décrivait : Ubuntu 24.04, GCC 13.3,
   Clang 18 et 20, CMake 3.28, python 3.12, sans autoconf, automake, libtool,
   meson ni ninja, 4 cœurs ; autoconf 2.72 s'y construit depuis le tarball de
   ftp.gnu.org, sha256 vérifié, meson et ninja s'installent dans un
   environnement virtuel), et une date et un commit dans l'en-tête, au lieu du
   renvoi au §12. *Décidé le 22 octobre (Q9a, Q9c, #155).* Pas de ligne pour
   `GAOL_V9/GAOL` au §3.3 (Q9b : garder « 4 cœurs au plus »).
2. **Deux pièges pour `process.md`, §11** [S] : (a) *(#149)* le test de
   `tests/expressions.cpp` compte les allocations par le `operator new` qu'il
   remplace, qui ne voit pas celles d'un `std::string` que fait la bibliothèque
   C++ quand elle est dans une DLL (Windows : libstdc++ de MSYS2 et de
   MinGW-w64, libc++ de CLANG64), alors que son destructeur en ligne appelle le
   `operator delete` du programme : un `std::string` de plus de 15 caractères
   dans le code de la lecture fait échouer « a reading frees all it allocated »
   (quatre jobs de #149) ; un `std::vector` est sans danger. (b) *(#150)* une
   écriture du registre de contrôle par `asm volatile` n'est ordonnée qu'avec
   les autres `asm`, pas avec les opérations sur des registres ; une fonction en
   ligne qui lit ses bornes avant `GAOL_RND_ENTER()` calcule dans le sens du
   programme. La règle est dans `gaol/gaol_fpu.h` (lire les bornes après la
   vérification, un double passé par valeur à travers `rnd_reread()`) ; un test
   doit appeler la fonction sur un intervalle construit dans la même fonction,
   pas sur un intervalle relu en mémoire, qui cache le défaut.
3. **`doc/accuracy.md`** [S] (#149) : le tableau cite encore `interval(sl, su)`
   pour `textToInterval(s)`, qui n'existe plus depuis GAOL v5.
4. **`doc/compare/itf1788/itf1788.py`, l. 821** [S] (#146) : nomme « SSE2 » le
   build par défaut, qui est NEON sur arm64.
5. **Les tests qui calculent un x avec `std::pow`** [S] (#158) : le `pow()` de
   la bibliothèque C de macOS n'est pas exact en arrondi dirigé pour certains
   résultats sous-normaux (`pow(3·2^-95, 11)` en arrondi vers le haut rend
   `0x1.59fd80000004p-1028` au lieu de `0x1.59fd8p-1028`), alors qu'il l'est au
   plus proche ; un programme lié à GAOL est en arrondi vers le haut jusqu'à
   `gaol::cleanup()`. `tests/arithmetic.cpp` donne donc ses x directement ; les
   autres tests qui calculent un x avec `std::pow` n'ont pas été relus pour un
   résultat sous-normal.
6. **Le masquage des exceptions autour de CORE-MATH** [S] (#184, issue #185) :
   `pow_up_without_overflow_exception()` (`gaol/gaol_interval_core.cpp`) appelle
   CORE-MATH avec les exceptions masquées pour les seules puissances dont log2
   est à moins de 10^-9 de 1024 (c voisin de `DBL_MAX` dans `pow_rel` et
   `pow_exponent_rel`, environ 2^12 valeurs de c), puis rétablit l'environnement
   du programme : MXCSR écrit (`_mm_setcsr(csr | 0x1f80)`, puis `csr`) sur x86,
   sauf avec Visual C++ pour x86 32 bits, et `feholdexcept()` puis `fesetenv()`
   ailleurs. C'est le seul endroit de la bibliothèque qui masque des exceptions
   ; celles de `<cfenv>` de MinGW-w64 12 à 14 laissent des drapeaux levés et
   plantent, d'où MXCSR. *À décider* (le mainteneur l'a reportée le 24 octobre,
   #184 garde le masquage en attendant) : le garder ; le retirer, ce qui laisse
   le drapeau pour ces seuls appels, retire les deux cas de test et exclut ces
   tirages du balayage de `tests/rounding_direction.cpp` ; ou ne le garder que
   sur x86-64 avec glibc et UCRT, l'appel direct ailleurs.

7. **Les mutants de `tests/tools/pow/mutate.py`** [S] (#188) : (a) quatre
   mutants ne s'appliquent plus au code, déjà avant la tâche PC
   (`std-empty-y-unchecked`, `std-infinite-exponent-unchecked`,
   `corner-lower-not-exact`, `corner-upper-no-limit` ; `mutate.py --list` les
   dit) : les remettre à jour ou les retirer ; (b) les dix mutants `neon-*`
   ajoutés par #188, qui reprennent ceux de `sse-fma-*`, n'ont jamais été
   lancés, faute de processeur ARM 64 bits dans la session (une compilation
   croisée sous qemu n'est pas prévue par l'outil) : les lancer sur un Apple M2
   ou un runner arm64, et dire lesquels survivent.
8. **Un échec de CI non reproduit** [S] (#189) : le job « macOS 14 x86_64
   (Rosetta) » a échoué une fois sur `arithmetic` (`gaol_ieee1788::pow(x, [2])
   is sqr(x & [0, +oo]), [0] for [0], the same bits`, avec x =
   [-0x1.0d8146092a4f3p+345, 0x1.d324166d86984p+204], « the caller's direction
   downward, flush modes 0 » : GAOL a rendu [0, 0x1.aa365774f4ea3p+409] et le
   calcul de référence, `sqr(x & [0, +oo])`, le vide) et est passé à la relance,
   sans lien visible avec le diff de #189. Non reproduit en local ni
   dans les autres jobs de #186 à #190. Si cela revient, regarder si Rosetta lit
   le sens d'arrondi ou les modes de MXCSR comme un processeur.

## PE. Les trois builds, la CI, les vérifications qui manquent [M]

Ensemble parce que ce sont les mêmes fichiers (`CMakeLists.txt`, `configure.ac`
et `configure`, `meson.build`, `.github/audit/`, `.github/workflows/`) et la
même validation : les trois builds sur la matrice de `process.md` §6.2, puis la
CI.

1. **95 (AE)** [M] : les détections compilent dans un build et lient dans les
   autres. La sonde de la cible AVX-512 lie avec CMake, mais ne fait que
   compiler avec autotools (`AC_COMPILE_IFELSE`) et meson, alors que
   `__builtin_cpu_supports` demande `__cpu_model` à l'édition de liens ; la
   sonde de FMA, à l'inverse, ne vérifie que le C++ avec CMake, et le C et le
   C++ avec édition de liens dans les deux autres. Non vérifié sur une chaîne où
   le résultat changerait. Correction : lier, et vérifier les deux langages,
   dans les trois builds.
2. **92 (AE)** [M] : les sources C n'ont pas les mêmes drapeaux dans les trois
   builds : CMake leur donne `-msse2 -msse3 -fvisibility=hidden`, autotools et
   meson au C++ seulement ; `-ffloat-store` va au C et au C++ avec CMake et
   meson, au C++ seul avec autotools ; le C est gnu99 avec CMake, c99 ou gnu11
   avec meson, celui du compilateur avec autotools. `.github/audit` ne compare
   que les drapeaux de `gaol_interval.cpp`. Correction : les mêmes drapeaux C
   partout, et un source C dans `probe_build.py`. (Principe 6 de `process.md` :
   les mêmes bibliothèque, en-têtes et options partout. Aucun effet connu sur
   les bornes.)
3. **Les jobs qui manquent** (#152, #173, #174) : aucun job ne construit
   autotools ni meson avec MinGW-w64 x86 (le MSYS2 de `build-systems.yml` est
   x64), et l'audit des trois builds ne tourne pas sous Windows : leur accord
   pour Windows 32 bits n'est vérifié que par simulation (`CXXFLAGS=-D_WIN32`).
   Pour Cygwin, restent non vérifiés « le Debug complet de Cygwin et autotools
   et meson sous Cygwin » (le job fait un build Debug sans appel de `fma()`).
4. **Les remarques des outils** (#160) : `actionlint` dit que
   `suppress-deprecation-warnings` n'est pas une entrée de
   `msys2/setup-msys2@v2` (job `msys2-refused` de `windows.yml`), et
   `shellcheck` fait des remarques sur des étapes de `build-systems.yml`,
   `linux.yml`, `manual.yml` et `windows.yml`.
5. **Les affirmations non vérifiées** (#163) : le commentaire de
   `CMakeLists.txt` dit que le bogue 54412 de GCC (la pile de Windows x64
   alignée sur 16 octets, non sur 32) est « fixed for GCC 17 only » : Bugzilla
   refuse l'accès, l'état vient de lectures indirectes (ouvert jusqu'à GCC 16,
   un correctif sur gcc-patches le 6 juin 2026), plausible et non vérifié. Les
   `fesetround()` et `fegetround()` de mingw-w64 ont été lus dans les sources de
   `master`, du CRT de MSYS2 de la CI (`4564ee4b5`) et des étiquettes v13 et
   v14, non dans ceux de Chocolatey : une version qui garderait les indicateurs
   ne gênerait pas la correction, mais n'a pas été cherchée.
6. **Les relectures indépendantes qui manquent** : les corrections que la
   relecture de #161 a fait faire (points A à F) n'ont pas été relues ; les
   corrections qui ont suivi les deux relectures de #163 (`704ca0c`, `e80b6f0`,
   `1de1df0`, `15399b5`) non plus.

## PF. x86 32 bits, Windows et le registre de contrôle [M]

Ensemble parce que c'est le même code (`gaol/gaol_fpu.h`,
`gaol/gaol_fpu_fenv.h`, `gaol/core_math_port.h`, `tests/rounding_direction.cpp`)
et les mêmes jobs (Visual Studio 2026 x86 et x64, MinGW-w64 et MSYS2, Intel
SDE).

1. **108 (AI), le reste** [M] : la lecture de x87 et de MXCSR en ligne est faite
   par #156 (`fnstcw` et `stmxcsr`, `GAOL_RND_X86_DIRECT`, sans `fegetround()`
   qui coûtait environ 80 ns). Restent, *décidés le 22 octobre (Q8, tout oui,
   #156)* : (a) lire MXCSR en ligne dans CORE-MATH sur x86 32 bits
   (`gaol/core_math_port.h` comme sur x86-64) : `sin`, `cos`, `tan`, `pow`,
   `cbrt`, `rsqrt` et `asinpi` y appellent encore `fegetround()`, qui coûte
   autant qu'avant ; (b) un test de `x86_registers::read_control()` contre
   `read_x87_word()` et `_mm_getcsr()` du test, dans les six états, sur x86
   seulement ; (c) garder le job « Visual Studio 2026 x86 Release » avec la
   direction préservée ; (d) ajouter au dépôt (`tests/tools/`) l'outil de
   mutation (`tools/mutate.py`), l'amalgamation pour Compiler Explorer et
   l'émulateur (`emu/`) du travail de #156 ; (e) retirer le chemin de repli de
   `round_upward_if_needed()` avec `fegetround()`, du code mort aujourd'hui. Le
   gain de #156 n'a pas été mesuré (la machine de travail n'a ni Windows ni x86)
   : il se lit dans l'étape « Performance » de la CI de #156, à comparer à celle
   de `configure-clean`.
2. **109 (AI)** [M] : `GAOL_PRESERVE_ROUNDING` coûte 24 ns par opération avec
   Visual C++ x64 depuis #148 (43 à 87 avant ; 2,5 sans) : les macros de #124
   (`GAOL_RND_ENTER()` n'écrit que MXCSR, une fois à l'entrée et une fois à la
   sortie) demandent des intervalles SSE2 et `__FMA__`, que `cl` ne définit pas
   ; il y reste `fegetround()`, `fesetround()` et deux écritures séparées.
   *Décidé le 20 octobre*, dans une pull request qui suit #148 : la condition
   des macros avec `_M_X64` et `__AVX2__` pour `cl` ; et chercher ce qui coûte
   22,6 ns avec clang-cl, qui définit `__FMA__` avec `/arch:AVX2` (518 ns pour
   la ligne de 15 `+` avec le chemin AVX-512 refusé), sans que l'on sache
   pourquoi. Fichiers : `gaol/gaol_fpu.h`, `gaol/gaol_fpu_fenv.h` (macros
   `GAOL_RND_ENTER_SSE`).
3. **Écrire seulement si ça change** [M] (#156) :
   `GAOL_RND_NEAREST_ENTER()`/`LEAVE()` et `GAOL_RND_ENTER_SSE()`/`LEAVE_SSE()`
   écrivent sans comparer. La même méthode (écrire si ça change) était possible
   ; non touchées, hors du point 108. *Décidé le 24 octobre* : l'appliquer aux
   deux familles, en mesurant avant et après par le banc A/B de la CI, avec les
   tests de `rounding_direction.cpp`.
4. **L'alignement de la pile 32 bits** (#152) : `-faligned-new=8` ne règle que
   le tas ; l'alignement de la pile 32 bits (GCC, bogue 40838) concernerait des
   intervalles locaux dans des fonctions que l'API de Windows appelle avec une
   pile alignée sur 4 octets (fils d'exécution, rappels) ; non testé.
   (`-mstackrealign` a été essayé par la CI pour le x86 32 bits : sans effet,
   #172.)
5. **Le même contrôle de l'assembleur pour x86-64** (#150) : l'ordre entre
   l'écriture du registre de contrôle et les opérations sur des registres (bogue
   34678 de GCC) a été vérifié sur l'assembleur de toute la bibliothèque pour
   AArch64 et ARM 32 bits (GCC 14.3 croisés : aucune instruction flottante qui
   dépende du sens entre `mrs`/`vmrs` et `msr`/`vmsr`), non pour x86-64, où
   `_mm_setcsr` n'est pas mieux ordonné. `width()` en est le cas connu (corrigé
   sur x86 32 bits avec GCC 9.4, 12.2 et 15.2 par `rnd_reread()`, #150) ; à
   refaire avec un GCC pour x86-64.

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
   compilateurs qui prennent l'option.
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
3. **106 (AI), le reste** [M] : (a) pour Visual C++ arm64, offrir NEON
   (`arm64_neon.h`) : NEON n'y est pas pris aujourd'hui, ni avec clang-cl ni
   avec MinGW-w64 ; (b) mesurer les noyaux de #157 sur POWER, s390x, RISC-V et
   MinGW-w64 arm64 (sur POWER8, s390x et RISC-V, GCC met des branches sur les
   comparaisons de produits : le noyau y est plus long que l'ancien code et y
   perd peut-être sans rien gagner) : il faut du matériel, les temps sous qemu
   ne sont pas ceux d'un processeur ; (c) les opérations avec un double (`x *
   2.0`) n'ont ces noyaux sur aucune cible (#124) ; (d) l'effet de
   `__vectorcall` (les noyaux `add_bounds()` et les autres de Visual C++ et
   clang-cl x64, #148) n'est pas séparé dans les temps de la CI. À rapprocher de
   `doc/simd.md` (#154).
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
6. **La division du chemin AVX-512** [S à M] (`doc/simd.md`, tableau « Verdicts
   ») : essayer deux `vdivsd {er}` scalaires pour `/` dans le chemin AVX-512
   (`GAOL_PREFER_AVX512`). Sur Intel, la division 512 bits coûte 16 cycles de
   débit, contre 4 en 128 bits (uops.info) ; Zen 5 a des unités pleine largeur.
   L'idée a réussi pour `sqrt` (deux racines scalaires à arrondi intégré : 3,75
   ns contre 6,30 avec SSE2) et n'a pas été essayée pour `/`. À mesurer sur
   l'i7-1185G7 avec le banc A/B de `performance.yml` : les deux `vdivsd {er}` et
   les permutations peuvent perdre sur un processeur Intel qui divise les
   registres en deux. À rapprocher du 106.
7. **La lecture d'un nombre sous-normal** [S] (#182) : `1e-310` et les nombres
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

8. **La cause du gain faible du noyau NEON de `pown`** [S] (#188) : le noyau à
   deux voies de `pown` pour NEON (`vfmaq_f64`, fusionné) ne gagne que 1,03 à
   1,07 sur `pow(x, 3)` avec un Apple M2 (1,30 pour les intervalles autour de 0
   dans `tests/tools/pow/bench.cpp`), 1,23 avec GCC et 0,90 avec Clang sur le
   Neoverse des jobs arm64 de la CI, au lieu du 1,5 attendu ; les rangées que
   #188 ne touche pas y bougent de 5 à 13 %. Une hypothèse, non vérifiée : les
   deux voies scalaires s'exécutaient déjà côte à côte, la chaîne de produits
   dépendants restant la même. *À décider* : chercher (compter les cycles,
   essayer de couper la chaîne en démarrant la boucle des carrés plus tôt), ou
   laisser tel quel.

## PH. Les signalements à d'autres projets [attend l'accord du mainteneur]

Chaque projet a son issue, avec ses brouillons en anglais, ses vérifications à
refaire avant l'envoi et ses décisions ; les issues « À communiquer… » ne sont
pas touchées par ce ménage. **Rien n'est envoyé**, et rien ne l'est sans
l'accord explicite du mainteneur. Ordre : ce qui est prêt et qui a une valeur
pour GAOL, puis ce qui demande une réplique à écrire, puis ce qui ne demande
probablement rien.

1. **#125 CORE-MATH** : huit correctifs de `3rd/README.md` à refaire sur le
   master du jour (le 1 entre en conflit avec des changements récents de
   `sinh.c` et `cosh.c` ; le 8, `exp_1()`, fait dans la copie de GAOL par #144,
   s'applique tel quel au master du 19 octobre, `6fc21797`), et des défauts sans
   correctif (`atan2l.c`, la borne de `ss` de `asinpi_acc()`, `return` d'une
   expression `void` dans `erfc.c`, `erfc` et `__builtin_ldexp()`) ; qui envoie
   et comment (merge request sur gitlab.inria.fr ou `git format-patch` à
   core-math@inria.fr). Après un import d'un CORE-MATH d'après `c01f08c5` :
   retirer le contournement du `<` de `cr_pow` (`pow_rounded_up()` et
   `pow_rounded_dn()`). Deux défauts de plus, que GAOL contourne sans toucher
   aux sources (*décidé le 24 octobre*, #184), sont à proposer aussi, pas encore
   écrits dans `3rd/README.md` : `cr_pow()` calcule `rh * rh` (rh = y log(x))
   pour effacer un sous-dépassement parasite, et ce carré déborde à partir de
   |rh| = 2^512 (`FE_OVERFLOW` pour une puissance qui s'annule, avec Clang et
   avec GCC sans optimisation ; correctif d'une ligne, `fabs(rh) < 0x1p-511`),
   que `pow_rounded_up()` évite pour une puissance inférieure à 1 et |y| au
   moins 2^450 ; `cr_atan()` calcule `x + x` pour un NaN avant le test qui le
   garde, ce qui lève `FE_OVERFLOW` à partir de |x| = 2^1023 avec Visual C++
   (`/fp:strict`), que `atan_lo()` et `atan_hi()` évitent en rendant les bornes
   de pi/2 à partir de |x| = 2^54. Un import de CORE-MATH qui reprendrait les
   deux permet de retirer ces contournements.
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
PA à PH faites ou décidées (PA l'est), et avant les mesures de PJ.

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
   à #174, #176 et #180 à #184 (celle de #176 dit : rien). Les points K, L et
   `make distclean` (#72 à #75) n'ont pas cette section : à écrire d'après leurs
   descriptions. À écrire d'après les commits, sans pull request : les
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
   que lisent les sources de GAOL » reste à corriger ici. *Décidé le 3 octobre*
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
   ou `make perf`), puis reprendre le tableau de `doc/compare/README.md`. Les
   temps pris sur une machine non au repos (#158, les racines d'un x
   sous-normal) sont à refaire ici si on les cite.
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
