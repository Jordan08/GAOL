# Les notes du travail sur le TODO.md

Ce répertoire garde la trace du travail fait sur les points de
[TODO.md](../TODO.md) depuis le 29 septembre 2026 : rapports des agents,
relectures, consignes et outils de l'orchestration. Il vient de la branche
`todo-status`, où il a été écrit jusqu'au 3 octobre, et a été ramené dans
`configure-clean` le 4 octobre, avant la suppression de cette branche. Les
rapports sont copiés tels qu'ils ont été écrits, le plus souvent en anglais.
Les chemins `/tmp/claude-1001/...` et `$SCR` qu'ils citent sont ceux de
sessions passées et n'existent plus.

La procédure en vigueur pour traiter un point est [process.md](../process.md).
Là où elle diffère des consignes de `orchestration/`, c'est elle qui compte :
depuis le 3 octobre, on travaille dans l'arbre principal sans worktree, et
l'agent pousse et ouvre lui-même la pull request.

| Fichier | Contenu |
| --- | --- |
| `NN.md` (`01.md` à `44.md`) | Rapports du 29 septembre, un par ancien numéro de point : commits, résumé, tests et preuve qu'ils échouent sans la correction, validation, questions ouvertes, textes proposés pour `ChangeLog` et `doc/differences.md`, relectures |
| `2026-09-30/` | Rapports et relectures du 30 septembre, brouillons des pull requests #50 et #51, et `hashes.txt` (anciennes et nouvelles empreintes des commits remis au nom de Jordan08) ; voir son `README.md` |
| `2026-10-01/` | Rapports des points 3, 4, 8 et 47 et de la correction armhf ; voir son `README.md` |
| `2026-10-02/reste-detaille.md` | Ce qui restait à faire au commit `61c7503`, en détail, avant le regroupement des points par lettres du 3 octobre |
| `TODO-todo-status.md` | Le `TODO.md` de la branche `todo-status` à son dernier commit (`662f93f`, 3 octobre) : l'avancement du 30 septembre au 3 octobre et les questions ouvertes des pull requests #31 à #63, que les rapports citent ; remplacé depuis par [TODO.md](../TODO.md) |
| `orchestration/` | La méthode du 29 et du 30 septembre, avec plusieurs agents en parallèle : `HOUSE_RULES.md` (consignes lues par chaque agent ; `HOUSE_RULES-2026-09-30.md` en est la version du 30 septembre), les scripts du workflow (`todo_body.js`, `gen_points.py`, `points_w4.py`, `todo_prs.js`) et les outils de `bin/` ; voir son `README.md` |

Deux outils de `orchestration/bin/` servent encore. Copie-les dans ton
scratchpad avant de t'en servir :

- `check_branch <branche> [base]` contrôle une branche avant de la pousser :
  messages de commit, registres, produits de build, espaces. Son `MAIN` est
  l'arbre principal de cette machine.
- `gcore [-n N] <commande>` répartit les 4 cœurs permis entre plusieurs
  agents. Il crée ses verrous dans `../corelocks` à côté de son `bin/` : lancé
  depuis le dépôt, il y laisserait des fichiers non suivis.
