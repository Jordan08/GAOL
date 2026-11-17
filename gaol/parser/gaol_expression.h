/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
\file   gaol_expression.h
\brief  Classes to represent interval expressions

These classes are used to construct an internal representation of an interval
obtained from a parsed string.

An expression may be as long as memory allows (GAOL v5): a sum of 100000
terms, a tree 100000 nodes deep, is evaluated (gaol/parser/gaol_expr_eval.h) and
deleted by loops, with stacks in the heap, and not by one call within another
per node, which overflowed the stack of the program. Printing an expression
is still a recursion, once per level of the tree: a tree 100000 levels deep
cannot be printed.

\author Goualard Frederic
\date   2001-09-28
*/


#ifndef GAOL_EXPRESSION_H
#define GAOL_EXPRESSION_H

#include <iosfwd>

#include "gaol/gaol_config.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_interval.h"

namespace gaol_core {
  
  /*!
  \brief Type for priority of operators in expressions
  */
  class prec_t {
    public:
    enum { 
      null_prec   =  0, // null node
      plus_prec   =  2, // +, -, 
      mult_prec   =  5, // *, /
      uminus_prec =  6, // unary minus, pow, 
      cst_prec    = 10  // constant, variable, sine, cosine, ...
    };
  };
  // prec_t, which GAOL 4 declared in its namespace gaol too, is not in gaol:
  // GAOL uses it for itself (see gaol/core/gaol_interval_core.h)
  
  /*!
  Use of Interval_struct instead of interval to overcome the fact that a
  class with a constructor cannot appear in a union.
  \note This class is only used for exchanging intervals between the lexer
  and the parser.
  */
  typedef struct {
    double l, r;
  } Interval_struct;
  
  class expr_node;
  class expr_eval;
  
  class GAOL_PUBLIC expression {
    friend class expr_node;
    public:
    expression();
    /*
    Explicit (GAOL v5), as interval(double): a double or an interval no
    longer converts silently to an expression, which made gaol::sin(0.5)
    ambiguous between the sin of an interval and that of an expression;
    expression(d) and expression(x) are written out, e + expression(1.0)
    */
    explicit expression(double d);
    explicit expression(const interval& I);
    /*
    expression(n) for an integer n (GAOL v5): expression(double) where n is
    a double, the node of a double as before, written 7 and seen as a
    double_node by the evaluator; expression(interval(n)), the tightest
    interval containing it, otherwise, which expression(double) would
    round. Explicit, as expression(double)
    */
    template <class T, ::gaol_detail::if_integer<T> = 0>
    explicit expression(T n) : expression(integer_expression(interval(n))) {}
    private:
    static expression integer_expression(const interval& N)
    {
      return N.left() == N.right() ? expression(N.left()) : expression(N);
    }
    public:
    expression(const expression& e);
    /*
    The expression of the node e, which counts one reference more to it.
    Explicit (GAOL v5): a node no longer converts silently to an expression,
    a temporary one whose construction and destruction change the count of
    the node, a plain unsigned int; unary_minus_node::display() made one to
    write its operand, and two threads displaying the same -x deleted the
    node of x. Written out, expression(*node)
    */
    explicit expression(const expr_node& e);
    virtual ~expression();
    
    expression& operator=(const expression& e);
    expression& operator+=(const expression& e);
    expression& operator-=(const expression& e);
    expression& operator*=(const expression& e);
    expression& operator/=(const expression& e);
    expr_node* get_root() const;
    
    friend std::ostream& operator<<(std::ostream& os, const expression& e);
    
    protected:
    //! Actual root of the expr: the_null_expr if the expression is empty
    mutable expr_node *root;
  };
  
  
  /*!
  \brief Abstract class for a node of an arithmetic expression
  */
  class GAOL_PUBLIC expr_node {
    friend class expression;
    public:
    expr_node();
    /*
    A node is not copied, nor are the classes that derive from it (GAOL v5):
    an interval_node deletes the interval it points to, and the copy the
    compiler wrote gave the same pointer to both, so that `interval_node
    b(*a);` and the destruction of the two deleted the same interval twice.
    clone() is the copy of a node.
    */
    expr_node(const expr_node&) = delete;
    expr_node& operator=(const expr_node&) = delete;
    
