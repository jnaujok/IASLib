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
    class CRWFile : public CObject
    {
        protected:
            CFile           m_oIndexFile;
            CFile           m_oDataFile;
            long            m_nReadPos;
            long            m_nReadLine;
            long            m_nWritePos;
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

            bool            Open( const char *strFileName, int nMode );
            bool            Close( bool bComplete = true );
            bool            Flush( void );

            bool            WriteLine( const CString &oString );
		    bool			Write( const char *pchBuffer, int nLength );
            CString         GetLine( void );

            long            GetPos( void );

            bool            IsOpen( void ) { return m_bFileOpen; }
            bool            IsWriterDone( void );
            bool            IsEOF( void );

        private:
            void            UpdateIndex( void );
    };
} // namespace IASLib

#endif // IASLIB_RWFILE_H__

