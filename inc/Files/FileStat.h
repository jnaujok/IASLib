/*
 * File Status Class
 *
 *      This class wraps the status function found in Unix. This 
 * function can be used to detemine information about a file 
 * without actually having to open that file.
 *      In Windows, or other operating systems, this command 
 * recreates the functionality of the Unix file-stat.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/15/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_FILESTAT_H__
#define IASLIB_FILESTAT_H__

#include "../BaseTypes/Date.h"
#include <sys/types.h>
#include <sys/stat.h>

namespace IASLib
{
    class CFileStat : public CObject
    {
        protected:
            CString         m_strFileName;
            struct stat     m_stFileStatus;
            bool            m_bIsFile;
            bool            m_bIsDir;
            CDate           m_dttLastAccess;
            CDate           m_dttLastModified;
            CDate           m_dttLastChanged;
            long            m_lFileSize;

        public:
                            CFileStat( const char *strFileName );
            virtual        ~CFileStat( void );

                            DEFINE_OBJECT( CFileStat );

            bool            IsFile( void ) { return m_bIsFile; }
            bool            IsDir( void ) { return m_bIsDir; }

            CDate           GetLastAccess( void ) { return m_dttLastAccess; }
            CDate           GetLastModified( void ) { return m_dttLastModified; }
            CDate           GetLastChanged( void ) { return m_dttLastChanged; }

            long            GetSize( void ) { return m_lFileSize; }
    };
} // namespace IASLib

#endif // __FILESTAT_H__
