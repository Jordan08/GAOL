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
  \file   gaol_expression.cpp
  \brief


  \author Goualard Frederic
  \date   2001-09-28
*/

#include <iostream>
#include <cmath>
#include <new>
#include <vector>


#include "gaol/gaol_expression.h"
#include "gaol/gaol_expr_eval.h"

namespace gaol_core {
  // Node used for empty expressions to have something to point at
  null_node* the_null_expr;

  const unsigned null_node::precedence = prec_t::null_prec;
  const unsigned double_node::precedence = prec_t::cst_prec;
  const unsigned interval_node::precedence = prec_t::cst_prec;
  const unsigned add_node::precedence = prec_t::plus_prec;
  const unsigned sub_node::precedence = prec_t::plus_prec;
  const unsigned mult_node::precedence = prec_t::mult_prec;
  const unsigned div_node::precedence = prec_t::mult_prec;
  const unsigned unary_minus_node::precedence = prec_t::uminus_prec;
  const unsigned pow_node::precedence = prec_t::uminus_prec;
  const unsigned pow_itv_node::precedence = prec_t::uminus_prec;
  const unsigned nth_root_node::precedence = prec_t::uminus_prec;
  const unsigned cos_node::precedence = prec_t::uminus_prec;
  const unsigned sin_node::precedence = prec_t::uminus_prec;
  const unsigned tan_node::precedence = prec_t::uminus_prec;
  const unsigned atan2_node::precedence = prec_t::uminus_prec;
  const unsigned cosh_node::precedence = prec_t::uminus_prec;
  const unsigned sinh_node::precedence = prec_t::uminus_prec;
  const unsigned tanh_node::precedence = prec_t::uminus_prec;
  const unsigned acos_node::precedence = prec_t::uminus_prec;
  const unsigned asin_node::precedence = prec_t::uminus_prec;
  const unsigned atan_node::precedence = prec_t::uminus_prec;
  const unsigned acosh_node::precedence = prec_t::uminus_prec;
  const unsigned asinh_node::precedence = prec_t::uminus_prec;
  const unsigned atanh_node::precedence = prec_t::uminus_prec;
  const unsigned exp_node::precedence = prec_t::uminus_prec;
  const unsigned log_node::precedence = prec_t::uminus_prec;
  const unsigned exp2_node::precedence = prec_t::uminus_prec;
  const unsigned log2_node::precedence = prec_t::uminus_prec;
  const unsigned sign_node::precedence = prec_t::uminus_prec;
  const unsigned trunc_node::precedence = prec_t::uminus_prec;

  namespace {
    /*
      The nodes that wait to be deleted in this thread, while the outermost
      release_node() of the thread runs (GAOL v5): a pointer to a variable of
      that call, 0 the rest of the time. A pointer, and not the list itself:
      a thread-local list would have a destructor, which the main thread runs
      before those of the static objects of the program, whose expressions
      are deleted after it. A list for each thread, as the counts of
      references are not atomic: the trees that two threads delete at once
      are apart.
    */
    thread_local std::vector<expr_node*>* waiting_nodes = 0;

    /*
      Gives up the reference that a destructor holds to its operand e, and
      deletes e if it was the last one (GAOL v5). The destructor of a node
      deleted its operands, which deleted theirs, one destructor within
      another: an expression built by adding 100000 terms, a chain of 100000
      nodes, overflowed the stack of the program when its last reference was
      given up. An operand whose last reference is given up while a
      destructor runs in the thread, so under a release_node() still
      running, now waits on the list of that call, whose loop deletes it once
      the destructor that put it there has returned. The stack holds the
      frames of the outermost call and of one destructor, whatever the depth
      of the tree, and the heap the nodes not deleted yet. When the list
      cannot grow, the node is deleted where it is, as it was.
    */
    void release_node(expr_node* e)
    {
      if (e->dec_refcount() != 0) {
        return;
      }
      if (waiting_nodes != 0) {
        try {
          waiting_nodes->push_back(e);
        } catch (const std::bad_alloc&) {
          delete e;
        }
        return;
      }
      std::vector<expr_node*> waiting;
      waiting_nodes = &waiting;
      delete e;
      while (!waiting.empty()) {
        expr_node* const node = waiting.back();
        waiting.pop_back();
        delete node;
      }
      waiting_nodes = 0;
    }
  } // anonymous namespace

  /*
   * expression --
   */

  expression::expression() : root(the_null_expr)
  {
    root->inc_refcount();
    GAOL_DEBUG(3,std::cout << "creating null expression" << std::endl);
  }

  expression::expression(double d)
  {
    GAOL_DEBUG(3,std::cout << "creating expression from " << d << std::endl);
    root= new double_node(d);
    root->inc_refcount();
  }

