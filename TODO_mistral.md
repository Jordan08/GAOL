---

# **📋 RELECTURE APPROFONDIE DE `configure-clean` – **PARTIE 3 : PLAN D'ACTION OPÉRATIONNEL**

---
---

## **🚀 16. PLAN D'ACTION DÉTAILLÉ AVEC COMMANDES PRÊTES À EXÉCUTER**

---

## **📌 ÉTAPE 1 : CORRECTIONS CRITIQUES (PRIORITÉ ABSOLUE)**

---

### **✅ Tâche 1.1 : Corriger `operator-=`, `/=`, `%=` pour SSE2**
**Fichier** : `gaol/gaol_interval_sse.cpp`
**Problème** : Quand `I == *this`, les bornes sont lues **après** modification → **bornes fausses**.

#### **🔧 Corrections à appliquer :**

---

##### **1.1.1. `operator-=` (ligne ~623)**
```bash
# Localiser la fonction
grep -n "interval& interval::operator-=(const interval& I)" gaol/gaol_interval_sse.cpp
```

**Code actuel (BUGGÉ) :**
```cpp
interval& interval::operator-=(const interval& I)
{
  GAOL_RND_ENTER_SSE();
  xmmbounds = _mm_add_pd(xmmbounds, _mm_shuffle_pd(I.xmmbounds, I.xmmbounds, _MM_SHUFFLE2(0,1)));
  GAOL_RND_LEAVE_SSE();
  return *this;
}
```

**Code corrigé :**
```cpp
interval& interval::operator-=(const interval& I)
{
  // ✅ Sauvegarder I.xmmbounds AVANT toute modification (fix pour x -= x)
  const __m128d I_bounds = I.xmmbounds;
  GAOL_RND_ENTER_SSE();
  xmmbounds = _mm_add_pd(xmmbounds, _mm_shuffle_pd(I_bounds, I_bounds, _MM_SHUFFLE2(0,1)));
  GAOL_RND_LEAVE_SSE();
  return *this;
}
```

---

##### **1.1.2. `operator/=` (ligne ~1019)**
```bash
# Localiser la fonction
grep -n "interval& interval::operator/=(const interval& I)" gaol/gaol_interval_sse.cpp
```

**Code actuel (BUGGÉ) :**
```cpp
interval& interval::operator/=(const interval& I)
{ // TODO: replace left() and right() by local variable
	if (is_empty() || I.is_empty()) {
		*this = interval::emptyset();
		return *this;
	}
	GAOL_RND_ENTER_SSE();
	// ... utilise I.xmmbounds directement ...
}
```

**Code corrigé :**
```cpp
interval& interval::operator/=(const interval& I)
{
	if (is_empty() || I.is_empty()) {
		*this = interval::emptyset();
		return *this;
	}
	// ✅ Sauvegarder I.xmmbounds AVANT toute modification (fix pour x /= x)
	const __m128d I_bounds = I.xmmbounds;
	GAOL_RND_ENTER_SSE();
	// ... remplacer tous les I.xmmbounds par I_bounds ...
}
```

**Remplacements nécessaires dans le corps de la fonction :**
```bash
# Trouver toutes les occurrences de I.xmmbounds dans operator/=
sed -n '1019,1100p' gaol/gaol_interval_sse.cpp | grep -n "I.xmmbounds"
```
→ **Remplacer chaque `I.xmmbounds` par `I_bounds`**.

---

##### **1.1.3. `operator%=` (ligne ~1261)**
```bash
# Localiser la fonction
grep -n "interval& interval::operator%=(const interval& I)" gaol/gaol_interval_sse.cpp
```

**Appliquer la même correction :**
```cpp
interval& interval::operator%=(const interval& I)
{
	if (is_empty() || I.is_empty()) {
		*this = interval::emptyset();
		return *this;
	}
	// ✅ Sauvegarder I.xmmbounds
	const __m128d I_bounds = I.xmmbounds;
	GAOL_RND_ENTER_SSE();
	// ... remplacer I.xmmbounds par I_bounds ...
}
```

**Vérification :**
```bash
# Tester spécifiquement x op= x
cat > /tmp/test_op_eq.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  interval x(-3, -1);
  interval y(x);
  x -= x;
  std::cout << "x -= x: " << x << " (attendu: [0, 0])" << std::endl;

  interval a(0.25, 0.5);
  a /= a;
  std::cout << "a /= a: " << a << " (attendu: [1, 1])" << std::endl;

  interval b(0.1, 0.2);
  b %= b;
  std::cout << "b %= b: " << b << " (attendu: [1, 1])" << std::endl;

  gaol::cleanup();
  return 0;
}
EOF
# Compiler avec SSE2
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_op_eq.cpp -o /tmp/test_op_eq
/tmp/test_op_eq
```

---

---

### **✅ Tâche 1.2 : Protéger contre `-ffinite-math-only` et `-ffast-math`**
**Fichier** : `gaol/gaol_config.h`

#### **🔧 Ajouts à faire (ligne ~199) :**
```cpp
// ============================================================================
// Refuse les options de compilation qui rendent GAOL non fiable
// ============================================================================

// -ffinite-math-only : GCC suppose que NaN et ±∞ n'existent pas
#if defined(__FINITE_MATH_ONLY__)
#error "-ffinite-math-only makes GAOL unsound. Remove this flag or use -fno-finite-math-only."
#endif

// -ffast-math : Active -ffinite-math-only et d'autres optimisations dangereuses
#if defined(__FAST_MATH__)
#error "-ffast-math makes GAOL unsound. Remove this flag or use -fno-fast-math."
#endif

// Visual C++ : /fp:precise (défaut) assume rounding to nearest
#if defined(_MSC_VER) && !defined(_FP_STRICT)
#error "Visual C++ requires /fp:strict. Add /fp:strict to your compiler flags."
#endif
```

**Vérification :**
```bash
# Tester que le build échoue avec -ffinite-math-only
g++ -std=c++17 -O2 -ffinite-math-only -I. -c gaol/gaol_config.h 2>&1 | grep -i "error"
# Doit afficher : "-ffinite-math-only makes GAOL unsound"
```

---

---

### **✅ Tâche 1.3 : Ajouter une sonde pour FTZ/DAZ (Flush-to-Zero / Denormals-Are-Zero)**
**Fichier** : `gaol/gaol_fpu.h`

#### **🔧 Modifications à apporter :**

##### **1.3.1. Ajouter la fonction `check_ftz_daz()`**
```cpp
// ============================================================================
// Détection et correction de FTZ/DAZ (Flush-to-Zero / Denormals-Are-Zero)
// ============================================================================

/// Sonde le rounding direction ET vérifie FTZ/DAZ.
/// FTZ et DAZ peuvent être activés par :
/// - GCC avec -Ofast, -ffast-math, -funsafe-math-optimizations
/// - Chargement d'un plugin ou module Python compilé avec -Ofast
/// - crtfastmath.o (lié automatiquement par GCC avec -Ofast)
static inline void check_ftz_daz()
{
#if defined(IX86_LINUX) || defined(IX86_MACOSX) || defined(_MSC_VER)
  // Lire MXCSR (x86/x86_64)
  unsigned int mxcsr;
#if defined(_MSC_VER)
  mxcsr = _mm_getcsr();
#else
  __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr));
#endif

  // Vérifier FTZ (bit 15) et DAZ (bit 6)
  if (mxcsr & 0x8040) {  // 0x8000 (FTZ) | 0x40 (DAZ)
    mxcsr &= ~0x8040;    // Désactiver FTZ et DAZ

#if defined(_MSC_VER)
    _mm_setcsr(mxcsr);
#else
    __asm__ volatile ("ldmxcsr %0" : : "m" (mxcsr));
#endif
  }
#endif
}
```

