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
  \file   gaol_expr_eval.h
  \brief  Evaluation of an interval represented internally via an expression

  <long description>

  \author Frederic Goualard
  \date   2001-10-30
*/


#ifndef __gaol_expr_eval_h__
#define __gaol_expr_eval_h__

#include "gaol/gaol_eval_stack.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_expression.h"

#include <limits>
#include "gaol/gaol_expr_visitor.h"

namespace gaol_core {
  /*!
    \brief Evaluation of an expression: the interval it denotes

    The tree is evaluated with stacks of the evaluator's own, not with the
    stack of the program (GAOL v5). GAOL visited the operands of a node from
    inside its own visit(), one call within another per level of the tree: a
    sum of 100000 terms, a tree 100000 nodes deep, overflowed the stack of the
    program. A node visited for the first time now only puts its operands, and
    itself once more below them, on a stack of jobs: a loop visits the
    operands from left to right, each leaving its value on the stack of values,
    and the node, visited again, pops the values of its operands and pushes its
    own. The operations, their operands and their order are those of the
    recursion, and so are the bounds.

    A null node has no value: it sets error_occurred(), and the whole line
    stands for it on the stack of values, so that the operands of the other
    nodes stay in place. The interval an evaluation that set it gives means
    nothing.
  */
  class expr_eval : public expr_visitor {
  public:
    expr_eval() : running(false), combining(false) {}
    virtual void visit(null_node* node) {
      error = true;
      stack.push(interval());
    }
    virtual void visit(double_node* node) {
      const double v = node->get_val();
      // interval(+oo) and interval(-oo) are the empty set: an infinity read in
      // an expression is the largest interval reaching it, so that "[dmax, inf]"
      // is [dmax, +oo] and "inf" is [dmax, +oo]
      if (v == GAOL_INFINITY) {
        stack.push(interval(std::numeric_limits<double>::max(), GAOL_INFINITY));
      } else if (v == -GAOL_INFINITY) {
        stack.push(interval(-GAOL_INFINITY, -std::numeric_limits<double>::max()));
      } else {
        stack.push(interval(v));
      }
    }
    virtual void visit(interval_node* node) {
      stack.push(node->get_val());
    }
    virtual void visit(add_node* node) {
      if (postpone(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=stack.pop();
      interval l=stack.pop();
      stack.push(l+r);
    }
    virtual void visit(unary_minus_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(-stack.pop());
    }
    virtual void visit(sub_node* node) {
      if (postpone(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=stack.pop();
      interval l=stack.pop();
      stack.push(l-r);
    }
    virtual void visit(mult_node* node) {
      if (postpone(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=stack.pop();
      interval l=stack.pop();
      stack.push(l*r);
    }
    virtual void visit(div_node* node) {
      if (postpone(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=stack.pop();
      interval l=stack.pop();
      stack.push(l/r);
    }
    virtual void visit(pow_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::gaol_pown(stack.pop(),node->get_exponent()));
    }
    virtual void visit(pow_itv_node* node) {
      if (postpone(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r = stack.pop();
      interval l = stack.pop();
      stack.push((*node->get_function())(l,r));
    }
    virtual void visit(nth_root_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::nth_root(stack.pop(),node->get_exponent()));
    }
    virtual void visit(cos_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::cos(stack.pop()));
    }
    virtual void visit(sin_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sin(stack.pop()));
    }
    virtual void visit(tan_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::tan(stack.pop()));
    }
    virtual void visit(atan2_node* node) {
      if (postpone(node, node->get_Y(), node->get_X())) {
        return;
      }
      interval X=stack.pop();
      interval Y=stack.pop();
      stack.push(gaol_core::atan2(Y,X));
    }
    virtual void visit(acos_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::acos(stack.pop()));
    }
    virtual void visit(asin_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::asin(stack.pop()));
    }
    virtual void visit(atan_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::atan(stack.pop()));
    }
    virtual void visit(cosh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::cosh(stack.pop()));
    }
    virtual void visit(sinh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sinh(stack.pop()));
    }
    virtual void visit(tanh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::tanh(stack.pop()));
    }
    virtual void visit(acosh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::acosh(stack.pop()));
    }
    virtual void visit(asinh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::asinh(stack.pop()));
    }
    virtual void visit(atanh_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::atanh(stack.pop()));
    }
    virtual void visit(log_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::log(stack.pop()));
    }
    virtual void visit(exp_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::exp(stack.pop()));
    }
    virtual void visit(exp2_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::exp2(stack.pop()));
    }
    virtual void visit(log2_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::log2(stack.pop()));
    }
    virtual void visit(sign_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sign(stack.pop()));
    }
    virtual void visit(trunc_node* node) {
      if (postpone(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::trunc(stack.pop()));
    }
    interval result() {
      return stack.pop();
    }
  private:
    /*
      What the loop of run() does with a node: visit it, or, when combine is
      true, visit it again once its operands are on the stack of values, to
      combine them.
    */
    struct job {
      job() : node(0), combine(false) {}
      job(expr_node* n, bool c) : node(n), combine(c) {}
      expr_node* node;
      bool combine;
    };
    /*
      Called first by the visit() of a node with operands, which returns if it
      gives true: the node is visited again once its operands are on the stack,
      then to compute. The first call puts the node, then its operands in
      reverse, on the stack of jobs, so that they come out from left to right;
      the call of a node visited from outside, which no loop is running for,
      starts the loop.
    */
    bool postpone(expr_node* node, expr_node* first, expr_node* second = 0) {
      if (combining) {
        return false;
      }
      jobs.push(job(node, true));
      if (second != 0) {
        jobs.push(job(second, false));
      }
      jobs.push(job(first, false));
      if (!running) {
        run();
      }
      return true;
    }
    // Visits the jobs until none is left, and leaves the value of the first
    // node on the stack of values
    void run() {
      running = true;
      try {
        while (!jobs.empty()) {
          const job j = jobs.pop();
          combining = j.combine;
          j.node->accept(*this);
        }
      } catch (...) {
        // Left as a new evaluator is, whatever made a visit throw
        jobs.clear();
        stack.clear();
        running = false;
        combining = false;
        throw;
      }
      running = false;
      combining = false;
    }
    //! The values of the nodes evaluated, which their parents have not combined yet
    eval_stack<interval> stack;
    //! The nodes to visit, 16 of them held without allocating
    eval_stack<job, 16> jobs;
    //! True while run() visits the jobs
    bool running;
    //! True while the visit of a node computes from the values of its operands
    bool combining;
  };

} // namespace gaol_core

#endif /* __gaol_expr_eval_h__ */