  expression::expression(const interval& I)
  {
    GAOL_DEBUG(3,std::cout << "creating expression from " << I << std::endl);
    root= new interval_node(I);
    root->inc_refcount();
  }

  expression::expression(const expression& e) : root(e.root)
  {
    GAOL_DEBUG(3,std::cout << "creating expression from expression "
		<< std::endl);
    root->inc_refcount();
  }

  expression::expression(const expr_node& e)
  {
    GAOL_DEBUG(3,std::cout << "creating expression from node " << std::endl);
    root = &const_cast<expr_node&>(e);
    root->inc_refcount();
  }


  expression::~expression()
  {
    GAOL_DEBUG(3,std::cout << "deleting expression" << std::endl);

    if (root->dec_refcount() == 0) {
      delete root;
    }
  }

  expression& expression::operator=(const expression& e)
  {
    e.root->inc_refcount();
    if (root->dec_refcount() == 0) {
      delete root;
    }
    root=e.root;
    return *this;
  }

  expression& expression::operator+=(const expression& e)
  {
    /* The node, built first, takes a reference on the root it replaces,
       which the decrement after gives up: the count of that root is
       unchanged, and the expression is left as it was when new throws (GAOL
       v5: the decrement came first, and a root shared with another
       expression lost a reference for good). The expression then takes a
       reference on its new root, as every constructor does; without it the
       count of the new root stayed 0, the destructor decremented it to
       UINT_MAX rather than to 0, and the node was never deleted (GAOL v5,
       found by LeakSanitizer through tests/expressions.cpp). */
    expr_node *const node = new add_node(*this,e);
    root->dec_refcount();
    root=node;
    root->inc_refcount();
    return *this;
  }

  expression& expression::operator-=(const expression& e)
  {
    /* The node, built first, takes a reference on the root it replaces,
       which the decrement after gives up: the count of that root is
       unchanged, and the expression is left as it was when new throws (GAOL
       v5: the decrement came first, and a root shared with another
       expression lost a reference for good). The expression then takes a
       reference on its new root, as every constructor does; without it the
       count of the new root stayed 0, the destructor decremented it to
       UINT_MAX rather than to 0, and the node was never deleted (GAOL v5,
       found by LeakSanitizer through tests/expressions.cpp). */
    expr_node *const node = new sub_node(*this,e);
    root->dec_refcount();
    root=node;
    root->inc_refcount();
    return *this;
  }

  expression& expression::operator*=(const expression& e)
  {
    /* The node, built first, takes a reference on the root it replaces,
       which the decrement after gives up: the count of that root is
       unchanged, and the expression is left as it was when new throws (GAOL
       v5: the decrement came first, and a root shared with another
       expression lost a reference for good). The expression then takes a
       reference on its new root, as every constructor does; without it the
       count of the new root stayed 0, the destructor decremented it to
       UINT_MAX rather than to 0, and the node was never deleted (GAOL v5,
       found by LeakSanitizer through tests/expressions.cpp). */
    expr_node *const node = new mult_node(*this,e);
    root->dec_refcount();
    root=node;
    root->inc_refcount();
    return *this;
  }

  // Declared, and not defined: a program using it did not link (GAOL v5)
  expression& expression::operator/=(const expression& e)
  {
    // As in operator*=: the new node, built first, takes a reference on the
    // root it replaces, and the expression one on its new root
    expr_node *const node = new div_node(*this,e);
    root->dec_refcount();
    root=node;
    root->inc_refcount();
    return *this;
  }

  expr_node* expression::get_root() const
  {
    return root;
  }


  std::ostream& operator<<(std::ostream& os, const expression& e)
  {
    (e.root)->display(os);
    return os;
  }


  /*
   * expr_node --
   */

  expr_node::expr_node() : refcount(0)
  {
    // nothing else to do
  }

  expr_node::expr_node(const expr_node& e) : refcount(0)
  {
    // nothing else to do
  }

  expr_node::~expr_node()
  {
    // nothing to do
  }

  /*
    the_null_expr, which every empty expression of every thread points to, is
    not counted: it lives from GAOL's initialization to its automatic cleanup,
    when the program ends or the library is unloaded, which deletes it (GAOL
    v5). Its count, a plain unsigned int that expressions
    created and destroyed in two threads changed at once, lost updates, came
    down to 0 while expressions still pointed to the node, and the node was
    deleted twice. The expressions count their references through these two
    functions only. Before the initialization, the_null_expr is 0: an
    expression made then has no node, and fails where it is made, as it did.
  */
  unsigned int expr_node::inc_refcount()
  {
    if (the_null_expr != 0 && this == the_null_expr) {
      return 1;
    }
    return ++refcount;
  }