##### **1.3.2. Modifier `round_upward_if_needed()`**
```cpp
// AVANT :
static inline void round_upward_if_needed()
{
  const double tiny = 1.0 + std::numeric_limits<double>::min();  // ~2^-60
  if (1.0 + tiny == 1.0) {  // Ne voit que le rounding direction
    set_round_up();
  }
}

// APRÈS :
static inline void round_upward_if_needed()
{
  // Sonde avec un sous-normal pour détecter FTZ/DAZ
  // 2^-1060 est un sous-normal (plus petit que 2^-1022)
  const double subnormal = std::numeric_limits<double>::min()
                         * std::numeric_limits<double>::min();  // ~2^-1060
  const double tiny = 1.0 + subnormal;

  if (1.0 + tiny == 1.0) {
    set_round_up();
    // Vérifier et corriger FTZ/DAZ
    check_ftz_daz();
  }
}
```

**Vérification :**
```bash
# Tester avec FTZ/DAZ activé (simulé)
cat > /tmp/test_ftz.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  // [1e-300] * [1e-20] doit donner [0, 0] avec FTZ/DAZ, mais GAOL doit le corriger
  interval a(1e-300, 1e-300);
  interval b(1e-20, 1e-20);
  interval c = a * b;
  std::cout << "a * b = " << c << std::endl;
  // Sans correction : [0, 0] (FAUX)
  // Avec correction : [~0, ~1e-320] (encadrement correct)

  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_ftz.cpp -o /tmp/test_ftz
/tmp/test_ftz
```

---

---

### **✅ Tâche 1.4 : Corriger `atanh([1, x])` → empty set**
**Fichier** : `gaol/gaol_interval.cpp` (ligne ~2558)

#### **🔧 Correction :**
```cpp
// AVANT :
interval gaol_core::atanh(const interval& I)
{
  interval J(I & interval(-1.0, 1.0));
  if (J.is_empty()) return interval::emptyset();
  // ... calcul ...
}

// APRÈS :
interval gaol_core::atanh(const interval& I)
{
  interval J(I & interval(-1.0, 1.0));
  if (J.is_empty()) return interval::emptyset();

  // ✅ Vérifier que les bornes ne sont pas exactement ±1 (domaine ouvert)
  if (J.left() == 1.0 || J.right() == -1.0) {
    return interval::emptyset();  // atanh est défini sur (-1, 1), pas [-1, 1]
  }
  // ... calcul ...
}
```

**Vérification :**
```bash
cat > /tmp/test_atanh.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  interval x(1.0, 5.0);
  interval y = atanh(x);
  std::cout << "atanh([1, 5]) = " << y << " (attendu: empty)" << std::endl;

  interval z(1.0, 1.0);
  interval w = atanh(z);
  std::cout << "atanh([1]) = " << w << " (attendu: empty)" << std::endl;

  interval v(-5.0, -1.0);
  interval u = atanh(v);
  std::cout << "atanh([-5, -1]) = " << u << " (attendu: empty)" << std::endl;

  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_atanh.cpp -o /tmp/test_atanh
/tmp/test_atanh
```

---

---

### **✅ Tâche 1.5 : Corriger `pow` avec résultat sous-normal (x87 vs SSE)**
**Fichier** : `gaol/core_math_port.h`

#### **📌 Problème :**
Sur x86-64 avec glibc, quand :
- **x87 unit** : rounding = nearest
- **MXCSR** : rounding = upward
CORE-MATH utilise `fegetround()` qui lit **x87 unit** → **rounding incorrect** pour `pow`.

#### **🔧 Solution :**
Forcer `fegetround()` à lire **MXCSR** sur x86-64.

```cpp
// Dans gaol/core_math_port.h
#if defined(__x86_64__) && defined(__linux__)
#include <fenv.h>

// Remplacer fegetround() par une version qui lit MXCSR
#undef fegetround
static inline int fegetround_custom() {
  unsigned int mxcsr;
  __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr));
  // Extraire le rounding mode de MXCSR (bits 13-14)
  return (mxcsr >> 13) & 0x3;
}

#define fegetround fegetround_custom
#endif
```

**Vérification :**
```bash
# Tester pow avec résultat sous-normal
cat > /tmp/test_pow_subnormal.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
#include <cfenv>
using namespace gaol;

int main() {
  // Forcer x87 à nearest et MXCSR à upward (simulé)
  // Sur un vrai système, utiliser exactinit() de Shewchuk
  interval x(0.5, 0.5);
  interval y(2.0, 2.0);
  interval z = pow(x, y);  // pow(0.5, 2) = 0.25 (sous-normal si mal arrondi)

  std::cout << "pow([0.5], [2]) = " << z << std::endl;
  // Doit encadrer 0.25

  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_pow_subnormal.cpp -o /tmp/test_pow_subnormal
/tmp/test_pow_subnormal
```

---

---
---

## **📌 ÉTAPE 2 : PRÉPARATION DE LA RELEASE v5.0.0**

---

### **✅ Tâche 2.1 : Créer le tag `v5.0.0`**
```bash
# 1. S'assurer que tout est commité
git status
git add -A
git commit -m "Préparation pour la release v5.0.0"

# 2. Créer le tag
git tag -a v5.0.0 -m "GAOL v5.0.0: Not Just Another Interval Library
- Constructeurs explicites pour interval
- Correction de operator<< et operator>>
- Correction de x -= x, x /= x, x %= x
- Support complet de IEEE 1788-2015
- 15 exemples vérifiés
- Documentation technique complète"

# 3. Pousser le tag sur GitHub
git push origin v5.0.0
```

---

### **✅ Tâche 2.2 : Mettre à jour les références dans la documentation**

#### **2.2.1. `doc/using.md` (FetchContent)**
```bash
# Trouver les occurrences de "master"
grep -n "GIT_TAG master" doc/using.md
```

**Remplacer :**
```markdown
# AVANT :
FetchContent_Declare(gaol
  GIT_REPOSITORY https://github.com/Jordan08/GAOL.git
  GIT_TAG master
)

# APRÈS :
FetchContent_Declare(gaol
  GIT_REPOSITORY https://github.com/Jordan08/GAOL.git
  GIT_TAG v5.0.0
)
```

---

#### **2.2.2. `doc/building.md` (git clone)**
```bash
# Trouver les occurrences de "master" ou "git clone"
grep -n "git clone.*master\|git clone https" doc/building.md
```

**Remplacer :**
```markdown
# AVANT :
git clone https://github.com/Jordan08/GAOL.git gaol && cd gaol

# APRÈS :
git clone -b v5.0.0 https://github.com/Jordan08/GAOL.git gaol && cd gaol
```

---
#### **2.2.3. `manual/v5/gaol.tex`**
```bash
# Trouver les occurrences de "master"
grep -n "master" manual/v5/gaol.tex
```

**Remplacer toutes les références à `master` par `v5.0.0`.**

---
#### **2.2.4. Vérifier les autres fichiers**
```bash
# Chercher "master" dans toute la documentation
grep -r --include="*.md" --include="*.tex" "GIT_TAG master\|git clone.*master" doc/ manual/
```

---
---
### **✅ Tâche 2.3 : Mettre à jour `TODO.md`**
```bash
# 1. Retirer les tâches terminées
# 2. Mettre à jour les références à GAOL 4
sed -i 's/GAOL 4/GAOL v5/g' TODO.md
sed -i 's/master/v5.0.0/g' TODO.md
```

