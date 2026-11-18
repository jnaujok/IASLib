/**
 * @file FileStat.h
 * @brief CFileStat, a file's status (type, size, dates) without opening it.
 *
 * The original header comment follows.
 */
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
    /// A file's status (type, size and dates), read with stat() when the object is made.
    ///
    /// The values are a snapshot; make a new object to read them again.
    ///
    /// @code
    /// CFileStat oStat( "data.bin" );
    /// if ( oStat.IsFile() )
    ///     printf( "%ld bytes\n", oStat.GetSize() );
    /// @endcode
    class CFileStat : public CObject
    {
        protected:
            CString         m_strFileName;      ///< The path, trimmed.
            struct stat     m_stFileStatus;     ///< What stat() returned.
            bool            m_bIsFile;          ///< A regular file.
            bool            m_bIsDir;           ///< A directory.
            CDate           m_dttLastAccess;    ///< When it was last read.
            CDate           m_dttLastModified;  ///< When its contents last changed.
            CDate           m_dttLastChanged;   ///< When its status last changed.
            long            m_lFileSize;        ///< The size in bytes (regular files only).

        public:
                /// Reads the status of a file.
                /// @param strFileName  The path (spaces around it are trimmed).
                /// @throws CFileException (with errno) if stat() fails, for
                ///         example when the file does not exist.
                            CFileStat( const char *strFileName );
                /// Destroys the object.
            virtual        ~CFileStat( void );

                            DEFINE_OBJECT( CFileStat );

                /// Whether it is a regular file.
            bool            IsFile( void ) { return m_bIsFile; }
                /// Whether it is a directory.
            bool            IsDir( void ) { return m_bIsDir; }

                /// When it was last read.
            CDate           GetLastAccess( void ) { return m_dttLastAccess; }
                /// When its contents last changed.
            CDate           GetLastModified( void ) { return m_dttLastModified; }
                /// When its status last changed (ctime).
            CDate           GetLastChanged( void ) { return m_dttLastChanged; }

                /// The size in bytes; 0 for anything but a regular file.
            long            GetSize( void ) { return m_lFileSize; }
    };
} // namespace IASLib

#endif // __FILESTAT_H__
