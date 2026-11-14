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
\file   gaol_expr_eval.h
\brief  Evaluation of an interval represented internally via an expression

<long description>

\author Frederic Goualard, Jordan NININ
\date   2001-10-30
*/


#ifndef GAOL_EXPR_EVAL_H
#define GAOL_EXPR_EVAL_H

#include "gaol/parser/gaol_eval_stack.h"
#include "gaol/gaol_interval.h"
#include "gaol/parser/gaol_expression.h"

#include <limits>
#include "gaol/parser/gaol_expr_visitor.h"

namespace gaol_core {
  /*!
  \brief Evaluation of an expression: the interval it denotes
  
  The stack of the program an evaluation takes does not depend on the depth
  of the tree (GAOL v5). GAOL visited the operands of a node from inside its
  own visit(), one call within another per level of the tree: a sum of
  100000 terms, a tree 100000 nodes deep, overflowed the stack of the
  program. The evaluation still visits the operands so, which is the fastest
  way for the short expressions, down to max_depth levels only. A node
  deeper than that puts its operands, and itself once more below them, on a
  stack of jobs of the evaluator's own, which grows in the heap: a loop
  visits the operands from left to right, each leaving its value on the
  stack of values, and the node, visited again, pops the values of its
  operands and pushes its own. Every node below it is evaluated by the same
  loop. The operations, their operands and their order are those of the
  recursion, and so are the bounds.
  
  A null node has no value: it sets error_occurred(), and the whole line
  stands for it on the stack of values, so that the operands of the other
  nodes stay in place. The interval an evaluation that set it gives means
  nothing.

  The evaluation of a tree leaves one value, which result() takes off the
  stack, once (GAOL v5). A node that takes a value when the stack holds none
  (the accept() of a node of a program of its own visited nothing) sets
  error_occurred() too, and so does a tree that leaves more than one value:
  the stack gave the value of an earlier evaluation, or none, and no error was
  set. Nodes that balance each other, one that visits too few operands and
  another too many, are not seen.
  */
  class expr_eval : public expr_visitor {
    public:
    expr_eval() : depth(0), running(false), combining(false) {}
    virtual void visit(null_node*) {
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
      if (operands(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=pop_value();
      interval l=pop_value();
      stack.push(l+r);
    }
    virtual void visit(unary_minus_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(-pop_value());
    }
    virtual void visit(sub_node* node) {
      if (operands(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=pop_value();
      interval l=pop_value();
      stack.push(l-r);
    }
    virtual void visit(mult_node* node) {
      if (operands(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=pop_value();
      interval l=pop_value();
      stack.push(l*r);
    }
    virtual void visit(div_node* node) {
      if (operands(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r=pop_value();
      interval l=pop_value();
      stack.push(l/r);
    }
    virtual void visit(pow_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::gaol_pown(pop_value(),node->get_exponent()));
    }
    virtual void visit(pow_itv_node* node) {
      if (operands(node, node->get_left(), node->get_right())) {
        return;
      }
      interval r = pop_value();
      interval l = pop_value();
      stack.push((*node->get_function())(l,r));
    }
    virtual void visit(nth_root_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::nth_root(pop_value(),node->get_exponent()));
    }
    virtual void visit(cos_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::cos(pop_value()));
    }
    virtual void visit(sin_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sin(pop_value()));
    }
    virtual void visit(tan_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::tan(pop_value()));
    }
    virtual void visit(atan2_node* node) {
      if (operands(node, node->get_Y(), node->get_X())) {
        return;
      }
      interval X=pop_value();
      interval Y=pop_value();
      stack.push(gaol_core::atan2(Y,X));
    }
    virtual void visit(acos_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::acos(pop_value()));
    }
    virtual void visit(asin_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::asin(pop_value()));
    }
    virtual void visit(atan_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::atan(pop_value()));
    }
    virtual void visit(cosh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::cosh(pop_value()));
    }
    virtual void visit(sinh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sinh(pop_value()));
    }
    virtual void visit(tanh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::tanh(pop_value()));
    }
    virtual void visit(acosh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::acosh(pop_value()));
    }
    virtual void visit(asinh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::asinh(pop_value()));
    }
    virtual void visit(atanh_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::atanh(pop_value()));
    }
    virtual void visit(log_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::log(pop_value()));
    }
    virtual void visit(exp_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::exp(pop_value()));
    }
    virtual void visit(exp2_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::exp2(pop_value()));
    }
    virtual void visit(log2_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::log2(pop_value()));
    }
    virtual void visit(sign_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::sign(pop_value()));
    }
    virtual void visit(trunc_node* node) {
      if (operands(node, node->get_subexpr())) {
        return;
      }
      stack.push(gaol_core::trunc(pop_value()));
    }
    /*
    The value the evaluation left, which the stack of values holds alone; the
    stack is empty after it, and the evaluator ready for another tree. When it
    holds none, or more than one, error_occurred() is true, and the interval
    means nothing (the values it holds are dropped).
    */
    interval result() {
      const interval value = pop_value();
      if (!stack.empty()) {
        error = true;
        stack.clear();
      }
      return value;
    }
    private:
    /*
    The value on the top of the stack of values. An empty stack, which a tree
    leaves when the accept() of one of its nodes visits nothing, sets the
    error, and gives an interval that means nothing: pop() of the stack takes
    the value of an earlier evaluation, or none (GAOL v5).
    */
    interval pop_value() {
      if (stack.empty()) {
        error = true;
        return interval();
      }
      return stack.pop();
    }
    // The levels of the tree visited one call within another, beyond which
    // the loop of run() takes over (see the class)
    static const unsigned int max_depth = 32;
    /*
    What the loop of run() does with a node: visit it, or, when combine is
    true, visit it again once its operands are on the stack of values, to
    combine them. No constructor: the jobs a stack holds without
    allocating are left as they are until pushed.
    */
    struct job {
      expr_node* node;
      bool combine;
    };
    // Counts a level of the recursion for as long as it lasts, an exception
    // included
    struct level {
      explicit level(unsigned int& d) : depth(d) { ++depth; }
      ~level() { --depth; }
      unsigned int& depth;
    };
    /*
    Called first by the visit() of a node with operands: puts the values of
    its operands, first then second, on the stack of values and gives false,
    the node then computing its own; or gives true, and the node returns,
    when the values are not there yet. Within max_depth levels, the operands
    are visited here, one call within another. Deeper, or in the loop of
    run(), the node, then its operands in reverse, go on the stack of jobs,
    so that they come out from left to right, and the node is visited again
    once its operands are on the stack of values, then to compute; the
    first node that does so starts the loop.
    */
    bool operands(expr_node* node, expr_node* first, expr_node* second = 0) {
      if (combining) {
        return false;
      }
      if (!running && depth < max_depth) {
        const level l(depth);
        first->accept(*this);
        if (second != 0) {
          second->accept(*this);
        }
        return false;
      }
      jobs.push(job{node, true});
      if (second != 0) {
        jobs.push(job{second, false});
      }
      jobs.push(job{first, false});
      if (!running) {
        run();
      }
      return true;
    }
    // Visits the jobs until none is left, and leaves the value of the node
    // that started the loop on the stack of values
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
    //! The nodes the loop of run() has to visit, 16 of them held without allocating
    eval_stack<job, 16> jobs;
    //! The levels of the recursion under way
    unsigned int depth;
    //! True while run() visits the jobs
    bool running;
    //! True while the visit of a node computes from the values of its operands
    bool combining;
  };
  
  } // namespace gaol_core

// In the namespace gaol too, as in GAOL 4 (see gaol/gaol_interval.h)
namespace gaol {
  using gaol_core::expr_eval;
} // namespace gaol

#endif /* GAOL_EXPR_EVAL_H */