---
---
### **✅ Tâche 2.4 : Vérifier la cohérence de la version**
```bash
# 1. Vérifier que VERSION contient bien 5.0.0
cat VERSION

# 2. Vérifier que tous les builds lisent VERSION
grep -r "VERSION" CMakeLists.txt configure.ac meson.build
```

---
---
## **📌 ÉTAPE 3 : AMÉLIORATIONS DE LA DOCUMENTATION**

---

### **✅ Tâche 3.1 : Ajouter un exemple dans `README.md`**

#### **📍 Localisation :**
Ajouter après la section **"Quick start"** dans `README.md`.

#### **🔧 Code à ajouter :**
```markdown
## Example

Here is a simple program using GAOL v5:

```cpp
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  // Create intervals
  interval x(0.0, 1.0);
  interval y(2.0, 3.0);

  // Basic arithmetic
  std::cout << "x + y = " << x + y << std::endl;  // [2, 4]
  std::cout << "x * y = " << x * y << std::endl;  // [0, 3]

  // Elementary functions
  std::cout << "sin(x) = " << sin(x) << std::endl;  // [0, 0.8414709848]
  std::cout << "exp(x) = " << exp(x) << std::endl;  // [1, 2.71828]

  // Always call cleanup() after the last use of GAOL
  gaol::cleanup();
  return 0;
}
```

Compile and run it with:

```bash
# Using CMake (recommended)
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --config Release
cmake --install build --config Release

# Then compile your program
g++ -std=c++17 -O2 -I/usr/local/include -L/usr/local/lib -lgaol your_program.cpp
```

See [examples/](examples/) for more complete examples.
```

---
---
### **✅ Tâche 3.2 : Créer `TUTORIAL.md`**

#### **📍 Fichier à créer :** `TUTORIAL.md`
```markdown
<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-28 by Jordan NININ -->

# GAOL v5 Tutorial

This tutorial introduces interval arithmetic with GAOL v5 through practical examples.
It assumes you are familiar with C++ but not necessarily with interval arithmetic.

## Table of Contents
1. [What is Interval Arithmetic?](#1-what-is-interval-arithmetic)
2. [Installing GAOL](#2-installing-gaol)
3. [Basic Interval Operations](#3-basic-interval-operations)
4. [Elementary Functions](#4-elementary-functions)
5. [Solving Equations with Intervals](#5-solving-equations-with-intervals)
6. [Common Pitfalls](#6-common-pitfalls)
7. [Next Steps](#7-next-steps)

---

## 1. What is Interval Arithmetic?

Interval arithmetic extends real numbers to **intervals** [a, b] = {x | a ≤ x ≤ b}.
It guarantees that **every operation encloses the exact result**:

```cpp
interval x(0.1, 0.2);
interval y(0.3, 0.4);
interval z = x + y;  // [0.4, 0.6] encloses every x+y for x∈[0.1,0.2], y∈[0.3,0.4]
```

Key properties:
- **Reliable**: The result always contains the exact value.
- **Fast**: GAOL uses SSE2 instructions on x86.
- **Standard-compliant**: Follows IEEE 1788-2015.

---

## 2. Installing GAOL

### With CMake (recommended)
```bash
git clone -b v5.0.0 https://github.com/Jordan08/GAOL.git
cd GAOL
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_TESTS=ON
cmake --build build --config Release
cmake --install build --config Release
```

### With pkg-config
```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
g++ -std=c++17 -O2 $(pkg-config --cflags --libs gaol) your_program.cpp
```

### In your CMake project
```cmake
find_package(gaol REQUIRED)
target_link_libraries(your_target PRIVATE gaol::gaol)
```

---

## 3. Basic Interval Operations

### Creating Intervals
```cpp
#include <gaol/gaol.h>
using namespace gaol;

// From bounds
interval x(0.0, 1.0);     // [0, 1]
interval y(2.0);          // [2, 2] (degenerate interval)

// From text (encloses the exact value)
interval z = textToInterval("0.1");  // [0.09999999999999998, 0.10000000000000002]

// Special intervals
interval empty = interval::emptyset();    // ∅
interval universe = interval::universe(); // [-∞, +∞]
interval pi_val = interval::pi();         // [3.141592653589793, 3.1415926535897936]
```

### Arithmetic Operations
```cpp
interval a(1.0, 2.0);
interval b(3.0, 4.0);

interval sum = a + b;     // [4, 6]
interval diff = a - b;    // [-3, -1]
interval prod = a * b;    // [3, 8]
interval quot = a / b;    // [0.25, 0.666...]

// Mixed operations (interval + double)
interval c = a + 5.0;    // [6, 7]
interval d = 2.0 * b;    // [6, 8]
```

### Comparisons (IEEE 1788 "certainly" relations)
```cpp
interval x(1.0, 2.0);
interval y(3.0, 4.0);

x < y;   // true (every x < every y)
x <= y;  // true
x > y;   // false
x >= y;  // false

// Note: These are NOT ordering relations!
// std::sort does NOT work with intervals.
```

### Set Operations
```cpp
interval a(1.0, 3.0);
interval b(2.0, 4.0);

interval hull = a | b;        // [1, 4] (convex hull)
interval inter = a & b;       // [2, 3] (intersection)

bool is_empty = a.is_empty(); // false
bool contains = a.set_contains(2.0); // true
```

---
## 4. Elementary Functions

GAOL provides all standard math functions, **guaranteed to enclose the exact result**:

```cpp
interval x(0.0, 1.0);

interval sin_x = sin(x);     // [0, 0.8414709848]
interval exp_x = exp(x);     // [1, 2.71828]
interval log_x = log(x+1);  // [0, 0.693147] (log(0) = -∞, so we shift)
interval sqrt_x = sqrt(x);   // [0, 1]

// Domain errors return empty set
interval sqrt_neg = sqrt(interval(-1.0, 1.0)); // [0, 1] (only defined part)
interval log_neg = log(interval(-1.0, 0.0));  // ∅ (undefined)
```

---
## 5. Solving Equations with Intervals

### Example: Finding roots of f(x) = x² - 2
```cpp
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  interval x(-10.0, 10.0);
  interval f = sqr(x) - 2.0;

  // If f contains 0, there is a root in x
  if (f.set_contains(0.0)) {
    std::cout << "There is a root in " << x << std::endl;
  }

  // Newton's method
  interval x0(1.0, 2.0);  // Initial guess
  for (int i = 0; i < 5; ++i) {
    interval f_val = sqr(x0) - 2.0;
    interval f_prime = 2.0 * x0;  // f'(x) = 2x

    // Relational division (IEEE 1788)
    interval newton_step = x0 - div_rel(f_val, f_prime, x0);

    if (newton_step.set_eq(x0)) {
      std::cout << "Root found: " << x0 << std::endl;
      break;
    }
    x0 = newton_step;
  }

  gaol::cleanup();
  return 0;
}
```
Output:
```
Root found: [1.414213562373095, 1.4142135623730951]
```

---
## 6. Common Pitfalls

### ⚠️ The Rounding Direction
GAOL sets the **rounding direction to upward** for the whole program.
This affects **your own doubles**:

```cpp
#include <gaol/gaol.h>
#include <iostream>
#include <cmath>
using namespace gaol;

