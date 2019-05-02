/*
 * File Class
 *
 *      This class provides an object to wrap standard file I/O processes
 * to provide the class library with file support. It is used mainly to
 * provide an object oriented file object.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/10/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_FILE_H__
#define IASLIB_FILE_H__

#include <stdio.h>
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CFile : public CObject
    {
        public:
            enum OpenFlags {
		    READ              = 0x0001,
		    WRITE             = 0x0002,
            APPEND            = 0x0004,
            BINARY            = 0x0008
		    };

            enum SeekFlags {
		    BEGIN             = 0x0001,
		    END               = 0x0002,
            CURRENT           = 0x0003
		    };
        protected:
            CString         m_strFileName;
            FILE           *m_fpFile;
            bool            m_bInput;
            bool            m_bOutput;
            bool            m_bAppend;
            bool            m_bBinary;
            long            m_lFileSize;
            long            m_lCurrentPos;
        public:
	                        CFile();
                            CFile( const char *strFileName, int nMode );
	        virtual         ~CFile();

                            DEFINE_OBJECT( CFile )

            virtual bool    IsEOF( void );
            virtual bool    Open( const char *strFileName, int nMode );
            virtual bool    Close( void );
            virtual bool    Flush( void );
            virtual int     Read( char *pchBuffer, int nBufferSize );
            virtual int     Write( const char *pchBuffer, int nBufferSize );
            virtual int     WriteString( const CString &oString );
            virtual long    GetPos( void );
            virtual long    GetSize( void );
            virtual long    Seek( long nPos, int nStart );
            virtual bool    IsOpen( void ) const { return (m_fpFile != NULL); }
            virtual CString GetLine( void );

            static bool     Delete( const char *strFileName );
            static bool     Rename( const char *strOldName, const char *strNewName );
    };
} // namespace IASLib

#endif // IASLIB_FILE_H__

