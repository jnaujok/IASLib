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
   CWindowsSystemException::CWindowsSystemException( DWORD dwError ) 
   : CException( "Windows System Exception" )
   {
      m_dwErrorCode = dwError;

      LPVOID lpMsgBuf;
      LPVOID lpDisplayBuf;

      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | 
          FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          m_dwErrorCode,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0, NULL );

      lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
          (lstrlen((LPCTSTR)lpMsgBuf)+40)*sizeof(TCHAR)); 
      wsprintf((LPTSTR)lpDisplayBuf, 
          TEXT("Windows System Error %d: %s"), 
          m_dwErrorCode, lpMsgBuf); 

      m_strErrorMessage = (const char *)lpDisplayBuf;

      LocalFree(lpMsgBuf);
      LocalFree(lpDisplayBuf);
   }

   CWindowsSystemException::~CWindowsSystemException( void )
   {
   }

} // namespace IASLib

#endif // IASLIB_WIN32__
