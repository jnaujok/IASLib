/**
 * @file MemMappedFile.h
 * @brief CMemMappedFile, a memory-mapped file (not implemented yet).
 *
 * The original header comment follows.
 */
/*
 * Memory Mapped File Class
 *
 *      This class provides an object to wrap the various memory mapped
 * file functions. Memory mapped files provide a quick, random access
 * means of using files.
 *      Since memory mapped files use operating system calls that are 
 * normally used to page memory, the access is usually very fast and 
 * optimized.
 *      We also allow the choice of maintaining a separate read and
 * write pointer in the same file. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/29/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEM_MAPPED_FILE_H__
#define IASLIB_MEM_MAPPED_FILE_H__

#include <stdio.h>
#include "../BaseTypes/String_.h"
#include "File.h"

namespace IASLib
{
    /// A memory-mapped file, with optional separate read and write positions.
    ///
    /// Meant to give fast, random access to a file through the operating
    /// system's paging.
    ///
    /// @warning Not implemented yet: every method is a stub. Open() and
    ///          the other bool methods return false, Read() and Write()
    ///          return 0, positions are 0, IsEOF() is true and GetLine()
    ///          returns "". Delete() and Rename() are declared but not
    ///          defined (calling them fails to link).
    class CMemMappedFile : public CFile
    {
        protected:
#ifdef IASLIB_WIN32__
            HANDLE          m_hFileHandle;      ///< The file's handle.
            HANDLE          m_hMemoryMap;       ///< The mapping's handle.
#else
            int             m_hFileHandle;      ///< The file's descriptor.
            int             m_hMemoryMap;       ///< The mapping (unused).
#endif
            long            m_nReadPointer;     ///< The read position.
            long            m_nWritePointer;    ///< The write position.
            bool            m_bSyncPointers;    ///< Keep the read and write positions together.
            bool            m_bIsOpen;          ///< Whether the file is open.

        public:
                /// Creates a closed file.
	                        CMemMappedFile();
#ifdef IASLIB_WIN32__
                /// Maps a file that is already open.
                /// @param nFileHandle  The file's handle.
                /// @param nMode  A combination of CFile::OpenFlags.
                            CMemMappedFile( HANDLE nFileHandle, int nMode );
#endif

                /// Meant to open and map a file; does nothing yet.
                /// @param strFileName  The file's path.
                /// @param nMode  A combination of CFile::OpenFlags.
                            CMemMappedFile( const char *strFileName, int nMode );
                /// Destroys the object.
	        virtual        ~CMemMappedFile();

                            DECLARE_OBJECT( CMemMappedFile, CFile )

                /// Meant to open and map a file. Stub: returns false.
            virtual bool    Open( const char *strFileName, int nMode );
                /// Stub: returns false.
            virtual bool    Close( void );
                /// Stub: returns false.
            virtual bool    Flush( void );
                /// Stub: returns 0.
            virtual int     Read( char *pchBuffer, int nBufferSize );
                /// Stub: returns 0.
            virtual int     Write( const char *pchBuffer, int nBufferSize );
                /// Stub: returns 0.
            virtual int     WriteString( const CString &oString );
                /// Stub: returns 0.
            virtual long    GetPos( void );
                /// Stub: returns 0.
            virtual long    GetSize( void );
                /// Meant to move both positions. Stub: returns 0.
            virtual long    Seek( long nPos, int nStart );
                /// Meant to move the read position. Stub: returns 0.
            virtual long    SeekRead( long nPos, int nStart );
                /// Meant to move the write position. Stub: returns 0.
            virtual long    SeekWrite( long nPos, int nStart );
                /// The position, through this class's own GetPos() (not CFile's stream).
            virtual int64_t GetPosBig( void ) { return GetPos(); }
                /// The size, through this class's own GetSize().
            virtual int64_t GetSizeBig( void ) { return GetSize(); }
                /// Seeks through this class's own Seek().
            virtual int64_t SeekBig( int64_t nPos, int nStart ) { return Seek( (long)nPos, nStart ); }
                /// Whether the file is open.
            virtual bool    IsOpen( void ) const { return m_bIsOpen; }
                /// Stub: returns true.
            virtual bool    IsEOF( void );
                /// Stub: returns "".
            virtual CString GetLine( void );

                /// Declared but not defined.
            static bool     Delete( const char *strFileName );
                /// Declared but not defined.
            static bool     Rename( const char *strOldName, const char *strNewName );
    };
} // namespace IASLib

#endif // IASLIB_MEM_MAPPED_FILE_H__