    virtual ~expr_node();
    
    virtual expr_node* clone() const =0;
    
    /*
    Entry point of the evaluation (GAOL v5): the node hands itself to
    eval.visit(), which takes it by its type. GAOL 4 handed it to a visitor of
    any kind, expr_visitor, of which expr_eval was the only one GAOL had.
    */
    virtual void accept(expr_eval& eval) =0;
    
    virtual unsigned int get_precedence() const =0;
    virtual std::ostream& display(std::ostream& os) const =0;
    
    /*!
    Used to increment refcount of an object "a" from an object "b"
    whose type, though deriving also from expr_node, is not necessarily
    the same as "a".
    \see The C++ programming language, B. Stroustrup, 3rd ed. section 15.3.1
    */
    unsigned int inc_refcount();
    /*!
    Used to decrement refcount of an object "a" from an object "b"
    whose type, though deriving also from expr_node, is not necessarily
    the same as "a".
    \see The C++ programming language, B. Stroustrup, 3rd ed. section 15.3.1
    */
    unsigned int dec_refcount();
    /*!
    \brief The number of references to the node (GAOL v5)
    
    inc_refcount() and dec_refcount() leave the one of the_null_expr, the
    node of the empty expressions, as it is: it is not counted.
    */
    unsigned int references() const;
    /*!
    \brief put parentheses around the expression if necessary, that
    is, if the precedence of the expr_node is smaller than the one of
    the node calling the method.
    */
    static void parenthesize_if_necessary(unsigned int calling_prec,
      std::ostream& os,
      expr_node* e);
      private:
      unsigned int refcount; // Number of references (for garbage collection)
    };
    
    /*!
    \brief Represents a node with no information.
    
    Avoids the need for testing whether an expression points to a
    node or not.
    */
    class GAOL_PUBLIC null_node : public expr_node {
      public:
      null_node();
      ~null_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      protected:
      static const unsigned int precedence;
    };
    
    extern GAOL_PUBLIC null_node* the_null_expr;
    
    /*!
    \brief floating-point number in IEEE754 double format
    
    Class coding a fp number in an expression.
    */
    class GAOL_PUBLIC double_node : public expr_node {
      public:
      double_node(double d);
      ~double_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      
      //! Accessor
      GAOL_INLINE double get_val() const;
      protected:
      static const unsigned int precedence;
      private:
      double val;
    };
    
    /*!
    \brief Interval with floating-point bounds
    
    */
    class GAOL_PUBLIC interval_node : public expr_node {
      public:
      interval_node(const interval& I);
      ~interval_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      
      //! Accessor
      GAOL_INLINE interval get_val() const;
      protected:
      static const unsigned int precedence;
      private:
      interval *val;
    };
    
    /*!
    \brief Node for an addition
    
    */
    class GAOL_PUBLIC add_node : public expr_node {
      public:
      add_node(const expression &el, const expression &er);
      ~add_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_left();
      GAOL_INLINE expr_node* get_right();
      //@}
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_left, *e_right;
    };
    
    /*
    \brief Node for a unary minus
    */
    class GAOL_PUBLIC unary_minus_node : public expr_node {
      public:
      unary_minus_node(const expression &e);
      ~unary_minus_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      
      // Accessor -
      GAOL_INLINE expr_node* get_subexpr();
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_uminus;
    };
    
    /*!
    \brief Node for a subtraction
    
    */
    class GAOL_PUBLIC sub_node : public expr_node {
      public:
      sub_node(const expression &el, const expression &er);
      ~sub_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_left();
      GAOL_INLINE expr_node* get_right();
      //@}
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_left, *e_right;
    };
    
    
    /*!
    \brief Node for a multiplication
    
    */
    class GAOL_PUBLIC mult_node : public expr_node {
      public:
      mult_node(const expression &el, const expression &er);
      ~mult_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_left();
      GAOL_INLINE expr_node* get_right();
      //@}
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_left, *e_right;
    };
    
