/**
 * @file
 * @brief File Exception Class.
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
    /// Thrown by the file classes when a system file call fails.
    ///
    /// It is built from an errno value, which it turns into a readable
    /// message and a severity.
    class CFileException : public CException
    {
        protected:
            int         m_nErrorNumber;     ///< The errno value given to the constructor.

        public:
                        /// Makes a file exception from an errno value.
                        /// Known values (EACCES, ENOENT, ENOSPC, EBADF and others) get a
                        /// matching message and severity. Unknown values keep the
                        /// message "Unknown File Exception" with FATAL severity.
                        /// @param nErrorNumber The errno value.
                        CFileException( int nErrorNumber );
            /// Destructor.
            virtual    ~CFileException( void );

                        DEFINE_OBJECT( CFileException );
            /// Throws a copy of this exception with its real type.
            /// @throws CFileException always.
            virtual void        Throw( void ) { throw *this; }


    };

/// Throws a CFileException for errno value @p x, recording the current file and line.
#define IASLIB_THROW_FILE_EXCEPTION(x) {CFileException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // __FILEEXCEPTION_H__
