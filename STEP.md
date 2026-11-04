# Ce qui a été fait sur GAOL v5, par étape

Tout ce qui a été fait sur GAOL v5 du 13 septembre au 4 novembre 2026 (commit
`1e0f368` de `configure-clean`), classé en 27 étapes : les pull requests
fusionnées, celles qui sont fermées sans fusion (des essais et des mesures
jetables) et le travail direct sur `configure-clean`, regroupé par sujet. Ce
fichier classe ; il ne décrit pas. Le détail de chaque pull request (chiffres,
décisions, relectures, CI) est dans `TODO_DONE.md` (2.3 et 2.4), ce qui reste
dans `TODO.md`, `TODO_AFTER.md` et `TODO_fin.md`.

## Comment lire ce fichier

- Une pull request apparaît une seule fois, sous son thème principal ; un
  travail qui en touche plusieurs est rangé là où il change le plus. Les
  étapes vont de la sûreté des bornes à la publication, et chacune garde
  l'ordre des dates.
- La date d'une pull request est celle de sa fusion (de sa fermeture pour une
  pull request fermée sans fusion), en UTC. #15 à #24 ont été fusionnées dans
  `master` (GAOL 4.3.1), #25 à #30 vers `MATH-CORE`, les autres vers
  `configure-clean`. Les numéros de #1 à #290 qui n'apparaissent pas sont des
  issues.
- Le travail direct est donné par groupes de commits d'un même sujet, du premier
  au dernier (ils ne sont pas toujours consécutifs). Les 162 commits qui ne
  touchent que les registres (`TODO.md`, `TODO_DONE.md`, `TODO_AFTER.md`,
  `process.md`) ne sont pas listés : ils sont à l'étape 27.
- La colonne « Points » donne les lettres et les anciens numéros de
  `TODO_DONE.md` (sections 4 et 5) que l'étape contient, et les tâches (PA à
  PL) qui y ont travaillé.
- Les résumés viennent de la lecture de chaque pull request le 4 novembre ; ils
  sont courts et ne remplacent pas les entrées de `TODO_DONE.md`.

| N° | Étape | Période | Fusionnées | Fermées | Points |
| --- | --- | --- | --- | --- | --- |
| 1 | La fondation (septembre) | 13 septembre au 28 septembre | 10 | 0 | revue d'origine (4 à 40) |
| 2 | Les exceptions flottantes | 29 septembre au 4 novembre | 7 | 0 | 24, 24b, D (24, 62), AB (77 à 80), PK |
| 3 | Le sens d'arrondi | 27 septembre au 29 octobre | 9 | 0 | 4, H (23, 36, 48), Q (4, 45), 107 à 109, PF, PG.10 |
| 4 | Les configurations refusées | 29 septembre au 19 octobre | 4 | 0 | 5, 6, G (5, 40, 53 à 55), 90 |
| 5 | CORE-MATH | 30 septembre au 3 novembre | 7 | 0 | A (31, 6, 56, 3, 47) |
| 6 | Puissances et racines | 3 octobre au 23 octobre | 9 | 0 | 1, 2, 3, 8, B (51, 72 à 74), D.21, AC.81 |
| 7 | Autres fonctions | 29 septembre au 25 octobre | 6 | 0 | 7, 9, 10, M (9, 57), R (7, 50) |
| 8 | IEEE 1788-2015 | 29 septembre au 4 novembre | 6 | 0 | 1, P (25, 30), AC.82 |
| 9 | Lecture et écriture | 26 septembre au 24 octobre | 15 | 0 | 11, 13, 15, 16, 18, C, E, F, AD (83 à 89, 114), 128 |
| 10 | Interface et avertissements | 27 septembre au 26 octobre | 11 | 0 | J (19, 20, 68, 16), Z (75), V (22), D, AO, AM, 131 |
| 11 | Exceptions C++ | 29 septembre au 4 novembre | 4 | 1 | 14, S, PL |
| 12 | Les trois builds | 26 septembre au 27 octobre | 11 | 1 | 42, O (41, 42, 52, 65), AE (90 à 95), 112, PE |
| 13 | Installation | 29 septembre au 6 octobre | 5 | 0 | 43, 44, N (44, 63) |
| 14 | La CI | 29 septembre au 1er novembre | 10 | 2 | 29, L (29, 43, 69), PE.3, PE.4 |
| 15 | Windows | 2 octobre au 28 octobre | 9 | 0 | K (49, 47), 110, 133, PF.4 |
| 16 | SSE2 et AVX-512 | 5 octobre au 25 octobre | 8 | 0 | 27, AA (76), ZZ, 106 |
| 17 | NEON | 19 octobre au 25 octobre | 4 | 1 | 106, 107, 129, 132, PD |
| 18 | FPU et sans SIMD | 20 octobre au 22 octobre | 2 | 0 | 106 (FPU, ARM 32 bits) |
| 19 | PG : GCC, Clang, placement | 23 octobre au 31 octobre | 8 | 14 | PG, 116 |
| 20 | PG : noyaux NEON | 25 octobre au 30 octobre | 8 | 10 | PG.7, PG.8 |
| 21 | Banc de performance | 3 octobre au 30 octobre | 8 | 2 | AG (104), 111, 115, PG.4, PG.9 |
| 22 | Contrôle de l'assembleur | 28 octobre au 29 octobre | 4 | 0 | PF.5 |
| 23 | Les tests | 29 septembre au 3 novembre | 13 | 0 | U (40, 64), PD.7, PE.6 |
| 24 | Documentation et manuel | 30 septembre au 25 octobre | 6 | 1 | 39, I (35, 37, 38), AF (96 à 103), T, AL, AN (126) |
| 25 | Synthèse de la version | 30 octobre au 2 novembre | 14 | 0 | Y, PI |
| 26 | Signalements préparés | 18 octobre | 0 | 0 | 31, PH (en attente) |
| 27 | Registres et procédure | 30 septembre au 3 novembre | 7 | 0 | 118, 123, 130 |

En tout : 205 pull requests fusionnées et 32 fermées sans fusion, plus
deux ouvertes le 4 novembre (#289 et #290, à la fin des étapes 25 et 8).
`configure-clean` a 1963 commits, dont 704 en premier parent : 198 fusions et
506 commits directs, dont 162 de registres.

## 1. La fondation de GAOL v5 (13 au 28 septembre)

Avant la procédure du 30 septembre. GAOL 4.3.1 d'abord (#15 à #24, dans
`master`) : CORE-MATH pour les fonctions hyperboliques, les puissances entières
par produits exacts, les racines prouvées, les réciproques trigonométriques.
Puis, directement sur `configure-clean`, la base de GAOL v5 : CORE-MATH à la
place de mathlib pour toutes les fonctions élémentaires, les trois builds à
configuration identique et leur audit, la CI de GitHub Actions, le refus des
configurations qui ne respectent pas le sens d'arrondi, les fonctions de la
norme IEEE 1788-2015 et l'espace de noms `gaol_ieee1788`, le manuel de v5, et la
revue d'origine du 27 septembre (`examples/examples.md`), d'où viennent les
anciens numéros 4 à 40. Les pull requests #25 à #30 de la même période sont
rangées dans leurs étapes.

Pull requests :

