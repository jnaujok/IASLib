/*
 * File Exception Class
 *
 *      This class is used to wrap file exception objects used by the
 * file class when it encounters an unexpected error condition. This
 * class is derived from the base exception class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_FILEEXCEPTION_H__
#define IASLIB_FILEEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    class CFileException : public CException
    {
        protected:
            int         m_nErrorNumber;

        public:
                        CFileException( int nErrorNumber );
            virtual    ~CFileException( void );

                        DEFINE_OBJECT( CFileException );
            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_FILE_EXCEPTION(x) {static CFileException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // __FILEEXCEPTION_H__