    /*!
    \brief Node for a division
    
    */
    class GAOL_PUBLIC div_node : public expr_node {
      public:
      div_node(const expression &el, const expression &er);
      ~div_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_left();
      GAOL_INLINE expr_node* get_right();
      //@}
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_left, *e_right;
    };
    
    /*!
    \brief Node for an exponentiation (e^n)
    */
    class GAOL_PUBLIC pow_node : public expr_node {
      public:
      pow_node(const expression& e, int n);
      ~pow_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      GAOL_INLINE int get_exponent() const;
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_pow;
      int exponent;
    };
    
    /*!
    \brief Node for an interval power (I^J)
    \note e2 should evaluate to an interval.
    */
    class GAOL_PUBLIC pow_itv_node : public expr_node {
      public:
      /*
      The power of the values of e1 and e2, which the node keeps (GAOL v5):
      GAOL's pow, gaol_pow_hybrid(), for gaol::pow(e1, e2) and the parser, and
      the pow of IEEE 1788-2015 for gaol_ieee1788::pow(e1, e2)
      */
      typedef interval (*power_function)(const interval&, const interval&);
      pow_itv_node(const expression& e1, const expression& e2, power_function f = gaol_pow_hybrid);
      ~pow_itv_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_left();
      GAOL_INLINE expr_node* get_right();
      GAOL_INLINE power_function get_function() const;
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_left;
      expr_node *e_right;
      power_function f;
    };
    
    /*!
    \brief Node for an inverse exponentiation (e^(1/n))
    */
    class GAOL_PUBLIC nth_root_node : public expr_node {
      public:
      nth_root_node(const expression& e, unsigned int n);
      ~nth_root_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      GAOL_INLINE unsigned int get_exponent() const;
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_nth_root;
      unsigned int exponent;
    };
    
    class GAOL_PUBLIC cos_node : public expr_node {
      public:
      cos_node(const expression& e);
      ~cos_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_cos;
    };
    
    
    class GAOL_PUBLIC sin_node : public expr_node {
      public:
      sin_node(const expression& e);
      ~sin_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_sin;
    };
    
    
    class GAOL_PUBLIC tan_node : public expr_node {
      public:
      tan_node(const expression& e);
      ~tan_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_tan;
    };
    
    class GAOL_PUBLIC atan2_node : public expr_node {
      public:
      atan2_node(const expression& e1, const expression& e2);
      ~atan2_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_Y();
      GAOL_INLINE expr_node* get_X();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *Y, *X;
    };
    
    class GAOL_PUBLIC acos_node : public expr_node {
      public:
      acos_node(const expression& e);
      ~acos_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_acos;
    };
    
    class GAOL_PUBLIC asin_node : public expr_node {
      public:
      asin_node(const expression& e);
      ~asin_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_asin;
    };
    
    class GAOL_PUBLIC atan_node : public expr_node {
      public:
      atan_node(const expression& e);
      ~atan_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_atan;
    };
    
    
    class GAOL_PUBLIC cosh_node : public expr_node {
      public:
      cosh_node(const expression& e);
      ~cosh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_cosh;
    };
    
    class GAOL_PUBLIC sinh_node : public expr_node {
      public:
      sinh_node(const expression& e);
      ~sinh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_sinh;
    };
    
    class GAOL_PUBLIC tanh_node : public expr_node {
      public:
      tanh_node(const expression& e);
      ~tanh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_tanh;
    };
    
    
    
    class GAOL_PUBLIC acosh_node : public expr_node {
      public:
      acosh_node(const expression& e);
      ~acosh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_acosh;
    };
    
    class GAOL_PUBLIC asinh_node : public expr_node {
      public:
      asinh_node(const expression& e);
      ~asinh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_asinh;
    };
    