- #15 (18 septembre) : back end libm (GAOL 4.3) : `round_down/round_up` justes
  aux infinis ; exp, sinh, cosh au débordement et log([0,1]) corrigés (issue
  #14).
- #16 (18 septembre) : quatre jobs de CI avec CRlibm et la libm du système ;
  gaol.pc donne l'en-tête de CRlibm ; tolérance d'atan2 (issue #12).
- #17 (18 septembre) : sin, cos, tan : signes exacts de la dérivée pour extrema
  et pôles, à un double près à toute magnitude (issue #6, début).
- #18 (18 septembre) : le manuel de GAOL 4 compile (bibentry au lieu de
  marginbib) ; gaol.pdf reconstruit ; cible pdf de meson ; workflow manual.yml
  (issue #13).
- #19 (18 septembre) : fonctions hyperboliques de CORE-MATH, correctement
  arrondies puis écartées d'un double, sans hypothèse sur la libm (issues #1,
  #11).
- #20 (18 septembre) : `nth_root` prouvée par des puissances entières
  arrondies : à un double près pour tout n, racines exactes (issue #7, première
  moitié).
- #21 (18 septembre) : puissances entières par produits exacts (fma) : bornes
  les plus fines ; `pow(x,3)` plus lent sans FMA (issue #7, seconde moitié).
- #22 (18 septembre) : `acos_rel`, `asin_rel`, `atan_rel` : k·π en
  double-double, un seul algorithme `periodic_rel`, cas d'un double décidés (fin
  de l'issue #6).
- #23 (18 septembre) : doc/compare refait sur master après les fusions ;
  PROFIL/BIAS ajouté comme cinquième bibliothèque.
- #24 (18 septembre) : GAOL 4.3.1 : FMA et -O3 partout, log de CORE-MATH vers le
  haut, doc/compare refait avec Clang 18.

Travail direct sur `configure-clean` :

- 10 février 2019 au 15 décembre 2020, `5469afd` à `343fcc0` : commits de GAOL 4
  avant le fork : README, licence LGPL, gdtoa retiré, spécifications
  d'exceptions retirées pour C++11/17.
- 13 septembre, `5894c48` à `d30ef8c` : FPU initialisé sans écrire le mot de
  contrôle x87, arrondi vers le haut rétabli après `midpoint()` et <<,
  `opposite()` renommé `gaol_opposite()` (correctif d'IBEX).
- 13 septembre, `9043b0f` à `abf357a` : compilation avec Visual C++, MinGW-w64
  13 et ARM 32 bits ; Clang refusé sur ARM 32 bits.
- 13 septembre, `6453e65` à `eef36ee` : build CMake exportant `gaol::gaol` ;
  meson aligné sur configure (`GETRUSAGE_IN_HEADER, USING_SSE3_INSTRUCTIONS`).
- 13 septembre, `3257c31` à `fdabc12` : bornes sûres quelle que soit la
  bibliothèque C : sqrt, lecture exacte des nombres, valeurs de la libm des
  fonctions hyperboliques écartées de trois flottants.
- 13 septembre, `0e2504f` à `bb55f8e` : tests des bornes contre les résultats
  exacts (ctest), tolérances au double des plus grands écarts observés.
- 13 septembre, `527b86f` à `dde06ad` : workflows GitHub Actions (Linux,
  conteneurs, macOS, Windows, autotools, meson) ; le README montre la
  documentation du fork.
- 14 septembre, `60ae203` à `bc91b2c` : refus des compilateurs qui n'honorent
  pas le sens d'arrondi (Clang 14 arm64) et de MinGW-w64 avant 12 ou 12,
  vérifiés par la CI.
- 14 septembre, `a4825db` à `af423cc` : `pow(interval, e)` d'un exposant
  flottant sans troncature, partie négative de la base gardée ; tests repris de
  Codac.
- 14 septembre, `a847332` à `3b51fbe` : corrections de mathlib : `mpcos()`,
  `fastiroot()`, `ulog()` aux sous-normaux, `utan()`.
- 14 septembre, `0973713` à `41f60fb` : option `GAOL_FIND_MATHLIB` et sa
  vérification en CI ; mathlib liée à la bibliothèque partagée d'autotools.
- 14 septembre, `ee9791a` à `083f0ca` : avertissements supprimés :
  `s_nextafter()` par memcpy, stdint.h avant flex, const `char*` de bison,
  `NaN_val` const, CVSid, -Wconversion.
- 14 septembre, `31df918` à `8c9d20e` : `interval2f::set_contains()` corrigé, kl
  et kr `d'atan_rel()` initialisés, `hausdorff()` au plus serré (issue #9).
- 14 septembre, `b23a62b` à `d90d10f` : sens vers le haut mis par chaque
  opération, sonde 1 + 2^-60, registres x87 et SSE écrits directement, aussi
  avec Visual C++.
- 14 septembre, `584b29e` : tests/performance.cpp : temps par opération de GAOL
  et des doubles, lancé par la CI.
- 14 septembre, `7ccabfd` à `429a9b9` : trois builds à configuration identique :
  -ffp-contract=off, -ffloat-store sur x87, release par défaut, GAOL silencieux,
  tests non construits par défaut, audit de CI.
- 14 septembre, `ac6f6e3` à `4224855` : CI : échecs des tests affichés, jobs
  d'accord des builds qui échouent vraiment, curl dans les conteneurs Debian.
- 14 septembre, `2de1f05` à `e0f5234` : Windows : intervalles SSE2 écartés en 32
  bits avec configure, DLL dans le PATH sous MSYS2.
- 14 septembre, `69fefef` : gaol.pc installé par les trois builds, avec les
  drapeaux de l'arithmétique d'intervalles et mathlib.
- 14 septembre, `3db6278` à `6fcbe88` : README réécrit : les trois builds et
  leurs options, install-mathlib.sh, la configuration commune ; Frédéric
  Goualard nommé auteur.
- 15 septembre, `a826521` à `beed8dc` : Visual C++ refusé sans /fp:strict, son
  test `fp_strict` corrigé ; 2^-60 écrit en littéral, masque de MXCSR non signé.
- 15 septembre, `5577a8e` à `a6eec4f` : README et commentaires corrigés : ce que
  `gaol_config.h` refuse, gaol.pc, `GAOL_USING_ASM`, bornes hyperboliques.
- 15 septembre, `302e7b5` à `c6de123` : build avec la libm du système
  (--with-mathlib=m) avec avertissement ; mathlib installée refusée si son
  `cos(2^52 - 1)` est faux.
- 15 septembre, `4c16b77` à `6efb439` : `pow(interval, double)` dans libgaol,
  cas particuliers de pow d'IEEE 1788-2015, FIXME de pow retiré.
- 15 septembre, `a9b65db` à `e00aaeb` : FIXME de GAOL 4 traités : `fesetenv()`
  et masque du FPU expliqués, test du vide des opérandes en SSE2 essayé, non
  gardé.
- 15 septembre, `e6c4b0b` à `4f68bdc` : `mid()` et `midpoint()` arrondis comme
  IEEE 1788 (aussi intervalf), moitiés de `mid()` protégées par `rnd_keep()`
  sous Visual C++.
- 15 septembre, `d89b297` à `c6ad0ca` : opérations avec un double et zéro fois
  infini alignées entre SSE2 et FPU ; échec `d'acos_rel()` sous AArch64
  expliqué.
- 15 septembre, `e403c4d` : intervalf et interval2f compilés seulement sur
  demande (`GAOL_FLOAT_INTERVALS`).
- 15 septembre au 16 septembre, `ddfca98` à `91bf553` : noms d'auteurs sans
  accents, puis le mot Frederic laissé dans `gaol_expression.h` retiré.
- 16 septembre, `a694cdc` à `17f7662` : conformité à IEEE 1788-2015 : vide pour
  un non-intervalle, rootn, sqrt au plus serré, `exp(0)` et `log(1)` exacts,
  precedes, rad, valeurs exactes, `pow(x, -n)`.
- 16 septembre, `5e5ba1d` : doc/ renommé manual/, README découpé en `doc/*.md`,
  doc/compare ajouté.
- 16 septembre, `6cfe422` à `1d5675b` : doc/compare : filib++ ajouté,
  comparaisons et temps refaits après les corrections.
- 16 septembre, `c80c7bd` à `5fc154c` : littéraux d'IEEE 1788-2015 lus ; le
  parser libère ses nœuds (issue #4).
- 16 septembre, `58e9013` à `3b63390` : `sin()` pris à ses bornes comme
  `cos()` ; sens au plus proche réglé une fois pour les deux bornes des
  fonctions élémentaires.
- 16 septembre, `0536f71` à `98055b6` : précision de chaque opération documentée
  (12.10.3), le README dit suivre IEEE 1788-2015.
- 16 septembre, `28ffb61` : `uipow()` déclarée et exportée aussi avec les
  intervalles SSE2 (issue #10).
- 17 septembre, `8902a9c` à `6b4ab82` : sources de mathlib 2.1.1 dans
  3rd/mathlib, construites par les trois builds, corrigées
  (`FENV_ACCESS, halfulp() de l'issue #5`).
- 17 septembre, `142d77d` à `42a8114` : documentation recalée : nœuds libérés,
  configurations refusées listées, `atan2()` dans le manuel.
- 17 septembre, `f98fd33` à `0db08b6` : banc de doc/compare relancé avec la
  mathlib de 3rd/ et après pow aux coins.
- 17 septembre, `a37994a` à `b818570` : écriture décimale arrondie vers
  l'extérieur quelle que soit la bibliothèque C (issue #3) ; format agreeing qui
  garde les chiffres.
- 17 septembre, `1bcda57` : `atan2(y, x)` d'IEEE 1788-2015 implémenté (issue
  #2).
- 17 septembre, `47dadd3` : `pow(I, J)` aux coins de la boîte, à un double du
  plus serré (issue #8).
- 17 septembre, `0a2e4cf` à `f4e29c9` : code indépendant de la position par
  FetchContent ; GAOL 4.3.0 dans les trois builds.
- 19 septembre, `6a5af04` à `e723a14` : -mfma seulement où la machine l'exécute
  (Rosetta), intersection disjointe vide, GAOL 4.3.2.
- 20 septembre, `2a437a7` à `fc47222` : fonctions relationnelles corrigées
  (`div_rel, invabs_rel, asin, acos, atan`) et testées contre les réciproques
  d'IEEE 1788.
- 20 septembre, `d6c8ac1` à `fffb862` : CORE-MATH remplace mathlib pour toutes
  les fonctions élémentaires, bornes au plus serré, cbrt ; `GAOL_LTO` ajouté
  puis retiré.
- 20 septembre, `53672a6` à `fe8f3e1` : option de couverture et page de
  conclusions dans coverage/ (87 %).
- 20 septembre, `1ad8ee9` à `c45ee95` : CI : anciens MinGW-w64 avec entier de
  128 bits émulé, défauts trouvés corrigés (`fuite, _umul128, roundeven`).
- 20 septembre, `0525841` à `cd6c469` : banc de doc/compare : GAOL V5.0.0 et
  4.3.1 côte à côte, arbres de travail ignorés.
- 20 septembre, `8af2aef` à `397f722` : mingw-w64 dont fenv.h garde un état
  propre : sens lu et écrit par fenv.h, ou refusé ; mingw-w64 plus refusé par
  `gaol_config.h`.
- 20 septembre, `cb59ec5` à `1b2c287` : exp2, exp10, log2, log10, `nth_root`
  négatif, sign, trunc, roundTiesToEven et roundTiesToAway ; vide tenu en bornes
  NaN.
- 20 septembre, `663b069` à `a541f8b` : copyright d'ENSTA et Jordan NININ auteur
  des fichiers ajoutés ; générateur de interval2f en Python 3.
- 20 septembre, `c1b6652` : référence de roundTiesToEven des tests calculée sans
  lire le sens d'arrondi.
- 20 septembre au 21 septembre, `38ca5ad` à `8bfbe4b` : la documentation, les
  tests et les commentaires disent GAOL v5 ; Jordan Ninin auteur, ChangeLog en
  ASCII.
- 20 septembre au 21 septembre, `2c3ba3c` à `e702bd8` : Windows 32 bits : trace
  des mots de contrôle, indicateurs d'exception écrits dans MXCSR,
  fesetexceptflag en static inline.
- 21 septembre, `df770bc` à `56ca043` : tableaux de performance avec Clang 18.1,
  comparés à GAOL 4.2.3 et 4.3.2.
- 21 septembre, `1ef8596` à `73d7088` : `gaol_roundeven.h` installé par
  autotools et meson, `gaol_version_mingw.h` lit la configuration, Makefile.in
  régénéré.
- 21 septembre, `38b205a` à `79cde72` : mathlib retirée des builds, de la CI et
  des textes ; configurations refusées revues avec CORE-MATH.
- 21 septembre, `ee026fe` à `89f6ea6` : le lecteur prend exp2, log2, cbrt, sign,
  trunc ; l'écriture hexadécimale relue bit pour bit.
- 21 septembre, `cd99bfc` : fins de ligne CR de `gaol_interval_fpu.h` rétablies.
- 21 septembre, `5c20af2` à `3dce391` : fonctions d'IEEE 1788-2015 (Table 10.5,
  fma, cancel...) et espace de noms `gaol_ieee1788` avec ses noms et son pow.
- 21 septembre, `aaf407a` à `58511c0` : tests : précision étendue x87, masques
  d'exception de cbrt et pow, triplets de `core_math`, références par
  `rnd_keep()` ; `coverage_summary.py`.
- 21 septembre, `9ed2aa7` à `2c267a0` : correctifs de CORE-MATH : cospi non
  signé, `gaol_u128` et son émulation, ldexp de Visual C++, GCC 14 sans SLP,
  rsqrt.
- 21 septembre, `34e068b` à `6cfd80b` : documentation des fonctions de la Table
  10.5 et des espaces de noms `gaol_core`, gaol et `gaol_ieee1788`.
- 22 septembre, `3e3459f` à `cf0f4ee` : le sens d'arrondi vérifié une fois par
  fonction ; `tan()` et `exp_dn()` le rétablissent avec
  `GAOL_PRESERVE_ROUNDING`.
- 22 septembre, `ad45633` à `e686982` : la documentation ne parle plus de
  mathlib ni de ce qui ne vaut plus pour GAOL v5.
- 22 septembre, `636daf7` à `d5a76d2` : `exact_string(I)` ; tests/ieee1788.cpp
  construit, avec les vérifications du test de using.
- 22 septembre, `77f9fc5` : TODO.md réduit au reste à faire ; INSTALL et TODO de
  GAOL 4 renvoient à la documentation de v5.
- 22 septembre, `e9b4729` : cas particuliers recalculés avec CORE-MATH : 270 sur
  286 comme IEEE 1788.
- 22 septembre, `b206db6` à `6f54d46` : une seule grammaire lit toute chaîne
  comme une expression ; exposant négatif de `nth_root` lu ; documenté.
- 22 septembre, `70efdfc` à `3422537` : manuel de GAOL 4 dans manual/v4, manuel
  de v5 écrit dans manual/v5, sans lignes qui débordent.
- 22 septembre, `56de219` à `dd3b2e6` : `GAOL_NODISCARD` ; relations possibly,
  `certainly_eq`, == et != retirées avec --enable-relations.
- 23 septembre, `03ad6d8` à `5e50444` : textToInterval lit les noms de son
  espace de noms ; `parse_interval()` sous un `std::mutex`.
- 23 septembre, `0048c79` : tests sans thread : ceux d'ieee1788 et d'expressions
  qui en lançaient sont commentés.
- 26 septembre, `d5cef71` : CORE-MATH mis à jour à l'amont 6b84457 : sin.c
  réécrit porté sur `gaol_u128`.
- 26 septembre, `f6e0af8` : `gaol::cleanup()` rétablit le sens trouvé par le
  premier `init()`.
- 27 septembre, `38ee6a7` à `d87e553` : couverture (87,2 %) et performance
  remesurées à 605728e ; .gitignore du banc.
- 27 septembre, `5b3d09c` à `424829d` : examples/ : 15 exemples autovérifiés,
  Goldstein-Price renommé ; templates/ de KDevelop retiré.
- 27 septembre, `fba22db` à `9cf6574` : FPU : x -= x, x /= x et x %= x lisent
  l'opérande avant d'écrire ; test de x op= x.
- 27 septembre, `841cdbd` à `0420e18` : operator<< et operator>> corrigés,
  expressions vides sans compte de références ; tests et documentation.
- 27 septembre, `f6a057f` à `fcaf72f` : `minimum()` et `maximum()` de deux zéros
  avec Visual C++ ; CI meson avec /MD.
- 27 septembre au 28 septembre, `29f280b` à `3ac9868` : trois builds rendus
  semblables (`gaol_configuration.h, WITH_TESTS, cibles`) ; autotools régénéré.
- 28 septembre, `46ff4ff` à `b2eb8d0` : constructeurs d'interval explicites ;
  tests passés de textToInterval à `interval(a, b)`.
- 28 septembre, `566c09f` à `5335039` : version lue de VERSION puis VERSION.txt
  par les trois builds, zéros de tête refusés.

## 2. Aucune exception flottante levée à tort

Aucune opération de l'interface ne lève d'exception flottante sur un opérande
vide, un NaN ou un résultat fini : des comparaisons silencieuses (#47, #50,
#60), le point D (#83), puis le point AB (77 à 80 : `pow` près du débordement,
`cancel_minus` et `cancel_plus`, la lecture de `1e400`, les fonctions qui
saturent). La tâche PK (un `FE_INVALID` de Clang avec les intervalles FPU) est
close sans code le 4 novembre : il ne se reproduit pas avec les drapeaux de GAOL
(`TODO_DONE.md` 3.17).

Pull requests :

- #47 (29 septembre) : `is_empty()` et `emptyset()` ne lèvent plus d'exception
  flottante ; exceptions flottantes documentées (ancien 24).
- #50 (30 septembre) : aucune `FE_INVALID` sur un opérande vide : comparaisons
  silencieuses, NaN tenus hors du constructeur (ancien 24b).
- #60 (3 octobre) : armhf : operator&= reconnaît l'opérande vide avant de
  comparer, contre le vcmpe de GCC 12 à 14 (suite du 24b).
- #83 (5 octobre) : point D (24, 62) : plus d'exception flottante parasite
  (`NaN, 0×oo, pow extrême, div_rel, midpoint`), ucomisd sous Visual C++,
  documentation.
- #144 (19 octobre) : point AB.77 : pow près du plus grand double ne lève plus
  `FE_INVALID` ; RHO2 de `exp_1()` de CORE-MATH abaissé (correctif 8).
- #151 (20 octobre) : points AB.78 et AB.80 : `cancel_minus` et `cancel_plus`
  sans `FE_INVALID` (Fast2Sum) ; l'exemple 1/[0, 1] retiré de la documentation.
- #184 (24 octobre) : AB.79 (fonctions) : plus de `FE_OVERFLOW` ni
  `FE_DIVBYZERO` pour un résultat fini (quotients saturants, racines à
  l'échelle, sin, cos, tan, `pow_rel`).

Travail direct sur `configure-clean` :

- 4 novembre, `1e0f368` : la tâche PK close sans code : le `FE_INVALID` de Clang
  avec les intervalles FPU ne se reproduit pas avec `-frounding-math`
  (`TODO_DONE.md` 3.17).

## 3. Le sens d'arrondi et les registres de contrôle

GAOL calcule vers le haut et, avec `GAOL_PRESERVE_ROUNDING`, rend au programme
son sens d'arrondi : la vérification une fois par opération (#28), les modes
flush-to-zero et denormals-are-zero (#62, point Q), `restore_rounding()` et
`rounding_guard` (point H), puis la lecture et l'écriture directes des registres
de contrôle (x87, MXCSR), écrits seulement s'ils changent (108, 109, tâche PF,
PG.10). FPCR et FPSCR sont aux étapes 17 et 18.

Pull requests :

- #28 (27 septembre) : sens d'arrondi vérifié une fois par opération ;
  `*_dn/*_up` privés ; intervalles de flottants coupés ; archives et .deb par
  CPack.
- #62 (3 octobre) : FTZ et DAZ détectés et effacés par chaque opération ;
  -mno-daz-ftz à l'édition de liens (ancien 4).
- #86 (5 octobre) : point H (23, 36, 48) : `cancel_minus` exact à -O3,
  `gaol::restore_rounding()`, garde RAII `rounding_guard`, coût de la
  préservation mesuré.
- #100 (6 octobre) : point Q (4, 45) : bornes comparées par leurs bits sous DAZ,
  écriture qui efface les modes, chemin AVX-512 sous FTZ.
- #156 (20 octobre) : point AI.108 : x86 32 bits, mot de contrôle x87 et MXCSR
  lus en ligne sans `fegetround()`, seul le registre qui diffère écrit.
- #206 (28 octobre) : point 108 (PF.1) : CORE-MATH lit MXCSR sur x86 32 bits ;
  tests des registres x86 ; outils x86-csr de #156 réécrits.
- #209 (28 octobre) : point 109 (PF.2) : Visual C++ x64 avec /arch:AVX2 n'écrit
  que MXCSR avec la préservation ; `GAOL_FMA_AS_INSTRUCTION`.
- #210 (28 octobre) : PF.3 : MXCSR et le mot de contrôle x87 ne sont écrits que
  s'ils changent (`sse_enter_upward, sse_leave, x86_set_modes`).
- #218 (29 octobre) : PG.10 : `get_rounding()`, `set_rounding()` sur x86-64
  lisent les registres et n'écrivent que celui qui diffère ; `midpoint()`
  préservé 27,9 à 17,5 ns.

Travail direct sur `configure-clean` :

- 18 octobre, `a6786af` à `4fb9f6a` : `f_negate`, `f_negate_simple`,
  `gaol_opposite`, `reset_fpu_cw` puis `get_fpu_cw` retirés, sans appelant.

## 4. Les configurations refusées

`gaol/gaol_config.h` et les trois builds refusent ce qui casserait un
encadrement : `-ffast-math` et `-ffinite-math-only` (#39, point G), MinGW-w64
lié à `msvcrt.dll` (#51), Clang sur ARM 32 bits et un `-frounding-math` non
honoré, aussi pour le compilateur C (AE.90, #145). `doc/three-builds.md` donne
la liste.

Pull requests :

- #39 (29 septembre) : -ffinite-math-only et -ffast-math refusés, tests
  `refused_*` ; pull request sans description (ancien 5).
- #51 (30 septembre) : MinGW-w64 x86-64 lié à msvcrt.dll refusé, avec la vraie
  raison (`fma(), round()`) ; test à l'exécution (ancien 6).
- #91 (6 octobre) : point G (5, 40, 53, 54, 55) : refus de -ffinite-math-only,
  témoin `refused_positive`, textes et audit.
- #145 (19 octobre) : point AE.90 : Clang pour ARM 32 bits et -frounding-math
  non honoré refusés aussi pour le compilateur C, dans les trois builds.

Travail direct sur `configure-clean` :

- 6 octobre, `c7915a7` à `ccf694a` : point G : GCC 9.4 réécrit la sonde des
  sous-normaux ; les drapeaux de GAOL comme remède à -ffast-math.

## 5. CORE-MATH : les fonctions élémentaires correctement arrondies

CORE-MATH (`3rd/math-core`) lit le bon sens d'arrondi sur chaque cible (#54,
#59) ; les changements propres à GAOL sont marqués `/* GAOL */` et décrits dans
`3rd/README.md`, comme correctifs à proposer en amont (#56, point A) ; l'amont
`0b2ae6d1` est importé le 27 octobre, avec ses suites (#212, #213) ; les
en-têtes de GAOL pour CORE-MATH sont dans `gaol/math/` et l'arbre est réduit à
`binary64` (#283).

Pull requests :

- #54 (30 septembre) : CORE-MATH lit le sens d'arrondi dans MXCSR sur x86-64 ;
  cbrt corrigé avec mingw-w64 x64 (ancien 6).
- #56 (30 septembre) : 3rd/README.md : six correctifs de CORE-MATH à proposer en
  amont, rien envoyé (ancien 31).
- #111 (12 octobre) : point A (31, 6, 56, 3, 47) : correctif 6 de CORE-MATH
  entier (Cygwin), mingw-w64, racine SSE2 sur tout x86 32 bits.
- #113 (12 octobre) : point A : chaque ligne changée de CORE-MATH marquée
  `/* GAOL */`, sans changer le code compilé.
- #212 (28 octobre) : test `cos_accurate_path()` : 45 arguments du chemin précis
  de cos.c contre mpmath, et outils tests/tools/cos/ (suite de l'import
  0b2ae6d1).
- #213 (28 octobre) : PH.1 : contournements de `cr_pow` retirés après c01f08c5 ;
  seule reste la puissance minuscule |y| ≥ 2^1013
  (`défaut is_exact() de l'amont`).
- #283 (3 novembre) : en-têtes de CORE-MATH dans gaol/math/, 3rd/math-core
  réduit à binary64, `gaol_roundeven.h` non installé.

Travail direct sur `configure-clean` :

- 23 octobre, `79746c2` : commentaire de `gaol_roundeven()` corrigé
  («` doublegaol_cr_ `»).
- 27 octobre, `cb731c0` à `2927cfe` : CORE-MATH importé à 0b2ae6d1, cos.c et
  sin.c portés sur `gaol_u128` ; correctifs 3 et 4 faits en amont.

## 6. Les puissances et les racines

Le point B (`pown` aux bornes les plus serrées pour tout signe, `pow` aux coins
de toute boîte, une vérification du sens par appel), le point D.21 (tout type
entier au-delà de 2^53), le point AC.81 (les racines d'un sous-normal), et
`pow(x, 2)` par `sqr(x)` (#162, #169). `pownRev`, `compoundm1` et les
réciproques de la norme sont à l'étape 8.

Pull requests :

- #61 (3 octobre) : `pow(x, n)` pour n grand : la borne inférieure garde le
  carré du reste ; mêmes produits arrondis en SSE2 et en FPU (ancien 8).
- #63 (3 octobre) : point 3 : `pow(x, y)` exact au coin où x^y est un double ;
  pow de CORE-MATH juste sous Visual C++, Windows 32 bits, ARM 32 bits
  (correctif 7).
- #85 (5 octobre) : point D.21 (suite) : `nth_root`, rootn, pownRev et
  puissances d'expressions pour tout type entier.
- #117 (13 octobre) : point B (1/3) : une vérification du sens d'arrondi par
  appel des puissances, racines et réciproques ; `nth_root(I, INT_MIN)`.
- #118 (13 octobre) : point B (2/3) : pow aux coins de toute boîte avec limites
  0 et +∞, `GAOL_RND_KEEP` par les bornes, `fast_modulo_k_pi` retirée.
- #121 (14 octobre) : point B (3/3 ; 8, 72-74) : pown aux bornes les plus
  serrées de tout signe, `gaol_pown_ll`, deux espaces de noms.
- #158 (20 octobre) : point AC.81 : `nth_root` et rootn d'un x sous-normal
  prouvés par puissances mises à l'échelle ; bornes à 2 doubles (4 pour q < 0).
- #162 (23 octobre) : suite du point AN : `pow(x, 2)` passe par `sqr(x)` sous
  toutes ses formes, 3,3 fois plus rapide avec NEON.
- #169 (23 octobre) : suites de #162 : quatre formes de pow rendent le vide sans
  toucher au sens ; `sqr()` FPU retrouve sa vitesse ; +0,8 ns de `pow(x, 3)`
  accepté.

Travail direct sur `configure-clean` :

- 16 octobre, `2fe8be7` : `ipow_up`, `ipow_dn`, `pow_up` et `pow_dn` retirés,
  sans appelant.

## 7. Les autres fonctions et opérations

Les cas aux bords de `atanh`, `tan`, `hausdorff()`, `nb_fp_numbers()` et
`bisect(ratio)` (points M et R, anciens 7, 9 et 10).

Pull requests :

- #31 (29 septembre) : atanh([1, x]) est vide et non [`DBL_MAX`, +oo] : atanh
  est défini sur (−1, 1) (ancien 7).
- #36 (29 septembre) : `tan([-M_PI_2, M_PI_2`]) est fini : la largeur `pi_dn`
  est sous π ; drapeau `narrower_than_pi` supprimé (ancien 9).
- #41 (29 septembre) : `hausdorff()` : bornes infinies égales à distance 0 ;
  `nb_fp_numbers()` compte −0 et +0 une seule fois (ancien 10).
- #89 (5 octobre) : point M (9, 57) : commentaires de `tan()` et largeurs entre
  `pi_dn` et π dites un cas théorique.
- #92 (6 octobre) : point R (50, 7) : sortie anticipée de `hausdorff()` à borne
  infinie d'un côté ; `atanh_rel` testé à borne infinie.
- #190 (25 octobre) : point Q (reste de #146) : `bisect(ratio)` et
  `is_bisectable()` comparent les bornes par leurs bits, indépendants de FZ et
  DAZ.

## 8. La norme IEEE 1788-2015

Le `pow` de la norme (#37), les outils des algorithmes, `erf`, les réciproques
et le banc ITF1788 (point P), `powRev` aux bornes infinies (AC.82), les
opérateurs de `gaol_ieee1788` (#284), `pownRev` pour p ≤ 0 (#287) et
`compoundm1` (#288). `overlap` est en cours (#290).

Pull requests :

- #37 (29 septembre) : le pow d'IEEE 1788 écrit une fois (`pow_standard()`),
  testé sur 89 boîtes ; pull request sans description (ancien 1).
- #99 (12 octobre) : point P (25, 30) : outils d'intervalles, erf, réciproques
  (atan2, pow, max…), mulRevToPair, ITF1788, Tang 2021, Boost.Interval.
- #183 (24 octobre) : AC.82 : `pow_rel` et `pow_exponent_rel` (powRev1, powRev2)
  vides pour une puissance, un exposant ou une base [±oo] (`holds_no_real`).
- #284 (3 novembre) : `gaol_ieee1788` nomme les opérateurs `+`, `-`, `*`, `/`,
  `%`, `&` et `|` de `gaol_core`.
- #287 (4 novembre) : pownRev pour p ≤ 0 par `nth_root_rel(J, p, I)` pour tout
  entier p ; un exposant double refusé à la compilation.
- #288 (4 novembre) : compoundm1 de la Table 10.5 : bornes les plus serrées en
  entiers (phase rapide 64 bits, boules 128 bits, y = −1 divisé).

En cours :

- #290 (ouverte le 4 novembre, pas encore fusionnée) : overlap d'IEEE 1788-2015
  (10.6.4) : `overlapping_state`, les 16 états de la Table 10.7 ; les 48 cas
  d'ITF1788 passent. Ouverte le 4 novembre.

## 9. La lecture et l'écriture des intervalles

Le lexeur réentrant (#25), `textToInterval()` à la place du constructeur de
chaîne (#30), la lecture exacte des nombres sous toute locale et sous DAZ, les
formats d'écriture (bornes, largeur, centre, `agreeing`, hexadécimal, `[empty]`,
`[a]`), `operator<<` plus rapide (point C), un parser qui tient les longues
sommes (point E), `operator>>` (point F), les exposants et les littéraux (point
AD), le pointeur nul (#147, #180).

Pull requests :

- #25 (26 septembre) : lexeur flex réentrant et parser bison pur : chaînes lues
  en parallèle sans `std::mutex` ; test à quatre threads laissé commenté.
- #30 (28 septembre) : `interval(const char*)` retiré : `textToInterval()` lit
  les chaînes, les constructeurs numériques prennent des doubles exacts.
- #40 (29 septembre) : lecteur de nombres : sous-normaux encadrés au plus juste
  sous denormals-are-zero, comparaison par les bits (ancien 11).
- #42 (29 septembre) : un long nombre se lit sous locale à virgule presque aussi
  vite que sous la locale C : texte analysé une fois (ancien 18).
- #43 (29 septembre) : un intervalle ponctuel inexact s'écrit [a, b], que le
  lecteur relit, au lieu de `<a, b>` (ancien 13).
- #46 (29 septembre) : operator>> saute les lignes vides au lieu de lever
  `input_format_error` (ancien 15).
- #57 (30 septembre) : formats width et center (milieu et rayon), [empty],
  intervalToText littéral ; un point s'écrit [a] (anciens 16 et 13).
- #71 (3 octobre) : point C (anciens 16, 17, 67) : operator<< 30 à 45 % plus
  rapide ; formats bounds, width, agreeing, hexa décidés le 3 octobre.
- #105 (12 octobre) : point E (12, 40, 45 ; #98) : longues sommes sans
  débordement de pile, NUL, `bad_alloc`, `<a, b>` par les bits.
- #108 (12 octobre) : le parser régénéré avec bison 3.8.2, celui d'Ubuntu 26.04
  (suite de #105).
- #114 (13 octobre) : point F (11, 15, 18, 46, 58-61) : noms de fonctions pliés
  en ASCII (locale turque), tests DAZ sur ARM, textes d'operator>>.
- #147 (19 octobre) : point AD.114 : `parse_interval(nullptr)` rend false au
  lieu de planter dans `strlen()`.
- #149 (20 octobre) : points AD.83 et AD.84 : exposants entiers hors des int lus
  dans un texte ; [l, u] avec l > u vide ; `<a, b>` exige des doubles.
- #180 (24 octobre) : AP.128 : surcharges const `char*` de textToInterval et
  exactToInterval, nullptr lève `input_format_error` ; contournement GCC 12.2 de
  numbers.cpp.
- #182 (24 octobre) : AD.85, AB.79 (lecture), `<a, b>` : exposant du lexeur en
  long long, lecture sans `FE_OVERFLOW` ni `FE_UNDERFLOW`, borne calculée
  refusée.

Travail direct sur `configure-clean` :

- 4 octobre, `ea8f36d` à `d1d3498` : parser et lexeur recompilés avec using
  namespace `gaol_core` ; TODO.md et .gitignore du mainteneur.
- 16 octobre, `faf71a9` à `302a035` : littéral `_iv` et `gaol_literals.h`
  retirés ; déclarations du lecteur dans `gaol/parser/gaol_parser.h`.
- 16 octobre, `91dcce2` : textToInterval lit les bases 16, 8 et 2 (0x10, 0o17,
  0b101).

## 10. L'interface publique, les en-têtes installés et les avertissements

Ce que GAOL installe et expose : `gaol::sin(0.5)` compile, les en-têtes internes
ne sont plus installés et les macros sont préfixées `GAOL_` (point J), les
constantes `double` sortent de l'interface (#77), les constructeurs `explicit`
(4 octobre), les noyaux dans `gaol_detail` (#186), les sources dans `gaol/core/`
(#199), l'ordre total `lexicographic_less` (point V), des en-têtes propres sous
`-Wundef` (point Z) ; et les avertissements : `-Wall -Wextra -Werror` et
`/W4 /WX` sur la bibliothèque, les tests et les exemples.

Pull requests :

- #27 (27 septembre) : constantes de GAOL initialisées à la compilation : les
  intervalles calculés dans des objets statiques sont justes ; `cst_*` retirés.
- #55 (30 septembre) : petites erreurs : `GAOL_NODISCARD` avant C++17, `chi()`
  du vide NaN, commentaires et sorties du manuel (ancien 40).
- #76 (4 octobre) : point J (19, 20, 68) : `gaol::sin(0.5)` compile, en-têtes
  internes non installés, macros préfixées `GAOL_`, job -Wall -Wextra -Werror.
- #77 (4 octobre) : les constantes double de `gaol_port.h`
  (`pi, pi_dn, ln2_dn…`) sortent de l'interface publique ; `interval::pi()` et
  ses sœurs restent.
- #84 (5 octobre) : point D.21 : entiers de tout type pris tels quels au-delà de
  2^53, `pow(x, n)` pour tout entier, internes dans `gaol_detail`.
- #87 (5 octobre) : point D, les en-têtes : `GAOL_PUBLIC`, gaol nomme les noms
  de `gaol_core` un par un, `gaol_allocator.h` retiré, plus de cast à la C.
- #96 (6 octobre) : point V (22) : `gaol::lexicographic_less`, ordre total
  strict pour `std::set`, `std::sort` et `std::map`.
- #120 (14 octobre) : point Z (75) : en-têtes installés propres sous -Wundef,
  options toujours définies à 0 ou 1 ; bisect par `quiet_*`.
- #123 (16 octobre) : tests compilés avec -Wno-psabi : plus de notes de GCC sur
  le passage des arguments (ARM, POWER).
- #186 (25 octobre) : AP.131 : les noyaux `add/sub/mul/div/sqr_bounds` de SSE2
  et de NEON passent de `gaol_core` à `gaol_detail`.
- #199 (26 octobre) : sources et en-têtes des intervalles (core, neon, fpu, sse,
  avx512) déplacés dans gaol/core/, installés dans include/gaol/core/.

Travail direct sur `configure-clean` :

- 4 octobre, `20550f1` à `7567f87` : jobs AppleClang -Werror et Visual C++ /W4
  /WX (x64, x86), C4310 corrigé, `gaol_parameters.h` retiré, fins de ligne LF.
- 4 octobre, `cdbde99` à `0867109` : avertissements /W4 (C4458, C4702, 1ll de
  CORE-MATH), gardes d'inclusion `GAOL_*_H`, en-têtes SSE de headers.sh sur x86.
- 4 octobre, `8d9c90b` : `interval(double)`, `expression(double)` et
  `expression(interval)` explicites ; =, &=, |= et relations prennent un double.
- 15 octobre, `35866d6` à `3e869cb` : gaol/parser/ installé dans
  include/gaol/parser/ ; code mort retiré ; `GAOL_ASSERT` remplacé par assert ;
  gaol/float/.
- 15 octobre, `2ac9d4e` : `gaol_interval_core.h/.cpp` pour le cœur,
  `gaol_interval.h/.cpp` pour l'espace de noms gaol.
- 16 octobre, `3ffde94` à `45ba8ce` : dllmain.cpp, `gaol_profile.h/.cpp` et
  `gaol_limits.h` retirés ; 23 en-têtes installés.
- 16 octobre, `5984cc8` : exemple 16 : la durée en long long (C4244 de Visual
  C++).
- 16 octobre, `cd6352a` à `6dd3321` : `gaol_ieee1788.cpp` à part,
  `gaol_internal.h` retiré, gaol ne nomme plus les exceptions ni les internes ;
  constantes de pi dans `gaol_core`.

## 11. Les exceptions C++

Les exceptions de GAOL 4, `what()` et `operator<<` (#32, point S, #181), puis la
tâche PL : `std::invalid_argument` et `std::domain_error` à la place de
`gaol_exception` et de ses trois classes (#285, et la fusion `221c8d4` de ses
suites).

Pull requests :

- #32 (29 septembre) : `what()` de `gaol_exception` rend l'explication,
  operator<< ne l'écrit qu'une fois (ancien 14 ; remplacé depuis par la tâche
  PL).
- #90 (5 octobre) : point S (14) : `what()` donne le nom de la classe sans
  explication ; un pointeur nul n'est pas une explication.
- #181 (24 octobre) : AP (question de #147) : un fichier nul vaut « (unknown
  file) », un message nul est vide (`gaol_exception, gaol_warning, gaol_error`).
- #285 (4 novembre) : tâche PL : `std::invalid_argument` et `std::domain_error`
  remplacent `gaol_exception` et ses trois classes ; `what()` « fichier:ligne:
  explication ».
- #286 (4 novembre, fermée sans fusion) : « Revert » de #285, ouvert et fermé en
  11 secondes, sans fusion.

## 12. Les trois builds : CMake, autotools et meson

CMake, autotools et meson donnent la même bibliothèque, les mêmes en-têtes, les
mêmes tests et les mêmes options (#200), le reste décidé par des tests de la
configuration ; `make distclean` rend l'arbre propre (#75) ; meson sert de
sous-projet et construit en Release par défaut (point O) ; le build universel de
macOS (#194) ; les drapeaux C de CORE-MATH, comparés par l'audit (#201, #204).

Pull requests :

- #26 (26 septembre) : meson compile le lexeur et le parser commités, sans flex
  ni bison ; la CI ne les installe plus.
- #44 (29 septembre) : meson : alias Python du Microsoft Store et ordre python3,
  python documentés (ancien 42).
- #29 (2 octobre, fermée sans fusion) : configure-clean vers MATH-CORE (trois
  builds alignés, libgaol.so.5.0.0, check/ dans tests/) ; fermée, fusion prévue
  à la version (point 34).
- #75 (4 octobre) : make distclean rend l'arbre tel que git l'a (dossiers .deps
  supprimés), documenté et vérifié par la CI autotools.
- #95 (6 octobre) : point O (41, 42, 52, 65) : GAOL sous-projet meson, Release
  par défaut, `GAOL_DEBUG/enable-debug`, `GAOL_DEBUG_VERBOSE`, pipefail.
- #141 (18 octobre) : supprime `GAOL_USING_ASM`, ses options
  (`GAOL_ASM, --enable-asm, enable-asm`) et `GAOL_HAVE_ROUNDING_MATH_OPTION`,
  que rien ne lisait ; objets inchangés.
- #143 (19 octobre) : point AO.127 : la phrase « les macros que lisent les
  sources » corrigée ; `GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX`,
  `GAOL_HAVE_AVX512_TARGET` gardées.
- #161 (22 octobre) : point AI.112 : `GAOL_PREFER_AVX512` dérivé à chaque
  cmake ; descriptions d'options à jour (sqr, sqrt, puissances entières avec
  FMA).
- #194 (25 octobre) : build universel de macOS avec CMake : NEON pour arm64,
  SSE2 pour `x86_64` ; job macOS 26 universel.
- #200 (26 octobre) : les trois builds ont les mêmes options ; FMA, AVX-512,
  u128, débogage décidés par des tests ; statique par défaut ; `GAOL_ARM64`
  retiré.
- #201 (26 octobre) : PE.2 (92) : mêmes drapeaux C de CORE-MATH dans les trois
  builds (-std=gnu99, -msse2 -msse3, visibilité), comparés par l'audit.
- #204 (27 octobre) : point 95 (PE.1, PE.5) : sonde AVX-512 liée avec autotools
  et meson, FMA vérifié en C avec CMake, bogue 54412 cité.

## 13. L'installation et l'intégration

`gaol.pc` sans `-lm` (#78), `VERSION.txt` lu par les mêmes règles dans les trois
builds (#45 et le 30 septembre), CPack et son archive des sources (#33, #79,
point N).

Pull requests :

- #33 (29 septembre) : CMake avertit quand configure a été généré pour une autre
  version que VERSION.txt ; test `cpack_stale_configure` (ancien 44).
- #45 (29 septembre) : VERSION.txt : les trois builds ignorent une marque
  d'ordre des octets en tête ; le message donne les octets (ancien 43).
- #78 (4 octobre) : plus de -lm explicite dans autotools, meson et gaol.pc : le
  compilateur C++ lie lui-même la bibliothèque mathématique.
- #79 (4 octobre) : CPack : TODO.md, process.md et todo-notes/ hors de l'archive
  des sources.
- #94 (6 octobre) : point N (44, 63) : `package_source` reconfigure après
  VERSION.txt, `cpack_stale_configure` avec ccache et Ninja,
  `CPACK_VERBATIM_VARIABLES`.

Travail direct sur `configure-clean` :

- 30 septembre, `9abbf15` à `f6c2b8e` : points 39, 42 et 43 : examples.md, meson
  sans Python, VERSION.txt lu par les mêmes règles partout.

## 14. L'intégration continue

Les workflows de GitHub Actions sur Linux, dans des conteneurs, sur macOS et sur
Windows (`doc/continuous-integration.md`) : la locale à virgule partout (#35,
point L), meson 0.53.2 (#110), actionlint et shellcheck (#202), les jobs qui
manquaient (#205), macOS 27 (#276 et `57650a4`).

Pull requests :

- #35 (29 septembre) : CI Linux : génère `fr_FR.UTF-8` et échoue si numbers n'a
  rien vérifié sous la locale à virgule (ancien 29).
- #38 (29 septembre, fermée sans fusion) : brouillon Copilot vers master pour le
  job Visual Studio 2026 x86 ; fermé, corrigé par le commit 17e7733 de #37.
- #48 (30 septembre) : CI Windows : MinGW-w64 x64 lance ses tests avec son bin
  en tête du PATH (libstdc++-6.dll de Git).
- #58 (1er octobre) : CI après #50 et #57 : [empty] dans `rounding_direction` ;
  runtime Debug de Visual C++ sans boîte de dialogue.
- #74 (4 octobre) : point L (29, 43, 69) : locale à virgule vérifiée par job,
  push sur les branches principales, Ubuntu 24.04 pour 22.04, badges.
- #110 (12 octobre) : CI : un job meson 0.53.2, la plus ancienne version que
  prend le build meson.
- #116 (13 octobre, fermée sans fusion) : pull request jetable pour faire
  afficher par la CI ce que numbers et expressions vérifient sous DAZ (point
  F.11).
- #165 (23 octobre) : suites de #154, #160, #161 : commentaires de la CI,
  setup.sh sans -U, process.md §10, NI de POWER et FS de MIPS documentés.
- #176 (24 octobre) : suite de #173 : le job Cygwin n'essaie plus les
  générateurs de CPack.
- #202 (27 octobre) : PE.4 : les 34 remarques d'actionlint et les 101 de
  shellcheck traitées dans les workflows et les scripts.
- #205 (27 octobre) : PE.3 : jobs MINGW32, Cygwin (autotools, meson, Debug),
  Clang vers MSVC, universels macOS 15, Xcode ; MINGW32 msvcrt refusé sans FMA.
- #276 (30 octobre) : CI macOS : les jobs macOS 14 passent sur macos-27 (sans
  exécuteur ; arm64 sur xcode-27 depuis 57650a4, Rosetta retiré).

Travail direct sur `configure-clean` :

- 4 octobre, `dc036c6` : 600 s pour numbers dans le job macOS 15 `x86_64` GCC
  avec sanitizers.
- 1er novembre, `57650a4` : job macOS 27 arm64 sur l'étiquette xcode-27 (commit
  du mainteneur).

## 15. Windows : Visual C++, clang-cl, MinGW-w64, MSYS2 et Cygwin

`GAOL_INFINITY` et le refus de clang-cl sans `/fp:strict` (point K), GCC pour
Windows avec `-mfma -mprefer-vector-width=128` en 64 et 32 bits (110, 133),
Cygwin porté (#173, #174), une fonction appelée avec la pile alignée sur 4
octets (#208).

Pull requests :

- #59 (2 octobre) : clang-cl x64 : cbrt, rsqrt et asinpi lisent le bon sens
  d'arrondi (`_WIN32`) ; job clang-cl dans windows.yml (ancien 47).
- #72 (3 octobre) : point K, ancien 49 : `GAOL_INFINITY` devient
  `numeric_limits::infinity()`, le `HUGE_VAL` de l'UCRT étant calculé à
  l'exécution par clang-cl.
- #73 (3 octobre) : point K, ancien 47 : clang-cl refusé sans /fp:strict, -msse2
  en x86, clang-cl de Visual Studio 2026 dans la CI.
- #163 (23 octobre) : point AI.110 : GCC pour Windows x64 reçoit -mfma
  -mprefer-vector-width=128 ; registres x87 et MXCSR écrits directement avec
  mingw-w64.
- #167 (23 octobre) : suites de #163 : Cygwin reçoit -mfma
  -mprefer-vector-width=128 ; `rounding_direction` depuis chaque direction
  partout ; -mavx512f, -march, -faligned-new=8 documentés.
- #172 (24 octobre) : point 133 : GCC pour Windows 32 bits reçoit -mfma
  -mprefer-vector-width=128 dans les trois builds ; job sans FMA ;
  -mstackrealign non retenu.
- #173 (24 octobre) : point 133 : Cygwin x86-64 porté (`_mm_malloc`,
  `next_float` aux bords, `__builtin_fma`, refus sans FMA) et job de CI ;
  `package_source` impossible.
- #174 (24 octobre) : restes de Cygwin : `is_bisectable()` par `next_float()` ;
  réserve sur `std::fma` sans optimisation (doc/using.md, manuel, exemple 13).
- #208 (28 octobre) : PF.4 : test d'une fonction entrée avec la pile alignée sur
  4 octets (Windows 32 bits) ; -mstackrealign documenté.

## 16. Les intervalles SSE2 et le chemin AVX-512

Le chemin AVX-512 à arrondi embarqué (#88, ancien 27), `sqr()` d'un seul produit
et le chemin décidé à la configuration (point AA), les opérations sans branche
sur les signes (point ZZ, #124), SSE2 avec Visual C++, clang-cl et sur Windows
32 bits (#148, #152, #193), les divisions du chemin AVX-512 (#191).

Pull requests :

- #88 (5 octobre) : ancien 27 : le chemin AVX-512 à arrondi embarqué pour `+`,
  `-`, `*`, `/` et `sqrt`, choisi à l'exécution par `GAOL_PREFER_AVX512`.
- #122 (17 octobre) : point AA (76) : `sqr()` d'un seul produit, aiguillage
  AVX-512 en appel terminal, chemin décidé à la configuration (Shekel 5).
- #124 (18 octobre) : point ZZ (et AG) : `+`, `-`, `*`, `/` et `sqr` sans
  branche sur les signes, bornes en registre, lignes longues au banc.
- #148 (20 octobre) : point AI.106 : intervalles SSE2 et chemin AVX-512 pour
  Visual C++ et clang-cl x64, `__vectorcall`, défauts de cl contournés, jobs
  sous Intel SDE.
- #152 (20 octobre) : intervalles SSE2 sur Windows 32 bits avec GCC et Clang :
  -faligned-new=8 donné au code utilisateur, exigé par `gaol_config.h`.
- #189 (25 octobre) : AC.82 (reste) : `interval(__m128d)` et
  `interval(float64x2_t)` rendent le vide pour [±oo] ; `unchecked_bounds` pour
  le code interne.
- #191 (25 octobre) : PG.6 : divisions du chemin AVX-512 en vdivsd scalaires à
  arrondi intégré (x / d de 3,62 à 2,76 ns).
- #193 (25 octobre) : SSE2 sur Windows 32 bits avec Visual C++ et clang-cl :
  bornes en deux double d'alignement 8 ; `__vectorcall` ; x87 réglé avec
  préservation.

## 17. Les intervalles NEON et arm64

Les intervalles NEON sur arm64, FPCR lu par `mrs` et écrit seulement s'il change
(#146), `sqrt_bounds()` indépendant de `FPCR.AH` (#187), le noyau de `pown`
(#188), NEON avec tous les compilateurs d'arm64, Visual C++ et clang-cl compris
(#192).

Pull requests :

- #146 (19 octobre) : points 106 et 107 : intervalles NEON sur arm64
  (`GAOL_ARM64`), FPCR lu par mrs et écrit seulement s'il change.
- #153 (20 octobre, fermée sans fusion) : `width()` et `bisect()` avec
  `GAOL_PRESERVE_ROUNDING` sur arm64 : même correction que #150, fermée sans
  fusion.
- #187 (25 octobre) : AP.129 : `sqrt_bounds()` de NEON par les bits de signe,
  sans `vmaxq_f64`, indépendant de FPCR.AH ; bloc de test AH.
- #188 (25 octobre) : AP.132 : noyau empaqueté de pown pour NEON ; noyaux de
  pown répartis par sorte d'intervalles ; gain faible (1,03 à 1,07).
- #192 (25 octobre) : PG.106 a : intervalles NEON avec tous les compilateurs
  arm64 (Visual C++, clang-cl, MinGW-w64, MSYS2) ; `mid()` de cl corrigé.

## 18. Les intervalles FPU et les cibles sans SIMD

Les intervalles FPU, pour ARM 32 bits, POWER, s390x et RISC-V : FPSCR lu et
écrit directement sur ARM 32 bits (#150), le produit, le quotient et le carré
sans branche sur les signes (#157).

Pull requests :

- #150 (20 octobre) : ARM 32 bits : FPSCR lu par vmrs et écrit par vmsr avec
  `GAOL_PRESERVE_ROUNDING` (`GAOL_RND_FPSCR_DIRECT`) ; `width()` et `bisect()`
  corrigés.
- #157 (22 octobre) : point AI.106 (reste) : produit, quotient, reste et carré
  des intervalles FPU sans branche sur les signes ; x - y corrigé.

## 19. La tâche PG : les écarts de GCC et de Clang, le placement du code

`nth_root(x, -2)` et `sqrt()` (116, #222, #224), le canari de GCC sur le chemin
rapide (#230), `-falign-functions=64` (#232), `x &= y`, `x | y`, `max` et `min`
(#234, #236), le placement de Visual C++ arm64 (#262 et ses huit essais),
`x <= y` (#279 et ses candidates). Chaque essai jetable est une pull request
fermée sans fusion.

Pull requests :

- #168 (23 octobre, fermée sans fusion) : expérience (#157) : noyau mixte du
  produit FPU (deux produits à signes égaux) mesuré par le banc A/B ; fermée
  sans fusion.
- #222 (29 octobre) : PG.2 (116) : `nth_root(x, -2)` repasse par
  `inverse(sqrt())` là où sqrt a un chemin vectoriel ; ligne `nth_root(x, -2)`
  au banc.
- #224 (29 octobre) : `denorm_min()` écrit en littéral : GCC le convertissait
  par l'unité x87 à chaque emploi ; `sqrt()` 16 fois plus rapide sur Intel.
- #226 (29 octobre, fermée sans fusion) : jetable : code de #222 sur une base
  qui a la ligne `nth_root(x, -2)` ; N2 1,57 (GCC) à 1,09 (Clang).
- #230 (29 octobre) : `round_upward_with_flush_cleared()` hors ligne
  (`GAOL_RND_OUT_OF_LINE`) : plus de canari GCC sur le chemin rapide ; x + y
  1,29 fois.
- #232 (29 octobre) : -falign-functions=64 en Release pour la bibliothèque seule
  dans les trois builds ; corrige le placement de Clang sur line of 15 +.
- #234 (29 octobre) : SSE2 x &= y et x | y gardent les bornes dans le registre
  (`_mm_move_sd, _mm_shuffle_pd`) ; x | y 1,62 fois avec GCC.
- #236 (29 octobre) : `max(x, y)` et `min(x, y)` SSE2 et NEON par noyaux dédiés,
  les deux bornes d'un coup ; 2,9 à 3,7 fois plus rapides ; deux tests.
- #254 (30 octobre, fermée sans fusion) : étude du placement de Visual C++
  arm64, base sans correctif (e0), avec la modification de max et min : cartes
  du lien pour #262.
- #255 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  (e0), avec la modification de x `*` y : cartes du lien pour #262.
- #256 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  (e0), avec les opérations avec un double : cartes du lien pour #262.
- #257 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  (e0), avec le constructeur de deux bornes : cartes du lien pour #262.
- #258 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  avec #pragma `code_seg` (e1), max et min : adresses multiples de 0x1000,
  lignes stables.
- #259 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  avec #pragma `code_seg` (e1), x `*` y : le correctif de #262 essayé.
- #260 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  avec #pragma `code_seg` (e1), opérations avec un double : le correctif de #262
  essayé.
- #261 (30 octobre, fermée sans fusion) : étude du placement de Visual C++ arm64
  avec #pragma `code_seg` (e1), constructeur de deux bornes : le correctif de
  #262 essayé.
- #262 (30 octobre) : Visual C++ arm64 : `add_bounds()` et `sub_bounds()` dans
  des sections à elles (.gadd, .gsub) ; fin du bruit de 8 à 17 % de x + y (PG).
- #246 (31 octobre, fermée sans fusion) : `certainly_le()` et sœurs : bornes
  comparées avant le test du second intervalle (x <= y, GCC 13 sur Intel) ;
  remplacée par #279.
- #247 (31 octobre, fermée sans fusion) : jetable ltfirst : comparaison des
  bornes avant les deux tests d'ensemble vide (x <= y) ; remplacée par #279.
- #277 (31 octobre, fermée sans fusion) : candidate jetable F1 de l'étude de x
  <= y ; sa CI trouve le repli de GCC 14 en comparaison signalante
  (`FE_INVALID`).
- #278 (31 octobre, fermée sans fusion) : candidate jetable F2 de l'étude de x
  <= y (les bornes d'abord, sauf RISC-V) ; corrigée, elle devient #279.
- #279 (31 octobre) : x <= y (PG) : bornes d'abord, `GAOL_LIKELY`, égalité par
  `quiet_less_equal` contre GCC 14, `GAOL_RELATION_BOUNDS_FIRST`, un job de CI.

## 20. La tâche PG : les noyaux NEON

Les noyaux NEON réécrits et mesurés sur le Neoverse N2 de la CI et sur un Apple
M2 : `pown` (#219), `x / y` (#225), `x * y` (#235, #249), `max` et `min` (#239),
`inverse()` (#242), `interval(l, r)` (#248), `interval(d)` et `x op= d` (#250),
avec leurs variantes jetables.

Pull requests :

- #195 (25 octobre, fermée sans fusion) : expérience (#192) : quotient NEON par
  deux divisions scalaires, plus lent sur N2 et M2 ; la division n'explique pas
  la perte.
- #219 (29 octobre) : PG.7 : noyau NEON de pown, comparaisons absolues
  vcageq/vcaleq que Clang ne scalarise plus ; `pow(x, 3)` 1,32 fois plus rapide
  sur M2.
- #221 (29 octobre, fermée sans fusion) : jetable PG.8 : quotient NEON, signes
  de y lus dans des registres généraux ; gagne 1,35 sur le N2 avec Clang, repris
  par #225.
- #225 (29 octobre) : PG.8 : x / y NEON lit les signes de y dans des registres
  généraux, 1,35 fois plus rapide sur Neoverse N2 (Clang).
- #233 (29 octobre, fermée sans fusion) : jetable : x `*` y NEON, test de NaN
  hors ligne puis derrière `reread_bounds()` ; première variante perd avec GCC
  et Visual C++ ; remplacée par #235.
- #235 (29 octobre) : NEON x `*` y : test de NaN derrière les asm de
  `reread_bounds()` ; Clang 1,29 fois plus rapide sur le N2, GCC et Visual C++
  inchangés.
- #239 (30 octobre) : NEON max et min par instructions entières sur les quatre
  mots stockés, variante F4b2 (opérande vide testé d'abord) ; N2 1,30 à 1,93.
- #240 (30 octobre, fermée sans fusion) : jetable : variante F3 du noyau NEON de
  max/min (résultat par un registre vectoriel) ; +33 à +38 % sur un opérande
  vide.
- #241 (30 octobre, fermée sans fusion) : jetable : variante F6 du noyau NEON de
  max/min (validation entière du résultat) ; M2 0,94, N2 Clang 0,96.
- #242 (30 octobre) : NEON 1 / y et `y.inverse()` par une seule division de -1
  par les bornes échangées ; `x.inverse()` 0,635 sur M2.
- #243 (30 octobre, fermée sans fusion) : jetable inv-divtests : tests de NaN et
  de zéro sur les bits dans x / y et x %= y NEON ; perd 3 à 10 % sur M2.
- #244 (30 octobre, fermée sans fusion) : jetable inv-divtests-tbl : idem avec
  échange des voies par `vqtbl1q_u8` ; ligne de 13 opérations 2,49 fois plus
  lente sur M2.
- #248 (30 octobre) : NEON `interval(l, r)` par `valid_bounds()`, une
  comparaison au lieu de trois ; `interval(a, b)` 1,80 sur N2 ; test constructor
  renforcé.
- #249 (30 octobre) : NEON x `*` y : test NaN/infini sur les bits d'exposant
  (vshrn), produits par `vmulq_laneq_f64` ; x `*` y 1,42 à 1,53 sur N2.
- #250 (30 octobre) : NEON `interval(d)` et x op= d : `quiet_less(fabs(d), +oo)`
  et `keep_branch()`, drapeaux de la base gardés ; test sNaN dans
  `rounding_direction`.
- #251 (30 octobre, fermée sans fusion) : variante jetable de #250 : d testé sur
  ses bits dans `interval(d)` et x op= d ; deux drapeaux changent, refusée
  (`TODO_AFTER 3.8`).
- #252 (30 octobre, fermée sans fusion) : variante jetable F4b du noyau NEON de
  max et min (#239) : le vide avant les clés ; +6 à +11 % sur un opérande vide.
- #253 (30 octobre, fermée sans fusion) : variante jetable F4b2 (F4b avec une
  indication de disposition) : retenue par le mainteneur, reprise par un commit
  de #239.

## 21. Le banc de performance et les comparaisons

`doc/compare` (les cas particuliers, les temps contre GAOL 4 et d'autres
bibliothèques), le banc A/B de la CI, base contre pull request (#160, #166), le
tableau des lignes longues sur ARM 64 bits (#216), et les lignes du banc
(`x / d`, `x % y`, `x.inverse()`).

Pull requests :

- #139 (18 octobre) : atanh([1]) et atanh([-1]) ajoutés aux cas particuliers de
  doc/compare (290 cas au lieu de 288), reste de l'ancien 7 (point Y).
- #160 (22 octobre) : points AI.111 et AI.115 : banc A/B base contre pull
  request (performance.yml) ; les dix lignes de `run_lines.sh` dans make perf.
- #166 (23 octobre) : suites de #160 : dix jobs de plus au banc A/B, cache des
  programmes de la base, `run_perf.sh` dans le filtre, `LINE_LIST` vide.
- #211 (27 octobre, fermée sans fusion) : jetable : contrôle A/B de #209 (base
  plus une fonction inutilisée) ; ne reproduit pas les inversions x / y, sqr,
  x - y : effet de disposition.
- #214 (27 octobre, fermée sans fusion) : jetable : mesure A/B de #206 (a) avec
  Visual C++ x86 ; `nth_root(x, 3)` 355 à 179 ns, asinpi 1005 à 794 ns.
- #216 (28 octobre) : tableau des lignes longues sur ARM 64 bits (Apple M2) dans
  doc/compare/performance.md, `run_lines_arm64.sh` et la commande arm64-report.
- #217 (28 octobre) : PG.4 : compare-performance.py liste les lignes à partir
  d'un rapport de 1,5 au lieu de 2.
- #220 (29 octobre) : PG.9 : ligne x / d dans `gaol_performance`
  (`REGISTER_ROWS`) ; `div_bounds()` SSE2 du chemin AVX-512 appelle `er_div2()`,
  code identique.
- #237 (30 octobre) : lignes x % y et `x.inverse()` dans `gaol_performance`,
  pour mesurer operator%= et `inverse()` (PG, autres noyaux NEON).
- #238 (30 octobre) : le | du nom x | y échappé dans le tableau Markdown de
  `gaol_performance` ; compare-performance.py lit \| comme | (21 tests).

Travail direct sur `configure-clean` :

- 3 octobre, `aac4e05` : fichiers de performance de doc/compare remesurés
  (commit du mainteneur).

## 22. Le contrôle de l'assembleur

`tests/tools/asm-order/check_order.py` vérifie dans l'assembleur de x86-64
qu'aucune opération ne passe de l'autre côté d'une écriture de MXCSR (#207),
avec ses constats `BEFORE`, `AFTER` et `DENORMAL` (#223, #227) et son job de CI
(#228).

Pull requests :

- #207 (28 octobre) : PF.5 : outil asm-order de contrôle de l'assembleur x86-64
  autour des écritures de MXCSR ; --selftest dans la CI.
- #223 (29 octobre) : outil asm-order : constats BEFORE de GCC triés, constante
  étendue exacte exemptée, KNOWN par mnémonique ; auto-test 103 cas.
- #227 (29 octobre) : asm-order : constat DENORMAL (fldt d'une constante
  sous-normale puis fstpl), le garde de #224 ; auto-test 130 cas.
- #228 (29 octobre) : job asm-order-library de linux.yml : contrôle de
  l'assembleur avec GCC 13.3 et Clang 18, avec et sans préservation ; --build
  relatif corrigé.

## 23. Les tests et leurs outils

Les tests de non-régression et leurs outils : les mutants (`tests/tools/pow`,
`x86-csr`, `diffarith`), les tests de compilation qui doivent échouer (point U),
`rounding_direction` (les registres, les masques d'exceptions), et les cas
d'IBEX et de Codac contre l'encadrement exact (#281, 3 905 vérifications).

Pull requests :

- #34 (29 septembre) : test numbers : pragma no-inline-functions contre une
  mauvaise compilation de GCC 12 sur aarch64 (job Debian 12 arm64).
- #81 (4 octobre) : les outils de vérification de pow (diffpow, checkrows,
  mutate…) rangés dans tests/tools/pow et repassés sur le code.
- #82 (4 octobre) : tests : oracle de strictPrecedes corrigé, résultats de pow
  et pown vérifiés selon IEEE 1788 dans tests/ieee1788.cpp.
- #97 (6 octobre) : point U (40, 64) : tests de compilation qui doivent échouer,
  `test_input`, `check_examples.py` en CI, tests sans exceptions.
- #170 (23 octobre) : suite de #157 : jobs FPU GCC et Clang avec le sens
  préservé tiennent la protection des stores de `*=`, /=, %= (mutants) ; pas de
  `GAOL_RND_KEEP()`.
- #171 (24 octobre) : suite de #161 (112) : test des bits du signe de zéro de x
  `*` d, chemins AVX-512 et SSE2, sauté ailleurs.
- #197 (25 octobre) : tâche PD (2/2), PD.7 : mutate.py à jour, mutants de
  `er_div`, `neon-*` lancés sur M2, tests de pow renforcés.
- #203 (27 octobre) : PE.6 : relecture des corrections de #163 ; trous de
  `rounding_direction` (bits x87 autres que RC, MXCSR) bouchés.
- #215 (28 octobre) : suite de #210 : 17 mutants de `sse_enter_upward()` et
  `sse_leave()` dans tests/tools/x86-csr/mutants.py (63 en tout, 62 tués).
- #229 (29 octobre) : grille de diffarith : ±0,5, ±2, ±4 en plus (29 bornes),
  voit le mutant << 2 du noyau NEON de #225.
- #231 (29 octobre) : réponse à #223 : `rounding_direction` vérifie que
  `pow_rel` et `pow_exponent_rel` près de `DBL_MAX` rendent les masques
  d'exceptions du programme.
- #245 (30 octobre) : test `quotient_on_nan_lanes()` (NEON) : x / y, x %= y,
  `inverse()` avec un NaN dans une voie stockée rendent `emptyset()`.
- #281 (3 novembre) : `tests/test_ibex_codac.cpp` : 3 905 vérifications des cas
  d'IBEX et de Codac contre l'encadrement exact ; numbers à 400 s sous
  sanitizers.

## 24. La documentation, les exemples et le manuel

Le premier programme du `README.md`, le tutoriel et les pièges (points I et AF,
#140), `doc/simd.md` (point AN, #154), les exemples (point T, #53), les textes
que le code contredisait (#119, #164), `doc/tools.md`, et le manuel de v5
(`manual/v5`), changé avec chaque pull request.

Pull requests :

- #52 (30 septembre, fermée sans fusion) : doc/math-libraries.md, ouverte par
  erreur vers master ; fermée, le document est entré par le commit d6c8ac1.
- #53 (30 septembre) : exemple 16, Goldstein-Price : constructeurs numériques,
  temps en ms, même source du maximum que l'exemple 03 (ancien 39).
- #93 (6 octobre) : point T (39, 66) : maximum de Goldstein-Price lu par
  textToInterval dans les exemples 03 et 06 ; textes des exemples.
- #119 (13 octobre) : suites de l'audit du 13 octobre : commentaires AVX-512,
  ancien 9 dit mesuré, `<a, b>` par les bits, process.md.
- #140 (18 octobre) : points I et AF : premier programme, tutoriel, pièges
  « Common errors », noms d'autres bibliothèques, sens d'arrondi laissé, coût de
  `GAOL_PRESERVE_ROUNDING` remesuré.
- #154 (20 octobre) : point AN (ancien 126) : pistes SIMD et FMA évaluées dans
  doc/simd.md ; le TODO SIMD de GAOL 4 remplacé.
- #164 (23 octobre) : point C (tâche QB) : textes, tests et ménage (anciens 117,
  119, 120, 87, 88, 89, 91, 94, 121, 122, C4702).

Travail direct sur `configure-clean` :

- 5 octobre, `2d80cc4` : examples.md : un bloc au plus proche coûte environ 13
  ns, non 7.
- 18 octobre, `26bb495` : README : Jordan Ninin présenté avant Frédéric
  Goualard, publications laissées à leurs sites.
- 24 octobre, `fa8bc14` : doc/tools.md : les scripts du dépôt et à quoi chacun
  sert.
- 25 octobre, `a6fe1d7` : README.md mis à jour par le mainteneur.

## 25. La synthèse des textes de la version

La tâche PI : `doc/differences.md` et le `ChangeLog` réécrits en douze thèmes
(#263 à #274), les phrases fausses hors de ces deux documents (#275), leurs
restes (#280). La synthèse de #283 à #288 est en cours (#289).

Pull requests :

- #263 (30 octobre) : synthèse 1/12 du point Y (tâche PI) : les trois builds,
  leurs options, la version et l'archive dans differences.md et ChangeLog.
- #264 (30 octobre) : synthèse 2/12 du point Y : les compilateurs et les
  plates-formes refusés ou construits, dans differences.md et ChangeLog.
- #265 (30 octobre) : synthèse 3/12 du point Y : l'interface installée
  (`en-têtes, espaces de noms, macros GAOL_`) dans differences.md et ChangeLog.
- #266 (30 octobre) : synthèse 4/12 du point Y : les sortes d'intervalles (NEON,
  SSE2, FPU, chemin AVX-512) dans differences.md et ChangeLog.
- #267 (30 octobre) : synthèse 5/12 du point Y : le sens d'arrondi, les
  registres de contrôle et les modes flush-to-zero.
- #268 (30 octobre) : synthèse 6/12 du point Y : les exceptions flottantes
  (aucune pour le vide, un NaN, un résultat fini).
- #269 (30 octobre) : synthèse 7/12 du point Y : CORE-MATH et les fonctions
  élémentaires dans differences.md et ChangeLog.
- #270 (30 octobre) : synthèse 8/12 du point Y : puissances et racines (pown le
  plus serré, pow aux coins, racines n-ièmes).
- #271 (30 octobre) : synthèse 9/12 du point Y : fonctions d'intervalles, outils
  des algorithmes, réciproques, résultats corrigés.
- #272 (30 octobre) : synthèse 10/12 du point Y : la lecture des nombres, des
  intervalles et des chaînes.
- #273 (30 octobre) : synthèse 11/12 du point Y : l'écriture des intervalles,
  ses formats et les messages des exceptions.
- #274 (30 octobre) : synthèse 12/12 du point Y : tests, exemples, documentation
  et intégration continue.
- #275 (30 octobre) : point Y, 13/13 : phrases fausses hors de differences.md et
  du ChangeLog (manuel, doc/, 3rd/README.md, commentaires, examples.md) ; aucun
  code.
- #280 (2 novembre) : restes de la tâche PI (point Y) : « À décider » de #263 à
  #274, « À savoir » de #275, archive CPack, @outputs du manuel.

En cours :

- #289 (ouverte le 4 novembre, pas encore fusionnée) : synthèse de #283 à #288
  dans differences.md, ChangeLog et synthese.md ; PDF du manuel régénéré (PJ 5
  et 7). Ouverte le 4 novembre.

## 26. Les signalements à d'autres projets, préparés

Les défauts trouvés chez d'autres projets ont chacun une issue « À
communiquer… » avec ses brouillons en anglais et ses vérifications (#125 à #138
sauf #128, et #175), et les correctifs de CORE-MATH sont écrits dans
`3rd/README.md` (#56). Rien n'est envoyé : c'est la tâche PH de `TODO.md`, qui
attend l'accord du mainteneur.

Travail direct sur `configure-clean` :

- 18 octobre, `cd90a58` à `eb4f45a` : signalements en amont renvoyés aux issues
  #125 à #138 dans TODO.md, 3rd/README.md et synthese.md.

## 27. Les registres, la procédure et le ménage

`process.md`, `HOUSE_RULES.md`, `todo-notes/` et `check_branch`, les registres
`TODO.md`, `TODO_DONE.md` et `TODO_AFTER.md` et leurs issues #177 à #179 (162
commits directs, non listés), les commits remis au nom de Jordan08 le 30
septembre, les lignes de crédit laissées (123, décidé le 19 octobre), les
branches supprimées (`TODO_DONE.md` 8).

Pull requests :

- #103 (12 octobre) : retrait de .continue/rules, copie de process.md ajoutée
  par 68ec12c.
- #109 (12 octobre) : process.md : arbre `GAOL_V9`, outils d'Ubuntu 26.04 et
  état au 12 octobre.
- #112 (12 octobre) : process.md : configure régénéré par autoconf seul ;
  `check_branch` prend l'arbre courant.
- #115 (13 octobre) : process.md : les arbres `GAOL_V8` et `GAOL_V9` se valent ;
  le mainteneur ouvre les pull requests si le jeton ne peut pas.
- #155 (20 octobre) : point AL.118 : process.md mis à jour (--no-track, variante
  FPU sur arm64, §6.1, §12, §2 et §9).
- #196 (25 octobre) : tâche PD (1/2), point 130 : process.md (en-tête daté,
  §6.1, §11), doc/accuracy.md, build « default » de doc/compare.
- #282 (3 novembre) : `HOUSE_RULES.md` : la ligne « Tools installed » renvoie à
  process.md §6.1 (question de #280).

Travail direct sur `configure-clean` :

- 30 septembre, `68fe887` à `195e4a2` : règles de commit dans
  .claude/CLAUDE.md : auteur Jordan08, messages courts, pas de Co-Authored-By.
- 4 octobre, `16a2f60` à `dcd3b11` : todo-notes/ ramené de todo-status, puis
  rapports relus et retirés, synthese.md gardé.
- 8 octobre, `68ec12c` : « oui ferme » : Jordan NININ corrigé dans 61 fichiers,
  .continue/rules ajouté (retiré par #103).
