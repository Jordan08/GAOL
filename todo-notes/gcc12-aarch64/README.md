<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-11-08)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# GCC 12.2 pour aarch64, sous qemu

Le job « Debian 12 Bookworm arm64 » de la CI compile les tests avec GCC 12.2,
qui produit sur aarch64 un code faux dans certaines boucles (`process.md`,
section 11). `tests/numbers.cpp` y est fragile : après chaque changement du
fichier, le défaut est dans une autre fonction (8 novembre 2026, #299 :
18 vérifications de la lecture des lignes vides en échec, puis un programme
qui ne se termine plus dans `decimal_output()`). La CI ne le dit qu'une fois la
pull request ouverte ; ce répertoire le dit en deux minutes, sans matériel ARM.

| Fichier | Contenu |
| --- | --- |
| `Dockerfile` | L'image `gaol-bookworm-cross` : Debian 12, GCC 12.2 pour aarch64, CMake 3.25 et qemu-aarch64 7.2 |
| `cross-test` | Construit un programme de `tests/` d'une référence git (ou de l'arbre de travail) en Release et le lance sous qemu |

```sh
sh todo-notes/gcc12-aarch64/cross-test -w <dir> HEAD numbers
sh todo-notes/gcc12-aarch64/cross-test -w <dir> -W ieee1788   # l'arbre
```

`<dir>` est un répertoire hors de l'arbre, où vont les sources et les builds.

Il faut Docker ; l'image se construit au premier appel. Le script donne 4 cœurs
au plus à Docker, affiche les vérifications en échec et les dernières lignes du
programme, et rend son code de sortie (124 : le programme ne se termine pas
dans le délai, 240 s par défaut, `-t` le change). Un programme sans défaut
prend de 15 s (`ieee1788`) à 2 minutes (`numbers`), compilation comprise.

Mesuré le 8 novembre 2026 avec ce script : `numbers` à `97c76c2`, la première
tête de #299, 451 806 vérifications dont 18 en échec ; `ieee1788` à `b954fdc`,
131 901 vérifications, aucune en échec.
