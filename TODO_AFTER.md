# Reporté

Tout ce qui est reporté sur GAOL v5 au 27 octobre 2026 (commit `9deb2e1` de
`configure-clean`) : ce qui attend la version v5.0.0, ce que le mainteneur a
reporté indéfiniment, ce qu'il a écarté en laissant une porte ouverte, et ce qui
attend du matériel. Ce fichier et l'issue #178 ont le même contenu. Ce qui reste
à faire avant la version est dans `TODO.md` (issue #179), ce qui est fait et
décidé dans `TODO_DONE.md` (l'issue #177 n'est plus mise à jour).

Rien de ce fichier n'est à faire avant v5.0.0, ni sans l'accord du mainteneur.
Après v5.0.0, le développement continue dans `configure-clean` et `master` ne
reçoit que les versions ; pas de GitHub Release, l'étiquette suffit (*décidé le
3 octobre*). Les points gardent leur lettre et leur ancien numéro (voir
`TODO_DONE.md`, « Comment lire ce fichier »).

## 1. Après la version

1. **113 (AJ)** [L] : se passer du changement du sens d'arrondi. *Reporté après
   la version le 22 octobre (Q7)* : l'étude peut changer ce que la bibliothèque
   promet (le sens d'arrondi du programme entre `gaol::init()` et
   `gaol::cleanup()`) et les noyaux, donc les temps de la tâche PJ (point 33) ;
   le comportement actuel, documenté, reste celui de v5.0.0. Question du
   mainteneur, le 18 octobre : avec les algorithmes de base d'aujourd'hui, le
   changement du sens d'arrondi est-il toujours nécessaire, les calculs ne
   seraient-ils pas valables sans lui, et dans quels opérateurs ou fonctions
   élémentaires l'est-il ? Ce qui suit vient de la lecture du code et de la
   documentation ; rien n'a été écrit ni mesuré.
   - **Étudier des noyaux qui laissent le sens d'arrondi au plus proche.** Les
     algorithmes actuels supposent l'arrondi vers le haut, et ne sont pas
     valables au plus proche : les noyaux SSE2 calculent `<-inf, sup>` par un
     seul arrondi vers le haut (`_mm_add_pd`, `_mm_mul_pd`, `_mm_div_pd` de
     `gaol/core/gaol_interval_sse.cpp`), et `sup` pourrait alors passer sous la
     valeur exacte ; CORE-MATH lit le sens dans MXCSR, `f_up(x)` y est la borne
     supérieure et `previous_float(f_up(x))` l'inférieure
     (`gaol/gaol_double_op.h`). Ce qu'il faut, c'est un arrondi dirigé, non un
     changement de sens :
     - sur le chemin AVX-512 (`GAOL_PREFER_AVX512`), `+ - * / sqrt sqr` et les
       puissances entières n'en changent pas, l'arrondi étant dans l'instruction
       (`er_mul2`, `_mm_div_round_sd`, `pown_large_er`) ;
       `GAOL_PRESERVE_ROUNDING` y coûte 1,0 à 1,5 fois
       (`doc/compare/performance.md`) ;
     - sur le chemin SSE2 seul, le sens vers le haut est indispensable, et
       `GAOL_PRESERVE_ROUNDING` y coûte deux écritures de MXCSR par opération
       (`sqr` : 8,1 ns contre 2,1 avec AVX-512) ;
     - les fonctions élémentaires et `pow` le demandent partout, CORE-MATH ne
       prenant pas le sens en paramètre (voir le 112 en 3).
   - **Ce que donnerait le sens laissé au plus proche**, par des formules
     connues qui ne sont ici ni écrites ni testées :
     - `+` et `-` : TwoSum donne l'erreur exacte, hors dépassement, d'où les
       mêmes bornes que maintenant, bit à bit ;
     - `*` : l'erreur `fma(a, b, -p)` est exacte hors sous-dépassement, où il
       faut mettre à l'échelle ;
     - `/` et `sqrt` : le reste `fma(-q, b, a)` ou `fma(s, s, -x)` donne de quel
       côté est le résultat arrondi ;
     - les fonctions élémentaires : seules des bornes valables mais plus larges,
       `[previous_float(r), next_float(r)]`, 2 ulp au lieu de 1 (c'est le
       `namespace nearest` de `gaol/gaol_double_op.h`, qu'aucune opération
       n'emploie) ; le signe de l'erreur de CORE-MATH n'est pas exposé, et le
       modifier irait contre la règle de garder `3rd/math-core` proche de
       l'amont ;
     - `pown` : le dernier `fma(K, h, l)` est un arrondi vers le haut, à
       remplacer par `next_float`.
     - À l'inverse, `midpoint`, `chi` et l'écriture des intervalles passent déjà
       par `round_nearest()`.
   - **À mesurer** : le gain, qui est que les `double` du programme ne sont plus
     arrondis vers le haut de `gaol::init()` à `gaol::cleanup()` (le tableau de
     `doc/using.md`) et que `GAOL_PRESERVE_ROUNDING` devient inutile, mince avec
     AVX-512 et réel sur les machines sans (SSE2 seul, FPU) ; et le coût, ces
     noyaux faisant plus d'opérations que la sonde et l'addition vers le haut du
     build par défaut, donc probablement plus lents. Un prototype de `+ - * /
     sqrt` au plus proche avec FMA, comparé aux noyaux actuels en résultats (bit
     à bit, `tests/rounding_direction.cpp`) et en temps (`doc/compare`), suffit
     à décider ; les fonctions élémentaires restent le point dur.
   - **Sur arm64**, #146 a ramené `GAOL_PRESERVE_ROUNDING` de 39 à 27 ns par
     opération sur un M2 : la vérification du sens est un `mrs` (0,8 ns, sans
     indicateur levé), mais deux `msr` qui changent le sens restent (11 ns
     chacun, le plancher), et `x.midpoint()` (28 ns) en paie deux aussi. Passer
     sous 5 ns demande de ne pas écrire FPCR : les transformations exactes à
     base de FMA ci-dessus, ou un noyau qui calcule vers le plus proche ; à
     étudier avant de s'y engager, les bornes étant en jeu.
2. **70 (AK)** [M] : un site de documentation sur GitHub Pages, *décidé le 3
   octobre* : un site généré depuis `doc/*.md`, avec le PDF du manuel, publié
   par un workflow, sans la référence HTML de Doxygen (supprimée par #29). Sorti
   du point I le 18 octobre, à la demande du mainteneur : le site vient après
   les textes de I, qu'il publie et qui sont fusionnés (#140). Sans dépendance,
   mais l'annonce étant privée, rien ne l'exige pour la version.
3. **28 (X)** [L] : des en-têtes optionnels au-dessus du cœur scalaire pour
   YalAA et VNODE-LP. *Décidé le 3 octobre* : des traits GAOL pour YalAA et un
   backend intervalle pour VNODE-LP, et non la promotion en `gaol/box.h`,
   `gaol/dual.h` et `gaol/affine.h` des `examples/box.h`, `examples/dual.h` et
   `examples/affine.h` (un type boîte, la différentiation directe, les formes
   affines, dont se servent les exemples 03 à 14 et qui ne sont pas installés).
   Le plan détaillé est dans `todo-notes/orchestration/points_w4.py` (son plan
   du 28 reprend la promotion des en-têtes, que la décision du 3 octobre écarte
   : la décision l'emporte). Après la version.
4. **Le ménage de la fin** : les images Docker de #111 (`i386/debian:trixie`,
   `debian:trixie`, `ubuntu:24.04`) restent sur la machine ; `todo-notes/`
   (ramené de la branche `todo-status`, supprimée ; ses rapports sont retirés de
   l'arbre, ils restent au commit `16a2f60`, et ce qui en sert encore est dans
   `todo-notes/synthese.md`) est à vider après la version, avec ce qu'il reste
   des registres `TODO.md`, `TODO_DONE.md` et `TODO_AFTER.md` et de leurs issues
   #177 à #179, qu'il faudra alors fermer ou garder.

## 2. Reporté indéfiniment

1. **26 (W)** : des décorations (issue #67). GAOL n'a que des intervalles nus
   (clause 11 d'IEEE 1788-2015) : `sqrt` d'une boîte négative est vide et passe
   tous les tests d'inclusion, `interval(DBL_MAX*10)`, `x + INFINITY` et `x *
   NAN` sont vides sans rien dire, et `gaol_ieee1788::textToInterval` rend
   l'ensemble vide pour un texte mal formé, qu'on ne distingue pas de `[empty]`.
   Le choix (issue #67) : les décorations d'IEEE 1788 (plus complet, mais change
   l'interface et le coût) ; un simple indicateur local au thread, positionné
   quand un argument sort du domaine (c'est le découpage prévu dans
   `todo-notes/orchestration/points_w4.py`) ; ou seulement documenter (les
   pièges de « Common errors », fait par #140). Reporté le 3 octobre, après la
   version. *Décidé le 22 octobre (Q5)* : rien pour l'instant, GAOL reste un
   intervalle nu et les pièges sont documentés (#140) ; reporté indéfiniment.
   L'issue #67 reste ouverte.
2. **Le contrôle de l'assembleur de x86-64 dans la CI** (#207) : le `--selftest`
   de `tests/tools/asm-order/check_order.py` tourne dans la CI (job
   `asm-order-selftest` de `linux.yml`), pas le contrôle des compilations. Un
   job de plus qui compile les sources avec `-S` en GCC 15 à `-O3` avec
   `GAOL_PRESERVE_ROUNDING` (une quinzaine de secondes) le ferait ; reporté, à
   faire si le mainteneur le veut.

## 3. Écarté, à rouvrir si une condition change

1. **Le noyau mixte du produit des intervalles FPU** (#168, fermée sans fusion
   le 23 octobre). Un noyau qui garde le chemin à une branche quand les deux
   opérandes ont le même signe (deux produits) et le noyau à huit produits sinon
   : l'essai est un changement de `mul_finite()` dans
   `gaol/core/gaol_interval_fpu.cpp` (commit `f86ada7`, lisible dans #168). En
   local (i7-1185G7, médianes de 5 passes alternées) la ligne de 7 `*` gagne
   1,44 fois avec GCC 15.2 mais `x * y` de signes aléatoires, `mul8` et
   `mix_tree` perdent 11 à 24 %, et avec Clang 21 rien ne gagne. Le banc A/B de
   la CI (base / PR, médianes de 3 passages) : la ligne de 7 `*` gagne 1,96 fois
   (GCC x86-64), 1,70 (arm64 GCC), 1,46 (Visual C++ arm64), 1,67 (clang-cl
   arm64) et 1,43 (armhf), et `x * y` de signes aléatoires perd 5 à 19 % (0,89
   GCC x86-64, 0,93 Visual C++ arm64, 0,81 clang-cl arm64, 0,91 armhf), sauf
   arm64 GCC (1,30) ; rien avec Clang x86-64 (0,90 la ligne de 7 `*`) ni Visual
   C++ x86 (0,95). Le mainteneur l'a fermée sans fusion, ces tableaux lus. *À
   reprendre si les signes réguliers deviennent le cas qu'on veut servir.*
2. **112 (AI), les fonctions élémentaires avec `GAOL_PRESERVE_ROUNDING`** :
   elles paient deux écritures de MXCSR, jusqu'à 27 % de plus que sans (`exp`),
   parce que CORE-MATH calcule avec le sens matériel et qu'aucun arrondi
   embarqué n'y est possible. Un mode de CORE-MATH où le sens serait un
   paramètre les supprimerait, au prix de modifier `3rd/math-core`. *Refusé le
   22 octobre (Q15c)* : `3rd/math-core` reste proche de l'amont. À rouvrir
   seulement si cette règle change, ou avec l'étude 113 (en 1).
3. **Le quotient NEON par deux divisions scalaires** (#195, fermée sans fusion
   le 25 octobre). Un essai du `div_bounds()` de NEON avec deux divisions
   scalaires à la place de la division vectorielle, pour chercher pourquoi `x /
   y` de clang-cl est plus lent en NEON qu'avec le FPU (#192) : plus lent
   partout, de 5,36 à 6,17 ns avec clang-cl et de 5,40 à 6,30 avec Visual C++
   sur le Neoverse N2 des runners arm64 de GitHub, de 2,38 à 2,76 ns sur l'Apple
   M2 (chiffres dans `doc/simd.md`). La branche est supprimée. *À rouvrir*
   seulement si un processeur ARM 64 bits divise nettement plus vite en
   scalaire.
4. **Des drapeaux propres à la tranche x86_64 d'un build universel de macOS**
   (#194). Cette tranche n'a pas le fusionné multiplie-addition (`-mfma` échoue
   pour la tranche arm64) : CORE-MATH y appelle le `fma()` de la bibliothèque de
   macOS et les puissances entières n'ont pas le noyau empaqueté. *Décidé le 25
   octobre* : le documenter seulement (`doc/building.md`, ligne `GAOL_FMA`),
   sans mesurer ce que cela coûte ni donner `-mfma` à cette seule tranche
   (drapeaux par architecture de Xcode, ou `-Xarch_x86_64`). *À rouvrir* si les
   builds universels deviennent courants ou si la perte, mesurée sur un Mac
   Intel, est grande. Le Rosetta de macOS 14 n'a pas AVX : une tranche compilée
   avec `-mfma` s'y arrêterait sur une instruction illégale.
5. **mingw-w64 11 lié à `msvcrt.dll`, sans FMA, sur x86 32 bits** (#205). Son
   `fma()` est le même code (`math/fma.c`, inchangé de la version 9 à la 13) que
   celui que `gaol_config.h` refuse depuis #205 pour la version 12 ou plus ; la
   11 reste acceptée sans FMA parce que le job « MinGW-w64 13.2.0 x86 Release,
   no FMA » de `windows.yml` (#172) la teste, avec la mesure : 125 résultats sur
   400 000 à un double près (`TODO_DONE.md`, section 3, réponse 6). C'est la
   question ouverte de #205, fusionnée sans réponse écrite. *À rouvrir* si le
   mainteneur veut la même règle pour la 11 : la condition de `gaol_config.h`
   vaut alors pour la 11 et plus, et le job « no FMA » est retiré.

## 4. En attente de matériel ou d'un environnement qui manque

1. **Les pistes de `doc/simd.md`** (#154, qui répond aux quatre étapes de
   l'issue #101, fermée) : le RISC-V scalaire à `rm` statique (il faut une
   carte), POWER et s390x (du matériel) ; l'arrondi embarqué scalaire pour `/`
   sur les processeurs Intel qui divisent les registres de 512 bits est fait par
   #191 (voir 4.6 pour d'autres processeurs). Et les gains sans nouveau jeu
   d'instructions que la page liste : `sqrt` en SSE2, les puissances entières en
   NEON (fait par #188 : 1,03 à 1,07 sur un M2), `mid()`, la réduction
   d'argument de `sin` et `cos`. Une voie AVX2, SVE, VSX ou RISC-V seulement
   après des mesures.
2. **ARM 32 bits sur du vrai matériel** (#150) : les mesures de #150 ne valent
   que pour les runners arm64 de GitHub, qui exécutent l'armhf en natif
   (Neoverse N2, « 32-bit EL0 Support » au démarrage, aucune entrée `qemu-arm`,
   ce que montre le job jetable du 20 octobre) ; le coût de `vmrs` et de `vmsr`
   sur un Cortex-A 32 bits, et le gain, ne sont pas connus. Non vérifiés :
   bionic (le `static_assert` de `gaol_fpu_fenv.h` sur les valeurs de
   `FE_UPWARD`, `FE_DOWNWARD` et `FE_TOWARDZERO` le dirait à la compilation, les
   sources n'en ont pas été lues) et GCC avant 12. Clang reste refusé sur ARM 32
   bits.
3. **Le maximum de `xsmaxcdp` sur un POWER9 réel** (#157) : son résultat pour
   deux zéros de signes opposés n'est lu que dans la source de QEMU
   (`VSX_MAX_MINC`) et dans un commentaire de GCC, ni dans l'ISA ni sur un
   processeur ; la phrase de `doc/using.md` (« premier opérande ») est celle de
   QEMU.
4. **Ce qui n'a pas été exécuté** (#157, #162) : l'exécution des noyaux FPU sur
   ARM 32 bits, POWER, s390x et RISC-V (la CI les lance sous qemu pour POWER9,
   s390x, ppc64le et riscv64, dont les temps ne sont pas ceux d'un processeur)
   et leur vitesse ; le noyau SSE2 (comparé à un modèle, jamais exécuté) ; pour
   #162, toute exécution sur x86 (le signe d'un zéro de l'ancien `pow` avec
   SSE2, le chemin AVX-512), avec GCC et sous Windows ; les temps de #162
   viennent d'un seul M2 partagé, et pas avec `GAOL_PRESERVE_ROUNDING`.
5. **`FPCR.AH` (FEAT_AFP, Armv8.7) sur un vrai processeur** (#187, issue #198) :
   le bloc de `tests/rounding_direction.cpp` qui met `AH` et compare les
   opérations avec `AH` mis et effacé n'avait été exécuté que sous QEMU 10.0
   (`-cpu max`) : l'Apple M2 et QEMU 7.2 et 8.2 ne retiennent pas le bit. Le 25
   octobre, un runner arm64 de la CI (le job « Ubuntu 26.04 arm64 Clang » de
   #196, vert à la relance) l'a retenu, et le bloc y a trouvé 52 échecs sur 441
   paires de `x / y` (des bornes différentes avec `AH` mis, dont une borne
   inférieure NaN pour `x = [-inf, 0]` et `y = [-inf, -0]`) : le défaut n'est
   plus théorique pour `x / y`, et aucun `vmaxq_f64` ne l'explique, car
   `div_bounds()` n'en a pas. Cause non trouvée, décidé le 25 octobre de n'en
   faire qu'une issue (#198), sans tâche. Revenu le 28 octobre sur un runner
   Windows arm64 (les jobs « MSYS2 CLANGARM64 Clang arm64 » de #215, run
   40672302929, image `windows-11-vs2026-arm64` 20261025.196.1) : les mêmes 52
   échecs sur 441 et les mêmes cinq cas, avec le Clang de MSYS2 ; ils sont
   repassés au vert à la relance, sur l'image 20261022.194.1, sans que le
   journal dise si `AH` y était retenu (commentaire de l'issue #198). *À
   reprendre* en faisant afficher au test le processeur du runner et si `AH` est
   retenu, puis en reproduisant sous QEMU 10 ou sur un processeur qui a FEAT_AFP
   (Neoverse V3 de Graviton 5, ou plus récent). Le défaut de `sqrt_bounds()` que
   #187 a écarté (un compilateur qui permute les opérandes de `vmaxq_f64`) reste
   théorique avec les compilateurs d'aujourd'hui.
6. **Le gain de `x / d` de #191 sur d'autres processeurs** : il n'est mesuré que
   sur un Tiger Lake (i7-1185G7) ; les deux `vdivsd {ru-sae}` peuvent perdre sur
   un Intel qui divise les registres en deux, et rien n'est mesuré sur AMD Zen 4
   et Zen 5 (unités pleine largeur) ni sur d'autres Intel. Les jobs SDE de la CI
   exécutent le chemin avec Visual C++ et clang-cl x64, sans le mesurer.
7. **Ce que #192 et #193 n'ont pas pu vérifier** : ARM64EC (la condition de
   `gaol/gaol_config.h` l'exclut) ; un MinGW-w64 avec GCC pour arm64 (la CI
   construit le Clang de MSYS2 CLANGARM64, pas GCC) ; les versions de Visual C++
   que la CI n'a pas (avant 19.44) ; l'exécution de Visual C++ pour x86 hors de
   la CI (Compiler Explorer échoue avec 0xC000007B) ; la tranche x86_64 d'un
   build universel de macOS, que la session MACarm2 (Apple M2, sans Rosetta)
   compile et lie sans l'exécuter, et que seul le job `macOS 26 universal`
   exécute.
8. **Un échec de CI sous Rosetta, non reproduit** (#189, #208) : le job « macOS
   14 x86_64 (Rosetta) » a échoué une fois sur `arithmetic`
   (`gaol_ieee1788::pow(x, [2]) is sqr(x & [0, +oo]), [0] for [0], the same
   bits`, avec x = [-0x1.0d8146092a4f3p+345, 0x1.d324166d86984p+204], « the
   caller's direction downward, flush modes 0 » : GAOL a rendu [0,
   0x1.aa365774f4ea3p+409] et le calcul de référence, `sqr(x & [0, +oo])`, le
   vide) et est passé à la relance, sans lien visible avec le diff de #189. Non
   reproduit en local ni dans les autres jobs de #186 à #190. Il est revenu le
   28 octobre sur #208 (`007c1f3`, run 40668659623, job 122750002962, « macOS 26
   universal (arm64 and x86_64), meson », tranche x86_64 sous Rosetta) :
   `ieee1788` a écrit « FAILED pown(x, n) beyond the long longs: x^(2^64 - 1) as
   x^(2^63 - 1), x^(2^64 - 2) as x^(2^63 - 2) ». Ce contrôle de
   `tests/ieee1788.cpp` relie quatre sous-vérifications par `&&` et n'affiche
   que le résultat de la première, conforme à l'attendu : on ne sait pas
   laquelle a échoué. Le job est repassé à la relance, sans changement de code,
   et #208 ne touche ni `pown` ni ce test. La session MACarm3 (Apple M2, macOS
   27.0.1) a fait tourner `ieee1788` 16000 fois sans un échec, sur
   `configure-clean` 2100eb5 avec un diagnostic qui affiche les quatre
   résultats : Release `-O3` (5000 passages), Debug (5000), Release `-O0` (3000)
   et Release `-O2` (3000), sous `arch -x86_64` et en arm64 natif, seul et avec
   quatre boucles en parallèle (charge de 4 à 5). Cette machine n'imite pas le
   runner de GitHub (autre macOS, autre Rosetta, exécuteur lent) : l'échec n'est
   ni expliqué ni écarté. *À reprendre si cela revient* : mettre ce diagnostic
   dans `tests/ieee1788.cpp` (calculer les quatre intervalles avant le `check`
   et les afficher tous les quatre dans son détail), pour que le journal dise
   laquelle des quatre sous-vérifications diffère, puis voir si Rosetta lit le
   sens d'arrondi ou les modes de MXCSR comme un processeur.
9. **Un délai de test juste pour un exécuteur lent** (#215) :
   `automatic_configuration` (600 s) a dépassé son délai le 28 octobre dans le
   job « macOS 26 Xcode generator, universal (arm64 and x86_64) » de #215, sur
   un exécuteur environ 2,5 fois plus lent que d'habitude
   (`cpack_stale_configure` 254 s contre 99 s, `arithmetic` 10,8 s contre 5,4
   s) ; relancé, il a pris 256 s, comme sur un exécuteur ordinaire (259 s dans
   #208) : la marge n'est que d'un facteur 2,3. Rien n'est changé. *À reprendre
   si cela revient* : relever le délai de ce test (process.md, §10).

## 5. Une limite connue, documentée, sans décision de la lever

1. **`[l, u]` ne départage pas** pi et un autre nombre de même encadrement
   (`[pi, 3.14159265358979323846]`), un nombre loin au-delà des doubles (plus de
   10^311 ou moins de 10^-331 en décimal, environ 2^1026 et 2^-1081 dans les
   autres bases : `[1e400, 1e399]`, `[1.8e308, 1e400]`), ni des bornes calculées
   (`[1/3, 0.3333333333333333]`) : le littéral est alors l'intervalle de la
   borne inférieure de l à la borne supérieure de u, un sur-ensemble du vide,
   sûr mais pas le plus serré (dit dans `gaol_parser.h`, le manuel,
   `doc/accuracy.md` et `doc/tests.md`). Pi demanderait un encadrement intégré à
   plus de chiffres ; les nombres loin au-delà, la comparaison de leurs
   exposants et de leurs premiers chiffres. Les autres cas (deux nombres entre
   les mêmes deux doubles) sont départagés exactement depuis #149.
2. **Trois inversions du banc A/B de `cl`** (#209) : `x / y` et `sqr(x)` sont
   plus lents avec #209 (+54 et +48 ns) et `x - y` plus rapide (−41 ns) dans les
   deux jobs de Visual C++ x64 à direction préservée, sans mécanisme : leurs
   noyaux n'emploient que `GAOL_RND_ENTER_SSE()`, que #209 ne touche pas, et le
   contrôle de #211 (la base contre la base plus une fonction que rien
   n'appelle) ne les reproduit pas, mais un décalage seul fait passer
   `x.midpoint()` de 68,9 à 26,8 ns et #210 déplace une ou deux opérations de
   base de 50 ns environ : la signature d'un effet de disposition du binaire sur
   ces exécuteurs, sans preuve (un `dumpbin /disasm` des noyaux la donnerait).
   Le mainteneur a fusionné #209 avec cette réserve, sans ce désassemblage dans
   la CI.
