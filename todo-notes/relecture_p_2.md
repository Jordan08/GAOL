# Relecture Indépendante 2 - Tâche P (Outils et ITF1788)

## Contexte
Relecture des implémentations des sous-tâches 25a à 25g de la tâche P.

**Branche** : `todo-p-tools-itf1788`  
**Base** : `origin/configure-clean`  
**Relecteur** : Agent indépendant (sans contexte de développement)

## Mission
Vérifier que les implémentations respectent :
1. **La norme IEEE 1788-2015** pour les fonctions concernées
2. **Les principes de GAOL v5** (sûreté, portabilité, ABI stable)
3. **Le style de code** existant
4. **La documentation** (Doxygen, manuel)

---

## Vérifications par fonction

### 📋 `mulRevToPair` (gaol/gaol_ieee1788.h)

**Spécification IEEE 1788-2015 (10.5.5)** :
```
mulRevToPair(b, c) = (I₁, I₂)
where I₁ ∪ I₂ = {x ∈ ℝ : ∃ y ∈ b : y·x ∈ c}
```

**À vérifier** :
- [ ] La décomposition en deux intervalles suit la convention de la norme
- [ ] Quand 0 ∈ b et 0 ∉ c, la solution est bien en deux morceaux disjoints
- [ ] Le premier intervalle contient les solutions positives
- [ ] Le second intervalle contient les solutions négatives
- [ ] `hull(I₁, I₂) = b % c` (opérateur étendu de division)
- [ ] Gestion correcte de `b` et `c` vides
- [ ] Gestion correcte de `b` ou `c` contenant 0

**Code à examiner** :
```cpp
GAOL_NODISCARD inline std::pair<interval, interval> mulRevToPair(const interval& b, const interval& c)
{
  if (b.is_empty() || c.is_empty()) {
    return {interval::emptyset(), interval::emptyset()};
  }
  
  bool zero_in_b = b.set_contains(0.0);
  bool zero_in_c = c.set_contains(0.0);
  
  if (zero_in_b && !zero_in_c) {
    interval piece_pos, piece_neg;
    interval b_pos = b & interval(0.0, interval::universe().right());
    interval b_neg = b & interval(interval::universe().left(), 0.0);
    
    if (!b_pos.is_empty()) {
      piece_pos = ::gaol_core::div_rel(c, b_pos, interval::universe());
    }
    if (!b_neg.is_empty()) {
      piece_neg = ::gaol_core::div_rel(c, b_neg, interval::universe());
    }
    
    return {piece_pos, piece_neg};
  } else {
    interval solution = ::gaol_core::div_rel(c, b, interval::universe());
    return {solution, interval::emptyset()};
  }
}
```

**Questions** :
1. Pourquoi utiliser `set_contains(0.0)` plutôt que `contains(0.0)` ?
2. La séparation de `b` en parties positive et négative est-elle correcte ?
3. Que se passe-t-il si `b_pos` ou `b_neg` est vide ?

---

### 📋 `inflate` (gaol/gaol_interval.h)

**Spécification** : x élargi de r des deux côtés

**À vérifier** :
- [ ] `inflate(x, r) = x + [-r, r]` pour r ≥ 0
- [ ] Coût ≤ coût de `x + interval(-r, r)`
- [ ] Gestion de r < 0 : retour emptyset ✓
- [ ] Gestion de r = NaN : retour emptyset ✓
- [ ] Gestion de x vide : retour emptyset ✓
- [ ] Arrondi dirigé : borne inf vers bas, borne sup vers haut

**Code à examiner** :
```cpp
GAOL_INLINE interval interval::inflate(double r) const
{
  if (is_empty() || r < 0.0 || std::isnan(r)) {
    return interval::emptyset();
  }
  if (r == 0.0) {
    return *this;
  }
  GAOL_RND_ENTER();
  double new_left = gaol_detail::double_below(left() - r);
  double new_right = gaol_detail::double_above(right() + r);
  GAOL_RND_LEAVE();
  return interval(new_left, new_right);
}
```

**Questions** :
1. Pourquoi ne pas simplement retourner `*this + interval(-r, r)` ?
2. L'utilisation de `double_below` et `double_above` est-elle nécessaire ou redondante ?

---

### 📋 `midrad` (gaol/gaol_interval.h)

**Spécification** : Constructeur milieu-rayon

**À vérifier** :
- [ ] Retourne l'encadrement le plus serré de [m-r, m+r]
- [ ] Borne inf = double_below(m - r)
- [ ] Borne sup = double_above(m + r)
- [ ] r < 0 → emptyset ✓
- [ ] r = NaN → emptyset ✓
- [ ] m = NaN → emptyset ✓
- [ ] r = 0 → [m, m] ✓

