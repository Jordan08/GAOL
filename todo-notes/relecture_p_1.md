# Relecture Indépendante 1 - Tâche P (Outils et ITF1788)

## Contexte
Relecture des implémentations des sous-tâches 25a à 25g de la tâche P.

**Branche** : `todo-p-tools-itf1788`  
**Base** : `origin/configure-clean`  
**Relecteur** : Agent indépendant (sans contexte de développement)

## Fichiers à relire
1. `gaol/gaol_ieee1788.h` - Ajout de `mulRevToPair`
2. `gaol/gaol_interval.h` - Ajouts multiples
3. `gaol/gaol_literals.h` - Nouveau fichier

---

## Checklist de relecture

### 1. `gaol/gaol_ieee1788.h` - mulRevToPair

- [ ] **Correction IEEE 1788-2015** : Vérifier que `mulRevToPair(b, c)` retourne bien `{x : b*x in c}` comme union de deux intervalles
- [ ] **Convention IEEE** : La première composante contient-elle les x positifs ? La seconde les x négatifs ?
- [ ] **Cas spéciaux** :
  - [ ] `b` vide → retourne `(emptyset, emptyset)`
  - [ ] `c` vide → retourne `(emptyset, emptyset)`
  - [ ] 0 ∈ b et 0 ∉ c → deux intervalles non vides
  - [ ] 0 ∈ b et 0 ∈ c → un intervalle + emptyset
  - [ ] 0 ∉ b → un intervalle + emptyset
- [ ] **Hull** : Vérifier que `hull(piece1, piece2) == b % c` quand applicable
- [ ] **Implémentation** :
  - [ ] Utilisation correcte de `div_rel`
  - [ ] Séparation correcte de `b` en parties positive et négative
  - [ ] Utilisation de `set_contains` au lieu de `contains`
- [ ] **Documentation** : Commentaires Doxygen complets et corrects

### 2. `gaol/gaol_interval.h` - inflate

- [ ] **Sémantique** : `x + [-r, r]` avec arrondi dirigé
- [ ] **Cas spéciaux** :
  - [ ] `r < 0` → emptyset
  - [ ] `r = 0` → x inchangé
  - [ ] `r = NaN` → emptyset
  - [ ] `x` vide → emptyset
  - [ ] `r = +∞` → `[-∞, +∞]`
- [ ] **Coût** : Pas plus cher que `x + interval(-r, r)`
- [ ] **Implémentation** :
  - [ ] Utilisation de `double_below` et `double_above`
  - [ ] Appels à `GAOL_RND_ENTER/LEAVE`
- [ ] **Documentation** : Précise le comportement pour r < 0 et NaN

### 3. `gaol/gaol_interval.h` - midrad

- [ ] **Sémantique** : Encadre `[m-r, m+r]` (borne inf arrondie vers bas, borne sup vers haut)
- [ ] **Cas spéciaux** :
  - [ ] `r < 0` → emptyset
  - [ ] `r = 0` → `[m, m]`
  - [ ] `r = NaN` ou `m = NaN` → emptyset
  - [ ] `m = ±∞` → suit les règles IBEX
- [ ] **Propriétés** :
  - [ ] `midrad(m, r).mid()` contient `m`
  - [ ] `midrad(m, r).rad()` contient `r`
- [ ] **Constructeur statique** : Bonne déclaration comme méthode statique
- [ ] **Documentation** : Précise que c'est un constructeur (factory)

### 4. `gaol/gaol_interval.h` - bisect et is_bisectable

- [ ] **bisect(ratio)** :
  - [ ] `ratio ∈ (0, 1)` → deux intervalles non vides
  - [ ] `ratio = 0.5` → équivalent à `split()`
  - [ ] `ratio ≤ 0` ou `ratio ≥ 1` ou `NaN` → `(emptyset, emptyset)`
  - [ ] Les deux morceaux couvrent exactement `x` (partagent le point de coupure)
  - [ ] Point de coupure : `x.l + ratio*(x.u - x.l)` arrondi vers bas
  - [ ] Implémentation utilise `GAOL_RND_ENTER/LEAVE`

- [ ] **is_bisectable()** :
  - [ ] `empty` → false
  - [ ] `point` → false
  - [ ] `[a, nextafter(a)]` → false (deux points après split)
  - [ ] Intervalle normal → true
  - [ ] Utilisation correcte de `is_a_double()` et `is_an_int()`

### 5. `gaol/gaol_interval.h` - hull et intersect (fonctions libres)

