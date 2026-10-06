# Relecture Indépendante 2 : Tests Unitaires pour Point P (25a-25g)

## Contexte

Relecture technique détaillée des 7 fichiers de tests pour les sous-tâches 25a-25g.

**Focus** : Vérification de la robustesse, de la portabilité et de la conformité aux standards.

---

## 1. `test_mulRevToPair.cpp` - Conformité IEEE 1788-2015

### ✅ Conformité standard
- **Section 10.5.5** : mulRevToPair(b, c) retourne {x : b*x ∈ c} comme union de deux intervalles
- **Convention** : Premier composant = parties positives, second = parties négatives ✓
- **Cas déconnecté** : 0 ∈ b et 0 ∉ c → deux intervalles non-bornés ✓

### 🔍 Analyse technique

```cpp
// Ligne 88-91
check("disconnected: first piece contains positive values",
      pieces.first.left() >= 2.0);
```

**Problème identifié** : Cette vérification accepte `[100, 200]` comme valide pour `b=[-1,1], c=[2,4]`. 

**Attendu** : `pieces.first` devrait être `[2, +∞)` (ou l'encadrement le plus serré).

**Correction requise** :
```cpp
check("disconnected: first piece lower bound is approximately 2.0",
      pieces.first.left() >= 2.0 && pieces.first.left() < 2.1);
// ET vérifier que c'est bien le résultat de div_rel(c, b_pos, entire())
```

### 📋 Checklist IEEE 1788-2015

| Cas | IEEE 1788 | Implémentation | Test | Statut |
|-----|-----------|----------------|------|--------|
| b vide | empty() | ✓ | ✓ | ✅ |
| c vide | empty() | ✓ | ✓ | ✅ |
| 0 ∈ b, 0 ∉ c | Deux intervalles | ✓ | ✓ | ✅ |
| 0 ∈ b, 0 ∈ c | Un intervalle | ✓ | ✓ | ✅ |
| b ⊂ (0,∞) | Un intervalle | ✓ | ✓ | ✅ |
| b ⊂ (-∞,0) | Un intervalle (second) | ✓ | ✓ | ✅ |
| b = [0,0], 0 ∉ c | empty() | ✓ | ✓ | ✅ |
| b = [0,0], 0 ∈ c | entire() | ✓ | ✓ | ✅ |

### ⚠️ Recommandations

1. **Ajouter des tests pour les bornes infinies** :
   - `b = [1, ∞), c = [1, ∞)` → `[1, ∞)`
   - `b = [-∞, -1], c = [-∞, -1]` → `[-∞, -1]`

2. **Vérifier la propriété hull** :
   ```cpp
   auto [p1, p2] = mulRevToPair(b, c);
   interval hull = p1 | p2;
   interval div = div_rel(c, b, entire());
   check("hull equals div_rel", hull == div);
   ```
   **Mais attention** : La comparaison `==` peut échouer à cause des arrondis. Utiliser `set_eq()` ou vérifier que les bornes sont « suffisamment proches ».

3. **Tester les cas pathologiques** :
   - `b = [nextafter(0,-1), nextafter(0,1)]` (intervalle autour de 0)
   - `c = [nextafter(0,1), 1]` (intervalle ne contenant pas 0 mais très proche)

---

## 2. `test_inflate.cpp` - Robustesse numérique

### ✅ Implémentation vérifiée

L'implémentation utilise :
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

### 🔍 Analyse technique

**Problème identifié** : La vérification `inflated.left() >= 0.5` pour `inflate([1,2], 0.5)` est **insuffisante**. 

- **Attendu** : `[0.5, 2.5]`
- **Accepté par le test** : `[100, 200]` (car 100 >= 0.5 et 200 <= 2.5 est faux, mais la logique est la même)

**Pire** : Le test passe même si `inflated = [0.4, 2.6]` car :
- `0.4 >= 0.5` est FAUX
- `2.6 <= 2.5` est FAUX
- Mais le test ne vérifie PAS que les DEUX conditions sont vraies simultanément !

**Correction requise** :
```cpp
interval expected(0.5, 2.5);
check("inflate([1,2], 0.5) = [0.5, 2.5]",
      inflated == expected,
      [&] { return hex(inflated) + " vs " + hex(expected); });
```

### ⚠️ Cas critiques à tester

1. **Débordement** :
   ```cpp
   interval x(DBL_MAX, DBL_MAX);
   interval inflated = x.inflate(1.0);
   // Devrait retourner universe() car DBL_MAX + 1.0 = ∞
   check("inflate([MAX,MAX], 1.0) = universe",
         inflated == interval::universe());
   ```

2. **Sous-débordement** :
   ```cpp
   interval x(-DBL_MAX, -DBL_MAX);
   interval inflated = x.inflate(1.0);
   // Devrait retourner universe() car -DBL_MAX - 1.0 = -∞
   ```

3. **Rayon = DBL_MAX** :
   ```cpp
   interval x(0.0, 0.0);
   interval inflated = x.inflate(DBL_MAX);
   // Devrait retourner universe()
   ```

4. **Intervalle déjà universe** :
   ```cpp
   interval x = interval::universe();
   interval inflated = x.inflate(1.0);
   check("inflate(universe, 1.0) = universe",
         inflated == interval::universe());
   ```

### 📊 Portabilité

- **GCC 9** : ✅ Support complet C++11
- **Clang 18** : ✅ Support complet C++11
- **Visual C++** : ✅ Support C++11, mais vérifier que `std::isnan` est disponible

---

## 3. `test_midrad.cpp` - Précision numérique

### ✅ Implémentation vérifiée

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

### 🔍 Analyse technique

**Problème majeur** : La vérification `x.left() >= 0.5 && x.right() <= 1.5` pour `midrad(1.0, 0.5)` est **complètement inadéquate**.

- **Accepté** : `[0.4, 1.6]` (car 0.4 >= 0.5 est FAUX, mais 1.6 <= 1.5 est FAUX aussi)
- **Accepté** : `[100, 200]` (car 100 >= 0.5 est VRAI, mais 200 <= 1.5 est FAUX)

Le test **ne vérifie pas** que les DEUX conditions sont vraies simultanément !

**Correction requise** :
```cpp
interval expected(0.5, 1.5);
check("midrad(1.0, 0.5) = [0.5, 1.5]",
      x == expected,
      [&] { return hex(x) + " vs " + hex(expected); });
```

### ⚠️ Cas à tester

1. **m = 0.0, r = 0.0** : Devrait retourner `[0,0]`
2. **m = -0.0, r = 0.0** : Devrait retourner `[-0, -0]` ou `[0,0]` selon la gestion des signes de zéro
3. **m = 1.0, r = nextafter(0.0, 1.0)** : Rayon subnormal
4. **m = nextafter(1.0, 2.0), r = 0.0** : Milieu = nextafter(1.0, 2.0)

### 📝 Vérification de l'arrondi

Le test devrait vérifier que :
- `new_left` est le plus grand double <= m - r
- `new_right` est le plus petit double >= m + r

```cpp
// Pour midrad(m, r) = [m-r, m+r]
// Vérifier que :
//   left() <= m - r < nextafter(left(), +∞)
//   previous_double(right()) < m + r <= right()
```

---

## 4. `test_bisect.cpp` - Correction des bugs identifiés

### ✅ Corrections déjà appliquées

Les bugs suivants ont été corrigés dans l'implémentation :
- ✅ `bisect(x, 0.0)` retourne `(emptyset, x)` au lieu de `(emptyset, emptyset)`
- ✅ `bisect(x, 1.0)` retourne `(x, emptyset)` au lieu de `(emptyset, emptyset)`
- ✅ `bisect` utilise `GAOL_RND_KEEP` pour la cohérence avec le mode d'arrondi courant

### 🔍 Analyse technique

**Problème** : La vérification `halves.first.right() <= 2.5` pour `bisect([0,10], 0.25)` est **trop lâche**.

- **Attendu** : `first = [0, 2.5]`, `second = [2.5, 10]`
- **Accepté par le test** : `first = [0, 100]` (car 100 <= 2.5 est FAUX)

**Pire** : Le test ne vérifie pas que `second.left() == first.right()` (point de coupe partagé).

**Correction requise** :
```cpp
check("bisect([0,10], 0.25): first = [0, 2.5]",
      halves.first.left() == 0.0 && halves.first.right() == 2.5,
      [&] { return hex(halves.first); });
check("bisect([0,10], 0.25): second = [2.5, 10]",
      halves.second.left() == 2.5 && halves.second.right() == 10.0,
      [&] { return hex(halves.second); });
```

### ⚠️ Cas à tester

1. **Intervalle avec bornes adjacentes** :
   ```cpp
   interval x(1.0, std::nextafter(1.0, 2.0));
   auto halves = x.bisect(0.5);
   // Devrait retourner deux intervalles vides ou un intervalle vide + l'original
   // selon is_bisectable()
   ```

2. **Ratio très petit** :
   ```cpp
   interval x(0.0, 1.0);
   auto halves = x.bisect(1e-100);
   // Devrait retourner ([0, ~0], [~0, 1])
   ```

3. **Ratio très proche de 1** :
   ```cpp
   interval x(0.0, 1.0);
   auto halves = x.bisect(0.9999999999);
   ```

### 📊 Vérification de is_bisectable

Le test vérifie correctement :
- `empty.is_bisectable() == false` ✓
- `point.is_bisectable() == false` ✓
- `normal.is_bisectable() == true` ✓

**À ajouter** :
```cpp
// Intervalle avec largeur = nextafter(0.0, 1.0) (plus petit double positif)
interval tiny(0.0, std::numeric_limits<double>::min());
check("tiny interval is bisectable", tiny.is_bisectable());
```

---

## 5. `test_hull_intersect.cpp` - Propriétés algébriques

### ✅ Points forts
- Excellente couverture des propriétés algébriques
- Vérification de la commutativité, associativité, idempotence
- Tests des cas spéciaux (vide, universe, point)

### 🔍 Analyse technique

**Amélioration possible** :

La vérification `h == a` pour `hull([0,3], [1,2])` est **correcte** mais pourrait être plus robuste :

```cpp
// Au lieu de :
check("hull([0,3], [1,2]) = [0, 3]", h == a);

// Utiliser :
check("hull([0,3], [1,2]) contains [0,3]", (a & h) == a);
check("hull([0,3], [1,2]) is contained in [0,3]", (h & a) == h);
```

Cela vérifie que `hull(a,b)` est **exactement** `a` sans dépendre de l'implémentation de `==`.

### ⚠️ Cas manquants

1. **Intervalles adjacents** :
   ```cpp
   interval a(0.0, 1.0);
   interval b(1.0, 2.0);
   interval i = intersect(a, b);
   // Devrait être empty() ou [1,1] selon si 1.0 est inclus dans les deux
   ```

2. **Hull avec intervalles semi-infinis** :
   ```cpp
   interval a(-oo, 0.0);
   interval b(1.0, oo);
   interval h = hull(a, b);
   // Devrait être universe()
   ```

3. **Intersect avec intervalles semi-infinis** :
   ```cpp
   interval a(-oo, 1.0);
   interval b(0.0, oo);
   interval i = intersect(a, b);
   // Devrait être [0,1]
   ```

---

## 6. `test_width_enclosure.cpp` - Minimalité de l'encadrement

### ✅ Implémentation vérifiée

```cpp
GAOL_INLINE interval interval::width_enclosure(void) const
{
    if (is_empty()) {
        return interval::emptyset();
    }
    // The width is right() - left(), and we need its enclosure.
    // Compute it once and create the enclosure from below and above.
    GAOL_RND_ENTER();
    double w = right() - left();
    double w_lower = gaol_detail::double_below(w);
    double w_upper = gaol_detail::double_above(w);
    GAOL_RND_LEAVE();
    return interval(w_lower, w_upper);
}
```

### 🔍 Analyse technique

**Problème majeur** : Le test ne vérifie **pas** que l'encadrement est **minimal**.

Pour `width_enclosure()` :
- **Attendu** : Le PLUS PETIT intervalle `[w_lower, w_upper]` tel que `w_lower <= width() <= w_upper`
- **Implémentation** : `w_lower = double_below(width())`, `w_upper = double_above(width())`

**Vérification manquante** :
```cpp
// Vérifier que w_lower est le PLUS GRAND double <= width()
check("w_lower is the greatest double <= width()",
      w_lower <= width() && 
      (w_lower == width() || 
       std::nextafter(w_lower, oo) > width()));

// Vérifier que w_upper est le PLUS PETIT double >= width()
check("w_upper is the smallest double >= width()",
      w_upper >= width() &&
      (w_upper == width() || 
       std::nextafter(w_upper, -oo) < width()));
```

### ⚠️ Cas à tester

1. **width() = 0** :
   ```cpp
   interval x(1.0, 1.0);
   interval we = x.width_enclosure();
   check("width_enclosure of point is [0,0]",
         we.left() == 0.0 && we.right() == 0.0);
   ```

2. **width() = nextafter(0.0, 1.0)** :
   ```cpp
   double w = std::numeric_limits<double>::min();
   interval x(0.0, w);
   interval we = x.width_enclosure();
   // Vérifier que we est bien [w, w] (point interval)
   ```

3. **width() = DBL_MAX** :
   ```cpp
   interval x(-DBL_MAX, DBL_MAX);
   interval we = x.width_enclosure();
   // Vérifier que we contient DBL_MAX
   ```

---

## 7. `test_literals.cpp` - Comportement des littéraux

### ✅ Points forts
- Bonne couverture des différents types de littéraux
- Documentation claire de la différence string vs numeric
- Tests des cas spéciaux (empty, entire)

### 🔍 Analyse technique

**Problème identifié** :

Ligne 85 : `auto x1 = -0.5_iv;`

Le lexer C++ interprète cela comme `-(0.5_iv)`, pas comme un littéral négatif. Cela fonctionne mais peut prêter à confusion.

**Recommandation** :
```cpp
// Remplacer par :
auto x1 = gaol::literals::operator"" _iv(-0.5L);
// ou documenter clairement que les littéraux négatifs sont -(littéral positif)
```

**Autre problème** :

Le test vérifie que `"0.1"_iv != 0.1_iv` mais ne vérifie **pas** que `"0.1"_iv` contient bien le décimal 0.1.

**Ajouter** :
```cpp
// Vérifier que "0.1"_iv contient la valeur exacte 0.1
check("\"0.1\"_iv contains exact decimal 0.1",
      "0.1"_iv.set_contains(0.1));

// Vérifier que 0.1_iv est le point interval du double 0.1
double d01 = 0.1;
check("0.1_iv equals interval(0.1)",
      0.1_iv == gaol_core::interval(d01));
```

### ⚠️ Portabilité

- **GCC 9** : ✅ Support complet C++11 UDL
- **Clang 18** : ✅ Support complet C++11 UDL
- **Visual C++** : ✅ Support C++11 UDL

**Attention** : Les UDL raw string nécessitent C++11. Vérifier que tous les compilateurs supportent :
```cpp
operator"" _iv(const char* str, std::size_t N)
```

---

## Synthèse Technique

### 🟢 Conformité aux standards

| Test | IEEE 1788-2015 | Arrondi dirigé | Portabilité |
|------|-----------------|----------------|------------|
| mulRevToPair | ✅ Partiel | ✅ | ✅ |
| inflate | N/A | ✅ | ✅ |
| midrad | N/A | ✅ | ✅ |
| bisect | N/A | ✅ | ✅ |
| hull/intersect | ✅ | ✅ | ✅ |
| width_enclosure | N/A | ✅ | ✅ |
| literals | N/A | N/A | ✅ |

### 🟡 Problèmes bloquants

1. **Vérifications trop lâches** dans mulRevToPair, inflate, midrad, bisect
   - **Impact** : Tests passent même avec des implémentations incorrectes
   - **Sévérité** : **BLOQUANT**
   - **Correction** : Remplacer par des vérifications précises

2. **Manque de vérification de minimalité** dans width_enclosure
   - **Impact** : Ne détecte pas si l'encadrement n'est pas serré
   - **Sévérité** : **MAJEUR**
   - **Correction** : Ajouter des vérifications de minimalité

### 🟡 Problèmes majeurs

1. **Littéraux négatifs** : Comportement ambigu
   - **Impact** : Peut causer des confusions
   - **Sévérité** : **MAJEUR**
   - **Correction** : Documenter clairement

### 🟢 Points forts

1. Excellente couverture des cas spéciaux
2. Bonne utilisation du framework de test
3. Tests des propriétés algébriques (hull/intersect)
4. Documentation claire

---

## Conclusion

**Statut : APPROUVÉ AVEC CORRECTIONS BLOQUANTES**

Les tests sont bien structurés et couvrent la plupart des cas. Cependant, **les vérifications trop lâches constituent des bugs bloquants** qui doivent être corrigés avant le merge.

### Corrections requises (BLOQUANT) :

1. ✅ **Corriger TOUTES les vérifications trop lâches** (>=, <=) dans :
   - test_mulRevToPair.cpp (4 occurrences)
   - test_inflate.cpp (3 occurrences)
   - test_midrad.cpp (4 occurrences)
   - test_bisect.cpp (3 occurrences)

2. ✅ **Ajouter la vérification de minimalité** dans test_width_enclosure.cpp

### Corrections recommandées (MAJEUR) :

1. ✅ **Clarifier le comportement des littéraux négatifs** dans test_literals.cpp
2. ✅ **Ajouter des tests pour les cas extrêmes** (DBL_MAX, etc.)
3. ✅ **Utiliser set_eq()** pour les comparaisons d'intervalles

### Améliorations optionnelles :

1. Ajouter des tests pour les cas adjacents (hull/intersect)
2. Ajouter des tests pour les intervalles semi-infinis
3. Ajouter des commentaires explicatifs sur les choix de vérification

---

*Relecteur 2 - Date : [À remplir]*
