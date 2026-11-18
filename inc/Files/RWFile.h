/**
 * @file RWFile.h
 * @brief CRWFile, a line file with an index file so a reader can follow a writer.
 *
 * The original header comment follows.
 */
/*
 *  Read/Write File Class
 *
 *      This class, inherited from CFile, provides for a file with an
 * associated index that stores where the file is being written or read
 * from. This allows two different processes or threads to read from a
 * file and coordinate their actions.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/21/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RWFILE_H__
#define IASLIB_RWFILE_H__

#include "../BaseTypes/String_.h"
#include "File.h"

namespace IASLib
{
    /// A file of lines with an index file beside it, so that one process
    /// or thread can read the lines another is still writing.
    ///
    /// The data goes to the named file; the index goes to the same name
    /// with ".idx" added. The index holds two numbers: how many lines the
    /// writer has written (-1 once the writer closed it as complete), and
    /// how many lines the reader has read. Opening again carries on from
    /// where the last writer or reader stopped. One writer and one reader
    /// at a time.
    ///
    /// @code
    /// CRWFile oReader( "queue.log", CRWFile::READ );
    /// while ( ! oReader.IsWriterDone() || ! oReader.IsEOF() )
    ///     if ( ! oReader.IsEOF() )
    ///         CString strLine = oReader.GetLine();
    /// @endcode
    class CRWFile : public CObject
    {
        protected:
            CFile           m_oIndexFile;       ///< The ".idx" file.
            CFile           m_oDataFile;        ///< The data file.
            long            m_nReadPos;         ///< The reader's byte position in the data file.
            long            m_nReadLine;        ///< How many lines the reader has read.
            long            m_nWritePos;        ///< The writer's byte position in the data file.
            long            m_nWriteLine;       ///< How many lines the writer has written (-1 when complete).
            CString         m_strBaseFileName;  ///< The data file's name.
            bool            m_bRead;            ///< Open as the reader (otherwise the writer).
            bool            m_bFileOpen;        ///< Whether it is open.
        public:
            /// How to open the file.
            enum OpenFlags {
		    READ              = 0x0001,   ///< As the reader.
		    WRITE             = 0x0002    ///< As the writer.
		    };

                /// Creates a closed file.
                            CRWFile();
                /// Opens a file; see Open().
                /// @param strFileName  The data file's path.
                /// @param nMode  READ or WRITE.
                            CRWFile( const char *strFileName, int nMode );
                /// Closes the file (as complete, for a writer).
	        virtual        ~CRWFile();

                            DEFINE_OBJECT( CRWFile )

                /// Opens the data and index files, as the writer (WRITE) or
                /// the reader (any other value).
                ///
                /// The writer goes past the lines already written; the reader
                /// skips the lines already read. A reader needs an index file
                /// that is not empty.
                /// @param strFileName  The data file's path.
                /// @param nMode  READ or WRITE.
                /// @return Whether the file is now open.
                /// @throws CFileException from CFile::Open() if a file cannot be opened.
            bool            Open( const char *strFileName, int nMode );
                /// Closes the files, saving the index.
                /// @param bComplete  For a writer: mark the file complete, so
                ///                   that IsWriterDone() is true for the reader.
                /// @return false if it was not open.
            bool            Close( bool bComplete = true );
                /// Flushes the data and index files.
                /// @return false if it is not open.
            bool            Flush( void );

                /// Writes a line (a newline is added) and updates the index.
                /// @param oString  The line.
                /// @return false if not open as the writer.
            bool            WriteLine( const CString &oString );
                /// Writes bytes and updates the index, counting the newlines in them.
                /// @param pchBuffer  The bytes.
                /// @param nLength  How many.
                /// @return false if not open as the writer, or not all were written.
		    bool			Write( const char *pchBuffer, int nLength );
                /// Reads the next line and updates the index.
                /// @return The line, with its newline; "" at the end or if not
                ///         open as the reader.
            CString         GetLine( void );

                /// The line number: lines read (reader) or written (writer).
                /// @return The count, or -1 if not open.
            long            GetPos( void );

                /// Whether it is open.
            bool            IsOpen( void ) { return m_bFileOpen; }
                /// For the reader: whether the writer has closed the file as complete.
                /// @return false for a writer, or if not open.
            bool            IsWriterDone( void );
                /// For the reader: whether it has read everything written so far.
                /// @return false for a writer, or if not open.
            bool            IsEOF( void );

        private:
                /// Writes this side's line count into the index file.
            void            UpdateIndex( void );
    };
} // namespace IASLib

#endif // IASLIB_RWFILE_H__

