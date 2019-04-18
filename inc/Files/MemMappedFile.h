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
    class CMemMappedFile : public CFile
    {
        protected:
#ifdef IASLIB_WIN32__
            HANDLE          m_hFileHandle;
            HANDLE          m_hMemoryMap;
#else
            int             m_hFileHandle;
            int             m_hMemoryMap;
#endif
            long            m_nReadPointer;
            long            m_nWritePointer;
            bool            m_bSyncPointers;
            bool            m_bIsOpen;

        public:
	                        CMemMappedFile();
#ifdef IASLIB_WIN32__
                            CMemMappedFile( HANDLE nFileHandle, int nMode );
#endif

                            CMemMappedFile( const char *strFileName, int nMode );
	        virtual        ~CMemMappedFile();

                            DECLARE_OBJECT( CMemMappedFile, CFile )

            virtual bool    Open( const char *strFileName, int nMode );
            virtual bool    Close( void );
            virtual bool    Flush( void );
            virtual int     Read( char *pchBuffer, int nBufferSize );
            virtual int     Write( const char *pchBuffer, int nBufferSize );
            virtual int     WriteString( const CString &oString );
            virtual long    GetPos( void );
            virtual long    GetSize( void );
            virtual long    Seek( long nPos, int nStart );
            virtual long    SeekRead( long nPos, int nStart );
            virtual long    SeekWrite( long nPos, int nStart );
            virtual bool    IsOpen( void ) const { return m_bIsOpen; }
            virtual bool    IsEOF( void );
            virtual CString GetLine( void );

            static bool     Delete( const char *strFileName );
            static bool     Rename( const char *strOldName, const char *strNewName );
    };
} // namespace IASLib

#endif // IASLIB_MEM_MAPPED_FILE_H__

