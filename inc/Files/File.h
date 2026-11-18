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
#include <stdint.h>
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CFile : public CObject
    {
        public:
                // READ: an existing file. WRITE: a new or emptied file.
                // READ | WRITE: an existing file, both ways. CREATE (with
                // WRITE, or READ | WRITE): a new or emptied file, both ways.
                // APPEND: writes go to the end (with READ, reads too).
                // BINARY: no line-ending translation (Windows).
            enum OpenFlags {
		    READ              = 0x0001,
		    WRITE             = 0x0002,
            APPEND            = 0x0004,
            BINARY            = 0x0008,
            CREATE            = 0x0010
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
            int64_t         m_lFileSize;        // -1 until known
            int64_t         m_lCurrentPos;      // -1 until known
        public:
	                        CFile();
                            CFile( const char *strFileName, int nMode );
#ifdef IASLIB_WIN32__
                                // A C runtime file descriptor (from _open).
                            CFile( int nHandle, int nMode );
#endif
	        virtual         ~CFile();

                            DEFINE_OBJECT( CFile )

            virtual bool    IsEOF( void );
            virtual bool    Open( const char *strFileName, int nMode );
            virtual bool    Close( void );
            virtual bool    Flush( void );
            virtual int     Read( char *pchBuffer, int nBufferSize );
            virtual int     Write( const char *pchBuffer, int nBufferSize );
            virtual int     WriteString( const CString &oString );
                // Positions and sizes as a long: -1 when the value does not
                // fit (a file past 2 GB where long is 32 bits, as on
                // Windows). The Big methods below take any size.
            virtual long    GetPos( void );
            virtual long    GetSize( void );
            virtual long    Seek( long nPos, int nStart );

                // 64-bit positions and sizes, for files of any size (up to
                // 2^63 bytes). SeekBig() returns the new position, or -1 if
                // the seek failed.
            virtual int64_t GetPosBig( void );
            virtual int64_t GetSizeBig( void );
            virtual int64_t SeekBig( int64_t nPos, int nStart );
                // Reads or writes any number of bytes (in pieces where the C
                // library needs it). Returns the bytes moved, or -1 if the
                // file is not open for it.
            virtual int64_t ReadBig( void *pBuffer, size_t nLength );
            virtual int64_t WriteBig( const void *pBuffer, size_t nLength );
            virtual bool    IsOpen( void ) const { return (m_fpFile != NULL); }
                // The next line, with its line ending ("" at the end of the
                // file). Any length; works in binary mode too.
            virtual CString GetLine( void );
            const CString  &GetFileName( void ) const { return m_strFileName; }

                // Throw a CFileException if they fail. Rename() replaces a
                // file already called strNewName, on every platform.
            static bool     Delete( const char *strFileName );
            static bool     Rename( const char *strOldName, const char *strNewName );
                // Whether a file (or directory) of that name exists.
            static bool     Exists( const char *strFileName );

        protected:
                // The fopen() mode for OpenFlags, setting the m_b* flags.
            CString         ModeString( int nMode );

        private:
                // (A copy would share the FILE and close it twice.)
                            CFile( const CFile & );
            CFile          &operator =( const CFile & );
    };
} // namespace IASLib

#endif // IASLIB_FILE_H__

