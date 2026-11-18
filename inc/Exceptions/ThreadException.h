/**
 * @file
 * @brief Thread Exception Class.
 *
 *      This class is used for throwing whenever an exception occurs in
 * the a thread related class. Thread exceptions include cancelling or stopping
 * an already completed thread, or returning a thread to a thread pool that
 * is not part of that thread pool
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/21/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADEXCEPTION_H__
#define IASLIB_THREADEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    /// Thrown by the thread classes, for example when stopping a thread
    /// that has already finished, or returning a thread to the wrong pool.
    class CThreadException : public CException
    {
        public:
                        /// Makes a thread exception with NORMAL severity.
                        /// @param strMessage The message text.
                        CThreadException( const char *strMessage );
            /// Destructor.
            virtual    ~CThreadException( void );

                        DECLARE_OBJECT( CThreadException,CException );

            /// Throws a copy of this exception with its real type.
            /// @throws CThreadException always.
            virtual void        Throw( void ) { throw *this; }


    };

/// Throws a CThreadException with message @p x, recording the current file and line.
#define IASLIB_THROW_THREAD_EXCEPTION(x) {CThreadException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_THREADEXCEPTION_H__