  unsigned int expr_node::dec_refcount()
  {
    if (the_null_expr != 0 && this == the_null_expr) {
      return 1;
    }
    return --refcount;
  }

  unsigned int expr_node::references() const
  {
    return refcount;
  }

  void expr_node::parenthesize_if_necessary(unsigned int calling_prec,
					    std::ostream& os,
					    expr_node* e)
  {
    if (e->get_precedence() <= calling_prec) {
      os.put('('); e->display(os); os.put(')');
    } else {
      e->display(os);
    }
  }

  /*
    null_node ---
  */
  null_node::null_node()
  {
    GAOL_DEBUG(3,std::cout << "null_node created" << std::endl);
  }


  null_node::~null_node()
  {
    GAOL_DEBUG(3,std::cout << "null_node destroyed" << std::endl);
  }

  expr_node* null_node::clone() const
  {
    return new null_node;
  }

  std::ostream& null_node::display(std::ostream& os) const
  {
    os << ":null:";
    return os;
  }

  unsigned int null_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * double_node --
   */

  double_node::double_node(double d) : val(d)
  {
    GAOL_DEBUG(3,std::cout << "double_node created" << std::endl);
  }

  double_node::~double_node()
  {
    GAOL_DEBUG(3,std::cout << "double_node destroyed" << std::endl);
  }

  std::ostream& double_node::display(std::ostream& os) const
  {
    os << val;
    return os;
  }

  expr_node* double_node::clone() const
  {
    double_node* e = new double_node(val);
    return e;
  }

  /*
    A number written with a minus sign is shown as the unary minus is (GAOL
    v5): (-2)^2 was shown as -2^2, which reads as -(2^2).
  */
  unsigned int double_node::get_precedence() const
  {
    return std::signbit(val) ? static_cast<unsigned int>(prec_t::uminus_prec) : precedence;
  }

  /*
   * interval_node --
   */

  interval_node::interval_node(const interval& I)
  {
    val = new interval(I);
    GAOL_DEBUG(3,std::cout << "interval_node created" << std::endl);
  }

  interval_node::~interval_node()
  {
    delete val;
    GAOL_DEBUG(3,std::cout << "interval_node destroyed" << std::endl);
  }

  std::ostream& interval_node::display(std::ostream& os) const
  {
    os << *val;
    return os;
  }

  expr_node* interval_node::clone() const
  {
    interval_node* e = new interval_node(*val);
    return e;
  }

  unsigned int interval_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * add_node --
   */

  add_node::add_node(const expression &el,
		     const expression &er) : e_left(el.get_root()),
					     e_right(er.get_root())
  {
    e_left->inc_refcount();
    e_right->inc_refcount();
     GAOL_DEBUG(3,std::cout << "add_node created" << std::endl);
  }

  add_node::~add_node()
  {
    release_node(e_left);
    release_node(e_right);
    GAOL_DEBUG(3,std::cout << "add_node destroyed" << std::endl);
  }

