/**
 * @file
 * @brief Memory Mapped File Class.
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
    /// A file read and written through the operating system's memory
    /// mapping, with CFile's interface.
    ///
    /// Read(), Write(), Seek() and the rest work on the mapped memory, and
    /// GetData() gives that memory itself. By default there is one position,
    /// as in CFile; with SetSyncPointers( false ) the read and write pointers
    /// move separately. Writing past the end grows the file; Close() trims it
    /// back to the length written. A write that grows the file maps it again,
    /// so a pointer from GetData() is good only until the next write,
    /// Resize() or Close(). The whole file is mapped. Not thread-safe; not
    /// copyable.
    ///
    /// @code
    /// CMemMappedFile oFile( "data.bin", CFile::CREATE );
    /// oFile.Write( "hello", 5 );
    /// const char *pBytes = oFile.GetData();   // "hello", until the next write
    /// oFile.Close();                          // the file is 5 bytes long
    /// @endcode
    class CMemMappedFile : public CFile
    {
        protected:
#ifdef IASLIB_WIN32__
            HANDLE          m_hFileHandle;      ///< The open file.
            HANDLE          m_hMemoryMap;       ///< The file mapping object, or NULL.
#else
            int             m_hFileHandle;      ///< The open file descriptor, or -1.
#endif
            char           *m_pView;            ///< the mapped memory (NULL if nothing mapped)
            int64_t         m_nMapped;          ///< bytes mapped (and on disk while open)
            int64_t         m_nSize;            ///< the file's length
            int64_t         m_nReadPointer;     ///< The read pointer (the one position when synced).
            int64_t         m_nWritePointer;    ///< The write pointer, when not synced.
            bool            m_bSyncPointers;    ///< Whether reads and writes share one position.
            bool            m_bIsOpen;          ///< Whether a file is open.
            bool            m_bWritable;        ///< Whether the file was opened for writing.

        public:
                            /// Makes a CMemMappedFile with no file open.
	                        CMemMappedFile();
#ifdef IASLIB_WIN32__
                                /// Takes over an open file handle; nMode says
                                /// whether it may be written (WRITE).
                                /// @throws CFileException if the handle is not valid
                                /// or the file cannot be mapped.
                            CMemMappedFile( HANDLE nFileHandle, int nMode );
#endif
                                /// Opens as Open() does (and throws as CFile's
                                /// constructor does).
                            CMemMappedFile( const char *strFileName, int nMode );
                            /// Destructor. Closes the file (see Close()).
	        virtual        ~CMemMappedFile();

                            DECLARE_OBJECT( CMemMappedFile, CFile )

                /// Opens and maps a file, closing one already open.
                /// The modes are CFile's: READ (an existing file, read only),
                /// READ | WRITE (an existing file), WRITE or CREATE (a new or
                /// emptied file), APPEND (writes at the end; created if need
                /// be). BINARY is implied.
                /// @return True (it does not return otherwise).
                /// @throws CFileException if the file cannot be opened or mapped.
            virtual bool    Open( const char *strFileName, int nMode );
                /// Unmaps, trims the file to its length and closes it.
                /// @return False if no file was open, or a step failed.
            virtual bool    Close( void );
                /// Writes the mapped pages out to the file.
                /// @return False if the file is not open for writing, or it failed.
            virtual bool    Flush( void );
                /// Reads up to @p nBufferSize bytes at the read pointer.
                /// @return Bytes read (0 at the end), or -1 if no file is open
                /// or @p nBufferSize is negative.
            virtual int     Read( char *pchBuffer, int nBufferSize );
                /// Writes @p nBufferSize bytes at the write pointer (at the end,
                /// for APPEND), growing the file as needed.
                /// @return @p nBufferSize, or -1 if the file is not open for
                /// writing, @p nBufferSize is negative or the file cannot grow.
            virtual int     Write( const char *pchBuffer, int nBufferSize );
                /// Writes the string's text. Returns as Write() does.
            virtual int     WriteString( const CString &oString );
            virtual int64_t ReadBig( void *pBuffer, size_t nLength );           ///< Same as Read(), for any length.
            virtual int64_t WriteBig( const void *pBuffer, size_t nLength );    ///< Same as Write(), for any length.

                /// The read position (the position, when synced), as a long
                /// (-1 if no file is open, or it does not fit).
            virtual long    GetPos( void );
            virtual long    GetSize( void );                    ///< The length, as a long (see GetPos()).
            virtual long    Seek( long nPos, int nStart );      ///< Same as SeekBig(), as a long.
            virtual int64_t GetPosBig( void );                  ///< The read position, or -1 if no file is open.
            virtual int64_t GetSizeBig( void );                 ///< The length, or -1 if no file is open.
                /// A seek that moves both pointers (CURRENT is from the read
                /// pointer). Seeking past the end does not grow the file.
                /// @return The new position, or -1 (for a position before the
                /// start, a bad @p nStart, or no open file).
            virtual int64_t SeekBig( int64_t nPos, int nStart );

                /// Moves the read pointer only (CURRENT is from that pointer).
                /// Returns as SeekBig() does, as a long.
            virtual long    SeekRead( long nPos, int nStart );
                /// Moves the write pointer only (the one position, when synced).
            virtual long    SeekWrite( long nPos, int nStart );
            int64_t         SeekReadBig( int64_t nPos, int nStart );    ///< SeekRead(), with 64-bit offsets.
            int64_t         SeekWriteBig( int64_t nPos, int nStart );   ///< SeekWrite(), with 64-bit offsets.
            int64_t         GetReadPos( void ) const { return m_nReadPointer; }     ///< The read pointer.
            int64_t         GetWritePos( void ) const { return m_bSyncPointers ? m_nReadPointer : m_nWritePointer; }   ///< The write pointer.
                /// Whether reads and writes share one position. Changing it puts
                /// the write pointer where the read pointer is.
            void            SetSyncPointers( bool bSync );
            bool            GetSyncPointers( void ) const { return m_bSyncPointers; }  ///< Whether the pointers are synced.

            virtual bool    IsOpen( void ) const { return m_bIsOpen; }      ///< Whether a file is open.
            bool            IsWritable( void ) const { return m_bWritable; }    ///< Whether the file may be written.
                /// Whether the read pointer is at the end (true if no file is open).
            virtual bool    IsEOF( void );
                /// The next line, with its line ending ("" at the end).
            virtual CString GetLine( void );

                /// The mapped bytes (NULL when the file is empty). See the
                /// class description for how long the pointer stays good.
            const char     *GetData( void ) const { return m_pView; }
                /// Same, to write to; NULL if the file is not writable.
            char           *GetWritableData( void ) { return m_bWritable ? m_pView : NULL; }
                /// Makes the file nBytes long (zeros when longer). The
                /// pointers are left where they are.
                /// @return False if the file is not open for writing, @p nBytes
                /// is negative or the file cannot grow.
            bool            Resize( int64_t nBytes );

            static bool     Delete( const char *strFileName ) { return CFile::Delete( strFileName ); }     ///< Same as CFile::Delete().
            static bool     Rename( const char *strOldName, const char *strNewName ) { return CFile::Rename( strOldName, strNewName ); }   ///< Same as CFile::Rename().

        private:
            void            Init( void );                   ///< Resets every member to "no file open".
                /// Maps the first nBytes of the file (growing the file to that
                /// on disk if need be). Leaves nothing mapped for 0 bytes.
            bool            Map( int64_t nBytes );
            void            Unmap( void );                  ///< Unmaps the file.
                /// Room for nNeeded bytes: half again (up to 64 MB more), or
                /// what is needed if that is more.
            bool            Grow( int64_t nNeeded );
                /// Moves @p nPointer; the work behind the Seek methods.
            int64_t         Seek( int64_t &nPointer, int64_t nPos, int nStart );

                            CMemMappedFile( const CMemMappedFile & );       ///< Not copyable.
            CMemMappedFile &operator =( const CMemMappedFile & );       ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_MEM_MAPPED_FILE_H__
