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
    /// It keeps the Windows error code and the system's text for it.
    /// The CException message stays "Windows System Exception"; use
    /// GetWindowsError() for the system text.
    class CWindowsSystemException : public CException
    {
        protected:
            DWORD       m_dwErrorCode;      ///< The Windows error code (from GetLastError()).
            CString     m_strErrorMessage;  ///< "Windows System Error N: text" from FormatMessage().

        public:

                        /// Makes the exception and looks up the system text for the code.
                        /// @param dwError A Windows error code, usually from GetLastError().
                        CWindowsSystemException( DWORD dwError );
            /// Destructor.
            virtual    ~CWindowsSystemException( void );

            /// The text "Windows System Error N: message" for the error code.
            CString     GetWindowsError( void ) { return m_strErrorMessage; }
            /// The Windows error code.
            DWORD       GetWindowsErrorCode( void ) { return m_dwErrorCode; }

            /// Throws a copy of this exception with its real type.
            /// @throws CWindowsSystemException always.
            virtual void        Throw( void ) { throw *this; }

                        DECLARE_OBJECT(CWindowsSystemException,CException);

    };
} // namespace IASLib

/// Throws a CWindowsSystemException for Windows error code @p x,
/// recording the current file and line.
#define IASLIB_THROW_WINDOWS_SYSTEM_ERROR(x) {CWindowsSystemException oE(x); oE.FileLine( __FILE__, __LINE__ ); oE.Throw(); }

#endif // IASLIB_WIN32__
#endif // IASLIB_WINDOWSSYSTEMEXCEPTION_H__