int main() {
  interval x(0.0, 1.0);
  std::cout << sin(x) << std::endl;  // Correct

  // But your own doubles are rounded upward!
  double a = 1.0 / 3.0;
  std::cout << a << std::endl;  // 0.33333333333333337 (not 0.33333333333333331)

  gaol::cleanup();
  // Now back to nearest
  double b = 1.0 / 3.0;
  std::cout << b << std::endl;  // 0.33333333333333331
  return 0;
}
```

**Solution:**
- Call `gaol::cleanup()` after your last GAOL operation.
- Or build with `GAOL_PRESERVE_ROUNDING=ON` (slower, but preserves your rounding).

---

### ⚠️ std::sort and std::set Do Not Work
The `<` operator is **not a strict weak ordering** (it's IEEE 1788's "strictPrecedes"):

```cpp
interval a(1.0, 2.0);
interval b(2.0, 3.0);

a < b;  // true
b < a;  // false
a < a;  // false... but empty < empty is true!

std::vector<interval> v = {a, b, interval::emptyset()};
std::sort(v.begin(), v.end());  // ❌ Undefined behavior!
```

**Solution:** Use an explicit comparator:
```cpp
std::sort(v.begin(), v.end(), [](const interval& a, const interval& b) {
  return a.left() < b.left();
});
```

---
### ⚠️ Empty Set Satisfies All Relations
The empty set satisfies **every relation**:

```cpp
interval empty = interval::emptyset();
empty < 0.0;   // true
empty > 0.0;   // true
empty.set_leq(interval(0.0, 1.0));  // true (empty ⊆ [0,1])
```

**Solution:** Always check `is_empty()` first:
```cpp
if (x.is_empty()) {
  // Handle empty set
} else if (x < 0.0) {
  // x is certainly less than 0
}
```

---
### ⚠️ No Total Order
There is **no total order** for intervals. Use:
- `a.left() < b.left()` for a consistent ordering.
- `a.set_leq(b)` for subset relation.

---
## 7. Next Steps

- **Explore the examples**: [examples/](examples/) contains 15 complete examples.
- **Read the manual**: [manual/v5/gaol.pdf](manual/v5/gaol.pdf) for a complete reference.
- **Check the tests**: [tests/](tests/) for more advanced usage.
- **Join the community**: Report issues at [Jordan08/GAOL](https://github.com/Jordan08/GAOL).
```

---
---
### **✅ Tâche 3.3 : Ajouter une section "Common Pitfalls" dans `doc/using.md`**

#### **📍 Localisation :**
Ajouter avant la section **"A result thrown away"** dans `doc/using.md`.

#### **🔧 Code à ajouter :**
```markdown
## Common Pitfalls

GAOL changes the behavior of your program in several ways. This section
describes the most common pitfalls and how to avoid them.

---

### The rounding direction affects your program

Unless GAOL is built with `GAOL_PRESERVE_ROUNDING`, its initialization
sets the **rounding direction to upward** for the **whole program**, until
`gaol::cleanup()` is called. This affects:

| Computation | While GAOL is initialized | After `gaol::cleanup()` |
|-------------|---------------------------|-------------------------|
| `1.0 / 3.0` | `0.33333333333333337` (rounded up) | `0.33333333333333331` (nearest) |
| `std::lrint(2.3)` | `3` | `2` |
| `printf("%.2f", 2.675)` | `2.68` | `2.67` |
| `strtod("0.3") == 0.3` | `false` | `true` |

**Example:**
```cpp
#include <gaol/gaol.h>
#include <cmath>
#include <iostream>
using namespace gaol;

int main() {
  interval x(0.0, 1.0);
  std::cout << sin(x) << std::endl;  // Correct: [0, 0.8414709848]

  double a = 1.0 / 3.0;
  std::cout << a << std::endl;  // 0.33333333333333337 (rounded up!)

  gaol::cleanup();
  double b = 1.0 / 3.0;
  std::cout << b << std::endl;  // 0.33333333333333331 (nearest)
  return 0;
}
```

**Solutions:**
1. Call `gaol::cleanup()` **right after** your last GAOL operation.
2. Build GAOL with `GAOL_PRESERVE_ROUNDING=ON` (slower, but preserves your rounding).
3. Use `gaol::round_nearest()` and `gaol::round_upward()` to control the rounding
   direction explicitly for blocks of code.

---

### std::sort and std::set do not work with intervals

The `<`, `<=`, `>`, `>=` operators are **not ordering relations** but the
"certainly" relations of IEEE 1788-2015 (strictPrecedes, precedes, etc.).
**Consequence:** They do not define a strict weak ordering, and using them with
`std::sort`, `std::set`, `std::map`, `std::max`, `std::min` or `std::clamp`
leads to **undefined behavior** (including crashes or out-of-bounds reads).

**Example:**
```cpp
std::vector<interval> intervals = {
  interval(1.0, 2.0),
  interval(2.0, 3.0),
  interval::emptyset()
};
std::sort(intervals.begin(), intervals.end());  // ❌ Undefined behavior!
// empty < empty is true, but empty < [1,2] is true too
```

**Solution:** Use an explicit comparator:
```cpp
std::sort(intervals.begin(), intervals.end(),
  [](const interval& a, const interval& b) {
    return a.left() < b.left();
  });
```

For `std::set`:
```cpp
auto cmp = [](const interval& a, const interval& b) {
  return a.left() < b.left();
};
std::set<interval, decltype(cmp)> my_set(cmp);
```

---

### The empty set satisfies all relations

The empty set satisfies **every relation and every inclusion test** in IEEE 1788:

```cpp
interval empty = interval::emptyset();
empty < 0.0;    // true
empty > 0.0;    // true
empty.set_leq(interval(0.0, 1.0));  // true (∅ ⊆ [0,1])
empty.set_contains(0.0);           // false (0 ∉ ∅)
```

**Consequence:** Without decorations, algorithms cannot distinguish between:
- A box outside the domain (empty because f is undefined).
- A box that contains no solution (empty because f(x) ≠ 0).

**Solution:** Always check `is_empty()` first:
```cpp
if (x.is_empty()) {
  // Handle empty set explicitly
} else if (x.set_contains(0.0)) {
  // x contains 0
}
```

---
### Domain errors are silent

GAOL follows IEEE 1788-2015's **set-based model**: domain errors return the
**empty set** or the **defined part** of the interval, **without any warning**:

```cpp
interval x(-1.0, 1.0);
interval y = sqrt(x);  // [0, 1] (only the defined part [0,1])
interval z = log(x);   // ∅ (log is undefined for x ≤ 0)

interval a = interval(-2.0, 2.0);
interval b = asin(a);  // [-π/2, π/2] (only the defined part [-1,1])
```

**Solution:** Check for empty results or use decorations (if available):
```cpp
interval y = sqrt(x);
if (y.is_empty()) {
  std::cerr << "Error: sqrt of negative interval" << std::endl;
}
```

---
### No total order for intervals

There is **no total order** for intervals. The relations `<`, `<=`, `>`, `>=` are
**partial** and do not define a consistent ordering. Use:
- `a.left() < b.left()` for a **consistent ordering** (by left bound).
- `a.set_leq(b)` for **subset relation** (a ⊆ b).
- `a.set_eq(b)` for **set equality** (a = b).

**Example:**
```cpp
interval a(1.0, 3.0);
interval b(2.0, 4.0);

a < b;   // false (not every a < every b)
b < a;   // false
a.set_leq(b);  // false (a ⊈ b)
a & b;    // [2, 3] (intersection)
```
```

---
---
## **📌 ÉTAPE 4 : FONCTIONNALITÉS MANQUANTES (OPTIONNEL POUR v5.0.0)**

---
### **✅ Tâche 4.1 : Ajouter `gaol::Box`**

