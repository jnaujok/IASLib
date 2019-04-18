/*
 * String Stream Class
 *
 *      This class provides the basis for streaming data into a CString
 * object. This allows us to use streaming operations to create a string
 * that can be used just like any other string.
 *      Note that this class can also be used to /read/ from a string,
 * just like any other stream.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/08/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRINGSTREAM_H__
#define IASLIB_STRINGSTREAM_H__

#include "../BaseTypes/String_.h"
#include "Stream.h"

namespace IASLib
{
    class CStringStream : public CStream
    {
        protected:
            CString                 m_strString;
            size_t                  m_nCurrentPosition;
        public:
                                    CStringStream( void );
                                    CStringStream( const CString &strSource );
            virtual                ~CStringStream( void );

                                    DECLARE_OBJECT(CStringStream,CStream);

			virtual CString         GetLine( void );
			virtual char            GetChar( void );
			virtual void            PutChar( const char chPut );
			virtual void            PutLine( const CString &strOutput );
			virtual unsigned char   GetUChar( void );
			virtual void            PutChar( const unsigned char chPut );
			virtual int             PutBuffer( const char *achBuffer, int nLength );
			virtual int             GetBuffer( char *achBuffer, int nLength );

            virtual bool            IsEOS( void ) { return ( m_nCurrentPosition == m_strString.GetCount() ); }
    }; // class CStringStream
} // namespace IASLib

#endif // IASLIB_STRINGSTREAM_H__