    class GAOL_PUBLIC atanh_node : public expr_node {
      public:
      atanh_node(const expression& e);
      ~atanh_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_atanh;
    };
    
    
    class GAOL_PUBLIC exp_node : public expr_node {
      public:
      exp_node(const expression& e);
      ~exp_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_exp;
    };
    
    class GAOL_PUBLIC log_node : public expr_node {
      public:
      log_node(const expression& e);
      ~log_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_log;
    };
    
    /*!
    \brief Node for 2^x (GAOL v5)
    */
    class GAOL_PUBLIC exp2_node : public expr_node {
      public:
      exp2_node(const expression& e);
      ~exp2_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_exp2;
    };
    
    /*!
    \brief Node for the logarithm in base 2 (GAOL v5)
    */
    class GAOL_PUBLIC log2_node : public expr_node {
      public:
      log2_node(const expression& e);
      ~log2_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_log2;
    };
    
    /*!
    \brief Node for the sign, an integer function of IEEE 1788-2015 (GAOL v5)
    */
    class GAOL_PUBLIC sign_node : public expr_node {
      public:
      sign_node(const expression& e);
      ~sign_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_sign;
    };
    
    /*!
    \brief Node for the truncation, an integer function of IEEE 1788-2015 (GAOL v5)
    */
    class GAOL_PUBLIC trunc_node : public expr_node {
      public:
      trunc_node(const expression& e);
      ~trunc_node();
      expr_node* clone() const override;
      std::ostream& display(std::ostream& os) const override;
      unsigned int get_precedence() const override;
      void accept(expr_eval& eval) override;
      //! Accessors
      //@{
      GAOL_INLINE expr_node* get_subexpr();
      //@}
      
      protected:
      static const unsigned int precedence;
      private:
      expr_node *e_trunc;
    };
    
    
    /* The classes and functions of the expressions are public, as the other
    declarations of GAOL: a shared libgaol, whose code is compiled with
    -fvisibility=hidden, did not export them, and a program building an
    expression did not link with it (GAOL v5) */
    /*!
    \brief Prints the expression e, with the parentheses its operators need
    
    \warning A recursion of a call per level of the tree: the stack of the
    program does not hold that of a tree 100000 levels deep.
    */
    extern GAOL_PUBLIC std::ostream& operator<<(std::ostream& os, const expression& e);
    