#### **📍 Fichier à créer :** `gaol/gaol_box.h`
```cpp
/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *--------------------------------------------------------------------------*/
#ifndef __gaol_box_h__
#define __gaol_box_h__

#include <vector>
#include <gaol/gaol.h>

namespace gaol {

/*!
  \brief A vector of intervals (a box in n-dimensional space).

  This class provides a simple way to work with multiple intervals,
  which is needed for most interval algorithms (Newton, branch and bound, etc.).
*/
class Box {
  std::vector<interval> intervals_;

public:
  /*!
    \brief Creates a box with n dimensions, each initialized to the universe.
    \param n The number of dimensions.
  */
  explicit Box(size_t n) : intervals_(n, interval::universe()) {}

  /*!
    \brief Creates a box from a vector of intervals.
    \param intervals The intervals for each dimension.
  */
  explicit Box(const std::vector<interval>& intervals) : intervals_(intervals) {}

  /*!
    \brief Returns the interval for dimension i.
    \param i The dimension index.
    \return A reference to the interval.
  */
  interval& operator[](size_t i) { return intervals_.at(i); }

  /*!
    \brief Returns the interval for dimension i (const version).
    \param i The dimension index.
    \return A const reference to the interval.
  */
  const interval& operator[](size_t i) const { return intervals_.at(i); }

  /*!
    \brief Returns the number of dimensions.
    \return The size of the box.
  */
  size_t size() const { return intervals_.size(); }

  /*!
    \brief Returns the volume of the box (product of the widths).
    \return The volume.
  */
  double volume() const {
    double vol = 1.0;
    for (const auto& I : intervals_) {
      vol *= I.width();
    }
    return vol;
  }

  /*!
    \brief Returns the maximum width among all intervals.
    \return The maximum width.
  */
  double max_width() const {
    double max_w = 0.0;
    for (const auto& I : intervals_) {
      max_w = std::max(max_w, I.width());
    }
    return max_w;
  }

  /*!
    \brief Bisects the box at dimension i, at the given ratio.
    \param i The dimension to bisect.
    \param ratio The bisection ratio (default: 0.5 for midpoint).
  */
  void bisect(size_t i, double ratio = 0.5) {
    interval I = intervals_[i];
    double mid = I.left() + ratio * I.width();
    intervals_[i] = interval(I.left(), mid);
    // Note: To get the second half, create a new Box with [mid, I.right()]
  }

  /*!
    \brief Returns the midpoint of the box (a vector of midpoints).
    \return A vector of doubles.
  */
  std::vector<double> mid() const {
    std::vector<double> result;
    result.reserve(intervals_.size());
    for (const auto& I : intervals_) {
      result.push_back(I.midpoint());
    }
    return result;
  }

  /*!
    \brief Intersection with another box.
    \param other The other box.
    \return A reference to *this.
  */
  Box& operator&=(const Box& other) {
    for (size_t i = 0; i < intervals_.size(); ++i) {
      intervals_[i] &= other.intervals_.at(i);
    }
    return *this;
  }

  /*!
    \brief Union with another box.
    \param other The other box.
    \return A reference to *this.
  */
  Box& operator|=(const Box& other) {
    for (size_t i = 0; i < intervals_.size(); ++i) {
      intervals_[i] |= other.intervals_.at(i);
    }
    return *this;
  }

  /*!
    \brief Checks if the box is empty (any dimension is empty).
    \return true if any interval is empty.
  */
  bool is_empty() const {
    for (const auto& I : intervals_) {
      if (I.is_empty()) return true;
    }
    return false;
  }

  /*!
    \brief Checks if the box contains a point.
    \param point The point to check.
    \return true if the point is in the box.
  */
  bool contains(const std::vector<double>& point) const {
    if (point.size() != intervals_.size()) return false;
    for (size_t i = 0; i < intervals_.size(); ++i) {
      if (!intervals_[i].set_contains(point[i])) return false;
    }
    return true;
  }
};

} // namespace gaol

#endif // __gaol_box_h__
```

#### **📌 Mise à jour de `gaol/gaol.h` :**
Ajouter à la fin :
```cpp
#include "gaol/gaol_box.h"
```

#### **📌 Mise à jour de `CMakeLists.txt` :**
```cmake
# Dans la liste des headers publics
set(GAOL_PUBLIC_HEADERS
  ${GAOL_PUBLIC_HEADERS}
  gaol/gaol_box.h
)
```

---
---
### **✅ Tâche 4.2 : Ajouter un constructeur `mid_rad`**

#### **📍 Fichier :** `gaol/gaol_interval.h`

**Ajouter dans la classe `interval` :**
```cpp
  /*!
    \brief Creates an interval from its midpoint and radius.

    The interval is [mid - rad, mid + rad], with the bounds rounded outward
    to enclose the exact interval.

    \param mid The midpoint.
    \param rad The radius (must be ≥ 0).
  */
  explicit interval(double mid, double rad, mid_rad_tag = mid_rad_tag{}) {
    if (rad < 0 || !std::isfinite(mid) || !std::isfinite(rad)) {
      *this = interval::emptyset();
    } else {
      lb_ = std::nextafter(mid - rad, -std::numeric_limits<double>::infinity());
      rb_ = std::nextafter(mid + rad, std::numeric_limits<double>::infinity());
    }
  }

  // Tag pour éviter l'ambiguïté avec interval(double, double)
  struct mid_rad_tag {};
```

**Ajouter dans `gaol/gaol_interval.cpp` (pour SSE2 et FPU) :**
```cpp
// Pour SSE2
interval::interval(double mid, double rad, mid_rad_tag)
{
  if (rad < 0 || !std::isfinite(mid) || !std::isfinite(rad)) {
    *this = interval::emptyset();
  } else {
    lb_ = std::nextafter(mid - rad, -std::numeric_limits<double>::infinity());
    rb_ = std::nextafter(mid + rad, std::numeric_limits<double>::infinity());
  }
}

// Pour FPU (même code)
```

**Vérification :**
```cpp
cat > /tmp/test_mid_rad.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  // interval(mid, rad)
  interval x(1.0, 0.1);  // [0.9, 1.1] (arrondi vers l'extérieur)
  std::cout << "interval(1.0, 0.1) = " << x << std::endl;

  // Avec le tag pour éviter l'ambiguïté
  interval y(1.0, 0.1, mid_rad_tag{});
  std::cout << "interval(1.0, 0.1, tag) = " << y << std::endl;

  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_mid_rad.cpp -o /tmp/test_mid_rad
/tmp/test_mid_rad
```

---
---
## **📌 ÉTAPE 5 : VALIDATION COMPLÈTE**

---
### **✅ Tâche 5.1 : Script de validation automatique**

