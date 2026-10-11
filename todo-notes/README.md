# Les notes du travail sur le TODO.md

Ce répertoire garde ce qui sert encore du travail fait sur les points de
[TODO.md](../TODO.md) depuis le 29 septembre 2026. Il vient de la branche
`todo-status`, où il a été écrit jusqu'au 3 octobre, et a été ramené dans
`configure-clean` le 4 octobre, avant la suppression de cette branche. Le même
jour, ses rapports ont été relus : leurs questions ont reçu une réponse, et ce
qu'ils contenaient d'encore utile est regroupé dans `synthese.md`.

La procédure en vigueur pour traiter un point est [process.md](../process.md).
Là où elle diffère des consignes de `orchestration/`, c'est elle qui compte :
depuis le 3 octobre, on travaille dans l'arbre principal sans worktree, et
l'agent pousse et ouvre lui-même la pull request.

| Fichier | Contenu |
| --- | --- |
| `synthese.md` | Les textes proposés pour `ChangeLog` et `doc/differences.md`, que reprendra la pull request de synthèse (point Y), et le contenu encore utile des rapports aux points qui restent |
| `orchestration/HOUSE_RULES.md` | Les consignes que lisait chaque agent le 29 et le 30 septembre, quand plusieurs travaillaient en parallèle |
| `orchestration/points_w4.py` | Les plans détaillés des gros points 25, 26, 27, 28 et 30 (points P, W, H et X), découpés en sous-fonctions, et pas encore faits |
| `orchestration/bin/` | `check_branch` et `gcore`, deux outils qui servent encore (voir `orchestration/README.md`) |

Les rapports des agents, leurs relectures, les brouillons de pull requests,
l'ancien `TODO.md` de `todo-status` (`TODO-todo-status.md`), le détail du reste
à faire au 2 octobre et les autres outils de l'orchestration ont été retirés de
l'arbre le 4 octobre. Ils restent dans l'historique de git, au commit
`16a2f60` :

```sh
git ls-tree -r --name-only 16a2f60 todo-notes   # la liste
git show 16a2f60:todo-notes/2026-10-01/04.md    # un fichier
```
