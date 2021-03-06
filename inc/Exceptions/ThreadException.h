/*
 * Thread Exception Class
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
    class CThreadException : public CException
    {
        public:
                        CThreadException( const char *strMessage );
            virtual    ~CThreadException( void );

                        DECLARE_OBJECT( CThreadException,CException );

            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_THREAD_EXCEPTION(x) {static CThreadException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_THREADEXCEPTION_H__