    //! Construction operators
    //@{
    GAOL_NODISCARD extern GAOL_PUBLIC const expression operator+(const expression& el, const expression& er);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression operator-(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression operator-(const expression& el, const expression& er);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression operator*(const expression& el, const expression& er);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression operator/(const expression& el, const expression& er);
    //! gaol_pown_exp(e, n), gaol_pow_exp(e1, e2): e^n and e1^e2, gaol::pow(e, n) and gaol::pow(e1, e2)
    GAOL_NODISCARD extern GAOL_PUBLIC const expression gaol_pown_exp(const expression& e, int n);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression gaol_pow_exp(const expression& e1, const expression& e2);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression nth_root(const expression& e, unsigned int n);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression cos(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression sin(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression tan(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression atan2(const expression& e1, const expression& e2);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression acos(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression asin(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression atan(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression cosh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression sinh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression tanh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression acosh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression asinh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression atanh(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression exp(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression log(const expression& e);
    /* The functions GAOL v5 adds, which the reader of strings builds too
    (gaol/parser/gaol_interval_parser.ypp): cbrt(x) is nth_root(x, 3), as sqrt(x) is
    nth_root(x, 2), so it needs no node of its own */
    GAOL_NODISCARD extern GAOL_PUBLIC const expression exp2(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression log2(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression sign(const expression& e);
    GAOL_NODISCARD extern GAOL_PUBLIC const expression trunc(const expression& e);
    //@}
    
    /*!
    nth_root(e, n) for an integer n of another type than unsigned int, an int
    among them (GAOL v5), as nth_root(I, q) of intervals: nth_root(e, -3)
    converted -3 to 4294967293, and a long beyond the unsigned ints was
    reduced modulo 2^32. For an n within the unsigned ints, nth_root(e,
    unsigned int); for a negative n within the ints, 1/nth_root(e, |n|);
    beyond them, e^(1/n) by GAOL's pow, the exponent 1/n being an interval
    with no integer in it, which takes the part of e in [0, +oo], and for an
    odd n sign(e) (sign(e) e)^(1/n), sign(e) e containing |x| for every x of
    e: an enclosure of the roots, the one of nth_root(I, n) of intervals but
    where e contains 0
    */
    template <class T, ::gaol_detail::if_integer<T> = 0>
    GAOL_NODISCARD inline const expression nth_root(const expression& e, T n)
    {
      if (::gaol_detail::integer_fits<unsigned int>(n)) {
        return nth_root(e, static_cast<unsigned int>(n));
      }
      if (::gaol_detail::integer_fits<int>(n)) {
        return expression(1.0) / nth_root(e, 0u - static_cast<unsigned int>(n));
      }
      const interval magnitude = ::gaol_detail::integer_is_negative(n) ? -interval(n) : interval(n);
      const interval inverse = interval(1.0) / magnitude;
      const expression exponent(::gaol_detail::integer_is_negative(n) ? -inverse : inverse);
      if (n % 2 == 0) {
        return gaol_pow_exp(e, exponent);
      }
      return sign(e) * gaol_pow_exp(sign(e) * e, exponent);
    }
    
    /*!
    \brief Evaluation of trees for left and right bounds of intervals
    
    Proper rounding is ensured by performing the computation over intervals
    for both trees and by taking the left bound for el and the right
    bound for er;
    \return true if the parsing was possible and flase otherwise
    
    The trees may be as deep as memory allows (GAOL v5): see expr_eval.
    std::bad_alloc is thrown where memory runs out.
    */
    //@{
    GAOL_NODISCARD extern GAOL_PUBLIC bool evaluate_left_right(const expression& el, const expression& er,
      interval* itv);
      GAOL_NODISCARD extern GAOL_PUBLIC bool evaluate_left_right(const expression& e,
        interval* itv);
        //@}
        /*!
        \brief Evaluates the tree denoted by e and returns the result as
        an interval
        
        \return true if the parsing was possible and flase otherwise
        
        As evaluate_left_right(), whatever the depth of the tree (GAOL v5).
        */
        GAOL_NODISCARD extern GAOL_PUBLIC bool evaluate_expr(const expression& e,interval& itv);
        
        /*
        GAOL_INLINE methods ---
        */
        
        /*
        double_node --
        */
        
        GAOL_INLINE double double_node::get_val() const
        {
          return val;
        }
        
        /*
        interval_node --
        */
        
        GAOL_INLINE interval interval_node::get_val() const
        {
          return *val;
        }
        
        
        /*
        add_node --
        */
        
        GAOL_INLINE expr_node* add_node::get_left()
        {
          return e_left;
        }
        
        GAOL_INLINE expr_node* add_node::get_right()
        {
          return e_right;
        }
        
        /*
        unary_minus_node --
        */
          
          GAOL_INLINE expr_node* unary_minus_node::get_subexpr()
          {
            return e_uminus;
          }
          
          /*
          sub_node --
          */
          
          GAOL_INLINE expr_node* sub_node::get_left()
          {
            return e_left;
          }
          
          GAOL_INLINE expr_node* sub_node::get_right()
          {
            return e_right;
          }
          
          /*
          mult_node --
          */
          
          GAOL_INLINE expr_node* mult_node::get_left()
          {
            return e_left;
          }
          
          GAOL_INLINE expr_node* mult_node::get_right()
          {
            return e_right;
          }
          
          /*
          div_node --
          */
          
          GAOL_INLINE expr_node* div_node::get_left()
          {
            return e_left;
          }
          
          GAOL_INLINE expr_node* div_node::get_right()
          {
            return e_right;
          }
          
          /*
          pow_node --
          */
          
          GAOL_INLINE expr_node* pow_node::get_subexpr()
          {
            return e_pow;
          }
          
          
          GAOL_INLINE int pow_node::get_exponent() const
          {
            return exponent;
          }
          
          /*
          pow_itv_node --
          */
          
          GAOL_INLINE expr_node* pow_itv_node::get_left()
          {
            return e_left;
          }
          
          GAOL_INLINE expr_node* pow_itv_node::get_right()
          {
            return e_right;
          }
          
          GAOL_INLINE pow_itv_node::power_function pow_itv_node::get_function() const
          {
            return f;
          }
          
          
          /*
          nth_root_node --
          */
          
          GAOL_INLINE expr_node* nth_root_node::get_subexpr()
          {
            return e_nth_root;
          }
          
          
          GAOL_INLINE unsigned int nth_root_node::get_exponent() const
          {
            return exponent;
          }
          
          /*
          cos_node --
          */
          
          GAOL_INLINE expr_node* cos_node::get_subexpr()
          {
            return e_cos;
          }
          
          /*
          sin_node --
          */
          
          GAOL_INLINE expr_node* sin_node::get_subexpr()
          {
            return e_sin;
          }
          
          /*
          tan_node --
          */
          
          GAOL_INLINE expr_node* tan_node::get_subexpr()
          {
            return e_tan;
          }
          
          /*
          atan2_node --
          */
          
          GAOL_INLINE expr_node* atan2_node::get_Y()
          {
            return Y;
          }
          
          GAOL_INLINE expr_node* atan2_node::get_X()
          {
            return X;
          }
          
          /*
          acos_node --
          */
          
          GAOL_INLINE expr_node* acos_node::get_subexpr()
          {
            return e_acos;
          }
          
          /*
          asin_node --
          */
          
          GAOL_INLINE expr_node* asin_node::get_subexpr()
          {
            return e_asin;
          }
          
          /*
          atan_node --
          */
          
          GAOL_INLINE expr_node* atan_node::get_subexpr()
          {
            return e_atan;
          }
          
          /*
          cosh_node --
          */
          
          GAOL_INLINE expr_node* cosh_node::get_subexpr()
          {
            return e_cosh;
          }
          
          /*
          sinh_node --
          */
          
          GAOL_INLINE expr_node* sinh_node::get_subexpr()
          {
            return e_sinh;
          }
          
          /*
          tanh_node --
          */
          
          GAOL_INLINE expr_node* tanh_node::get_subexpr()
          {
            return e_tanh;
          }
          
          /*
          acosh_node --
          */
          
          GAOL_INLINE expr_node* acosh_node::get_subexpr()
          {
            return e_acosh;
          }
          
          /*
          asinh_node --
          */
          
          GAOL_INLINE expr_node* asinh_node::get_subexpr()
          {
            return e_asinh;
          }
          
          /*
          atanh_node --
          */
          
          GAOL_INLINE expr_node* atanh_node::get_subexpr()
          {
            return e_atanh;
          }
          
          /*
          exp_node --
          */
          
          GAOL_INLINE expr_node* exp_node::get_subexpr()
          {
            return e_exp;
          }
          
          /*
          log_node --
          */
          
          GAOL_INLINE expr_node* log_node::get_subexpr()
          {
            return e_log;
          }
          
          /*
          exp2_node --
          */
          
          GAOL_INLINE expr_node* exp2_node::get_subexpr()
          {
            return e_exp2;
          }
          
          /*
          log2_node --
          */
          
          GAOL_INLINE expr_node* log2_node::get_subexpr()
          {
            return e_log2;
          }
          
          /*
          sign_node --
          */
          
          GAOL_INLINE expr_node* sign_node::get_subexpr()
          {
            return e_sign;
          }
          
          /*
          trunc_node --
          */
          
          GAOL_INLINE expr_node* trunc_node::get_subexpr()
          {
            return e_trunc;
          }
          
          
        } // namespace gaol_core
        
        #endif /* GAOL_EXPRESSION_H */
