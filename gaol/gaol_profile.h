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
  \file   gaol_profile.h
  \brief

  <long description>

  \author Frederic Goualard
  \date   2001-10-02
*/


#ifndef GAOL_PROFILE_H
#define GAOL_PROFILE_H

#include "gaol/gaol_config.h"

namespace gaol_core {

  /*!
    \brief Object to record time for profiling purpose
   */
  class GAOL_PUBLIC timepiece {
  public:
    timepiece();
    //! Starts the chronometer
    void start(void);
    //! Stops the chronometer temporarily
    void stop(void);
    //! Stops the chronometer and reset the counter
    void reset(void);
    //! Returns the time elapsed since the first call to start()
    long get_total_time(void) const;
    //! Returns the time elapsed since the last call to start()
    long get_intermediate_time(void) const;
  private:
    long the_last_time;
    long total_time;
  };

extern GAOL_PUBLIC long get_time(void);
  /*!
    Sets the base for time tracking.
   */
extern GAOL_PUBLIC void reset_time(void);
  /*!
    Returns the elapsed time since the last call to reset_time()
   */
extern GAOL_PUBLIC long elapsed_time(void);
  /*!
    Returns the elapsed time since the last call to reset_time() or
    to intermediate_elapsed_time.
  */
extern GAOL_PUBLIC long intermediate_elapsed_time(void);

} // namespace gaol_core

#endif /* GAOL_PROFILE_H */
