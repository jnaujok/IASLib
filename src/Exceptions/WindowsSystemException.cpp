/*
 * Windows System Exception Class
 *
 *      This class supports throwing exceptions returned from Windows 
 * system level calls. It stores the basic error code and the translated
 * windows error message.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/02/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "WindowsSystemException.h"

#ifdef IASLIB_WIN32__

namespace IASLib
{
      // (When FormatMessage() failed the buffer was used uninitialised, and
      // the text never reached the exception's message.)
   CWindowsSystemException::CWindowsSystemException( DWORD dwError )
   : CException( "Windows System Exception" )
   {
      m_dwErrorCode = dwError;

      LPSTR pszMessage = NULL;
      DWORD dwLength = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       NULL, m_dwErrorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&pszMessage, 0, NULL );
      CString strSystem;
      if ( dwLength > 0 && pszMessage != NULL )
      {
            // Without the trailing line break FormatMessage adds.
         while ( dwLength > 0 && ( pszMessage[ dwLength - 1 ] == '\r' || pszMessage[ dwLength - 1 ] == '\n' || pszMessage[ dwLength - 1 ] == ' ' ) )
            pszMessage[ --dwLength ] = '\0';
         strSystem = pszMessage;
      }
      else
      {
         strSystem = "(no description)";
      }
      if ( pszMessage != NULL )
         LocalFree( pszMessage );

      m_strErrorMessage.Format( "Windows System Error %lu: %s", (unsigned long)m_dwErrorCode, (const char *)strSystem );
      m_strExceptionMessage = m_strErrorMessage;
      SyncMessage();
   }

   CWindowsSystemException::~CWindowsSystemException( void )
   {
   }

} // namespace IASLib

#endif // IASLIB_WIN32__
