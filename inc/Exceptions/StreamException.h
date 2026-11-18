/**
 * @file
 * @brief Stream Exception Class.
 *
 *      This class is used for throwing whenever an exception occurs in
 * the stream handling classes. Stream exceptions can fall into multiple
 * classes, including attempting to read past the end of a file, or
 * attempting to write to an input-only stream.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/08/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STREAMEXCEPTION_H__
#define IASLIB_STREAMEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    /// Thrown by the stream classes, for example on reading past the end of
    /// a stream or writing to an input-only one.
    class CStreamException : public CException
    {
        public:
                        /// Makes an exception with a message (NORMAL severity).
                        CStreamException( const char *strMessage );
            virtual    ~CStreamException( void );   ///< Destructor.

                        DECLARE_OBJECT( CStreamException,CException );

            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.

            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.


    };

/// Throws a CStreamException with message @p x, recording the file and line.
#define IASLIB_THROW_STREAM_EXCEPTION(x) {CStreamException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_STREAMEXCEPTION_H__
