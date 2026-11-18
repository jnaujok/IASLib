/**
 * @file File.h
 * @brief CFile, a file opened through the C library's stdio.
 *
 * The original header comment follows.
 */
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
    /// A file, read and written through the C library's stdio (FILE *).
    ///
    /// Open it with a combination of OpenFlags. A failed open throws a
    /// CFileException; the other methods report failure by their return
    /// value. The object closes the file when it is destroyed. A CFile is
    /// not safe to share between threads without a lock.
    ///
    /// @code
    /// CFile oFile( "notes.txt", CFile::READ );
    /// while ( ! oFile.IsEOF() )
    ///     CString strLine = oFile.GetLine();
    /// @endcode
    class CFile : public CObject
    {
        public:
            /// How to open a file (combine with |).
            enum OpenFlags {
		    READ              = 0x0001,   ///< Open for reading ("r"); the default when no other flag is given.
		    WRITE             = 0x0002,   ///< Open for writing: with READ, "r+" (the file must exist); alone, "w" (created or emptied).
            APPEND            = 0x0004,   ///< Append to the end ("a", or "a+" with READ).
            BINARY            = 0x0008    ///< Binary mode ("b"): no newline translation.
		    };

            /// Where a seek counts from.
            enum SeekFlags {
		    BEGIN             = 0x0001,   ///< From the start of the file.
		    END               = 0x0002,   ///< From the end of the file.
            CURRENT           = 0x0003    ///< From the current position.
		    };
        protected:
            CString         m_strFileName;      ///< The file's name, as given to Open().
            FILE           *m_fpFile;           ///< The open stream, or NULL.
            bool            m_bInput;           ///< Open for reading.
            bool            m_bOutput;          ///< Open for writing.
            bool            m_bAppend;          ///< Open for appending.
            bool            m_bBinary;          ///< Open in binary mode.
            int64_t         m_lFileSize;        ///< The size in bytes; -1 until known.
            int64_t         m_lCurrentPos;      ///< The position; -1 until known.
        public:
                /// Creates a closed file.
	                        CFile();
                /// Opens a file; see Open().
                /// @param strFileName  The file's path.
                /// @param nMode  A combination of OpenFlags.
                /// @throws CFileException if the file cannot be opened.
                            CFile( const char *strFileName, int nMode );
#ifdef IASLIB_WIN32__
                /// Wraps a C runtime file descriptor (from _open).
                /// @param nHandle  The descriptor.
                /// @param nMode  A combination of OpenFlags matching how it was opened.
                            CFile( int nHandle, int nMode );
#endif
                /// Closes the file.
	        virtual         ~CFile();

                            DEFINE_OBJECT( CFile )

                /// Whether the position is at the end of the file.
            virtual bool    IsEOF( void );
                /// Opens a file, closing the one open before.
                /// @param strFileName  The file's path.
                /// @param nMode  A combination of OpenFlags; 0 means READ.
                /// @return true.
                /// @throws CFileException (with errno) if the file cannot be opened.
            virtual bool    Open( const char *strFileName, int nMode );
                /// Closes the file.
                /// @return false if it was not open.
            virtual bool    Close( void );
                /// Flushes buffered output to the file.
                /// @return false if the file is not open for writing.
            virtual bool    Flush( void );
                /// Reads up to nBufferSize bytes.
                /// @param pchBuffer  Where to put them.
                /// @param nBufferSize  The most to read.
                /// @return The bytes read (0 at the end), or -1 if the file is
                ///         not open for reading.
            virtual int     Read( char *pchBuffer, int nBufferSize );
                /// Writes nBufferSize bytes.
                /// @param pchBuffer  The bytes.
                /// @param nBufferSize  How many.
                /// @return The bytes written, or -1 if the file is not open for writing.
            virtual int     Write( const char *pchBuffer, int nBufferSize );
                /// Writes a string (without a terminating NUL or newline).
                /// @param oString  The string.
                /// @return As Write().
            virtual int     WriteString( const CString &oString );
                /// The position, as a long.
                ///
                /// Positions and sizes as a long: -1 when the value does not
                /// fit (a file past 2 GB where long is 32 bits, as on
                /// Windows). The Big methods below take any size.
                /// @return The position, or -1 if it is unknown or does not fit.
            virtual long    GetPos( void );
                /// The size, as a long (-1 if it is unknown or does not fit; see GetPos()).
            virtual long    GetSize( void );
                /// Moves the position, as a long (see GetPos()).
                /// @param nPos  The offset.
                /// @param nStart  A SeekFlags value.
                /// @return The new position, or -1 if the seek failed or it does not fit.
            virtual long    Seek( long nPos, int nStart );

                /// The position, for files of any size (up to 2^63 bytes).
                /// @return The position, or -1 if the file is not open.
            virtual int64_t GetPosBig( void );
                /// The size in bytes, for files of any size.
                /// @return The size, or -1 if it cannot be found.
            virtual int64_t GetSizeBig( void );
                /// Moves the position, for files of any size.
                /// @param nPos  The offset.
                /// @param nStart  A SeekFlags value.
                /// @return The new position, or -1 if the seek failed.
                /// @note Seek() used to go through fseek() and ftell() with a
                ///       long, so nothing past 2 GB could be reached where long
                ///       is 32 bits; and a failed seek returned the old position
                ///       as if it had worked.
            virtual int64_t SeekBig( int64_t nPos, int nStart );
                /// Reads any number of bytes (in pieces where the C library
                /// needs it).
                /// @param pBuffer  Where to put them.
                /// @param nLength  How many to read.
                /// @return The bytes read, or -1 if the file is not open for reading.
            virtual int64_t ReadBig( void *pBuffer, size_t nLength );
                /// Writes any number of bytes (in pieces where the C library
                /// needs it).
                /// @param pBuffer  The bytes.
                /// @param nLength  How many.
                /// @return The bytes written, or -1 if the file is not open for writing.
            virtual int64_t WriteBig( const void *pBuffer, size_t nLength );
                /// Whether a file is open.
            virtual bool    IsOpen( void ) const { return (m_fpFile != NULL); }
                /// Reads one line of text, of any length.
                /// @return The line, with its newline if it had one; "" at the
                ///         end, or if the file is closed or open in binary mode.
                /// @note Uses a static buffer, so two threads must not call it
                ///       at once, even on different files.
            virtual CString GetLine( void );

                /// Deletes a file.
                /// @param strFileName  The file's path.
                /// @return true.
                /// @throws CFileException (with errno) if it cannot be deleted.
            static bool     Delete( const char *strFileName );
                /// Renames (moves) a file.
                /// @param strOldName  The current path.
                /// @param strNewName  The new path.
                /// @return true.
                /// @throws CFileException (with errno) if it cannot be renamed.
            static bool     Rename( const char *strOldName, const char *strNewName );
    };
} // namespace IASLib

#endif // IASLIB_FILE_H__

