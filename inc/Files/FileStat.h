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

#include <stdint.h>
#include "../BaseTypes/Date.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
        // What the file system says about a path, without opening it. The
        // constructor throws a CFileException (errno) if there is no such
        // path.
    class CFileStat : public CObject
    {
        protected:
            CString         m_strFileName;
            bool            m_bIsFile;
            bool            m_bIsDir;
            CDate           m_dttLastAccess;
            CDate           m_dttLastModified;
            CDate           m_dttLastChanged;
            int64_t         m_lFileSize;

        public:
                            CFileStat( const char *strFileName );
            virtual        ~CFileStat( void );

                            DEFINE_OBJECT( CFileStat );

            bool            IsFile( void ) const { return m_bIsFile; }
            bool            IsDir( void ) const { return m_bIsDir; }

            CDate           GetLastAccess( void ) const { return m_dttLastAccess; }
            CDate           GetLastModified( void ) const { return m_dttLastModified; }
                // The status change time (on Windows, the creation time).
            CDate           GetLastChanged( void ) const { return m_dttLastChanged; }

                // The size of a file in bytes (0 for a directory); GetSize()
                // is -1 when that does not fit in a long.
            int64_t         GetSizeBig( void ) const { return m_lFileSize; }
            long            GetSize( void ) const;
            const CString  &GetFileName( void ) const { return m_strFileName; }
    };
} // namespace IASLib

#endif // IASLIB_FILESTAT_H__