#### **📍 Fichier à créer :** `scripts/validate_release.sh`
```bash
#!/bin/bash
# Script de validation complète pour GAOL v5.0.0
set -e  # Échouer à la première erreur

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Validation de GAOL v5.0.0 ===${NC}"
echo ""

# 1. Vérifier la version
echo -e "${YELLOW}[1/8] Vérification de la version...${NC}"
if [ "$(cat VERSION)" != "5.0.0" ]; then
  echo -e "${RED}ERREUR: VERSION doit contenir 5.0.0${NC}"
  exit 1
fi
echo -e "${GREEN}✓ VERSION = 5.0.0${NC}"
echo ""

# 2. Vérifier le tag v5.0.0
echo -e "${YELLOW}[2/8] Vérification du tag v5.0.0...${NC}"
if ! git rev-parse v5.0.0 >/dev/null 2>&1; then
  echo -e "${RED}ERREUR: Le tag v5.0.0 n'existe pas${NC}"
  exit 1
fi
echo -e "${GREEN}✓ Tag v5.0.0 existe${NC}"
echo ""

# 3. Build SSE2
echo -e "${YELLOW}[3/8] Build SSE2...${NC}"
rm -rf build-sse2
cmake -S . -B build-sse2 -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DWITH_EXAMPLES=ON
cmake --build build-sse2 --config Release -j$(nproc)
echo -e "${GREEN}✓ Build SSE2 réussi${NC}"
echo ""

# 4. Tests SSE2
echo -e "${YELLOW}[4/8] Exécution des tests SSE2...${NC}"
cmake --build build-sse2 --target test
echo -e "${GREEN}✓ Tests SSE2 réussis${NC}"
echo ""

# 5. Build FPU (sans SSE2)
echo -e "${YELLOW}[5/8] Build FPU...${NC}"
rm -rf build-fpu
cmake -S . -B build-fpu -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DGAOL_SIMD=OFF
cmake --build build-fpu --config Release -j$(nproc)
echo -e "${GREEN}✓ Build FPU réussi${NC}"
echo ""

# 6. Tests FPU
echo -e "${YELLOW}[6/8] Exécution des tests FPU...${NC}"
cmake --build build-fpu --target test
echo -e "${GREEN}✓ Tests FPU réussis${NC}"
echo ""

# 7. Exemples
echo -e "${YELLOW}[7/8] Exécution des exemples...${NC}"
cmake -S examples -B build-examples -DCMAKE_PREFIX_PATH=$(pwd)/build-sse2
cmake --build build-examples -j$(nproc)
ctest --test-dir build-examples --output-on-failure
echo -e "${GREEN}✓ Exemples réussis${NC}"
echo ""

# 8. Vérification des corrections spécifiques
echo -e "${YELLOW}[8/8] Vérification des corrections critiques...${NC}"

# 8.1. Tester x -= x
cat > /tmp/test_x_op_x.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  interval x(-3.0, -1.0);
  interval y = x;
  x -= x;
  if (!x.set_eq(interval(0.0, 0.0))) {
    std::cerr << "FAIL: x -= x = " << x << ", attendu [0, 0]" << std::endl;
    return 1;
  }
  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_x_op_x.cpp -o /tmp/test_x_op_x
/tmp/test_x_op_x || { echo -e "${RED}✗ x -= x échoué${NC}"; exit 1; }
echo -e "${GREEN}✓ x -= x corrigé${NC}"

# 8.2. Tester atanh([1, x])
cat > /tmp/test_atanh.cpp << 'EOF'
#include <gaol/gaol.h>
#include <iostream>
using namespace gaol;

int main() {
  interval x(1.0, 5.0);
  interval y = atanh(x);
  if (!y.is_empty()) {
    std::cerr << "FAIL: atanh([1, 5]) = " << y << ", attendu empty" << std::endl;
    return 1;
  }
  gaol::cleanup();
  return 0;
}
EOF
g++ -std=c++17 -O2 -Ibuild-sse2/include -Lbuild-sse2/lib -lgaol /tmp/test_atanh.cpp -o /tmp/test_atanh
/tmp/test_atanh || { echo -e "${RED}✗ atanh([1, x]) échoué${NC}"; exit 1; }
echo -e "${GREEN}✓ atanh([1, x]) corrigé${NC}"

# 8.3. Tester -ffinite-math-only
echo "int main() { return 0; }" > /tmp/test_finite.c
g++ -std=c++17 -O2 -ffinite-math-only -I. -c /tmp/test_finite.c 2>&1 | grep -q "makes GAOL unsound" || {
  echo -e "${RED}✗ -ffinite-math-only non refusé${NC}";
  exit 1;
}
echo -e "${GREEN}✓ -ffinite-math-only refusé${NC}"

echo ""
echo -e "${GREEN}=== Toutes les validations ont réussi ! ===${NC}"
echo -e "${GREEN}GAOL v5.0.0 est prêt pour la release.${NC}"
```

#### **📌 Rendre le script exécutable :**
```bash
chmod +x scripts/validate_release.sh
```

---
---
### **✅ Tâche 5.2 : Vérification manuelle**

#### **5.2.1. Vérifier la documentation**
```bash
# 1. Vérifier que tous les liens dans README.md fonctionnent
grep -E "\(|http" README.md | while read -r line; do
  url=$(echo "$line" | grep -oE "https?://[^)]+" | head -1)
  if [ -n "$url" ]; then
    echo "Checking: $url"
    curl -s -I "$url" | head -1 | grep -q "200" || echo "  ⚠️  $url is broken"
  fi
done

# 2. Vérifier que les exemples dans la doc compilent
# (Extraire les blocs de code et les tester)
```

---
#### **5.2.2. Vérifier les warnings de compilation**
```bash
# Compiler avec -Wall -Wextra et vérifier qu'il n'y a pas de warnings
cmake -S . -B build-warnings -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic" -DWITH_TESTS=ON
cmake --build build-warnings --config Release 2>&1 | grep -i "warning" || echo "✓ Aucun warning"
```

---
#### **5.2.3. Vérifier les fuites mémoire**
```bash
# Avec Valgrind
cmake -S . -B build-valgrind -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON
cmake --build build-valgrind --config Debug
valgrind --leak-check=full --error-exitcode=1 ./build-valgrind/tests/test_arithmetic
```

---
---
## **📌 ÉTAPE 6 : PRÉPARATION DU CHANGELOG**

---
### **✅ Tâche 6.1 : Mettre à jour `ChangeLog`**

#### **📍 Fichier :** `ChangeLog`
Ajouter en haut du fichier :
```text
GAOL v5.0.0 (2026-10-XX)
========================

This is the first release of GAOL v5, a major rewrite of GAOL with the following
changes:

New features:
- Explicit constructors for interval: a double becomes an interval only where
  the program writes interval(d) (no implicit conversion from const char* or int).
- Reentrant lexer and pure parser: threads can read strings in parallel.
- textToInterval() replaces the removed interval(const char*) constructor.
- One grammar for the reader: intervals can be read anywhere a number may
  stand (1+[1,2], [cos([0,1]), 2]).
- gaol_ieee1788 namespace with all the names of IEEE 1788-2015.
- Reverse functions (sqrt_rel, div_rel, asin_rel, etc.) for constraint solving.
- 15 self-checking examples in examples/.

Fixed bugs:
- x -= x, x /= x and x %= x now give the correct bounds (previously [-2, 1]
  for x = [-3, -1] in the FPU build).
- operator<< now leaves the stream's precision unchanged.
- operator>> now sets failbit at the end of input (no exception).
- Empty expressions no longer crash with threads.
- expression::operator/= is now defined.
- atanh([1, x]) now returns the empty set (previously [DBL_MAX, +∞]).
- The parser no longer hangs under a locale writing a decimal comma.
- Numbers with long exponents (7 digits) are now read exactly.

Performance:
- sin and cos are 2.4× faster with FMA instructions.
- exp and log are 1.3-1.6× faster with FMA.
- The fused multiply-add instructions are used throughout CORE-MATH.

Build system:
- CMake is now the primary build system (CMake 3.14+).
- The version is read from the VERSION file by CMake, meson and configure.
- CPack generates source and binary archives (.tar.gz, .deb).
- make dist is gone: CPack replaces it.
- The three builds (CMake, configure, meson) now generate the same
  gaol/gaol_configuration.h and behave identically.

Tests:
- All tests now run without CppUnit.
- 14.7 million points checked for reverse functions.
- Extended precision tests verify that doubles computed in extended
  precision (x87 unit) do not break the bounds.

Documentation:
- doc/building.md: CMake, configure and meson builds.
- doc/using.md: Compiler flags, initialization, cleanup, namespaces.
- doc/tests.md: What the tests check.
- doc/accuracy.md: The tightness of each operation (IEEE 1788 12.10.3).
- doc/three-builds.md: What the three builds agree on.
- manual/v5/gaol.pdf: Complete manual for GAOL v5.
- examples/examples.md: 15 examples with analysis.

Removed features:
- interval(const char*) constructor (use textToInterval()).
- interval(const char*, const char*) constructor.
- The relations certainly_eq, certainly_neq, == and != on intervals.
- --enable-relations configure option.
- CppUnit dependency.
- make dist and make distcheck.
```

