/**
 * @file
 * @brief File Class.
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
    /// A file on disk, read and written through the C library's FILE.
    ///
    /// The file is closed when the object is destroyed. CFile cannot be
    /// copied. Open() and the constructor that opens throw a CFileException
    /// when the file cannot be opened. One CFile must not be used from two
    /// threads at once.
    ///
    /// @code
    /// CFile oFile( "notes.txt", CFile::WRITE );
    /// oFile.WriteString( "first line\n" );
    /// oFile.Close();
    ///
    /// oFile.Open( "notes.txt", CFile::READ );
    /// CString strLine = oFile.GetLine();      // "first line\n"
    /// @endcode
    class CFile : public CObject
    {
        public:
                /// How Open() opens a file; combine with |.
                /// READ: an existing file. WRITE: a new or emptied file.
                /// READ | WRITE: an existing file, both ways. CREATE (with
                /// WRITE, or READ | WRITE): a new or emptied file, both ways.
                /// APPEND: writes go to the end (with READ, reads too).
                /// BINARY: no line-ending translation (Windows).
                /// With none of READ, WRITE, APPEND and CREATE, READ is used.
            enum OpenFlags {
		    READ              = 0x0001,     ///< Open for reading.
		    WRITE             = 0x0002,     ///< Open for writing.
            APPEND            = 0x0004,     ///< Writes go to the end.
            BINARY            = 0x0008,     ///< No line-ending translation.
            CREATE            = 0x0010      ///< Make a new or emptied file (implies WRITE).
		    };

                /// Where a Seek() offset counts from.
            enum SeekFlags {
		    BEGIN             = 0x0001,     ///< From the start of the file.
		    END               = 0x0002,     ///< From the end of the file.
            CURRENT           = 0x0003      ///< From the current position.
		    };
        protected:
            CString         m_strFileName;      ///< The name the file was opened with.
            FILE           *m_fpFile;           ///< The open file, or NULL.
            bool            m_bInput;           ///< Open for reading.
            bool            m_bOutput;          ///< Open for writing.
            bool            m_bAppend;          ///< Open for appending.
            bool            m_bBinary;          ///< Opened in binary mode.
            int64_t         m_lFileSize;        ///< The size; -1 until known
            int64_t         m_lCurrentPos;      ///< The position; -1 until known
        public:
                            /// Makes a CFile with no file open.
	                        CFile();
                            /// Opens a file (see Open()).
                            /// @throws CFileException if it cannot be opened.
                            CFile( const char *strFileName, int nMode );
#ifdef IASLIB_WIN32__
                                /// A C runtime file descriptor (from _open).
                            CFile( int nHandle, int nMode );
#endif
                            /// Destructor. Closes the file.
	        virtual         ~CFile();

                            DEFINE_OBJECT( CFile )

                /// True at the end of the file, or if no file is open.
            virtual bool    IsEOF( void );
                /// Opens a file, closing one already open.
                /// @param strFileName The file's path.
                /// @param nMode OpenFlags, combined with |.
                /// @return True (it does not return otherwise).
                /// @throws CFileException (with errno) if it cannot be opened.
                /// @note The flags used to be kept from the last Open(), and with
                /// no READ or WRITE flag BINARY was dropped.
            virtual bool    Open( const char *strFileName, int nMode );
                /// Closes the file.
                /// @return False if none was open, or the final write failed.
                /// @note fclose()'s result used to be ignored.
            virtual bool    Close( void );
                /// Writes out buffered data.
                /// @return False if the file is not open for writing, or it failed.
            virtual bool    Flush( void );
                /// Reads up to @p nBufferSize bytes.
                /// @return Bytes read (0 at the end), or -1 if the file is not open for reading.
            virtual int     Read( char *pchBuffer, int nBufferSize );
                /// Writes @p nBufferSize bytes.
                /// @return Bytes written, or -1 if the file is not open for writing.
            virtual int     Write( const char *pchBuffer, int nBufferSize );
                /// Writes the string's text. Returns as Write() does.
            virtual int     WriteString( const CString &oString );
                /// Positions and sizes as a long: -1 when the value does not
                /// fit (a file past 2 GB where long is 32 bits, as on
                /// Windows), or no file is open. The Big methods below take
                /// any size.
            virtual long    GetPos( void );
            virtual long    GetSize( void );                        ///< The file's size; see GetPos().
            virtual long    Seek( long nPos, int nStart );          ///< Moves to @p nPos from @p nStart (SeekFlags); see GetPos().

                /// 64-bit positions and sizes, for files of any size (up to
                /// 2^63 bytes); -1 if no file is open.
            virtual int64_t GetPosBig( void );
            virtual int64_t GetSizeBig( void );                     ///< The file's size, or -1.
                /// Moves to @p nPos from @p nStart (SeekFlags). Seeking past
                /// the end does not make the file longer.
                /// @return The new position, or -1 if the seek failed.
                /// @note Seek() used to use a long, so nothing past 2 GB could be
                /// reached where long is 32 bits, and a failed seek returned the
                /// old position as if it had worked.
            virtual int64_t SeekBig( int64_t nPos, int nStart );
                /// Reads or writes any number of bytes (in pieces where the C
                /// library needs it). Returns the bytes moved, or -1 if the
                /// file is not open for it.
            virtual int64_t ReadBig( void *pBuffer, size_t nLength );
            virtual int64_t WriteBig( const void *pBuffer, size_t nLength );    ///< Same, writing.
            virtual bool    IsOpen( void ) const { return (m_fpFile != NULL); } ///< Whether a file is open.
                /// The next line, with its line ending ("" at the end of the
                /// file, or if the file is not open for reading). Any length;
                /// works in binary mode too.
                /// @note It used to use one static buffer for every file and
                /// thread, and refused binary files.
            virtual CString GetLine( void );
            const CString  &GetFileName( void ) const { return m_strFileName; }    ///< The file's name.

                /// Deletes a file. Returns true.
                /// @throws CFileException (with errno) if it fails.
            static bool     Delete( const char *strFileName );
                /// Renames a file, replacing a file already called strNewName,
                /// on every platform. Returns true.
                /// @throws CFileException if it fails.
            static bool     Rename( const char *strOldName, const char *strNewName );
                /// Whether a file (or directory) of that name exists.
            static bool     Exists( const char *strFileName );

        protected:
                /// The fopen() mode for OpenFlags, setting the m_b* flags.
            CString         ModeString( int nMode );

        private:
                /// Not copyable. (A copy would share the FILE and close it twice.)
                            CFile( const CFile & );
            CFile          &operator =( const CFile & );    ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_FILE_H__

