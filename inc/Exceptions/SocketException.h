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
    /// Thrown by the socket classes when a socket operation fails.
    ///
    /// Made from an errno value (CSocket translates Winsock's codes first);
    /// the message describes it and the severity depends on it.
    class CSocketException : public CException
    {
        protected:
            int         m_nErrorNumber;     ///< The errno value.

        public:
                        /// Makes an exception for errno value @p nErrorNumber.
                        /// Unknown numbers give "Unknown error occurred on socket.
                        /// Error Number = n".
                        CSocketException( int nErrorNumber );
            virtual    ~CSocketException( void );   ///< Destructor.

                        DEFINE_OBJECT( CSocketException );
            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.
            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.
                /// The errno value (Winsock's codes are translated to these).
            int                 GetErrorNumber( void ) const { return m_nErrorNumber; }

    };
} // namespace IASLib

/// Throws a CSocketException for errno value @p x, recording the file and line.
#define IASLIB_THROW_SOCKET_EXCEPTION(x) {CSocketException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

#endif // __SOCKETEXCEPTION_H__