**Code à examiner** :
```cpp
GAOL_INLINE interval interval::midrad(double m, double r)
{
  if (r < 0.0 || std::isnan(r) || std::isnan(m)) {
    return interval::emptyset();
  }
  if (r == 0.0) {
    return interval(m);
  }
  GAOL_RND_ENTER();
  double new_left = gaol_detail::double_below(m - r);
  double new_right = gaol_detail::double_above(m + r);
  GAOL_RND_LEAVE();
  return interval(new_left, new_right);
}
```

**Questions** :
1. Pourquoi `midrad` est une méthode statique et non un constructeur ?
2. Le nom `midrad` est-il clair ? (Alternative : `from_mid_rad`, `midpoint_radius`)

---

### 📋 `bisect` et `is_bisectable` (gaol/gaol_interval.h)

**Spécification** :
- `bisect(x, ratio)` : coupe x au point `x.l + ratio*(x.u - x.l)`
- `bisect(x, 0.5)` doit être égal à `split(x)`

**À vérifier** :
- [ ] Point de coupure calculé avec arrondi dirigé
- [ ] Les deux morceaux partagent le point de coupure
- [ ] Les deux morceaux couvrent exactement x
- [ ] ratio ∈ (0, 1) seulement
- [ ] ratio ≤ 0 ou ≥ 1 ou NaN → (emptyset, emptyset)

**Code à examiner** :
```cpp
GAOL_INLINE std::pair<interval, interval> interval::bisect(double ratio) const
{
  if (is_empty() || std::isnan(ratio) || ratio <= 0.0 || ratio >= 1.0) {
    return {interval::emptyset(), interval::emptyset()};
  }
  GAOL_RND_ENTER();
  double cut = gaol_detail::double_below(left() + ratio * (right() - left()));
  GAOL_RND_LEAVE();
  return {interval(left(), cut), interval(cut, right())};
}
```

**Problèmes potentiels** :
1. ❌ **Bug** : `ratio >= 1.0` devrait être `ratio >= 1.0` mais la condition exclut ratio = 1.0. Que faire pour ratio = 1.0 ?
2. ❌ **Bug** : Le point de coupure utilise `double_below` mais pas `double_above`. Est-ce cohérent ?
3. ⚠️ **Question** : Pourquoi `double_below` sur le point de coupure ? Ne devrait-on pas utiliser le mode d'arrondi courant ?

---

### 📋 `is_bisectable` (gaol/gaol_interval.h)

**Spécification** : Retourne true si x peut être coupé en deux intervalles non-vides et différents de x

**À vérifier** :
- [ ] empty → false ✓
- [ ] point → false ✓
- [ ] [a, nextafter(a)] → false ✓
- [ ] intervalle normal → true ✓

**Code à examiner** :
```cpp
GAOL_INLINE bool interval::is_bisectable() const
{
  if (is_empty() || is_a_double()) {
    return false;
  }
  if (is_an_int()) {
    return false;
  }
  return true;
}
```

**Problèmes potentiels** :
1. ⚠️ **Question** : `is_an_int()` retourne true pour les intervalles contenant un seul entier. Mais `[1, 2]` contient plusieurs entiers et n'est pas un point. Est-ce que `is_an_int()` est la bonne condition ?
2. ❌ **Bug** : `[1, 3]` contient plusieurs entiers mais n'est pas un point. `is_an_int()` retourne false pour cet intervalle, donc `is_bisectable` retourne true. Est-ce correct ?

---

### 📋 `hull` et `intersect` (gaol/gaol_interval.h)

**À vérifier** :
- [ ] `hull(a, b) == a | b`
- [ ] `intersect(a, b) == a & b`
- [ ] Fonctions inline
- [ ] Pas de changement ABI
- [ ] Déclarées comme `friend` dans la classe

**Code à examiner** :
```cpp
friend GAOL_INLINE interval hull(const interval& a, const interval& b);
friend GAOL_INLINE interval intersect(const interval& a, const interval& b);

GAOL_INLINE interval hull(const interval& a, const interval& b)
{
  return a | b;
}

GAOL_INLINE interval intersect(const interval& a, const interval& b)
{
  return a & b;
}
```

**Questions** :
1. Pourquoi déclarer comme `friend` plutôt que comme fonction libre dans le namespace ?
2. Est-ce que ces fonctions sont vraiment nécessaires ou est-ce juste du sucre syntaxique ?

---

### 📋 `width_enclosure` (gaol/gaol_interval.h)

**Spécification** : Retourne un intervalle qui encadre la largeur exacte

**À vérifier** :
- [ ] `width_enclosure().right() == width()`
- [ ] Contient la largeur exacte
- [ ] empty → emptyset
- [ ] point → [0]

