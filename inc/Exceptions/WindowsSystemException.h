/**
 * @file
 * @brief Windows System Exception Class.
 *
 *      This class supports throwing exceptions returned from Windows 
 * system level calls. It stores the basic error code and the translated
 * windows error message.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/28/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_WINDOWSSYSTEMEXCEPTION_H__
#define IASLIB_WINDOWSSYSTEMEXCEPTION_H__

#include "Exception.h"

#ifdef IASLIB_WIN32__

namespace IASLib
{
    /// Thrown when a Windows system call fails (Windows builds only).
    ///
    /// Holds the Windows error code and the system's text for it.
    class CWindowsSystemException : public CException
    {
        protected:
            DWORD       m_dwErrorCode;          ///< The Windows error code.
            CString     m_strErrorMessage;      ///< "Windows System Error n: text".

        public:

                        /// Makes an exception for Windows error code @p dwError (as
                        /// from GetLastError()). The message is "Windows System Error
                        /// n: text", with "(no description)" if Windows has no text.
                        /// @note When FormatMessage() failed the buffer used to be
                        /// used uninitialised, and the text never reached the message.
                        CWindowsSystemException( DWORD dwError );
            virtual    ~CWindowsSystemException( void );   ///< Destructor.

            CString     GetWindowsError( void ) { return m_strErrorMessage; }     ///< The message, with the system's text.
            DWORD       GetWindowsErrorCode( void ) { return m_dwErrorCode; }     ///< The Windows error code.

            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.

            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.

                        DECLARE_OBJECT(CWindowsSystemException,CException);

    };
} // namespace IASLib

/// Throws a CWindowsSystemException for Windows error code @p x, recording the file and line.
#define IASLIB_THROW_WINDOWS_SYSTEM_ERROR(x) {CWindowsSystemException oE(x); oE.FileLine( __FILE__, __LINE__ ); oE.Throw(); }

#endif // IASLIB_WIN32__
#endif // IASLIB_WINDOWSSYSTEMEXCEPTION_H__
