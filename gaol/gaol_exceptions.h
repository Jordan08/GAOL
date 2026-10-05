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
  \file   gaol_exceptions.h
  \brief  Declaration of standard exceptions in gaol.

  Exceptions are used only if gaol has been configured with --enable-exceptions

  \author Frederic Goualard
  \date   2002-12-02
*/


#ifndef GAOL_EXCEPTIONS_H
#define GAOL_EXCEPTIONS_H

#include "gaol/gaol_config.h"

#if GAOL_EXCEPTIONS_ENABLED

// std::exception and std::string by their names: no using-declaration puts
// them in the global namespace of the code including GAOL any more (GAOL v5)
#include <stdexcept>
#include <iosfwd>
#include <string>

namespace gaol_core {

  /*!
    \brief Base class for all gaol exceptions

    This class serves as a base class for all gaol exceptions in order
    to provide a uniform framework. Every exception thrown should at
    least contain the file and line where the exception was thrown.
  */
  class GAOL_PUBLIC gaol_exception : public std::exception {
  public:
    /*!
      \param f Name of the file where the exception is thrown
      \param l Line in the file where the exception is thrown
    */
    gaol_exception(const char* f, unsigned l);

    /*!
      \param f Name of the file where the exception is thrown
      \param l Line in the file where the exception is thrown
      \param e Explanation concerning the throwing; a null pointer is no
      explanation (GAOL v5: GAOL 4 built a std::string from it, which is
      undefined behavior)
    */
    gaol_exception(const char* f, unsigned l, const char* e);

    /*!
      \param f Name of the file where the exception is thrown
      \param l Line in the file where the exception is thrown
      \param e Explanation concerning the throwing
    */
    gaol_exception(const char* f, unsigned l, const std::string& e);

    ~gaol_exception() override {}
    /*!
      \brief Accessor for the file name

      \return the name of the C++ file in which occurred the exception
    */
    const char* file() const;
    /*!
      \brief Accessor for the line number

      \return the line in the C++ file in which occurred the exception
    */
    unsigned int line() const;

    /*!
      \brief Accessor for the explanation string

      \return the string providing some explanation for having thrown
      the exception or the empty string if none was given.
    */
    std::string explanation() const;

    /*!
      \brief The explanation, as a C string: what a handler of std::exception
      gets (GAOL v5)

      GAOL 4 left the what() of std::exception, which gave the text of the
      standard class ("std::exception" with libstdc++ and libc++) whatever
      went wrong: that is what a catch (const std::exception&) printed, and
      what a program ended with when nothing caught the exception.

      \return the explanation, as explanation() gives it, or the name of the
      class ("gaol_exception", "input_format_error"...) if none was given or
      if its text as a C string is empty (a NUL first), never an empty text.
      The text is valid as long as the exception is.
    */
    const char* what() const noexcept override;

  protected:
    /*!
     \brief File where the exception occurred.

     This information must be provided by using the macro __FILE__
     */
    const char* which_file_;
    /*!
      \brief Line where the exception occurred.

      This information must be provided by using the macro __LINE__
    */
    unsigned which_line_;
    /*!
      \brief Short explanation concerning the exception thrown.
     */
    std::string explanation_;

  private:
    /*!
      \brief The name of the class, which what() gives where there is no
      explanation (GAOL v5)
    */
    virtual const char* class_name() const noexcept;
  };


  /*!
    \brief Exception thrown when an input interval does not follow
    the syntax.

    \see Refer to the documentation of the parser for intervals
    for the expected syntax (file gaol_interval_parser.ypp)
  */
  class GAOL_PUBLIC input_format_error : public gaol_exception {
  public:
    input_format_error(const char* f, unsigned l) : gaol_exception(f,l) {}
    input_format_error(const char* f, unsigned l, const char* e) :
      gaol_exception(f,l,e) {}
    input_format_error(const char* f, unsigned l, const std::string& e) :
      gaol_exception(f,l,e) {}

  private:
    const char* class_name() const noexcept override {
      return "input_format_error";
    }
  };


  /*!
    \brief Exception thrown whenever a feature is called for while not
    yet implemented, or is not available due to the way the library was
    configured.
  */
  class GAOL_PUBLIC unavailable_feature_error : public gaol_exception {
  public:
    unavailable_feature_error(const char* f, unsigned l) : gaol_exception(f,l) {}
    unavailable_feature_error(const char* f, unsigned l, const char* e) :
      gaol_exception(f,l,e) {}
    unavailable_feature_error(const char* f, unsigned l, const std::string& e) :
      gaol_exception(f,l,e) {}

  private:
    const char* class_name() const noexcept override {
      return "unavailable_feature_error";
    }
  };


  /*!
    \brief Exception thrown whenever an invalid action has been requested
    (such as computing the number of floating point numbers between
    two floating-point numbers, at least one of them being either a NaN or
    an infinite value).
  */
  class GAOL_PUBLIC invalid_action_error : public gaol_exception {
  public:
    invalid_action_error(const char* f, unsigned l) : gaol_exception(f,l) {}
    invalid_action_error(const char* f, unsigned l, const char* e) :
      gaol_exception(f,l,e) {}
    invalid_action_error(const char* f, unsigned l, const std::string& e) :
      gaol_exception(f,l,e) {}

  private:
    const char* class_name() const noexcept override {
      return "invalid_action_error";
    }
  };

  /*!
    \brief Display of an exception.

    Convenient operator to display the file, line and explanation for
    the exception thrown: "file, line n: exception thrown: explanation",
    without the colon and the explanation if none was given. The operator
    wrote what() too, between "exception" and "thrown": what() being the
    explanation now, it is written once (GAOL v5).
   */
  extern GAOL_PUBLIC std::ostream& operator<<(std::ostream& out,
					      const gaol_exception &e);

} // namespace gaol_core

// In the namespace gaol too, as in GAOL 4, with the operator<< of the
// exceptions (see gaol/gaol_interval.h)
namespace gaol {
  using gaol_core::gaol_exception;
  using gaol_core::input_format_error;
  using gaol_core::unavailable_feature_error;
  using gaol_core::invalid_action_error;
  using gaol_core::operator<<;
} // namespace gaol

#endif /* GAOL_EXCEPTIONS_ENABLED */
#endif /* GAOL_EXCEPTIONS_H */
