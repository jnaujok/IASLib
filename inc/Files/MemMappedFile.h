/*
 * Memory Mapped File Class
 *
 *      A file read and written through the operating system's memory
 * mapping (mmap() on Unix, file mapping objects on Windows), with CFile's
 * interface: Read(), Write(), Seek() and the rest work on the mapped
 * memory, and GetData() gives that memory itself for direct access.
 *
 *      Read and write pointers: by default there is one position, as in
 * CFile. With SetSyncPointers( false ) the read and write pointers move
 * separately (SeekRead(), SeekWrite()); Seek() moves both.
 *
 *      Writing past the end grows the file (in steps: half its size, at
 * most 64 MB, at least what the write needs); Close() trims it back to the
 * length written. A write that grows the file maps it again, so a pointer
 * from GetData() is good only until the next write, Resize() or Close().
 * The whole file is mapped: on a 32-bit system that limits it to what fits
 * in the address space. Offsets are 64-bit. Not thread-safe.
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
#include <stdint.h>
#include "../BaseTypes/String_.h"
#include "File.h"
#ifdef IASLIB_WIN32__
    #include "../BaseTypes/Platform.h"
#endif

namespace IASLib
{
    class CMemMappedFile : public CFile
    {
        protected:
#ifdef IASLIB_WIN32__
            HANDLE          m_hFileHandle;
            HANDLE          m_hMemoryMap;
#else
            int             m_hFileHandle;
#endif
            char           *m_pView;            // the mapped memory (NULL if nothing mapped)
            int64_t         m_nMapped;          // bytes mapped (and on disk while open)
            int64_t         m_nSize;            // the file's length
            int64_t         m_nReadPointer;     // (the one position when synced)
            int64_t         m_nWritePointer;
            bool            m_bSyncPointers;
            bool            m_bIsOpen;
            bool            m_bWritable;

        public:
	                        CMemMappedFile();
#ifdef IASLIB_WIN32__
                                // Takes over an open file handle; nMode says
                                // whether it may be written (WRITE).
                            CMemMappedFile( HANDLE nFileHandle, int nMode );
#endif
                                // Opens as Open() does (and throws as CFile's
                                // constructor does).
                            CMemMappedFile( const char *strFileName, int nMode );
	        virtual        ~CMemMappedFile();

                            DECLARE_OBJECT( CMemMappedFile, CFile )

                // The modes are CFile's: READ (an existing file, read only),
                // READ | WRITE (an existing file), WRITE or CREATE (a new or
                // emptied file), APPEND (writes at the end; created if need
                // be). BINARY is implied. Throws a CFileException if the file
                // cannot be opened or mapped.
            virtual bool    Open( const char *strFileName, int nMode );
                // Unmaps, trims the file to its length and closes it.
            virtual bool    Close( void );
                // Writes the mapped pages out to the file.
            virtual bool    Flush( void );
            virtual int     Read( char *pchBuffer, int nBufferSize );
            virtual int     Write( const char *pchBuffer, int nBufferSize );
            virtual int     WriteString( const CString &oString );
            virtual int64_t ReadBig( void *pBuffer, size_t nLength );
            virtual int64_t WriteBig( const void *pBuffer, size_t nLength );

                // The read position (the position, when synced), the length,
                // and a seek that moves both pointers.
            virtual long    GetPos( void );
            virtual long    GetSize( void );
            virtual long    Seek( long nPos, int nStart );
            virtual int64_t GetPosBig( void );
            virtual int64_t GetSizeBig( void );
            virtual int64_t SeekBig( int64_t nPos, int nStart );

                // One pointer or the other (CURRENT is from that pointer).
            virtual long    SeekRead( long nPos, int nStart );
            virtual long    SeekWrite( long nPos, int nStart );
            int64_t         SeekReadBig( int64_t nPos, int nStart );
            int64_t         SeekWriteBig( int64_t nPos, int nStart );
            int64_t         GetReadPos( void ) const { return m_nReadPointer; }
            int64_t         GetWritePos( void ) const { return m_bSyncPointers ? m_nReadPointer : m_nWritePointer; }
            void            SetSyncPointers( bool bSync );
            bool            GetSyncPointers( void ) const { return m_bSyncPointers; }

            virtual bool    IsOpen( void ) const { return m_bIsOpen; }
            bool            IsWritable( void ) const { return m_bWritable; }
                // Whether the read pointer is at the end.
            virtual bool    IsEOF( void );
                // The next line, with its line ending ("" at the end).
            virtual CString GetLine( void );

                // The mapped bytes (NULL when the file is empty). See above
                // for how long the pointer stays good.
            const char     *GetData( void ) const { return m_pView; }
            char           *GetWritableData( void ) { return m_bWritable ? m_pView : NULL; }
                // Makes the file nBytes long (zeros when longer). The
                // pointers are left where they are.
            bool            Resize( int64_t nBytes );

            static bool     Delete( const char *strFileName ) { return CFile::Delete( strFileName ); }
            static bool     Rename( const char *strOldName, const char *strNewName ) { return CFile::Rename( strOldName, strNewName ); }

        private:
            void            Init( void );
            bool            Map( int64_t nBytes );
            void            Unmap( void );
            bool            Grow( int64_t nNeeded );
            int64_t         Seek( int64_t &nPointer, int64_t nPos, int nStart );

                            CMemMappedFile( const CMemMappedFile & );
            CMemMappedFile &operator =( const CMemMappedFile & );
    };
} // namespace IASLib

#endif // IASLIB_MEM_MAPPED_FILE_H__
