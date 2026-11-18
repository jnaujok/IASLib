/**
 * @file
 * @brief Socket Exception Class.
 *
 *      This class wraps socket based exceptions. When a socket fails,
 * it can throw an exception of this type that can be caught by a handler
 * at a lower level.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKETEXCEPTION_H__
#define IASLIB_SOCKETEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    /// Thrown by the socket classes when a socket call fails.
    ///
    /// It is built from an errno value, which it turns into a readable
    /// message and a severity.
    class CSocketException : public CException
    {
        protected:
            int         m_nErrorNumber;     ///< The errno value given to the constructor.

        public:
                        /// Makes a socket exception from an errno value.
                        /// Known values (ECONNRESET, ETIMEDOUT, EWOULDBLOCK and others)
                        /// get a matching message and severity. Unknown values get
                        /// "Unknown error occurred on socket. Error Number = N" with
                        /// NORMAL severity.
                        /// @param nErrorNumber The errno value. On Windows, CSocket
                        ///        translates Winsock's WSAE* codes to errno values first.
                        CSocketException( int nErrorNumber );
            /// Destructor.
            virtual    ~CSocketException( void );

                        DEFINE_OBJECT( CSocketException );
            /// Throws a copy of this exception with its real type.
            /// @throws CSocketException always.
            virtual void        Throw( void ) { throw *this; }
            /// The errno value (Winsock's codes are translated to these).
            int                 GetErrorNumber( void ) const { return m_nErrorNumber; }

    };
} // namespace IASLib

/// Throws a CSocketException for errno value @p x, recording the current file and line.
#define IASLIB_THROW_SOCKET_EXCEPTION(x) {CSocketException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

#endif // __SOCKETEXCEPTION_H__
