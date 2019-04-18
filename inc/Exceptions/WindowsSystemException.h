/*
 * Windows System Exception Class
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
    class CWindowsSystemException : public CException
    {
        protected:
            DWORD       m_dwErrorCode;
            CString     m_strErrorMessage;

        public:

                        CWindowsSystemException( DWORD dwError );
            virtual    ~CWindowsSystemException( void );

            CString     GetWindowsError( void ) { return m_strErrorMessage; }
            DWORD       GetWindowsErrorCode( void ) { return m_dwErrorCode; }

            virtual void        Throw( void ) { throw *this; }

                        DECLARE_OBJECT(CWindowsSystemException,CException);

    };
} // namespace IASLib

#define IASLIB_THROW_WINDOWS_SYSTEM_ERROR(x) {static CWindowsSystemException oE(x); oE.FileLine( __FILE__, __LINE__ ); oE.Throw(); }

#endif // IASLIB_WIN32__
#endif // IASLIB_WINDOWSSYSTEMEXCEPTION_H__
