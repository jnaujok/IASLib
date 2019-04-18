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

#ifndef IASLIB_EXCEPTION_H__
#define IASLIB_EXCEPTION_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CExceptionCallStack : public CObject
    {
        protected:
            int                     m_nLine;
            CString                 m_strFileName;
            CString                 m_strMessage;
            CExceptionCallStack    *m_pDown;
        public:
                                    CExceptionCallStack( void )
                                    {
                                       m_nLine = 0;
                                       m_strFileName = "Unknown File";
                                       m_strMessage = "Unknown Message";
                                       m_pDown = NULL;
                                    }

                                    CExceptionCallStack( const char *strFileName, int nLine, const char *strMessage )
                                    {
                                       m_nLine = nLine;
                                       m_strFileName = strFileName;
                                       m_strMessage = strMessage;
                                       m_pDown = NULL;
                                    }

                                    virtual                ~CExceptionCallStack( void ) { delete m_pDown; }

                                    DECLARE_OBJECT( CExceptionCallStack, CObject );

            void                    SetLine( int nLine ) { m_nLine = nLine; }
            void                    SetFile( const char *strFileName ) { m_strFileName = strFileName; }
            void                    SetMessage( const char *strMessage ) { m_strMessage = strMessage; }
            void                    SetDownCall( CExceptionCallStack *pDown ) { m_pDown = pDown; }

            int                     GetLine( void ) { return m_nLine; }
            CString                 GetFile( void ) { return m_strFileName; }
            CString                 GetMessage( void ) { return m_strMessage; }
            CExceptionCallStack    *GetDownCall( void ) { return m_pDown; }
            CString                 UnwindStack( void ) 
                                    {
                                        CString strWork;
                                        strWork.Format( "Call: %s - %d\n%s\n\n", (const char *)m_strFileName, m_nLine, (const char *)m_strMessage );
                                        if ( m_pDown )
                                        {
                                            strWork += m_pDown->UnwindStack();
                                        }
                                        return strWork;
                                    }
    };

    class CException : public CObject
    {
        public:
            enum ExceptionSeverity {
            WARNING           = 0x0001,
            LOW               = 0x0002,
            NORMAL            = 0x0004,
            HIGH              = 0x0008,
            FATAL             = 0x0010
            };
        protected:
            ExceptionSeverity    m_nPriority;
            CString              m_strExceptionMessage;
            CExceptionCallStack *m_pCallStack;

        public:
                                CException( const char *strExceptionMessage, 
                                    CException::ExceptionSeverity nPriority = CException::NORMAL );
            virtual            ~CException( void );

                                DEFINE_OBJECT( CException )

                                operator const char *( void ) { return (const char *) m_strExceptionMessage; }

            static const char  *ErrnoToText( int nErrno );

            virtual void        Throw( void ) { throw *this; }
            virtual void        Rethrow( void ) { throw *this; }

            virtual void         FileLine( const char *strFile, int nLine );
            virtual void         AddStack( const char *strFile, int nLine, const char *strMessage );

            CString              DumpStack( void );
    };
} // namespace IASLib

#define IASLIB_THROW_DEFAULT( x ) {static CException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

#define IASLIB_RETHROW(x,y) {x.AddStack(__FILE__,__LINE__,y);x.Rethrow();}

#endif // __EXCEPTION_H__