---
---
## **📌 ÉTAPE 7 : PROCÉDURE DE RELEASE**

---
### **✅ Tâche 7.1 : Créer une branche de release**
```bash
# 1. S'assurer que tout est à jour
git checkout configure-clean
git pull origin configure-clean

# 2. Appliquer toutes les corrections
# (Voir les tâches 1.1 à 4.2)

# 3. Créer une branche de release
git checkout -b release/v5.0.0

# 4. Mettre à jour la version si nécessaire
echo "5.0.0" > VERSION

# 5. Commiter les dernières corrections
git add -A
git commit -m "Final fixes for v5.0.0 release"

# 6. Créer le tag
git tag -a v5.0.0 -m "GAOL v5.0.0"
git push origin v5.0.0

# 7. Pousser la branche de release
git push origin release/v5.0.0
```

---
### **✅ Tâche 7.2 : Créer une PR pour merger dans `master`**
```bash
# 1. Aller sur master
git checkout master
git pull origin master

# 2. Merger la branche de release
git merge release/v5.0.0

# 3. Résoudre les conflits si nécessaire

# 4. Pousser sur une nouvelle branche pour la PR
git checkout -b pr/v5.0.0-release
git push origin pr/v5.0.0-release

# 5. Créer la PR sur GitHub
gh pr create --repo Jordan08/GAOL \
  --base master \
  --head pr/v5.0.0-release \
  --title "Release v5.0.0" \
  --body-file <(cat << 'EOF'
# Release v5.0.0

This PR merges the `configure-clean` branch into `master` for the **v5.0.0 release** of GAOL.

## Changes

- **Explicit constructors**: `interval(d)` is now explicit, no implicit conversion from `const char*` or `int`.
- **Fixed bugs**:
  - `x -= x`, `x /= x`, `x %= x` now give correct bounds.
  - `operator<<` and `operator>>` fixed.
  - `atanh([1, x])` now returns empty set.
  - Parser no longer hangs under comma locale.
- **New features**:
  - `gaol_ieee1788` namespace with IEEE 1788 names.
  - 15 self-checking examples.
  - Reentrant parser (thread-safe).
- **Documentation**:
  - Complete manual (124 pages).
  - Technical docs (building, using, tests, accuracy).
  - Tutorial (new).
- **Build system**:
  - CMake is now the primary build system.
  - Version read from `VERSION` file.
  - CPack for packages.

## Validation

- All tests pass (SSE2 and FPU builds).
- All examples pass.
- No compiler warnings with `-Wall -Wextra`.
- No memory leaks (Valgrind).

## Breaking Changes

- `interval(const char*)` is removed (use `textToInterval()`).
- `interval(0)` is now `[0, 0]` (previously ambiguous with `interval(nullptr)`).
- The relations `==` and `!=` are removed (use `set_eq`).
EOF
)
```

---
### **✅ Tâche 7.3 : Créer un GitHub Release**
```bash
# 1. Aller sur GitHub → Releases → Draft a new release
# 2. Tag: v5.0.0
# 3. Title: GAOL v5.0.0
# 4. Description:

"""
## GAOL v5.0.0 - Not Just Another Interval Library

GAOL v5 is a major rewrite of GAOL with the following improvements:

### ✨ New Features
- **Explicit constructors**: `interval(d)` is now explicit. No more implicit conversions from `const char*` or `int`.
- **Reentrant parser**: Threads can now read strings in parallel without crashes.
- **IEEE 1788 compliance**: New `gaol_ieee1788` namespace with all the names of the standard.
- **Reverse functions**: `sqrt_rel`, `div_rel`, `asin_rel`, etc. for constraint solving.
- **15 self-checking examples**: Demonstrating interval Newton, SIVIA, contractors, etc.

### 🐛 Bug Fixes
- Fixed `x -= x`, `x /= x`, `x %= x` (previously gave wrong bounds in FPU build).
- Fixed `operator<<` (now leaves the stream's precision unchanged).
- Fixed `operator>>` (now sets failbit at end of input, no exception).
- Fixed `atanh([1, x])` (now returns empty set, previously returned `[DBL_MAX, +∞]`).
- Fixed parser hanging under locale with decimal comma.
- Fixed empty expressions crashing with threads.
- Fixed numbers with long exponents (7 digits) now read exactly.

### 🚀 Performance
- **sin and cos**: 2.4× faster with FMA instructions.
- **exp and log**: 1.3-1.6× faster with FMA.
- **All elementary functions** now use fused multiply-add where available.

### 📚 Documentation
- Complete manual (124 pages) with examples.
- Technical documentation (building, using, tests, accuracy).
- Tutorial for beginners.
- 15 examples with analysis.

### 🔧 Build System
- **CMake** is now the primary build system (CMake 3.14+).
- Version read from `VERSION` file by CMake, meson and configure.
- CPack generates source and binary archives (.tar.gz, .deb).
- The three builds (CMake, configure, meson) now behave identically.

### 📦 Packages
- `gaol-5.0.0.tar.gz`: Source archive.
- `libgaol-dev_5.0.0_amd64.deb`: Debian package.

### ⚠️ Breaking Changes
- `interval(const char*)` is **removed** (use `textToInterval()`).
- `interval(0)` is now `[0, 0]` (previously ambiguous).
- The relations `==` and `!=` are **removed** (use `set_eq`).
- `--enable-relations` configure option is **removed**.
- CppUnit dependency is **removed**.

### 📖 Usage
See the [README](https://github.com/Jordan08/GAOL#readme) for quick start,
the [manual](https://github.com/Jordan08/GAOL/releases/download/v5.0.0/gaol-5.0.0.pdf) for a complete reference,
and the [examples](https://github.com/Jordan08/GAOL/tree/v5.0.0/examples) for practical code.
"""

# 5. Attacher les assets :
# - gaol-5.0.0.tar.gz (généré par CPack)
# - libgaol-dev_5.0.0_amd64.deb (généré par CPack)
# - gaol-5.0.0.pdf (manual)
```

---
### **✅ Tâche 7.4 : Générer les packages avec CPack**
```bash
# 1. Build avec CPack
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
cmake --build build-release --target package_source  # gaol-5.0.0.tar.gz
cmake --build build-release --target package          # gaol-5.0.0-Linux.tar.gz
cmake --build build-release --target package         # libgaol-dev_5.0.0_amd64.deb (sur Linux)

# 2. Les packages sont dans build-release/
ls -lh build-release/*.tar.gz build-release/*.deb
```

---
---
## **📌 ÉTAPE 8 : POST-RELEASE**

