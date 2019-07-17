/*
 * Socket Stream Implementor Class
 *
 *      This class extends the basic stream interface to support streaming
 * data to and from network sockets. This provides several layers of 
 * functionality that make network programming much simpler.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/19/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SocketStream.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
   IMPLEMENT_OBJECT( CSocketStream, CStream );

   CSocketStream::CSocketStream( void )
   {
       m_bNoDelete = false;
       m_pSocket = NULL;
   }

   CSocketStream::CSocketStream( CSocket *pSocket )
   {
       m_bNoDelete = false;
       m_pSocket = pSocket;
   }

   CSocketStream::~CSocketStream( void )
   {
       if ( ( m_pSocket ) && ( ! m_bNoDelete ) )
       {
           delete m_pSocket;
       }
       m_pSocket = NULL;
       m_bIsOpen = false;
   }

   /***********************************************************************
   **  GetLine 
   **
   **  Description:
   **      This method reads a single line from the socket stream. Please 
   ** note that this method can block and should only be called from a 
   ** sub-thread that can block safely.
   **
   ***********************************************************************/

   CString CSocketStream::GetLine( void )
   {
       CString strRetVal;
       bool bDone = false;
       char chWork;

       if ( m_pSocket )
       {
           while ( ! bDone )
           {
               chWork = GetChar();

               if ( ( chWork != '\r' ) && ( chWork != '\0' ) && ( chWork != '\n' ) )
               {
                   strRetVal += chWork;
               }
               else
               {
                   if ( chWork != '\r' )
                   {
                       bDone = true;
                   }
               }
           }
       }

       return strRetVal;
   }

   /***********************************************************************
   **  GetChar 
   **
   **  Description:
   **      This method reads a single character from the socket stream. Please
   ** note that this method can block, and should only be called from a 
   ** separate thread that can block safely.
   **
   ***********************************************************************/
   char CSocketStream::GetChar( void )
   {
       if ( m_pSocket )
       {
           char chRetVal;

           chRetVal = (char)m_pSocket->Read( );

           return chRetVal;
       }
       return '\0';
   }

   unsigned char CSocketStream::GetUChar( void )
   {
       if ( m_pSocket )
       {
           unsigned char chRetVal;

           chRetVal = m_pSocket->Read( );

           return chRetVal;
       }
       return '\0';
   }

   void CSocketStream::PutChar( const char chPut )
   {
       if ( m_pSocket )
       {
           char chWork = chPut;
           m_pSocket->Send( &chWork, sizeof( char ) );
       }
   }

   void CSocketStream::PutChar( const unsigned char chPut )
   {
       if ( m_pSocket )
       {
           unsigned char chWork = chPut;
           m_pSocket->Send( chWork );
       }
   }

   void CSocketStream::PutLine( const CString &strOutput )
   {
       if ( m_pSocket )
       {
           size_t      lLength = strOutput.GetLength();
           const char *pszOutput = (const char *)strOutput;

           m_pSocket->Send( pszOutput, (int)( lLength * sizeof(char) ) );
           unsigned char chRet = '\n';
           m_pSocket->Send( chRet );
       }
   }

   int CSocketStream::PutBuffer( const char *achBuffer, int nLength )
   {
       int nSent = 0;

       while ( nSent < nLength )
       {
           nSent += m_pSocket->Send( &(achBuffer[nSent]), nLength - nSent );
       }

       return nSent;
   }

   int CSocketStream::GetBuffer( char *achBuffer, int nLength )
   {
       int nReceived = 0;

       while ( nReceived < nLength )
       {
           nReceived += m_pSocket->Read( &(achBuffer[nReceived]), nLength - nReceived );
       }

       return nReceived;
   }

   bool CSocketStream::IsEOS( void )
   {
       return ( m_pSocket->HasData() );
   }
} // Namespace IASLib

#endif // IASLIB_NETWORKING__
