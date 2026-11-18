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

#include <stdint.h>
#include "../BaseTypes/String_.h"
#include "File.h"

namespace IASLib
{
        // A line file shared by one writer and one reader, which may be
        // different processes. <name> holds the lines; <name>.idx holds two
        // 16-byte records: the writer's line count (-1 once it has finished)
        // and the reader's. A reader picks up where the last one stopped,
        // and a writer where the last one stopped (or, if it finished, at
        // the end). A reader only takes complete lines: a line still being
        // written is left until its newline arrives.
    class CRWFile : public CObject
    {
        protected:
            CFile           m_oIndexFile;
            CFile           m_oDataFile;
            int64_t         m_nReadPos;
            long            m_nReadLine;
            int64_t         m_nWritePos;
            long            m_nWriteLine;
            CString         m_strBaseFileName;
            bool            m_bRead;
            bool            m_bFileOpen;
        public:
            enum OpenFlags {
		    READ              = 0x0001,
		    WRITE             = 0x0002
		    };

                            CRWFile();
                            CRWFile( const char *strFileName, int nMode );
	        virtual        ~CRWFile();

                            DEFINE_OBJECT( CRWFile )

                // WRITE creates the files if need be; READ needs the index to
                // exist (a writer has started). False if they cannot be
                // opened.
            bool            Open( const char *strFileName, int nMode );
                // A writer closing with bComplete marks the file finished.
            bool            Close( bool bComplete = true );
            bool            Flush( void );

                // A line (a newline is added), or bytes (their newlines
                // count as lines).
            bool            WriteLine( const CString &oString );
		    bool			Write( const char *pchBuffer, int nLength );
                // The next complete line, with its newline ("" if there is
                // none yet).
            CString         GetLine( void );

                // The line number reached (read or written).
            long            GetPos( void );

            bool            IsOpen( void ) { return m_bFileOpen; }
                // Whether the writer has finished (readers only).
            bool            IsWriterDone( void );
                // Whether the reader has read everything there is so far.
            bool            IsEOF( void );

        private:
            void            UpdateIndex( void );
            bool            ReadIndex( long &nWriteLine, long &nReadLine );
            static bool     OpenFile( CFile &oFile, const CString &strName, bool bCreate );

                            CRWFile( const CRWFile & );
            CRWFile        &operator =( const CRWFile & );
    };
} // namespace IASLib

#endif // IASLIB_RWFILE_H__
