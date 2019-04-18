/*
 * Stream Exception Class
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
    class CStreamException : public CException
    {
        public:
                        CStreamException( const char *strMessage );
            virtual    ~CStreamException( void );

                        DECLARE_OBJECT( CStreamException,CException );

            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_STREAM_EXCEPTION(x) {static CStreamException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_STRINGEXCEPTION_H__
