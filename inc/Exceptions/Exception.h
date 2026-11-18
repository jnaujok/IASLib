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
    /// One frame of the call stack recorded in a CException.
    ///
    /// Each frame holds a file name, a line number and a message, plus a
    /// pointer to the frame below it. Frames form a singly linked list with
    /// the most recently added frame at the top. A frame owns the frame
    /// below it and deletes it in its destructor, so deleting the top frame
    /// frees the whole chain.
    class CExceptionCallStack : public CObject
    {
        protected:
            int                     m_nLine;        ///< Source line of this frame.
            CString                 m_strFileName;  ///< Source file of this frame.
            CString                 m_strMessage;   ///< Message recorded for this frame.
            CExceptionCallStack    *m_pDown;        ///< Next frame down (owned), or NULL.
        public:
                                    /// Makes a frame with "Unknown File", line 0 and "Unknown Message".
                                    CExceptionCallStack( void )
                                    {
                                       m_nLine = 0;
                                       m_strFileName = "Unknown File";
                                       m_strMessage = "Unknown Message";
                                       m_pDown = NULL;
                                    }

                                    /// Makes a frame for the given place and message.
                                    /// @param strFileName Source file name.
                                    /// @param nLine Source line number.
                                    /// @param strMessage Message for this frame.
                                    CExceptionCallStack( const char *strFileName, int nLine, const char *strMessage )
                                    {
                                       m_nLine = nLine;
                                       m_strFileName = strFileName;
                                       m_strMessage = strMessage;
                                       m_pDown = NULL;
                                    }

                                    /// Deletes this frame and every frame below it.
                                    virtual                ~CExceptionCallStack( void ) { delete m_pDown; }

                                    DECLARE_OBJECT( CExceptionCallStack, CObject );

            /// Sets the source line.
            void                    SetLine( int nLine ) { m_nLine = nLine; }
            /// Sets the source file name.
            void                    SetFile( const char *strFileName ) { m_strFileName = strFileName; }
            /// Sets the message.
            void                    SetMessage( const char *strMessage ) { m_strMessage = strMessage; }
            /// Sets the frame below this one. This frame takes ownership of
            /// @p pDown; a frame already set there is not deleted.
            void                    SetDownCall( CExceptionCallStack *pDown ) { m_pDown = pDown; }

            /// The source line.
            int                     GetLine( void ) { return m_nLine; }
            /// The source file name.
            CString                 GetFile( void ) { return m_strFileName; }
            /// The message.
            CString                 GetMessage( void ) { return m_strMessage; }
            /// The frame below this one (still owned by this frame), or NULL.
            CExceptionCallStack    *GetDownCall( void ) { return m_pDown; }
            /// Formats this frame and every frame below it as text.
            /// @return For each frame, top first: "Call: file - line", the message, and a blank line.
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

    /// Base class of every exception the library throws.
    ///
    /// An exception carries a message, a severity and a call stack of
    /// CExceptionCallStack frames. The constructor makes the bottom frame;
    /// FileLine() fills in where it was thrown, and AddStack() pushes a new
    /// frame each time the exception is passed up with IASLIB_RETHROW.
    ///
    /// Exceptions are thrown and caught by value. Each copy owns a deep copy
    /// of the call stack. Derived classes override Throw() so that the
    /// throw keeps the derived type.
    ///
    /// @code
    /// try
    /// {
    ///     IASLIB_THROW_DEFAULT( "Something failed" );
    /// }
    /// catch ( CException &oE )
    /// {
    ///     puts( (const char *)oE.GetMessage() );
    ///     puts( (const char *)oE.DumpStack() );
    /// }
    /// @endcode
    class CException : public CObject
    {
        public:
            /// How serious an exception is.
            enum ExceptionSeverity {
            WARNING           = 0x0001,     ///< Informational; the operation may be retried.
            LOW               = 0x0002,     ///< Low severity.
            NORMAL            = 0x0004,     ///< Normal severity (the default).
            HIGH              = 0x0008,     ///< High severity.
            FATAL             = 0x0010      ///< The operation or object cannot continue.
            };
        protected:
            ExceptionSeverity    m_nPriority;           ///< Severity of this exception.
            CString              m_strExceptionMessage; ///< Message text.
            CExceptionCallStack *m_pCallStack;          ///< Top of the call stack (owned).

        public:
                                /// Makes an exception with a message and severity.
                                /// The bottom stack frame gets the message; its file and line
                                /// are set later by FileLine().
                                /// @param strExceptionMessage The message text.
                                /// @param nPriority The severity.
                                CException( const char *strExceptionMessage, 
                                    CException::ExceptionSeverity nPriority = CException::NORMAL );
                                /// Copies an exception, including a deep copy of its call stack.
                                /// Exceptions are thrown by value, so copies must not
                                /// share the call stack (it is deleted by each copy).
                                CException( const CException &oSource );
            /// Assigns from another exception, deep-copying its call stack.
            /// Self-assignment is safe.
            CException         &operator =( const CException &oSource );

            /// Deletes the call stack.
            virtual            ~CException( void );

                                DEFINE_OBJECT( CException )

                                /// The message text, as a C string owned by the exception.
                                operator const char *( void ) { return (const char *) m_strExceptionMessage; }
            /// The message text.
            const CString      &GetMessage( void ) const { return m_strExceptionMessage; }

            /// Describes an errno value.
            /// @param nErrno An errno value. Small negative values (-1 to -3)
            ///        are BSD kernel return codes.
            /// @return A static string such as "[ENOENT]         No such file or directory".
            ///         Values outside the table give an "Unused Error Code!" string.
            ///         The table follows historic BSD/AIX numbering, so for higher
            ///         codes the name may not match this platform's errno.
            static const char  *ErrnoToText( int nErrno );

            /// Throws a copy of this exception. Derived classes override it
            /// so the thrown object keeps its real type.
            /// @throws CException (or the derived type) always.
            virtual void        Throw( void ) { throw *this; }
            /// Throws a copy of this exception again (used by IASLIB_RETHROW).
            /// Derived classes do not override it, so it throws a CException,
            /// sliced from the derived object.
            /// @throws CException always.
            virtual void        Rethrow( void ) { throw *this; }

            /// Sets the file and line of the top stack frame.
            /// @param strFile Source file name, usually __FILE__.
            /// @param nLine Source line, usually __LINE__.
            virtual void         FileLine( const char *strFile, int nLine );
            /// Pushes a new frame on top of the call stack.
            /// @param strFile Source file name, usually __FILE__.
            /// @param nLine Source line, usually __LINE__.
            /// @param strMessage Message for the new frame.
            virtual void         AddStack( const char *strFile, int nLine, const char *strMessage );

            /// Formats the whole call stack, top frame first.
            /// @see CExceptionCallStack::UnwindStack()
            CString              DumpStack( void );
    };
} // namespace IASLib

/// Throws a CException with message @p x, recording the current file and line.
#define IASLIB_THROW_DEFAULT( x ) {CException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

/// Adds a stack frame (current file, line and message @p y) to exception
/// @p x and throws it again with CException::Rethrow().
#define IASLIB_RETHROW(x,y) {x.AddStack(__FILE__,__LINE__,y);x.Rethrow();}

#endif // __EXCEPTION_H__
