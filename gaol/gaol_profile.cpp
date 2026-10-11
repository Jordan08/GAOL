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
  \file   gaol_profile.cpp
  \brief

  <long description>

  \author Frederic Goualard, Joran NININ
  \date   2001-10-02
*/

#include "gaol/gaol_profile.h"

namespace gaol_core {

  static long last_usr_time;
  static long last_reset_time;
} // namespace gaol_core

#if GAOL_HAVE_GETRUSAGE
  //================

  #  include <cstdlib>
  #  include <sys/time.h>
  #  include <sys/resource.h>


  #  include <unistd.h>

  namespace gaol_core {

    long get_time(void)
    {
      struct rusage RsrUsage;
      getrusage(RUSAGE_SELF,&RsrUsage);
      return (RsrUsage.ru_utime.tv_sec*1000 + RsrUsage.ru_utime.tv_usec/1000);
    }
  } // namespace gaol_core

#else
  //==============
  // clock(), which the C standard provides: where getrusage() is not
  // (GAOL_HAVE_GETRUSAGE, which the three builds check in <sys/resource.h>, as Visual
  // C++) (GAOL v5)
  #  include <time.h>

  namespace gaol_core {

    long get_time(void)
    {
      return long((1000.*clock())/CLOCKS_PER_SEC);
    }
  } // namespace gaol_core

#endif /* GAOL_HAVE_GETRUSAGE */

namespace gaol_core {

  void reset_time(void)
  {
    last_reset_time=last_usr_time=get_time();
  }

  long intermediate_elapsed_time(void)
  {
    long usrtime,res;

    usrtime=get_time();
    res=usrtime-last_usr_time;
    last_usr_time=usrtime;
    return res;
  }

  long elapsed_time(void)
  {
    return get_time()-last_reset_time;
  }


  /*
    timepiece --

   */

  timepiece::timepiece()
  {
    the_last_time = 0;
    total_time = 0;
  }

  void timepiece::start(void)
  {
    the_last_time = get_time();
  }

  void timepiece::stop(void)
  {
    long tmp = get_time();
    total_time += tmp - the_last_time;
  }

  void timepiece::reset(void)
  {
    total_time = 0;
  }

  long timepiece::get_total_time(void) const
  {
    return total_time;
  }

  long timepiece::get_intermediate_time(void) const
  {
    return get_time()-the_last_time;
  }

} // namespace gaol_core
