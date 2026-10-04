# Orchestration du travail sur les points du TODO.md

Ces fichiers sont ceux qui ont servi, le 2026-09-29, à reprendre les points du `TODO.md` un par un avec des agents. Ils sont
conservés pour pouvoir reprendre le travail ; les chemins (`/tmp/claude-1001/...`) sont ceux du répertoire de travail de la
session et sont à adapter.

- `HOUSE_RULES.md` : les consignes que chaque agent lit d'abord (règles du mainteneur : messages de commit d'une ou deux lignes
  sans mention de Claude, fichiers ajoutés un par un, quatre cœurs au plus, tests de non-régression qui échouent sans la
  correction, sécurité des bornes d'abord, norme IEEE 1788-2015, etc.).
- `todo_body.js`, `gen_points.py`, `points_w4.py` : le workflow (implémentation dans un worktree, relecture par un ou deux
  relecteurs indépendants, correction des points bloquants) et la table des consignes propres à chaque point, y compris le
  découpage prévu des gros points (25 en onze sous-fonctions a à k, 28 en trois en-têtes, 30 en trois bancs d'essai, 26, 27).
  `gen_points.py` assemble `todo_implement_w4.js` ; `todo_prs.js` écrit, puis relit, le texte français des pull requests.
- `bin/` : `gcore` (au plus quatre cœurs en tout, par jetons `flock`), `newpoint` (worktree et branche d'un point),
  `check_branch` (contrôle d'une branche avant de la pousser), `ci`, `ci_watch`, `ci_summary` (état de la CI des branches),
  `bundle`, `pr_render` (texte de la pull request, avec le registre `registry.json` des numéros), `conflicts` (conflits de
  fusion deux à deux), `resolve_both`, et les outils de suivi des agents.
- `registry.json` : les numéros des pull requests et les branches.
