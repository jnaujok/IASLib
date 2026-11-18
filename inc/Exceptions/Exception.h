/**
 * @file
 * @brief Exception Class.
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
    /// One entry in an exception's call stack: a file, a line and a message.
    ///
    /// Entries form a singly linked list through the "down call". Each entry
    /// owns the entry below it and deletes it when it is destroyed.
    class CExceptionCallStack : public CObject
    {
        protected:
            int                     m_nLine;            ///< Source line of this entry.
            CString                 m_strFileName;      ///< Source file of this entry.
            CString                 m_strMessage;       ///< Message for this entry.
            CExceptionCallStack    *m_pDown;            ///< The entry below this one (owned), or NULL.
        public:
                                    /// Makes an entry for "Unknown File", line 0, "Unknown Message".
                                    CExceptionCallStack( void )
                                    {
                                       m_nLine = 0;
                                       m_strFileName = "Unknown File";
                                       m_strMessage = "Unknown Message";
                                       m_pDown = NULL;
                                    }

                                    /// Makes an entry for the given file, line and message.
                                    CExceptionCallStack( const char *strFileName, int nLine, const char *strMessage )
                                    {
                                       m_nLine = nLine;
                                       m_strFileName = strFileName;
                                       m_strMessage = strMessage;
                                       m_pDown = NULL;
                                    }

                                    /// Destructor. Deletes the entries below this one too.
                                    virtual                ~CExceptionCallStack( void ) { delete m_pDown; }

                                    DECLARE_OBJECT( CExceptionCallStack, CObject );

            void                    SetLine( int nLine ) { m_nLine = nLine; }                       ///< Sets the source line.
            void                    SetFile( const char *strFileName ) { m_strFileName = strFileName; } ///< Sets the source file.
            void                    SetMessage( const char *strMessage ) { m_strMessage = strMessage; } ///< Sets the message.
                                    /// Sets the entry below this one. This entry takes ownership
                                    /// of @p pDown; a previous one is not deleted.
            void                    SetDownCall( CExceptionCallStack *pDown ) { m_pDown = pDown; }

            int                     GetLine( void ) { return m_nLine; }             ///< The source line.
            CString                 GetFile( void ) { return m_strFileName; }       ///< The source file.
            CString                 GetMessage( void ) { return m_strMessage; }     ///< The message.
            CExceptionCallStack    *GetDownCall( void ) { return m_pDown; }         ///< The entry below this one (still owned by this one), or NULL.
                                    /// This entry and all below it as text, one
                                    /// "Call: file - line", message, blank line block each.
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

    /// The base class of every exception the library throws.
    ///
    /// An exception carries a message, a severity and a call stack of
    /// CExceptionCallStack entries (owned by the exception; a copy gets its
    /// own copy of the stack). Exceptions are thrown and caught by value or
    /// reference; derived classes override Throw() and Rethrow() so that the
    /// IASLIB_RETHROW macro keeps their type.
    ///
    /// @code
    /// try
    /// {
    ///     IASLIB_THROW_DEFAULT( "Something went wrong" );
    /// }
    /// catch ( CException &e )
    /// {
    ///     printf( "%s\n%s", (const char *)e.GetMessage(), (const char *)e.DumpStack() );
    /// }
    /// @endcode
    class CException : public CObject
    {
        public:
            /// How serious an exception is.
            enum ExceptionSeverity {
            WARNING           = 0x0001,     ///< A warning only.
            LOW               = 0x0002,     ///< Low severity.
            NORMAL            = 0x0004,     ///< Normal severity (the default).
            HIGH              = 0x0008,     ///< High severity.
            FATAL             = 0x0010      ///< The program cannot go on.
            };
        protected:
            ExceptionSeverity    m_nPriority;               ///< The severity.
            CString              m_strExceptionMessage;     ///< The message.
            CExceptionCallStack *m_pCallStack;              ///< The top of the call stack (owned), or NULL.

        public:
                                /// Makes an exception with a message and a severity. The
                                /// call stack starts with one entry holding the message.
                                CException( const char *strExceptionMessage, 
                                    CException::ExceptionSeverity nPriority = CException::NORMAL );
                                    /// Copy constructor. Exceptions are thrown by value, so
                                    /// copies must not share the call stack (it is deleted by
                                    /// each copy): the copy gets its own copy of it.
                                CException( const CException &oSource );
                                    /// Assignment; copies the call stack, as the copy constructor does.
            CException         &operator =( const CException &oSource );

                                    /// Destructor. Deletes the call stack.
            virtual            ~CException( void );

                                DEFINE_OBJECT( CException )

                                /// The message, as a char *.
                                operator const char *( void ) { return (const char *) m_strExceptionMessage; }
                                /// The message.
            const CString      &GetMessage( void ) const { return m_strExceptionMessage; }

                /// The system's description of an errno value ("No such file
                /// or directory"). The text stays valid until the same thread
                /// calls this again.
                /// @note It used to come from a fixed table, from one Unix,
                /// that was wrong for most numbers on other systems.
            static const char  *ErrnoToText( int nErrno );

                /// Throw a copy of this exception, as its own class: every
                /// derived class overrides both, so IASLIB_RETHROW keeps the
                /// type.
            virtual void        Throw( void ) { throw *this; }
                /// Same as Throw(); used by IASLIB_RETHROW.
            virtual void        Rethrow( void ) { throw *this; }

                /// Sets the file and line of the top call stack entry (making
                /// one if there is none). Used by the throw macros.
            virtual void         FileLine( const char *strFile, int nLine );
                /// Pushes a new entry for the file, line and message on top of
                /// the call stack. Used by IASLIB_RETHROW.
            virtual void         AddStack( const char *strFile, int nLine, const char *strMessage );

                /// The whole call stack as text (see
                /// CExceptionCallStack::UnwindStack()); "" if there is none.
            CString              DumpStack( void );

        protected:
                /// For derived constructors that set m_strExceptionMessage:
                /// the call stack's first entry takes the same text.
            void                 SyncMessage( void ) { if ( m_pCallStack ) m_pCallStack->SetMessage( m_strExceptionMessage ); }
    };
} // namespace IASLib

/// Throws a CException with message @p x, recording the current file and line.
#define IASLIB_THROW_DEFAULT( x ) {CException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

/// Adds the current file, line and message @p y to exception @p x's call
/// stack and throws it again, keeping its class.
#define IASLIB_RETHROW(x,y) {x.AddStack(__FILE__,__LINE__,y);x.Rethrow();}

#endif // __EXCEPTION_H__
