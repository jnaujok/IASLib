/*
 * String Stream Class
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
    class CStringStream : public CStream
    {
        protected:
            std::string             m_strInput;
            size_t                  m_nCurrentPosition;
            std::string             m_strOutput;

        public:
                                    CStringStream( void );
                                    CStringStream( const CString &strSource );
                                    CStringStream( const char *pData, size_t nLength );
                                    CStringStream( const std::string &strSource );
            virtual                ~CStringStream( void );

                                    DECLARE_OBJECT(CStringStream,CStream);

            virtual int             GetBuffer( char *achBuffer, int nLength );
            virtual int             PutBuffer( const char *achBuffer, int nLength );
            virtual bool            IsEOS( void ) { return m_nCurrentPosition >= m_strInput.size(); }
            virtual char            PeekChar( void );
            virtual char            GetChar( void );
            virtual CString         GetLine( void );
            virtual size_t          bytesRemaining( void ) { return m_strInput.size() - m_nCurrentPosition; }
            virtual void            Close( void ) { m_strInput.clear(); m_nCurrentPosition = 0; m_bIsOpen = false; }

                // What has been written (as text, up to any NUL; or as bytes).
            virtual CString         GetString( void ) { return CString( m_strOutput.data(), m_strOutput.size() ); }
            const std::string      &GetOutput( void ) const { return m_strOutput; }
            void                    ClearOutput( void ) { m_strOutput.clear(); }
    }; // class CStringStream
} // namespace IASLib

#endif // IASLIB_STRINGSTREAM_H__