  std::ostream& add_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_left);
    os.put('+');
    parenthesize_if_necessary(precedence,os,e_right);
    return os;
  }


  expr_node* add_node::clone() const
  {
    add_node* e = new add_node(*e_left,*e_right);
    return e;
  }

   unsigned int add_node::get_precedence() const
  {
    return precedence;
  }

 /*
    unary_minus_node ---
  */
  unary_minus_node::unary_minus_node(const expression& e) : e_uminus(e.get_root())
  {
    e_uminus->inc_refcount();
    GAOL_DEBUG(3,std::cout << "unary_minus_node created" << std::endl);
  }

  unary_minus_node::~unary_minus_node()
  {
    release_node(e_uminus);
    GAOL_DEBUG(3,std::cout << "unary_minus_node destroyed" << std::endl);
  }

  std::ostream& unary_minus_node::display(std::ostream& os) const
  {
    os.put('-');
    if (e_uminus->get_precedence()<precedence) {
      os.put('('); os << (*e_uminus); os.put(')');
    } else {
      os << (*e_uminus);
    }
    return os;
  }

  expr_node* unary_minus_node::clone() const
  {
    unary_minus_node* e = new unary_minus_node(*e_uminus);
    return e;
  }

  unsigned int unary_minus_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * sub_node --
   */

  sub_node::sub_node(const expression &el,
		     const expression &er) : e_left(el.get_root()),
					     e_right(er.get_root())
  {
    e_left->inc_refcount();
    e_right->inc_refcount();
     GAOL_DEBUG(3,std::cout << "sub_node created" << std::endl);
  }

  sub_node::~sub_node()
  {
    release_node(e_left);
    release_node(e_right);
    GAOL_DEBUG(3,std::cout << "sub_node destroyed" << std::endl);
  }

  std::ostream& sub_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_left);
    os.put('-');
    parenthesize_if_necessary(precedence,os,e_right);
    return os;
  }


  expr_node* sub_node::clone() const
  {
    sub_node* e = new sub_node(*e_left,*e_right);
    return e;
  }

  unsigned int sub_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * mult_node --
   */

  mult_node::mult_node(const expression &el,
		       const expression &er) : e_left(el.get_root()),
					       e_right(er.get_root())
  {
    e_left->inc_refcount();
    e_right->inc_refcount();
    GAOL_DEBUG(3,std::cout << "mult_node created" << std::endl);
  }

  mult_node::~mult_node()
  {
    release_node(e_left);
    release_node(e_right);
    GAOL_DEBUG(3,std::cout << "mult_node destroyed" << std::endl);
  }

  std::ostream& mult_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_left);
    os.put('*');
    parenthesize_if_necessary(precedence,os,e_right);
    return os;
  }


  expr_node* mult_node::clone() const
  {
    mult_node* e = new mult_node(*e_left,*e_right);
    return e;
  }

  unsigned int mult_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * div_node --
   */

  div_node::div_node(const expression &el,
		       const expression &er) : e_left(el.get_root()),
					       e_right(er.get_root())
  {
    e_left->inc_refcount();
    e_right->inc_refcount();
    GAOL_DEBUG(3,std::cout << "div_node created" << std::endl);
  }

  div_node::~div_node()
  {
    release_node(e_left);
    release_node(e_right);
    GAOL_DEBUG(3,std::cout << "div_node destroyed" << std::endl);
  }

  // '/', where GAOL wrote '*': x/(y*z) was shown as x*(y*z) (GAOL v5)
  std::ostream& div_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_left);
    os.put('/');
    parenthesize_if_necessary(precedence,os,e_right);
    return os;
  }


  expr_node* div_node::clone() const
  {
    div_node* e = new div_node(*e_left,*e_right);
    return e;
  }

  unsigned int div_node::get_precedence() const
  {
    return precedence;
  }

  /*
    pow_node
  */
  pow_node::pow_node(const expression& e, int n) : e_pow(e.get_root())
  {
    exponent = n;
    e_pow->inc_refcount();
    GAOL_DEBUG(3,std::cout << "pow_node created" << std::endl);
  }

  pow_node::~pow_node()
  {
    release_node(e_pow);
    GAOL_DEBUG(3,std::cout << "pow_node destroyed" << std::endl);
  }

  std::ostream& pow_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_pow);
    os.put('^');
    os << exponent;
    return os;
  }

  expr_node* pow_node::clone() const
  {
    pow_node* e = new pow_node(*e_pow,exponent);
    return e;
  }

  unsigned int pow_node::get_precedence() const
  {
    return precedence;
  }

  /*
    pow_itv_node
  */
  pow_itv_node::pow_itv_node(const expression& e1, const expression &e2, power_function f) : e_left(e1.get_root()),
									   e_right(e2.get_root()),
									   f(f)
  {
    e_left->inc_refcount();
    e_right->inc_refcount();
    GAOL_DEBUG(3,std::cout << "pow_itv_node created" << std::endl);
  }

  pow_itv_node::~pow_itv_node()
  {
    release_node(e_left);
    release_node(e_right);
    GAOL_DEBUG(3,std::cout << "pow_itv_node destroyed" << std::endl);
  }

  std::ostream& pow_itv_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_left);
    os.put('^');
    parenthesize_if_necessary(precedence,os,e_right);
    return os;
  }

  expr_node* pow_itv_node::clone() const
  {
    pow_itv_node* e = new pow_itv_node(*e_left,*e_right,f);
    return e;
  }

  unsigned int pow_itv_node::get_precedence() const
  {
    return precedence;
  }

  /*
    nth_root_node
  */
  nth_root_node::nth_root_node(const expression& e, unsigned int n) :
    e_nth_root(e.get_root())
  {
    exponent = n;
    e_nth_root->inc_refcount();
    GAOL_DEBUG(3,std::cout << "nth_root_node created" << std::endl);
  }

  nth_root_node::~nth_root_node()
  {
    release_node(e_nth_root);
    GAOL_DEBUG(3,std::cout << "nth_root_node destroyed" << std::endl);
  }

  std::ostream& nth_root_node::display(std::ostream& os) const
  {
    parenthesize_if_necessary(precedence,os,e_nth_root);
    os << "^(1/" << exponent << ')';
    return os;
  }

  expr_node* nth_root_node::clone() const
  {
    nth_root_node* e = new nth_root_node(*e_nth_root,exponent);
    return e;
  }

  unsigned int nth_root_node::get_precedence() const
  {
    return precedence;
  }

  /*
    cos_node
  */
  cos_node::cos_node(const expression& e) : e_cos(e.get_root())
  {
    e_cos->inc_refcount();
    GAOL_DEBUG(3,std::cout << "cos_node created" << std::endl);
  }

  cos_node::~cos_node()
  {
    release_node(e_cos);
    GAOL_DEBUG(3,std::cout << "cos_node destroyed" << std::endl);
  }

  std::ostream& cos_node::display(std::ostream& os) const
  {
    os << "cos(";
    e_cos->display(os);
    os.put(')');
    return os;
  }

  expr_node* cos_node::clone() const
  {
    cos_node* e = new cos_node(*e_cos);
    return e;
  }

  unsigned int cos_node::get_precedence() const
  {
    return precedence;
  }

  /*
    sin_node
  */
  sin_node::sin_node(const expression& e) : e_sin(e.get_root())
  {
    e_sin->inc_refcount();
    GAOL_DEBUG(3,std::cout << "sin_node created" << std::endl);
  }

  sin_node::~sin_node()
  {
    release_node(e_sin);
    GAOL_DEBUG(3,std::cout << "sin_node destroyed" << std::endl);
  }

  std::ostream& sin_node::display(std::ostream& os) const
  {
    os << "sin(";
    e_sin->display(os);
    os.put(')');
    return os;
  }

  expr_node* sin_node::clone() const
  {
    sin_node* e = new sin_node(*e_sin);
    return e;
  }

  unsigned int sin_node::get_precedence() const
  {
    return precedence;
  }

  /*
    tan_node
  */
  tan_node::tan_node(const expression& e) : e_tan(e.get_root())
  {
    e_tan->inc_refcount();
    GAOL_DEBUG(3,std::cout << "tan_node created" << std::endl);
  }

  tan_node::~tan_node()
  {
    release_node(e_tan);
    GAOL_DEBUG(3,std::cout << "tan_node destroyed" << std::endl);
  }

  std::ostream& tan_node::display(std::ostream& os) const
  {
    os << "tan(";
    e_tan->display(os);
    os.put(')');
    return os;
  }

  expr_node* tan_node::clone() const
  {
    tan_node* e = new tan_node(*e_tan);
    return e;
  }

  unsigned int tan_node::get_precedence() const
  {
    return precedence;
  }

  /*
    atan2_node
  */
  atan2_node::atan2_node(const expression& e1, const expression& e2) : Y(e1.get_root()),
								       X(e2.get_root())
  {
    Y->inc_refcount();
    X->inc_refcount();
    GAOL_DEBUG(3,std::cout << "atan2_node created" << std::endl);
  }

  atan2_node::~atan2_node()
  {
    release_node(Y);
    release_node(X);
   GAOL_DEBUG(3,std::cout << "atan2_node destroyed" << std::endl);
  }

  std::ostream& atan2_node::display(std::ostream& os) const
  {
    os << "atan2(";
    Y->display(os);
    os.put(',');
    X->display(os);
    os.put(')');
    return os;
  }

  expr_node* atan2_node::clone() const
  {
    atan2_node* e = new atan2_node(*Y,*X);
    return e;
  }

  unsigned int atan2_node::get_precedence() const
  {
    return precedence;
  }

  /*
    acos_node
  */
  acos_node::acos_node(const expression& e) : e_acos(e.get_root())
  {
    e_acos->inc_refcount();
    GAOL_DEBUG(3,std::cout << "acos_node created" << std::endl);
  }

  acos_node::~acos_node()
  {
    release_node(e_acos);
    GAOL_DEBUG(3,std::cout << "acos_node destroyed" << std::endl);
  }

  std::ostream& acos_node::display(std::ostream& os) const
  {
    os << "acos(";
    e_acos->display(os);
    os.put(')');
    return os;
  }

  expr_node* acos_node::clone() const
  {
    acos_node* e = new acos_node(*e_acos);
    return e;
  }

  unsigned int acos_node::get_precedence() const
  {
    return precedence;
  }

  /*
    asin_node
  */
  asin_node::asin_node(const expression& e) : e_asin(e.get_root())
  {
    e_asin->inc_refcount();
    GAOL_DEBUG(3,std::cout << "asin_node created" << std::endl);
  }

  asin_node::~asin_node()
  {
    release_node(e_asin);
    GAOL_DEBUG(3,std::cout << "asin_node destroyed" << std::endl);
  }

  std::ostream& asin_node::display(std::ostream& os) const
  {
    os << "asin(";
    e_asin->display(os);
    os.put(')');
    return os;
  }

  expr_node* asin_node::clone() const
  {
    asin_node* e = new asin_node(*e_asin);
    return e;
  }

  unsigned int asin_node::get_precedence() const
  {
    return precedence;
  }

  /*
    atan_node
  */
  atan_node::atan_node(const expression& e) : e_atan(e.get_root())
  {
    e_atan->inc_refcount();
    GAOL_DEBUG(3,std::cout << "atan_node created" << std::endl);
  }

  atan_node::~atan_node()
  {
    release_node(e_atan);
    GAOL_DEBUG(3,std::cout << "atan_node destroyed" << std::endl);
  }

  std::ostream& atan_node::display(std::ostream& os) const
  {
    os << "atan(";
    e_atan->display(os);
    os.put(')');
    return os;
  }

  expr_node* atan_node::clone() const
  {
    atan_node* e = new atan_node(*e_atan);
    return e;
  }

  unsigned int atan_node::get_precedence() const
  {
    return precedence;
  }

  /*
    cosh_node
  */
  cosh_node::cosh_node(const expression& e) : e_cosh(e.get_root())
  {
    e_cosh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "cosh_node created" << std::endl);
  }

  cosh_node::~cosh_node()
  {
    release_node(e_cosh);
    GAOL_DEBUG(3,std::cout << "cosh_node destroyed" << std::endl);
  }

  std::ostream& cosh_node::display(std::ostream& os) const
  {
    os << "cosh(";
    e_cosh->display(os);
    os.put(')');
    return os;
  }

  expr_node* cosh_node::clone() const
  {
    cosh_node* e = new cosh_node(*e_cosh);
    return e;
  }

  unsigned int cosh_node::get_precedence() const
  {
    return precedence;
  }

  /*
    sinh_node
  */
  sinh_node::sinh_node(const expression& e) : e_sinh(e.get_root())
  {
    e_sinh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "sinh_node created" << std::endl);
  }

  sinh_node::~sinh_node()
  {
    release_node(e_sinh);
    GAOL_DEBUG(3,std::cout << "sinh_node destroyed" << std::endl);
  }

  std::ostream& sinh_node::display(std::ostream& os) const
  {
    os << "sinh(";
    e_sinh->display(os);
    os.put(')');
    return os;
  }

  expr_node* sinh_node::clone() const
  {
    sinh_node* e = new sinh_node(*e_sinh);
    return e;
  }

  unsigned int sinh_node::get_precedence() const
  {
    return precedence;
  }

  /*
    tanh_node
  */
  tanh_node::tanh_node(const expression& e) : e_tanh(e.get_root())
  {
    e_tanh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "tanh_node created" << std::endl);
  }

  tanh_node::~tanh_node()
  {
    release_node(e_tanh);
    GAOL_DEBUG(3,std::cout << "tanh_node destroyed" << std::endl);
  }

  std::ostream& tanh_node::display(std::ostream& os) const
  {
    os << "tanh(";
    e_tanh->display(os);
    os.put(')');
    return os;
  }

  expr_node* tanh_node::clone() const
  {
    tanh_node* e = new tanh_node(*e_tanh);
    return e;
  }

  unsigned int tanh_node::get_precedence() const
  {
    return precedence;
  }


  /*
    acosh_node
  */
  acosh_node::acosh_node(const expression& e) : e_acosh(e.get_root())
  {
    e_acosh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "acosh_node created" << std::endl);
  }

  acosh_node::~acosh_node()
  {
    release_node(e_acosh);
    GAOL_DEBUG(3,std::cout << "acosh_node destroyed" << std::endl);
  }

  std::ostream& acosh_node::display(std::ostream& os) const
  {
    os << "acosh(";
    e_acosh->display(os);
    os.put(')');
    return os;
  }

  expr_node* acosh_node::clone() const
  {
    acosh_node* e = new acosh_node(*e_acosh);
    return e;
  }

  unsigned int acosh_node::get_precedence() const
  {
    return precedence;
  }

  /*
    asinh_node
  */
  asinh_node::asinh_node(const expression& e) : e_asinh(e.get_root())
  {
    e_asinh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "asinh_node created" << std::endl);
  }

  asinh_node::~asinh_node()
  {
    release_node(e_asinh);
    GAOL_DEBUG(3,std::cout << "asinh_node destroyed" << std::endl);
  }

  std::ostream& asinh_node::display(std::ostream& os) const
  {
    os << "asinh(";
    e_asinh->display(os);
    os.put(')');
    return os;
  }

  expr_node* asinh_node::clone() const
  {
    asinh_node* e = new asinh_node(*e_asinh);
    return e;
  }

  unsigned int asinh_node::get_precedence() const
  {
    return precedence;
  }

  /*
    atanh_node
  */
  atanh_node::atanh_node(const expression& e) : e_atanh(e.get_root())
  {
    e_atanh->inc_refcount();
    GAOL_DEBUG(3,std::cout << "atanh_node created" << std::endl);
  }

  atanh_node::~atanh_node()
  {
    release_node(e_atanh);
    GAOL_DEBUG(3,std::cout << "atanh_node destroyed" << std::endl);
  }

  std::ostream& atanh_node::display(std::ostream& os) const
  {
    os << "atanh(";
    e_atanh->display(os);
    os.put(')');
    return os;
  }

  expr_node* atanh_node::clone() const
  {
    atanh_node* e = new atanh_node(*e_atanh);
    return e;
  }

  unsigned int atanh_node::get_precedence() const
  {
    return precedence;
  }


  /*
    exp_node
  */
  exp_node::exp_node(const expression& e) : e_exp(e.get_root())
  {
    e_exp->inc_refcount();
    GAOL_DEBUG(3,std::cout << "exp_node created" << std::endl);
  }

  exp_node::~exp_node()
  {
    release_node(e_exp);
    GAOL_DEBUG(3,std::cout << "exp_node destroyed" << std::endl);
  }

  std::ostream& exp_node::display(std::ostream& os) const
  {
    os << "exp(";
    e_exp->display(os);
    os.put(')');
    return os;
  }

  expr_node* exp_node::clone() const
  {
    exp_node* e = new exp_node(*e_exp);
    return e;
  }

  unsigned int exp_node::get_precedence() const
  {
    return precedence;
  }

  /*
    log_node
  */
  log_node::log_node(const expression& e) : e_log(e.get_root())
  {
    e_log->inc_refcount();
    GAOL_DEBUG(3,std::cout << "log_node created" << std::endl);
  }

  log_node::~log_node()
  {
    release_node(e_log);
    GAOL_DEBUG(3,std::cout << "log_node destroyed" << std::endl);
  }

  std::ostream& log_node::display(std::ostream& os) const
  {
    os << "log(";
    e_log->display(os);
    os.put(')');
    return os;
  }

  expr_node* log_node::clone() const
  {
    log_node* e = new log_node(*e_log);
    return e;
  }

  unsigned int log_node::get_precedence() const
  {
    return precedence;
  }

  /*
    exp2_node
  */
  exp2_node::exp2_node(const expression& e) : e_exp2(e.get_root())
  {
    e_exp2->inc_refcount();
    GAOL_DEBUG(3,std::cout << "exp2_node created" << std::endl);
  }

  exp2_node::~exp2_node()
  {
    release_node(e_exp2);
    GAOL_DEBUG(3,std::cout << "exp2_node destroyed" << std::endl);
  }

  std::ostream& exp2_node::display(std::ostream& os) const
  {
    os << "exp2(";
    e_exp2->display(os);
    os.put(')');
    return os;
  }

  expr_node* exp2_node::clone() const
  {
    exp2_node* e = new exp2_node(*e_exp2);
    return e;
  }

  unsigned int exp2_node::get_precedence() const
  {
    return precedence;
  }

  /*
    log2_node
  */
  log2_node::log2_node(const expression& e) : e_log2(e.get_root())
  {
    e_log2->inc_refcount();
    GAOL_DEBUG(3,std::cout << "log2_node created" << std::endl);
  }

  log2_node::~log2_node()
  {
    release_node(e_log2);
    GAOL_DEBUG(3,std::cout << "log2_node destroyed" << std::endl);
  }

  std::ostream& log2_node::display(std::ostream& os) const
  {
    os << "log2(";
    e_log2->display(os);
    os.put(')');
    return os;
  }

  expr_node* log2_node::clone() const
  {
    log2_node* e = new log2_node(*e_log2);
    return e;
  }

  unsigned int log2_node::get_precedence() const
  {
    return precedence;
  }

  /*
    sign_node
  */
  sign_node::sign_node(const expression& e) : e_sign(e.get_root())
  {
    e_sign->inc_refcount();
    GAOL_DEBUG(3,std::cout << "sign_node created" << std::endl);
  }

  sign_node::~sign_node()
  {
    release_node(e_sign);
    GAOL_DEBUG(3,std::cout << "sign_node destroyed" << std::endl);
  }

  std::ostream& sign_node::display(std::ostream& os) const
  {
    os << "sign(";
    e_sign->display(os);
    os.put(')');
    return os;
  }

  expr_node* sign_node::clone() const
  {
    sign_node* e = new sign_node(*e_sign);
    return e;
  }

  unsigned int sign_node::get_precedence() const
  {
    return precedence;
  }

  /*
    trunc_node
  */
  trunc_node::trunc_node(const expression& e) : e_trunc(e.get_root())
  {
    e_trunc->inc_refcount();
    GAOL_DEBUG(3,std::cout << "trunc_node created" << std::endl);
  }

  trunc_node::~trunc_node()
  {
    release_node(e_trunc);
    GAOL_DEBUG(3,std::cout << "trunc_node destroyed" << std::endl);
  }

  std::ostream& trunc_node::display(std::ostream& os) const
  {
    os << "trunc(";
    e_trunc->display(os);
    os.put(')');
    return os;
  }

  expr_node* trunc_node::clone() const
  {
    trunc_node* e = new trunc_node(*e_trunc);
    return e;
  }

  unsigned int trunc_node::get_precedence() const
  {
    return precedence;
  }

  /*
   * Construction operators --
   */

  const expression operator+(const expression& el, const expression& er)
  {
    return *(new add_node(el,er));
  }

  const expression operator-(const expression& e)
  {
    return *(new unary_minus_node(e));
  }

  const expression operator-(const expression& el, const expression& er)
  {
    return *(new sub_node(el,er));
  }

  const expression operator*(const expression& el, const expression& er)
  {
    return *(new mult_node(el,er));
  }

  const expression operator/(const expression& el, const expression& er)
  {
    return *(new div_node(el,er));
  }

  /*
    The exponent is an int, as the header declares it (GAOL v5): it was an
    unsigned int here, and pow(e, n) on an expression did not link
  */
  const expression gaol_pown_exp(const expression& e, int n)
  {
    return *(new pow_node(e,n));
  }

  const expression gaol_pow_exp(const expression& e1, const expression &e2)
  {
    return *(new pow_itv_node(e1,e2));
  }

  const expression nth_root(const expression& e, unsigned int n)
  {
    return *(new nth_root_node(e,n));
  }

  const expression cos(const expression& e)
  {
    return *(new cos_node(e));
  }
  const expression sin(const expression& e)
  {
    return *(new sin_node(e));
  }
  const expression tan(const expression& e)
  {
    return *(new tan_node(e));
  }

  const expression atan2(const expression& e1, const expression& e2)
  {
    return *(new atan2_node(e1,e2));
  }

  const expression acos(const expression& e)
  {
    return *(new acos_node(e));
  }
  const expression asin(const expression& e)
  {
    return *(new asin_node(e));
  }
  const expression atan(const expression& e)
  {
    return *(new atan_node(e));
  }

  const expression cosh(const expression& e)
  {
    return *(new cosh_node(e));
  }
  const expression sinh(const expression& e)
  {
    return *(new sinh_node(e));
  }
  const expression tanh(const expression& e)
  {
    return *(new tanh_node(e));
  }

  const expression acosh(const expression& e)
  {
    return *(new acosh_node(e));
  }
  const expression asinh(const expression& e)
  {
    return *(new asinh_node(e));
  }
  const expression atanh(const expression& e)
  {
    return *(new atanh_node(e));
  }

  const expression exp(const expression& e)
  {
    return *(new exp_node(e));
  }
  const expression log(const expression& e)
  {
    return *(new log_node(e));
  }
  const expression exp2(const expression& e)
  {
    return *(new exp2_node(e));
  }
  const expression log2(const expression& e)
  {
    return *(new log2_node(e));
  }
  const expression sign(const expression& e)
  {
    return *(new sign_node(e));
  }
  const expression trunc(const expression& e)
  {
    return *(new trunc_node(e));
  }

  bool evaluate_left_right(const expression& el, const expression& er,
			   interval* itv)
  {
    expr_eval eval;
    (el.get_root())->accept(eval);
    interval tmp_l = eval.result();
    if (eval.error_occurred()) {
      return false;
    }
    (er.get_root())->accept(eval);
    interval tmp_r = eval.result();
    if (eval.error_occurred()) {
      return false;
    }
    *itv = interval(tmp_l.left(),tmp_r.right());
    return true;
  }

  bool evaluate_left_right(const expression& e, interval* itv)
  {
    expr_eval eval;
    (e.get_root())->accept(eval);
    if (eval.error_occurred()) {
      return false;
    }
    *itv = eval.result();
    return true;

  }

  bool evaluate_expr(const expression& e, interval& itv)
  {
    expr_eval eval;
    (e.get_root())->accept(eval);
    if (eval.error_occurred()) {
      return false;
    }
    itv = eval.result();
    return true;
  }

} // namespace gaol_core
