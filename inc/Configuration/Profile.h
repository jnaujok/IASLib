/**
 * @file
 * @brief Profile Class.
 *
 * This module is used to maintain profile ( *.INI ) files. These
 * files are of the same format that Windows uses to store
 * configuration data.
 *
 * Major revision - 03/06/2002 - converted from C to C++ format
 * and used internal genlib (IASLib) functionality to replace
 * most of the code. Basically equivalent to a 90% rewrite.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/09/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_PROFILE_H__
#define IASLIB_PROFILE_H__

#include "../Files/File.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CEntryStorage;
    class CSectionStorage;


    /// Reads and writes Windows-style INI profile files.
    ///
    /// A profile file holds "[section]" headings followed by "name = value"
    /// lines. Lines starting with ';' or '#', blank lines and lines without
    /// '=' are kept as comments. Section and entry names are matched without
    /// regard to case. Names and values are trimmed of surrounding spaces.
    ///
    /// The file is read lazily: a lookup reads sections from disk until it
    /// finds the one it wants, and keeps everything read so far in memory.
    /// When the whole file has been read the profile is "cached" and the file
    /// is closed. If the file on disk changes after the last write, the cache
    /// is thrown away and the file is read again.
    ///
    /// A write changes the value in memory and then rewrites the whole file,
    /// keeping comments and other entries. Not thread-safe.
    ///
    /// @code
    /// CProfile oProfile( "app.ini" );
    /// CString strHost;
    /// oProfile.GetString( "Server", "Host", "localhost", strHost );
    /// int nPort = oProfile.GetInt( "Server", "Port", 80 );
    /// oProfile.WriteString( "Server", "LastRun", "today" );
    /// @endcode
    class CProfile : public CObject
    {
        private:
            CString             m_strFileName;      ///< Path of the profile file.
            CFile               m_ProfileFile;      ///< The open profile file, while reading.
            CDate               m_dttLastChanged;   ///< When this object last wrote the file.
            bool                m_bExistsFlag;      ///< True if the file existed when opened.
            CSectionStorage    *m_listSections;     ///< First section of the in-memory copy (owned).
            bool                m_bCached;          ///< True once the whole file is in memory.

        public:
                                /// Opens a profile file for reading.
                                /// A missing file is not an error: DoesExist() returns false,
                                /// lookups return their defaults, and the first write creates it.
                                /// @param pszFileName Path of the INI file.
                                /// @param bCacheFile If true, read the whole file into memory now.
                                CProfile( const char *pszFileName, bool bCacheFile = false );
            /// Frees the in-memory sections and closes the file.
            virtual            ~CProfile( void );

                                DEFINE_OBJECT( CProfile )

            /// @name Reading
            /// Get functions return data from the file. A missing section or
            /// entry gives the default.
            ///@{

            /// Copies an entry's value into a caller's buffer.
            /// The copy is cut to fit and always null-terminated.
            /// @param pszSectionName Section name, without brackets.
            /// @param pszEntryName Entry name.
            /// @param pszDefault Value to copy if the entry is not found.
            /// @param pszBuffer Buffer to receive the value.
            /// @param nBufferLen Size of @p pszBuffer in bytes, including the terminator.
            /// @return Length of the string left in @p pszBuffer.
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferLen );
            /// Copies an entry's value into a CString.
            /// @param pszSectionName Section name, without brackets.
            /// @param pszEntryName Entry name.
            /// @param pszDefault Value to use if the entry is not found.
            /// @param strBuffer Receives the value.
            /// @return Length of @p strBuffer.
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strBuffer );
            /// Returns a pointer to an entry's value.
            /// @param pszSectionName Section name, without brackets.
            /// @param pszEntryName Entry name.
            /// @param pszDefault Returned if the entry is not found.
            /// @return The value, owned by the profile and valid until it is next
            ///         written or reloaded; or @p pszDefault itself.
            const char         *GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault );

            /// Returns an entry's value converted with atoi().
            /// @param pszSectionName Section name, without brackets.
            /// @param pszEntryName Entry name.
            /// @param nDefault Returned if the entry is not found.
            /// @return The value; 0 if it is not a number.
            int                 GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault );
            ///@}

            /// @name Writing
            /// Write functions add data to the file.
            ///@{

            /// Sets an entry's value and rewrites the file.
            /// The section and entry are added if they are missing. If the file
            /// did not exist, it is created with a "; Profile created by
            /// CProfile class" comment at the top.
            /// @param pszSectionName Section name, without brackets.
            /// @param pszEntryName Entry name.
            /// @param pszBuffer The new value.
            /// @return Always true.
            bool                WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer );
            /// Sets an entry to an integer value.
            /// @warning Declared but not implemented: calling it fails to link.
            bool                WriteInt( const char *pszSectionName, const char *pszEntryName, const int nValue );
            ///@}

            /// The profile file's path.
            const char         *GetFileName( void ) const { return (const char *)m_strFileName; }

            /// True while the file is open for reading (it is closed once fully cached).
            bool                IsOpen( void ) const { return m_ProfileFile.IsOpen(); }
            /// True if the file existed when the profile was opened (or has been created since).
            bool                DoesExist( void ) const { return m_bExistsFlag; }
            /// True once the whole file has been read into memory.
            bool                IsCached( void ) const { return m_bCached; }

        private:
            /// Appends a new, empty section to the in-memory list.
            CSectionStorage    *AddSection( const char *strName );
            /// Finds a section, reading more of the file if needed.
            /// @return The section, or NULL if the file has no such section.
            CSectionStorage    *FindSection( const char *strSection );
            /// True if the file on disk changed after this object last wrote it.
            bool                IsModified( void );
            /// Reads the rest of the file, then rewrites it from memory.
            bool                WriteFile( void );

            /// Closes the file. @return True if it is now closed.
            bool                CloseFile( void );
            /// Opens the file for reading, creating it first if it did not exist.
            bool                OpenFile( void );
            /// Not implemented.
            short               ReadSection( long lOffset, char *pszBuffer, short nBuffSize );

    };

    /// Optional global profile object for the application.
    /// It starts as NULL; the application creates it.
    extern CProfile *g_Profile;
} // namespace IASLib

#endif // IASLIB_PROFILE_H__
