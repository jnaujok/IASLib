/**
 * @file
 * @brief Profile Class.
 *
 *      Reads and writes profile (.INI) files, the format Windows uses for
 * configuration:
 *
 *          ; a comment (or # a comment)
 *          [Section]
 *          name = value
 *
 * Entries before the first [Section] belong to the root section, which
 * a NULL or "" section name selects. Section and entry names match
 * without regard to case. A file that does not exist yet reads as empty
 * and is created by the first write.
 *
 *      Every write saves the file at once (to a temporary file that then
 * replaces it), keeping its comments, blank lines and order. Without the
 * cache, reads see changes other programs make to the file; with it, the
 * file is read once.
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

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

#include <map>
#include <string>
#include <vector>

namespace IASLib
{
    /// A profile (.INI) file: sections of "name = value" entries.
    ///
    /// Entries before the first [Section] belong to the root section, which
    /// a NULL or "" section name selects. Names match without regard to
    /// case. Every write saves the file at once, keeping its comments, blank
    /// lines and order. Without the cache, each read checks whether the file
    /// changed on disk and reads it again if so. Not thread-safe; not
    /// copyable.
    ///
    /// @code
    /// CProfile oProfile( "app.ini" );
    /// oProfile.WriteInt( "Window", "Width", 640 );
    /// int nWidth = oProfile.GetInt( "Window", "Width", 800 );           // 640
    /// CString strTitle;
    /// oProfile.GetString( "Window", "Title", "Untitled", strTitle );   // "Untitled"
    /// @endcode
    class CProfile : public CObject
    {
        public:
                                /// Opens the profile @p pszFileName; with @p bCacheFile
                                /// the file is read only once.
                                /// Never throws for a missing or unreadable
                                /// file: DoesExist() says whether it was there.
                                /// @note A missing file used to throw CFileException.
                                CProfile( const char *pszFileName, bool bCacheFile = false );
            virtual            ~CProfile( void );       ///< Destructor.

                                DEFINE_OBJECT( CProfile )

                /// The entry's value, or the default. The buffer form copies
                /// at most nBufferLen - 1 characters and always ends the text;
                /// all return the length of what they give (0 for a NULL
                /// buffer or a length of 0 or less).
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferLen );
                /// Same, into a CString (a NULL default gives "").
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strBuffer );
                /// The entry's value, or @p pszDefault itself if there is none.
                /// The pointer stays valid until the same entry is read again
                /// this way or the profile is destroyed.
                /// @note It used to point into storage that the next read or
                /// write could free.
            const char         *GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault );

                /// The entry's value as a number (decimal, or 0x hex, or 0
                /// octal), or @p nDefault if there is none or it is not a number.
                /// @note atoi() used to give 0 for text.
            int                 GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault );

                /// Sets the entry (adding the section and entry as needed) and
                /// saves the file: false if it could not be saved, or a name or
                /// value would not read back the same (an empty entry name, a
                /// name with '=', a section with '[' or ']', or a line break).
            bool                WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer );
                /// Same, for a number.
            bool                WriteInt( const char *pszSectionName, const char *pszEntryName, const int nValue );
                /// Removes an entry and saves the file. False if there was no
                /// such entry, or the file could not be saved.
            bool                DeleteEntry( const char *pszSectionName, const char *pszEntryName );
                /// Removes a whole section (not the root) and saves the file.
                /// Returns as DeleteEntry() does.
            bool                DeleteSection( const char *pszSectionName );

                /// Whether the section has the entry.
            bool                HasEntry( const char *pszSectionName, const char *pszEntryName );
                /// The names of the sections (not the root), in file order.
            std::vector<CString> GetSectionNames( void );
                /// The names of a section's entries, in file order (none if
                /// there is no such section).
            std::vector<CString> GetEntryNames( const char *pszSectionName );

            const char         *GetFileName( void ) const { return (const char *)m_strFileName; }      ///< The file's name.

                /// Whether the file is there (as of the last read or write).
            bool                IsOpen( void ) const { return m_bExistsFlag; }
            bool                DoesExist( void ) const { return m_bExistsFlag; }     ///< Same as IsOpen().
            bool                IsCached( void ) const { return m_bCached; }          ///< Whether the file is read only once.

                /// Reads the file again now.
            void                Reload( void );

        private:
                /// One line of the file: a comment or blank line (kept as
                /// written), or an entry.
            struct SLine
            {
                bool            bEntry;         ///< An entry (not a comment or blank line).
                CString         strRaw;         ///< a comment's text
                CString         strName;        ///< The entry's name.
                CString         strValue;       ///< The entry's value.
                CString         strIndent;      ///< White space before the name.
            };
                /// A section and its lines.
            struct SSection
            {
                CString         strName;        ///< "" for the root
                CString         strHeader;      ///< the line as written
                std::vector<SLine> aLines;      ///< The lines after the header.
            };

            CString             m_strFileName;      ///< The file's name.
            bool                m_bExistsFlag;      ///< Whether the file was there.
            bool                m_bCached;          ///< Read the file only once.
            bool                m_bLoaded;          ///< Whether the file has been read.
            long long           m_llFileStamp;      ///< time last read
            long long           m_llFileSize;       ///< size last read
            std::vector<SSection> m_aSections;      ///< The root, then each section.
            std::map<std::string, CString> m_mapReturned;   ///< Values returned as const char *.

            void                Refresh( void );    ///< Reads the file again if it changed (unless cached).
            bool                Load( void );       ///< Reads the file. False if it is not there.
            bool                Save( void );       ///< Writes the file (through a temporary file).
            SSection           *FindSection( const char *pszSectionName );     ///< The section, or NULL.
            SLine              *FindEntry( const char *pszSectionName, const char *pszEntryName );     ///< The entry, or NULL.
                /// The file's modification time and size, to see whether it changed.
            bool                Stamp( long long &llTime, long long &llSize ) const;

                                CProfile( const CProfile & );       ///< Not copyable.
            CProfile           &operator =( const CProfile & );     ///< Not copyable.
    };

        /// Allows for global profile object.
    extern CProfile *g_Profile;
} // namespace IASLib

#endif // IASLIB_PROFILE_H__
