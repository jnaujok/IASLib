/**
 * @file
 * @brief File Status Class.
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
        /// What the file system says about a path, without opening it. The
        /// constructor throws a CFileException (errno) if there is no such
        /// path.
        ///
        /// @code
        /// CFileStat oStat( "notes.txt" );
        /// if ( oStat.IsFile() )
        ///     printf( "%lld bytes\n", (long long)oStat.GetSizeBig() );
        /// @endcode
    class CFileStat : public CObject
    {
        protected:
            CString         m_strFileName;      ///< The path asked about.
            bool            m_bIsFile;          ///< A regular file.
            bool            m_bIsDir;           ///< A directory.
            CDate           m_dttLastAccess;    ///< Last access time.
            CDate           m_dttLastModified;  ///< Last modification time.
            CDate           m_dttLastChanged;   ///< Status change (Windows: creation) time.
            int64_t         m_lFileSize;        ///< Size in bytes (0 if not a regular file).

        public:
                            /// Reads the status of @p strFileName.
                            /// @throws CFileException (with errno) if there is no such path.
                            /// @note It used to test the type with "mode & S_IFDIR",
                            /// which is also true of sockets and block devices, trim
                            /// spaces from the name, and keep the size in a long.
                            CFileStat( const char *strFileName );
            virtual        ~CFileStat( void );      ///< Destructor.

                            DEFINE_OBJECT( CFileStat );

            bool            IsFile( void ) const { return m_bIsFile; }      ///< Whether the path is a regular file.
            bool            IsDir( void ) const { return m_bIsDir; }        ///< Whether the path is a directory.

            CDate           GetLastAccess( void ) const { return m_dttLastAccess; }       ///< The last access time.
            CDate           GetLastModified( void ) const { return m_dttLastModified; }   ///< The last modification time.
                /// The status change time (on Windows, the creation time).
            CDate           GetLastChanged( void ) const { return m_dttLastChanged; }

                /// The size of a file in bytes (0 for a directory or anything
                /// else that is not a regular file).
            int64_t         GetSizeBig( void ) const { return m_lFileSize; }
                /// Same, as a long: -1 when that does not fit in a long.
            long            GetSize( void ) const;
            const CString  &GetFileName( void ) const { return m_strFileName; }    ///< The path asked about.
    };
} // namespace IASLib

#endif // IASLIB_FILESTAT_H__