**Code à examiner** :
```cpp
GAOL_INLINE interval interval::width_enclosure(void) const
{
  if (is_empty()) {
    return interval::emptyset();
  }
  GAOL_RND_ENTER();
  double w_lower = gaol_detail::double_below(right() - left());
  double w_upper = gaol_detail::double_above(right() - left());
  GAOL_RND_LEAVE();
  return interval(w_lower, w_upper);
}
```

**Problèmes potentiels** :
1. ⚠️ **Inefficacité** : `right() - left()` est calculé deux fois. Peut-on optimiser ?
2. ⚠️ **Question** : Pourquoi calculer `double_below` et `double_above` de la même valeur ? La largeur est déjà arrondie vers le haut par `width()`. Est-ce que `width_enclosure` est vraiment utile ?

---

### 📋 Littéraux `_iv` (gaol/gaol_literals.h)

**À vérifier** :
- [ ] Namespace `gaol::literals` ne pollue pas l'espace global
- [ ] Opérateur brut compile en C++11
- [ ] Pas d'avertissements sous `-Wall -Wextra -Wpedantic`
- [ ] `"0.1"_iv` utilise `textToInterval`
- [ ] `3_iv` retourne `[3]`
- [ ] `0.1_iv` retourne un intervalle contenant 0.1

**Code à examiner** :
```cpp
namespace gaol {
  namespace literals {
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(const char* str, std::size_t N)
    {
      return ::gaol::textToInterval(std::string(str, N));
    }
    
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(unsigned long long n)
    {
      return ::gaol_core::interval(static_cast<double>(n));
    }
    
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(long double d)
    {
      double val = static_cast<double>(d);
      return ::gaol_core::interval(val);
    }
  }
}
```

**Problèmes potentiels** :
1. ❌ **Bug** : L'opérateur pour `long double` ne donne pas un encadrement, mais un intervalle point. Est-ce conforme à la spécification ?
2. ⚠️ **Question** : Pourquoi ne pas utiliser `textToInterval` pour les littéraux numériques aussi, pour plus de cohérence ?
3. ⚠️ **Portabilité** : Les opérateurs littéraux sont-ils supportés par tous les compilateurs cibles (GCC 9, Clang 18, Visual C++) ?

---

## Résumé des problèmes trouvés

### 🔴 **Bugs bloquants**

| # | Fichier | Ligne | Problème | Impact |
|---|--------|-------|---------|--------|
| 1 | gaol/gaol_interval.h | ~1237 | `bisect` : `ratio >= 1.0` exclut ratio = 1.0, mais que faire pour ratio = 1.0 ? | La solution devrait être (x, emptyset) ou (emptyset, x) |
| 2 | gaol/gaol_interval.h | ~1237 | `bisect` : Point de coupure utilise `double_below` seulement. Incohérent. | Peut donner un point de coupure trop bas |
| 3 | gaol/gaol_interval.h | ~1250 | `is_bisectable` : `is_an_int()` n'est pas la bonne condition. `[1, 3]` n'est pas un point mais `is_an_int()` retourne false. | Comportement incorrect pour les intervalles contenant plusieurs entiers |
| 4 | gaol/gaol_literals.h | ~75 | Opérateur `long double` : Ne donne pas un encadrement | Non conforme à la spécification |

### 🟡 **Problèmes majeurs**

| # | Fichier | Ligne | Problème | Impact |
|---|--------|-------|---------|--------|
| 5 | gaol/gaol_interval.h | ~1920 | `width_enclosure` : Calcule `right() - left()` deux fois | Inefficacité |
| 6 | gaol/gaol_literals.h | ~75 | Opérateurs littéraux : Pas de vérification de la portabilité | Peut ne pas compiler sur certains compilateurs |

### 🟢 **Améliorations possibles**

| # | Fichier | Ligne | Problème | Impact |
|---|--------|-------|---------|--------|
| 7 | gaol/gaol_interval.h | ~1920 | `inflate` : Pourquoi ne pas utiliser `*this + interval(-r, r)` ? | Complexité inutile |
| 8 | gaol/gaol_literals.h | ~75 | Opérateurs pour entiers et flottants : Pourquoi ne pas utiliser `textToInterval` ? | Incohérence |

---

## Conclusion

**Statut** : ❌ **Rejeté** - Plusieurs bugs bloquants doivent être corrigés

**Recommandations** :
1. Corriger `bisect` pour gérer correctement ratio = 0.0, 1.0 et utiliser un arrondi cohérent
2. Corriger `is_bisectable` pour utiliser la bonne condition (peut-être `is_a_double()` suffit ?)
3. Corriger l'opérateur `long double` pour donner un encadrement
4. Vérifier la portabilité des opérateurs littéraux
5. Optimiser `width_enclosure` pour éviter le calcul double

**Faut-il une relecture supplémentaire après corrections ?** Oui
