/**
 * @file
 * @brief Read/Write File Class.
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

#include <stdint.h>
#include "../BaseTypes/String_.h"
#include "File.h"

namespace IASLib
{
        /// A line file shared by one writer and one reader, which may be
        /// different processes. \<name\> holds the lines; \<name\>.idx holds two
        /// 16-byte records: the writer's line count (-1 once it has finished)
        /// and the reader's. A reader picks up where the last one stopped,
        /// and a writer where the last one stopped (or, if it finished, at
        /// the end). A reader only takes complete lines: a line still being
        /// written is left until its newline arrives.
        ///
        /// @code
        /// CRWFile oWriter( "queue", CRWFile::WRITE );
        /// oWriter.WriteLine( "job 1" );
        ///
        /// CRWFile oReader( "queue", CRWFile::READ );
        /// CString strLine = oReader.GetLine();    // "job 1\n"
        /// oWriter.Close();                        // marks the file finished
        /// bool bDone = oReader.IsWriterDone();    // true
        /// @endcode
    class CRWFile : public CObject
    {
        protected:
            CFile           m_oIndexFile;       ///< \<name\>.idx.
            CFile           m_oDataFile;        ///< \<name\>, the lines.
            int64_t         m_nReadPos;         ///< Byte offset of the next line to read.
            long            m_nReadLine;        ///< Lines read.
            int64_t         m_nWritePos;        ///< Byte offset to write at.
            long            m_nWriteLine;       ///< Lines written (-1 once finished).
            CString         m_strBaseFileName;  ///< \<name\>.
            bool            m_bRead;            ///< Opened as the reader.
            bool            m_bFileOpen;        ///< Whether the files are open.
        public:
                /// How Open() opens the file.
            enum OpenFlags {
		    READ              = 0x0001,     ///< As the reader.
		    WRITE             = 0x0002      ///< As the writer.
		    };

                            /// Makes a CRWFile with nothing open.
                            CRWFile();
                            /// Opens a file (see Open()); check IsOpen() afterwards.
                            CRWFile( const char *strFileName, int nMode );
                            /// Destructor. Closes the file (as complete, for a writer).
	        virtual        ~CRWFile();

                            DEFINE_OBJECT( CRWFile )

                /// Opens \<strFileName\> and its index, closing what was open.
                /// WRITE creates the files if need be; READ needs the index to
                /// exist (a writer has started). Any other mode opens as the
                /// reader. False if they cannot be opened (it does not throw).
                /// @note Reopening a finished file as the writer used to start
                /// again at line 0, over the old lines.
            bool            Open( const char *strFileName, int nMode );
                /// Closes the files. A writer closing with bComplete marks the
                /// file finished. False if nothing was open.
            bool            Close( bool bComplete = true );
                /// Writes out buffered data. False if nothing is open or it failed.
            bool            Flush( void );

                /// Writes a line (a newline is added). False if not open as the
                /// writer, or the write failed.
                /// @note A line with newlines in it used to count as one.
            bool            WriteLine( const CString &oString );
                /// Writes bytes (their newlines count as lines). Returns as
                /// WriteLine() does.
		    bool			Write( const char *pchBuffer, int nLength );
                /// The next complete line, with its newline ("" if there is
                /// none yet, or this is not the reader).
            CString         GetLine( void );

                /// The line number reached (read or written); -1 if not open.
            long            GetPos( void );

            bool            IsOpen( void ) { return m_bFileOpen; }     ///< Whether the files are open.
                /// Whether the writer has finished (readers only).
            bool            IsWriterDone( void );
                /// Whether the reader has read everything there is so far
                /// (false for a writer).
            bool            IsEOF( void );

        private:
            void            UpdateIndex( void );        ///< Writes this side's line count to the index.
                /// Reads both line counts from the index. False if it cannot.
            bool            ReadIndex( long &nWriteLine, long &nReadLine );
                /// Opens @p strName for reading and writing (binary), creating
                /// it if @p bCreate. False if it cannot.
            static bool     OpenFile( CFile &oFile, const CString &strName, bool bCreate );

                            CRWFile( const CRWFile & );         ///< Not copyable.
            CRWFile        &operator =( const CRWFile & );     ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_RWFILE_H__
