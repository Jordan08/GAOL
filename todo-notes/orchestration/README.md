# Orchestration du travail sur les points du TODO.md

Le 29 et le 30 septembre 2026, les points du `TODO.md` ont été repris par
plusieurs agents en parallèle. Il reste de cette organisation ce qui sert
encore ; le reste (les scripts du workflow, le registre des pull requests, les
outils de suivi des agents et de la CI) est dans l'historique de git, au
commit `16a2f60`. La procédure en vigueur est [process.md](../../process.md).

- `HOUSE_RULES.md` : les consignes que chaque agent lisait d'abord. Elles sont
  à l'origine de `process.md`, qui les remplace là où elles diffèrent.
- `points_w4.py` : les plans détaillés des gros points 25, 26, 27, 28 et 30
  (points P, W, H et X), découpés en sous-fonctions faites chacune dans sa
  branche, puis réunies en une pull request par point. Rien n'en a été fait.
- `bin/check_branch <branche> [base]` : le contrôle d'une branche avant de la
  pousser (messages de commit, registres, produits de build, espaces). Son
  `MAIN` est l'arbre principal de cette machine.
- `bin/gcore [-n N] <commande>` : la répartition des 4 cœurs permis entre
  plusieurs agents, par jetons `flock`. Il crée ses verrous dans `../corelocks`,
  à côté de son `bin/` : copie-le dans ton scratchpad avant de t'en servir,
  sinon il laisse des fichiers non suivis dans le dépôt.
