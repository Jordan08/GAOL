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
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated 
* COPYING file for information.
*--------------------------------------------------------------------------*/

/*!
\file   gaol_expr_visitor.h
\brief  Base classes to represent visitors manipulating interval expressions


\author Frederic Goualard, Joran NININ
\date   2001-10-30
*/


#ifndef GAOL_EXPR_VISITOR_H
#define GAOL_EXPR_VISITOR_H

namespace gaol_core {
  // Forward declarations
  class null_node;
  class double_node;
  class interval_node;
  class add_node;
  class unary_minus_node;
  class sub_node;
  class mult_node;
  class pow_node;
  class pow_itv_node;
  class nth_root_node;
  class div_node;
  class sin_node;
  class cos_node;
  class tan_node;
  class atan2_node;
  class asin_node;
  class acos_node;
  class atan_node;
  class sinh_node;
  class cosh_node;
  class tanh_node;
  class asinh_node;
  class acosh_node;
  class atanh_node;
  class exp_node;
  class log_node;
  class exp2_node;
  class log2_node;
  class sign_node;
  class trunc_node;
  
  /*!
  \brief Base class for all visitors that want to manipulate expressions
  
  A derived class can override only a subset of the visit() methods. It
  is then assumed that all the non-overrided ones are illegal in the
  context of this visitor. The error attribute is used to report such an
  error.
  */
  class expr_visitor {
    public:
    expr_visitor() {
      error = false;
    }
    virtual ~expr_visitor() {}
    bool error_occurred() const {
      return error;
    }
    void reset() {
      error = false;
    }
    virtual void visit(null_node*) {
      error = true;
    }
    virtual void visit(double_node*) {
      error = true;
    }
    virtual void visit(interval_node*) {
      error = true;
    }
    virtual void visit(add_node*) {
      error = true;
    }
    virtual void visit(unary_minus_node*) {
      error = true;
    }
    virtual void visit(sub_node*) {
      error = true;
    }
    virtual void visit(mult_node*) {
      error = true;
    }
    virtual void visit(div_node*) {
      error = true;
    }
    virtual void visit(pow_node*) {
      error = true;
    }
    virtual void visit(pow_itv_node*) {
      error = true;
    }
    virtual void visit(nth_root_node*) {
      error = true;
    }
    virtual void visit(cos_node*) {
      error = true;
    }
    virtual void visit(sin_node*) {
      error = true;
    }
    virtual void visit(tan_node*) {
      error = true;
    }
    virtual void visit(atan2_node*) {
      error = true;
    }
    virtual void visit(acos_node*) {
      error = true;
    }
    virtual void visit(asin_node*) {
      error = true;
    }
    virtual void visit(atan_node*) {
      error = true;
    }
    virtual void visit(cosh_node*) {
      error = true;
    }
    virtual void visit(sinh_node*) {
      error = true;
    }
    virtual void visit(tanh_node*) {
      error = true;
    }
    virtual void visit(acosh_node*) {
      error = true;
    }
    virtual void visit(asinh_node*) {
      error = true;
    }
    virtual void visit(atanh_node*) {
      error = true;
    }
    virtual void visit(log_node*) {
      error = true;
    }
    virtual void visit(exp_node*) {
      error = true;
    }
    virtual void visit(exp2_node*) {
      error = true;
    }
    virtual void visit(log2_node*) {
      error = true;
    }
    virtual void visit(sign_node*) {
      error = true;
    }
    virtual void visit(trunc_node*) {
      error = true;
    }
    protected:
    //! True if an error occurred during the last visit
    bool error;
  };
  
} // namespace gaol_core

// In the namespace gaol too, as in GAOL 4, with the nodes, those of the
// functions GAOL v5 adds among them (see gaol/gaol_interval.h)
namespace gaol {
  using gaol_core::expr_visitor;
  using gaol_core::null_node;
  using gaol_core::double_node;
  using gaol_core::interval_node;
  using gaol_core::add_node;
  using gaol_core::unary_minus_node;
  using gaol_core::sub_node;
  using gaol_core::mult_node;
  using gaol_core::div_node;
  using gaol_core::pow_node;
  using gaol_core::pow_itv_node;
  using gaol_core::nth_root_node;
  using gaol_core::cos_node;
  using gaol_core::sin_node;
  using gaol_core::tan_node;
  using gaol_core::atan2_node;
  using gaol_core::acos_node;
  using gaol_core::asin_node;
  using gaol_core::atan_node;
  using gaol_core::cosh_node;
  using gaol_core::sinh_node;
  using gaol_core::tanh_node;
  using gaol_core::acosh_node;
  using gaol_core::asinh_node;
  using gaol_core::atanh_node;
  using gaol_core::exp_node;
  using gaol_core::log_node;
  using gaol_core::exp2_node;
  using gaol_core::log2_node;
  using gaol_core::sign_node;
  using gaol_core::trunc_node;
} // namespace gaol

#endif /* GAOL_EXPR_VISITOR_H */