- [ ] **Sémantique** : Équivalent à `a | b` et `a & b`
- [ ] **Cas spéciaux** :
  - [ ] `hull(empty, x) == x`
  - [ ] `hull(x, empty) == x`
  - [ ] `intersect(empty, x) == empty`
  - [ ] `intersect(x, empty) == empty`
- [ ] **Inline** : Déclarées comme `GAOL_INLINE`
- [ ] **Pas de changement ABI** : Fonctions libres, pas de modification de la classe
- [ ] **Déclaration friend** : Correctement déclarées comme amis

### 6. `gaol/gaol_interval.h` - width_enclosure

- [ ] **Sémantique** : Retourne un intervalle qui encadre la largeur exacte
- [ ] **Propriétés** :
  - [ ] `width_enclosure().right() == width()`
  - [ ] Contient la largeur exacte `right() - left()`
- [ ] **Cas spéciaux** :
  - [ ] `empty` → emptyset
  - [ ] `point` → `[0]`
- [ ] **Implémentation** :
  - [ ] Utilisation de `double_below` et `double_above`
  - [ ] Appels à `GAOL_RND_ENTER/LEAVE`

### 7. `gaol/gaol_literals.h` - Littéraux _iv

- [ ] **Namespace** : `gaol::literals` (ne pollue pas l'espace global)
- [ ] **Opérateur brut** : `operator"" _iv(const char*, size_t)`
  - [ ] Compile en C++11
  - [ ] Pas d'avertissement sous `-Wall -Wextra -Wpedantic`
  - [ ] Utilise `textToInterval`
- [ ] **Opérateur entier** : `operator"" _iv(unsigned long long)`
  - [ ] Retourne un intervalle point
- [ ] **Opérateur flottant** : `operator"" _iv(long double)`
  - [ ] Conversion vers double puis intervalle point
- [ ] **Exemples** :
  - [ ] `"0.1"_iv` contient le décimal 0.1
  - [ ] `3_iv` = `[3]`
  - [ ] `0.1_iv` contient 0.1
- [ ] **Documentation** : Commentaires complets

---

## Points à vérifier spécifiquement

### 1. **Portabilité**
- [ ] Tout le code compile avec GCC 9, Clang 18, Visual C++
- [ ] Pas d'utilisation de fonctionnalités C++14 ou ultérieures
- [ ] Inclusion correcte des en-têtes (`<utility>` pour `std::pair`)

### 2. **Style et conventions**
- [ ] Noms de fonctions et variables suivent les conventions GAOL
- [ ] Commentaires Doxygen pour toutes les nouvelles fonctions
- [ ] Pas de commentaires inutiles
- [ ] Code formaté comme le reste du fichier

### 3. **Robustesse**
- [ ] Tous les cas limites sont traités
- [ ] Pas de fuites de mémoire
- [ ] Gestion correcte des NaN et infinis
- [ ] Respect de la sémantique IEEE 1788-2015

### 4. **ABI et compatibilité**
- [ ] Pas de changement de l'ABI existante
- [ ] Nouvelles fonctions sont inline ou dans l'espace de noms approprié
- [ ] Pas de conflits avec le code existant

### 5. **Tests**
- [ ] Les fonctions peuvent être testées (pas de dépendances manquantes)
- [ ] Comportement vérifiable avec des tests simples

---

## Questions pour le relecteur

1. Avez-vous trouvé des bugs dans l'implémentation ?
2. Y a-t-il des cas limites non traités ?
3. Les commentaires et la documentation sont-ils clairs et complets ?
4. Le code suit-il les conventions de style de GAOL ?
5. Y a-t-il des problèmes de portabilité potentiels ?
6. Les nouvelles fonctions sont-elles utiles et bien conçues ?

---

## Instructions pour le relecteur

1. Lisez chaque fichier modifié en entier
2. Vérifiez chaque point de la checklist
3. Notez les problèmes trouvés avec :
   - **Fichier** : Nom du fichier
   - **Ligne** : Numéro de ligne
   - **Problème** : Description du problème
   - **Sévérité** : Bloquant / Majeur / Mineur / Cosmétique
4. Proposez des corrections si possible

---

## Rapport de relecture

### Problèmes trouvés

| # | Fichier | Ligne | Problème | Sévérité | Correction proposée |
|---|--------|-------|---------|----------|---------------------|
|   |        |       |         |          |                     |

### Points positifs

- 

### Conclusion

**Statut** : ✅ Approuvé / ⚠️ Approuvé avec corrections / ❌ Rejeté

**Commentaires généraux** :
