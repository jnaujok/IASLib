/**
 * @file
 * @brief String Stream Class.
 *
 *      A stream over memory: it reads from the text it was given, and
 * collects what is written separately (GetString(), or GetOutput() for
 * bytes, NULs and all). Both sides hold any bytes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997 (reworked 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRINGSTREAM_H__
#define IASLIB_STRINGSTREAM_H__

#include "../BaseTypes/String_.h"
#include "Stream.h"

#include <string>

namespace IASLib
{
    /// A stream in memory, with separate input and output.
    ///
    /// Reads come from the bytes given to the constructor. Writes are
    /// appended to a separate output buffer, read back with GetString() or
    /// GetOutput(); writing does not change what can be read. Binary data
    /// (NUL bytes included) is kept as is.
    ///
    /// @code
    /// CStringStream oIn( CString( "one\ntwo\n" ) );
    /// CString strFirst = oIn.GetLine();       // "one"
    ///
    /// CStringStream oOut;
    /// oOut << "n=" << 5;
    /// CString strText = oOut.GetString();     // "n=5"
    /// @endcode
    class CStringStream : public CStream
    {
        protected:
            std::string             m_strInput;             ///< Bytes to read.
            size_t                  m_nCurrentPosition;     ///< Read position in m_strInput.
            std::string             m_strOutput;            ///< Bytes written so far.

        public:
                                    /// Makes a stream with nothing to read.
                                    CStringStream( void );
                                    /// Makes a stream that reads the string's bytes.
                                    CStringStream( const CString &strSource );
                                    /// Makes a stream that reads @p nLength bytes from @p pData
                                    /// (copied). NULL gives nothing to read.
                                    CStringStream( const char *pData, size_t nLength );
                                    /// Makes a stream that reads the string's bytes.
                                    CStringStream( const std::string &strSource );
            /// Destructor.
            virtual                ~CStringStream( void );

                                    DECLARE_OBJECT(CStringStream,CStream);

            /// Reads up to @p nLength bytes.
            /// @return Bytes read; 0 at the end, or if @p achBuffer is NULL or @p nLength <= 0.
            virtual int             GetBuffer( char *achBuffer, int nLength );
            /// Appends @p nLength bytes to the output.
            /// @return @p nLength; 0 if @p achBuffer is NULL or @p nLength <= 0.
            virtual int             PutBuffer( const char *achBuffer, int nLength );
            /// True when all the input has been read.
            virtual bool            IsEOS( void ) { return m_nCurrentPosition >= m_strInput.size(); }
            /// The next input byte, without taking it.
            /// @throws CStreamException at the end of the input.
            virtual char            PeekChar( void );
            /// Reads one input byte.
            /// @throws CStreamException at the end of the input.
            virtual char            GetChar( void );
            /// Reads up to the next newline, leaving off it and a carriage return
            /// before it. Unlike CStream::GetLine(), NUL bytes are kept.
            /// @return The line; "" at the end.
            virtual CString         GetLine( void );
            /// Input bytes not yet read.
            virtual size_t          bytesRemaining( void ) { return m_strInput.size() - m_nCurrentPosition; }
            /// Discards the input and marks the stream closed. The output is kept.
            virtual void            Close( void ) { m_strInput.clear(); m_nCurrentPosition = 0; m_bIsOpen = false; }

            /// @name Output
            /// What has been written (as text, up to any NUL; or as bytes).
            ///@{
            virtual CString         GetString( void ) { return CString( m_strOutput.data(), m_strOutput.size() ); }   ///< The output as a CString.
            const std::string      &GetOutput( void ) const { return m_strOutput; }                                ///< The output bytes.
            void                    ClearOutput( void ) { m_strOutput.clear(); }                                     ///< Empties the output.
            ///@}
    }; // class CStringStream
} // namespace IASLib

#endif // IASLIB_STRINGSTREAM_H__