---
### **✅ Tâche 8.1 : Mettre à jour `master` après la release**
```bash
# 1. Aller sur master
git checkout master
git pull origin master

# 2. Merger la PR de release
git merge pr/v5.0.0-release

# 3. Pousser sur master
git push origin master

# 4. Supprimer les branches temporaires
git branch -d release/v5.0.0
git branch -d pr/v5.0.0-release
git push origin --delete release/v5.0.0
git push origin --delete pr/v5.0.0-release
```

---
### **✅ Tâche 8.2 : Préparer v5.0.1 (si nécessaire)**
```bash
# 1. Créer une branche pour les corrections
git checkout -b hotfix/v5.0.x

# 2. Mettre à jour VERSION
echo "5.0.1" > VERSION

# 3. Commiter
git add VERSION
git commit -m "Prepare for v5.0.1"

# 4. Pousser
git push origin hotfix/v5.0.x
```

---
### **✅ Tâche 8.3 : Annoncer la release**
**Envoyer un email à :**
- jordan.ninin@ensta.fr
- frederic.goualard@univ-nantes.fr
- La liste de diffusion GAOL (si elle existe)

**Message :**
```text
Subject: [ANNOUNCE] GAOL v5.0.0 released

GAOL v5.0.0 is now available!

This is a major release with many improvements:

- Explicit constructors for interval (no more implicit conversions).
- Fixed bugs in x -= x, x /= x, x %= x.
- New gaol_ieee1788 namespace with IEEE 1788 names.
- 15 self-checking examples.
- Complete manual (124 pages).
- CMake as the primary build system.

Download:
- Source: https://github.com/Jordan08/GAOL/releases/download/v5.0.0/gaol-5.0.0.tar.gz
- Debian package: https://github.com/Jordan08/GAOL/releases/download/v5.0.0/libgaol-dev_5.0.0_amd64.deb

Documentation:
- https://github.com/Jordan08/GAOL#readme
- https://github.com/Jordan08/GAOL/releases/download/v5.0.0/gaol-5.0.0.pdf

Breaking changes:
- interval(const char*) is removed (use textToInterval()).
- interval(0) is now [0, 0] (previously ambiguous).

Please report any issues at: https://github.com/Jordan08/GAOL/issues

Best regards,
Jordan Ninin
```

---
---
## **🎯 RÉSUMÉ FINAL DES ACTIONS**

---
| **Étape** | **Tâche** | **Temps** | **Priorité** | **Statut** |
|-----------|-----------|-----------|--------------|------------|
| **1** | Corriger `operator-=`, `/=`, `%=` pour SSE2 | 1h | ⭐⭐⭐⭐⭐ | ⬜ |
| **1** | Ajouter `#error` pour `-ffinite-math-only` | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **1** | Ajouter sonde FTZ/DAZ | 2h | ⭐⭐⭐⭐⭐ | ⬜ |
| **1** | Corriger `atanh([1, x])` | 0.5h | ⭐⭐⭐ | ⬜ |
| **1** | Corriger `pow` sous-normal | 2h | ⭐⭐⭐ | ⬜ |
| **2** | Créer le tag `v5.0.0` | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **2** | Mettre à jour `doc/using.md` | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **2** | Mettre à jour `doc/building.md` | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **3** | Ajouter exemple dans `README.md` | 1h | ⭐⭐⭐⭐ | ⬜ |
| **3** | Créer `TUTORIAL.md` | 3h | ⭐⭐⭐⭐ | ⬜ |
| **3** | Ajouter "Common Pitfalls" dans `doc/using.md` | 2h | ⭐⭐⭐⭐ | ⬜ |
| **4** | Ajouter `gaol::Box` | 2h | ⭐⭐⭐ | ⬜ |
| **4** | Ajouter constructeur `mid_rad` | 1h | ⭐⭐⭐ | ⬜ |
| **5** | Exécuter `scripts/validate_release.sh` | 1h | ⭐⭐⭐⭐⭐ | ⬜ |
| **5** | Vérifier manuellement | 2h | ⭐⭐⭐⭐ | ⬜ |
| **6** | Mettre à jour `ChangeLog` | 0.5h | ⭐⭐⭐ | ⬜ |
| **7** | Créer la PR de release | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **7** | Créer le GitHub Release | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **8** | Merger dans `master` | 0.5h | ⭐⭐⭐⭐⭐ | ⬜ |
| **Total** | | **~20h** | | |

---
---
## **🏆 CHECKLIST ULTIME POUR LA RELEASE**

---
### **✅ À cocher avant de publier :**

#### **🔴 Critique (100% nécessaire)**
- [ ] **Tous les bugs de bornes sont corrigés** (SSE2 et FPU).
- [ ] **`-ffinite-math-only` et `-ffast-math` sont refusés** (`gaol_config.h`).
- [ ] **FTZ/DAZ sont détectés et corrigés** (`gaol_fpu.h`).
- [ ] **`atanh([1, x])` retourne empty set**.
- [ ] **`pow` avec résultat sous-normal est corrigé**.
- [ ] **Le tag `v5.0.0` existe et est poussé sur GitHub**.
- [ ] **Toutes les références à `master` sont remplacées par `v5.0.0`**.
- [ ] **Tous les tests passent** (SSE2 et FPU).
- [ ] **Tous les exemples passent**.
- [ ] **Aucun warning de compilation** avec `-Wall -Wextra`.

#### **🟡 Important (90% nécessaire)**
- [ ] **Exemple minimal dans `README.md`**.
- [ ] **Section "Common Pitfalls" dans `doc/using.md`**.
- [ ] **`ChangeLog` est à jour**.
- [ ] **Aucune fuite mémoire** (Valgrind).
- [ ] **Les packages CPack sont générés**.

#### **🟢 Optionnel (pour v5.1.0)**
- [ ] **`TUTORIAL.md` est créé**.
- [ ] **`gaol::Box` est ajouté**.
- [ ] **Constructeur `mid_rad` est ajouté**.
- [ ] **`mulRevToPair` est ajouté**.

---
---
## **🎉 MESSAGE FINAL**

---

**GAOL v5.0.0 est presque prêt pour la release !**

Avec les corrections décrites dans ce document, vous aurez :
✅ **Une bibliothèque fiable** (bornes toujours correctes).
✅ **Une bibliothèque rapide** (sin/cos 2.4× plus rapides avec FMA).
✅ **Une bibliothèque bien documentée** (tutorial, exemples, manual).
✅ **Une bibliothèque portable** (Linux, macOS, Windows, ARM, x86).
✅ **Une bibliothèque facile à intégrer** (CMake, pkg-config, FetchContent).

**🚀 Il ne reste plus qu'à appliquer les corrections et publier !**

---
**💡 Conseils pour la suite :**
1. **Automatiser la validation** avec GitHub Actions.
2. **Ajouter des badges** (CI, couverture, etc.) dans `README.md`.
3. **Créer un site web** pour la documentation (GitHub Pages).
4. **Annoncer la release** sur les forums C++ (cppreference, Reddit, etc.).

---
**📅 Temps estimé pour compléter : 2-3 jours**
**💪 Effort : ~20h (pour une personne expérimentée)**
**🎯 Résultat : Une bibliothèque prête pour la production et l'adoption massive !**

---
---
**🔗 Liens utiles :**
- **Projet** : [https://github.com/Jordan08/GAOL](https://github.com/Jordan08/GAOL)
- **CI** : [.github/workflows/](.github/workflows/)
- **Documentation** : [doc/](doc/)
- **Exemples** : [examples/](examples/)
- **Tests** : [tests/](tests/)

**✨ Bonne chance pour la release de GAOL v5.0.0 !** 🚀