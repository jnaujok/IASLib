/*
 * Exception Class
 *
 *      This is the base class for all exception types that the system
 * can throw during an error condition. All other types of exceptions
 * must be derived from this class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Exception.h"

#include <string>
#include <system_error>
#include <iostream>

namespace IASLib
{
    IMPLEMENT_OBJECT( CException, CObject );

    CException::CException( const char *strExceptionMessage, ExceptionSeverity nPriority )
    {
        m_nPriority = nPriority;
        m_strExceptionMessage = strExceptionMessage;
        m_pCallStack = new CExceptionCallStack();
        m_pCallStack->SetMessage( strExceptionMessage );
    }

    static CExceptionCallStack *cloneCallStack( CExceptionCallStack *pSource )
    {
        if ( pSource == NULL )
        {
            return NULL;
        }
        CExceptionCallStack *pCopy = new CExceptionCallStack( pSource->GetFile(), pSource->GetLine(), pSource->GetMessage() );
        pCopy->SetDownCall( cloneCallStack( pSource->GetDownCall() ) );
        return pCopy;
    }

    CException::CException( const CException &oSource ) : CObject()
    {
        m_nPriority = oSource.m_nPriority;
        m_strExceptionMessage = oSource.m_strExceptionMessage;
        m_pCallStack = cloneCallStack( oSource.m_pCallStack );
    }

    CException &CException::operator =( const CException &oSource )
    {
        if ( this != &oSource )
        {
            CExceptionCallStack *pCopy = cloneCallStack( oSource.m_pCallStack );
            delete m_pCallStack;
            m_pCallStack = pCopy;
            m_nPriority = oSource.m_nPriority;
            m_strExceptionMessage = oSource.m_strExceptionMessage;
        }
        return *this;
    }

    CException::~CException( void )
    {
       delete m_pCallStack;
       m_pCallStack = NULL;
    }

    const char *CException::ErrnoToText( int nErrno )
    {
        static thread_local std::string strText;
        strText = std::error_code( nErrno, std::generic_category() ).message();
        return strText.c_str();
    }

    void CException::FileLine( const char *strFile, int nLine )
    {
       if ( m_pCallStack == NULL )
           m_pCallStack = new CExceptionCallStack();
       m_pCallStack->SetFile( strFile );
       m_pCallStack->SetLine( nLine );
    }

    void CException::AddStack( const char *strFile, int nLine, const char *strMessage )
    {
       CExceptionCallStack *pTemp = new CExceptionCallStack( strFile, nLine, strMessage );
       pTemp->SetDownCall( m_pCallStack );
       m_pCallStack = pTemp;
    }

    CString CException::DumpStack( void )
    {
        return ( m_pCallStack ) ? m_pCallStack->UnwindStack() : CString( "" );
    }

} // namespace IASLib
