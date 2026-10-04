/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_expression.h
  \brief  Classes to represent interval expressions

  These classes are used to construct an internal representation of an interval
  obtained from a parsed string.

  \author Goualard Frederic
  \date   2001-09-28
*/


#ifndef GAOL_EXPRESSION_H
#define GAOL_EXPRESSION_H

#include <iosfwd>

#include "gaol/gaol_config.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_flags.h"
#include "gaol/gaol_expr_visitor.h"

namespace gaol_core {

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

  class __GAOL_PUBLIC__ expression {
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
    expression(const expression& e);
    expression(const expr_node& e);
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
  class __GAOL_PUBLIC__ expr_node {
    friend class expression;
  public:
    expr_node();
    expr_node(const expr_node& e);

    virtual ~expr_node();

    virtual expr_node* clone() const =0;

    //! Entry point for visitors.
    virtual void accept(expr_visitor& visitor) =0;

    virtual unsigned int get_precedence() const =0;
    virtual std::ostream& display(std::ostream& os) const =0;

    /*!
      Used to increment refcount of an object "a" from an object "b"
      whose type, though deriving also from expr_node, is not necessarily
      the same as "a".
      \see The C++ programming language, B. Stroustrup, 3rd ed. �15.3.1
    */
    unsigned int inc_refcount();
    /*!
      Used to decrement refcount of an object "a" from an object "b"
      whose type, though deriving also from expr_node, is not necessarily
      the same as "a".
      \see The C++ programming language, B. Stroustrup, 3rd ed. �15.3.1
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
  class __GAOL_PUBLIC__ null_node : public expr_node {
  public:
    null_node();
    ~null_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
  protected:
    static const unsigned int precedence;
  };

  extern __GAOL_PUBLIC__ null_node* the_null_expr;

  /*!
    \brief floating-point number in IEEE754 double format

    Class coding a fp number in an expression.
  */
  class __GAOL_PUBLIC__ double_node : public expr_node {
  public:
    double_node(double d);
    ~double_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);

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
  class __GAOL_PUBLIC__ interval_node : public expr_node {
  public:
    interval_node(const interval& I);
    ~interval_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);

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
  class __GAOL_PUBLIC__ add_node : public expr_node {
  public:
    add_node(const expression &el, const expression &er);
    ~add_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);

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
  class __GAOL_PUBLIC__ unary_minus_node : public expr_node {
  public:
    unary_minus_node(const expression &e);
    ~unary_minus_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);

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
  class __GAOL_PUBLIC__ sub_node : public expr_node {
  public:
    sub_node(const expression &el, const expression &er);
    ~sub_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ mult_node : public expr_node {
  public:
    mult_node(const expression &el, const expression &er);
    ~mult_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ div_node : public expr_node {
  public:
    div_node(const expression &el, const expression &er);
    ~div_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ pow_node : public expr_node {
  public:
    pow_node(const expression& e, int n);
    ~pow_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ pow_itv_node : public expr_node {
  public:
    /*
      The power of the values of e1 and e2, which the node keeps (GAOL v5):
      GAOL's pow, gaol_pow_hybrid(), for gaol::pow(e1, e2) and the parser, and
      the pow of IEEE 1788-2015 for gaol_ieee1788::pow(e1, e2)
    */
    typedef interval (*power_function)(const interval&, const interval&);
    pow_itv_node(const expression& e1, const expression& e2, power_function f = gaol_pow_hybrid);
    ~pow_itv_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ nth_root_node : public expr_node {
  public:
    nth_root_node(const expression& e, unsigned int n);
    ~nth_root_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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

  class __GAOL_PUBLIC__ cos_node : public expr_node {
  public:
    cos_node(const expression& e);
    ~cos_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_cos;
  };


  class __GAOL_PUBLIC__ sin_node : public expr_node {
  public:
    sin_node(const expression& e);
    ~sin_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_sin;
  };


  class __GAOL_PUBLIC__ tan_node : public expr_node {
  public:
    tan_node(const expression& e);
    ~tan_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_tan;
  };

  class __GAOL_PUBLIC__ atan2_node : public expr_node {
  public:
    atan2_node(const expression& e1, const expression& e2);
    ~atan2_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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

  class __GAOL_PUBLIC__ acos_node : public expr_node {
  public:
    acos_node(const expression& e);
    ~acos_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_acos;
  };

  class __GAOL_PUBLIC__ asin_node : public expr_node {
  public:
    asin_node(const expression& e);
    ~asin_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_asin;
  };

  class __GAOL_PUBLIC__ atan_node : public expr_node {
  public:
    atan_node(const expression& e);
    ~atan_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_atan;
  };


  class __GAOL_PUBLIC__ cosh_node : public expr_node {
  public:
    cosh_node(const expression& e);
    ~cosh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_cosh;
  };

  class __GAOL_PUBLIC__ sinh_node : public expr_node {
  public:
    sinh_node(const expression& e);
    ~sinh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_sinh;
  };

  class __GAOL_PUBLIC__ tanh_node : public expr_node {
  public:
    tanh_node(const expression& e);
    ~tanh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_tanh;
  };



  class __GAOL_PUBLIC__ acosh_node : public expr_node {
  public:
    acosh_node(const expression& e);
    ~acosh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_acosh;
  };

  class __GAOL_PUBLIC__ asinh_node : public expr_node {
  public:
    asinh_node(const expression& e);
    ~asinh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_asinh;
  };

  class __GAOL_PUBLIC__ atanh_node : public expr_node {
  public:
    atanh_node(const expression& e);
    ~atanh_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_atanh;
  };


  class __GAOL_PUBLIC__ exp_node : public expr_node {
  public:
    exp_node(const expression& e);
    ~exp_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
    //! Accessors
    //@{
    GAOL_INLINE expr_node* get_subexpr();
    //@}

  protected:
    static const unsigned int precedence;
  private:
    expr_node *e_exp;
  };

  class __GAOL_PUBLIC__ log_node : public expr_node {
  public:
    log_node(const expression& e);
    ~log_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ exp2_node : public expr_node {
  public:
    exp2_node(const expression& e);
    ~exp2_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ log2_node : public expr_node {
  public:
    log2_node(const expression& e);
    ~log2_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ sign_node : public expr_node {
  public:
    sign_node(const expression& e);
    ~sign_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  class __GAOL_PUBLIC__ trunc_node : public expr_node {
  public:
    trunc_node(const expression& e);
    ~trunc_node();
    expr_node* clone() const;
    std::ostream& display(std::ostream& os) const;
    unsigned int get_precedence() const;
    GAOL_INLINE void accept(expr_visitor& visitor);
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
  extern __GAOL_PUBLIC__ std::ostream& operator<<(std::ostream& os, const expression& e);

  //! Construction operators
  //@{
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression operator+(const expression& el, const expression& er);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression operator-(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression operator-(const expression& el, const expression& er);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression operator*(const expression& el, const expression& er);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression operator/(const expression& el, const expression& er);
  //! gaol_pown_exp(e, n), gaol_pow_exp(e1, e2): e^n and e1^e2, gaol::pow(e, n) and gaol::pow(e1, e2)
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression gaol_pown_exp(const expression& e, int n);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression gaol_pow_exp(const expression& e1, const expression& e2);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression nth_root(const expression& e, unsigned int n);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression cos(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression sin(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression tan(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression atan2(const expression& e1, const expression& e2);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression acos(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression asin(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression atan(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression cosh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression sinh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression tanh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression acosh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression asinh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression atanh(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression exp(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression log(const expression& e);
  /* The functions GAOL v5 adds, which the reader of strings builds too
     (gaol/gaol_interval_parser.ypp): cbrt(x) is nth_root(x, 3), as sqrt(x) is
     nth_root(x, 2), so it needs no node of its own */
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression exp2(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression log2(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression sign(const expression& e);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ const expression trunc(const expression& e);
  //@}

  /*!
    \brief Evaluation of trees for left and right bounds of intervals

    Proper rounding is ensured by performing the computation over intervals
    for both trees and by taking the left bound for el and the right
    bound for er;
    \return true if the parsing was possible and flase otherwise
  */
  //@{
  GAOL_NODISCARD extern __GAOL_PUBLIC__ bool evaluate_left_right(const expression& el, const expression& er,
			   interval* itv);
  GAOL_NODISCARD extern __GAOL_PUBLIC__ bool evaluate_left_right(const expression& e,
			   interval* itv);
  //@}
  /*!
    \brief Evaluates the tree denoted by e and returns the result as
    an interval

    \return true if the parsing was possible and flase otherwise
  */
  GAOL_NODISCARD extern __GAOL_PUBLIC__ bool evaluate_expr(const expression& e,interval& itv);

  /*
    GAOL_INLINE methods ---
   */

  /*
    null_node --
  */
  GAOL_INLINE void null_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "null_node accepting visitor" << std::endl);
    visitor.visit(this);
  }


  /*
    double_node --
  */
  GAOL_INLINE void double_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "double_node accepting visitor" << std::endl);
    visitor.visit(this);
  }

  GAOL_INLINE double double_node::get_val() const
  {
    return val;
  }

  /*
    interval_node --
  */
  GAOL_INLINE void interval_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "interval_node accepting visitor" << std::endl);
    visitor.visit(this);
  }

  GAOL_INLINE interval interval_node::get_val() const
  {
    return *val;
  }


  /*
    add_node --
  */
  GAOL_INLINE void add_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "add_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void unary_minus_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "unary_minus_node accepting visitor"
		<< std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* unary_minus_node::get_subexpr()
  {
    return e_uminus;
  }

  /*
    sub_node --
  */
  GAOL_INLINE void sub_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "sub_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void mult_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "mult_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void div_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "div_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void pow_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "pow_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void pow_itv_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "pow_itv_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void nth_root_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "nth_root_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void cos_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "cos_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* cos_node::get_subexpr()
  {
    return e_cos;
  }

  /*
    sin_node --
  */
  GAOL_INLINE void sin_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "sin_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* sin_node::get_subexpr()
  {
    return e_sin;
  }

  /*
    tan_node --
  */
  GAOL_INLINE void tan_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "tan_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* tan_node::get_subexpr()
  {
    return e_tan;
  }

  /*
    atan2_node --
  */
  GAOL_INLINE void atan2_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "atan2_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

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
  GAOL_INLINE void acos_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "acos_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* acos_node::get_subexpr()
  {
    return e_acos;
  }

  /*
    asin_node --
  */
  GAOL_INLINE void asin_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "asin_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* asin_node::get_subexpr()
  {
    return e_asin;
  }

  /*
    atan_node --
  */
  GAOL_INLINE void atan_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "atan_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* atan_node::get_subexpr()
  {
    return e_atan;
  }

  /*
    cosh_node --
  */
  GAOL_INLINE void cosh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "cosh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* cosh_node::get_subexpr()
  {
    return e_cosh;
  }

  /*
    sinh_node --
  */
  GAOL_INLINE void sinh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "sinh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* sinh_node::get_subexpr()
  {
    return e_sinh;
  }

  /*
    tanh_node --
  */
  GAOL_INLINE void tanh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "tanh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* tanh_node::get_subexpr()
  {
    return e_tanh;
  }

  /*
    acosh_node --
  */
  GAOL_INLINE void acosh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "acosh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* acosh_node::get_subexpr()
  {
    return e_acosh;
  }

  /*
    asinh_node --
  */
  GAOL_INLINE void asinh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "asinh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* asinh_node::get_subexpr()
  {
    return e_asinh;
  }

  /*
    atanh_node --
  */
  GAOL_INLINE void atanh_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "atanh_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* atanh_node::get_subexpr()
  {
    return e_atanh;
  }

  /*
    exp_node --
  */
  GAOL_INLINE void exp_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "exp_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* exp_node::get_subexpr()
  {
    return e_exp;
  }

  /*
    log_node --
  */
  GAOL_INLINE void log_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "log_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* log_node::get_subexpr()
  {
    return e_log;
  }

  /*
    exp2_node --
  */
  GAOL_INLINE void exp2_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "exp2_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* exp2_node::get_subexpr()
  {
    return e_exp2;
  }

  /*
    log2_node --
  */
  GAOL_INLINE void log2_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "log2_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* log2_node::get_subexpr()
  {
    return e_log2;
  }

  /*
    sign_node --
  */
  GAOL_INLINE void sign_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "sign_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* sign_node::get_subexpr()
  {
    return e_sign;
  }

  /*
    trunc_node --
  */
  GAOL_INLINE void trunc_node::accept(expr_visitor& visitor)
  {
    GAOL_DEBUG(2,std::cout << "trunc_node accepting visitor" << std::endl;);
    visitor.visit(this);
  }

  GAOL_INLINE expr_node* trunc_node::get_subexpr()
  {
    return e_trunc;
  }


} // namespace gaol_core

/*
  pow(e, n), pow(e1, e2): the expressions of GAOL's power, gaol_pow_exp(), in
  the namespace gaol as the pow of intervals (gaol/gaol_interval.h), no pow
  being in gaol_core (GAOL v5). An interval converting to an expression,
  argument-dependent lookup found a pow of expressions in gaol_core for a call
  pow(x, 3) on an interval too, which was ambiguous with
  gaol_ieee1788::pow(x, [3]).
*/
namespace gaol {
  using namespace gaol_core;
  GAOL_NODISCARD inline const expression pow(const expression& e, int n) { return gaol_core::gaol_pown_exp(e, n); }
  GAOL_NODISCARD inline const expression pow(const expression& e1, const expression& e2) { return gaol_core::gaol_pow_exp(e1, e2); }
} // namespace gaol

/*
  The expressions in gaol_ieee1788 (gaol/gaol_ieee1788.h), which no longer
  includes this file (GAOL v5): gaol::sin(0.5) was ambiguous between the sin
  of intervals and that of expressions, a double converting to both, wherever
  gaol/gaol was included. A using-declaration names the overloads declared
  before it: those of gaol_ieee1788 are declared again here, after the
  overloads of expressions, so that gaol_ieee1788::sin(e) takes them whichever
  of the two files is included first.
*/
#include "gaol/gaol_ieee1788.h"

namespace gaol_ieee1788 {
  using ::gaol_core::exp;
  using ::gaol_core::exp2;
  using ::gaol_core::log;
  using ::gaol_core::log2;
  using ::gaol_core::sin;
  using ::gaol_core::cos;
  using ::gaol_core::tan;
  using ::gaol_core::asin;
  using ::gaol_core::acos;
  using ::gaol_core::atan;
  using ::gaol_core::atan2;
  using ::gaol_core::sinh;
  using ::gaol_core::cosh;
  using ::gaol_core::tanh;
  using ::gaol_core::asinh;
  using ::gaol_core::acosh;
  using ::gaol_core::atanh;
  using ::gaol_core::sign;
  using ::gaol_core::trunc;

  /*!
    pown(e, n), pow(e1, e2): the expressions of pown and pow. pown(e, n) is
    gaol_pown_exp(e, n), whose node is computed by gaol_pown(), the pown of
    the standard. The node of pow(e1, e2) keeps the function computing it:
    gaol_ieee1788::pow(x, y), where gaol_pow_exp(e1, e2), which is
    gaol::pow(e1, e2), keeps GAOL's pow, [1, 16] for [-4, -1]^[2].
  */
  GAOL_NODISCARD inline const ::gaol_core::expression pown(const ::gaol_core::expression& e, int n)
  {
    return ::gaol_core::gaol_pown_exp(e, n);
  }
  GAOL_NODISCARD inline const ::gaol_core::expression pow(const ::gaol_core::expression& e1, const ::gaol_core::expression& e2)
  {
    const ::gaol_core::pow_itv_node::power_function standard_pow = pow;
    return *(new ::gaol_core::pow_itv_node(e1, e2, standard_pow));
  }
} // namespace gaol_ieee1788

#endif /* GAOL_EXPRESSION_H */
