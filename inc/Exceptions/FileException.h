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
    /// Thrown by the file classes when a system call fails.
    ///
    /// Made from an errno value; the message describes it (a short text of
    /// the library's own for common values, the system's text for others)
    /// and the severity depends on it.
    class CFileException : public CException
    {
        protected:
            int         m_nErrorNumber;     ///< The errno value.

        public:
                        /// Makes an exception for errno value @p nErrorNumber.
                        /// @note Numbers the library did not know used to give
                        /// "Unknown File Exception"; they now give the system's text.
                        CFileException( int nErrorNumber );
            virtual    ~CFileException( void );   ///< Destructor.

                        DEFINE_OBJECT( CFileException );
            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.
                /// The errno value it was made from.
            int                 GetErrorNumber( void ) const { return m_nErrorNumber; }
            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.


    };

/// Throws a CFileException for errno value @p x, recording the file and line.
#define IASLIB_THROW_FILE_EXCEPTION(x) {CFileException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // __FILEEXCEPTION_H__
