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
  \file   gaol_eval_stack.h
  \brief  Stack for performing evaluation of expressions

  An eval_stack is a stack for which underflow is supposed never to occur,
  since the arity of the operators is known in advance.

  \author Frederic Goualard
  \date   2001-10-20
*/


#ifndef __gaol_eval_stack_h__
#define __gaol_eval_stack_h__

#include <limits>
#include <new>

#include "gaol/gaol_common.h"

namespace gaol_core {

  /*!
    \brief stack for evaluation of expressions

    The first SZ elements are held in the stack itself, so that the evaluation
    of a small expression allocates nothing. A stack that is full moves its
    elements to a block of the heap twice as large (GAOL v5): the stack was an
    array of SZ elements, and the evaluation of gaol/gaol_expr_eval.h now keeps
    on it the values of the operands not combined yet, as many as the tree is
    deep, where GAOL kept them on the stack of the program.

    \warning Underflows are not reported

    \param T type of the elements on the stack
    \param SZ number of elements the stack holds without allocating
  */
  template <typename T, unsigned int SZ = 8>
    class eval_stack {
      public:
      eval_stack();
      ~eval_stack();
      void push(const T& e);
      T pop();
      //! True if the stack holds no element
      bool empty() const;
      //! Removes every element
      void clear();

      private:
      // the_stack points into the stack itself as long as it is not full: a
      // copy would point to the elements of the original
      eval_stack(const eval_stack&) = delete;
      eval_stack& operator=(const eval_stack&) = delete;
      /*!
        Moves the elements to a block of twice the size. Throws std::bad_alloc,
        the stack being as it was, when there is no memory left.
      */
      void grow();

      //! Next position available on the stack
      unsigned int next_pos;
      //! Number of elements the_stack holds
      unsigned int size;
      //! The elements: first_block, then a block of the heap
      T* the_stack;
      T first_block[SZ];
    };

  template<typename T, unsigned int SZ>
    eval_stack<T,SZ>::eval_stack() : next_pos(0), size(SZ), the_stack(first_block)
    {
    }

  template<typename T, unsigned int SZ>
    eval_stack<T,SZ>::~eval_stack()
    {
      if (the_stack != first_block) {
        delete[] the_stack;
      }
    }

  template<typename T, unsigned int SZ>
    void eval_stack<T,SZ>::grow()
    {
      if (size > std::numeric_limits<unsigned int>::max() / 2) {
        throw std::bad_alloc();
      }
      const unsigned int bigger_size = 2 * size;
      T* const bigger = new T[bigger_size];
      for (unsigned int i = 0; i < next_pos; ++i) {
        bigger[i] = the_stack[i];
      }
      if (the_stack != first_block) {
        delete[] the_stack;
      }
      the_stack = bigger;
      size = bigger_size;
    }

  template<typename T, unsigned int SZ>
    void eval_stack<T,SZ>::push(const T& e)
    {
      if (next_pos == size) {
        // e may be an element of the stack, which grow() moves
        const T copy(e);
        grow();
        the_stack[next_pos++] = copy;
        return;
      }
      the_stack[next_pos++] = e;
    }

  template<typename T, unsigned int SZ>
    T eval_stack<T,SZ>::pop()
    {
      /*
	Case of underflow: only happens if the type of the result of 
	the evaluation is not the expected one. We return 0, knowing that
	the visitor MUST somehow set the error variable to true.
      */
      if (next_pos == 0) {
	return the_stack[0];
      } else {
	return the_stack[--next_pos]; 
      }
    }

  template<typename T, unsigned int SZ>
    bool eval_stack<T,SZ>::empty() const
    {
      return next_pos == 0;
    }

  template<typename T, unsigned int SZ>
    void eval_stack<T,SZ>::clear()
    {
      next_pos = 0;
    }
  
} // namespace gaol_core

#endif /* __gaol_eval_stack_h__ */
