/*
 * Profile Class
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
    class CProfile : public CObject
    {
        public:
                                // Never throws for a missing or unreadable
                                // file: DoesExist() says whether it was there.
                                // (A missing file threw CFileException.)
                                CProfile( const char *pszFileName, bool bCacheFile = false );
            virtual            ~CProfile( void );

                                DEFINE_OBJECT( CProfile )

                // The entry's value, or the default. The buffer form copies
                // at most nBufferLen - 1 characters and always ends the text;
                // all return the length of what they give.
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferLen );
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strBuffer );
                // The pointer stays valid until the same entry is read again
                // this way or the profile is destroyed. (It pointed into
                // storage that the next read or write could free.)
            const char         *GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault );

            int                 GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault );

                // Sets the entry (adding the section and entry as needed) and
                // saves the file: false if it could not be saved.
            bool                WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer );
            bool                WriteInt( const char *pszSectionName, const char *pszEntryName, const int nValue );
                // Removes an entry, or a whole section, and saves the file.
            bool                DeleteEntry( const char *pszSectionName, const char *pszEntryName );
            bool                DeleteSection( const char *pszSectionName );

            bool                HasEntry( const char *pszSectionName, const char *pszEntryName );
                // The names of the sections (not the root), and of a
                // section's entries, in file order.
            std::vector<CString> GetSectionNames( void );
            std::vector<CString> GetEntryNames( const char *pszSectionName );

            const char         *GetFileName( void ) const { return (const char *)m_strFileName; }

                // Whether the file is there (as of the last read or write).
            bool                IsOpen( void ) const { return m_bExistsFlag; }
            bool                DoesExist( void ) const { return m_bExistsFlag; }
            bool                IsCached( void ) const { return m_bCached; }

                // Reads the file again now.
            void                Reload( void );

        private:
                // One line of the file: a comment or blank line (kept as
                // written), or an entry.
            struct SLine
            {
                bool            bEntry;
                CString         strRaw;         // a comment's text
                CString         strName;
                CString         strValue;
                CString         strIndent;
            };
            struct SSection
            {
                CString         strName;        // "" for the root
                CString         strHeader;      // the line as written
                std::vector<SLine> aLines;
            };

            CString             m_strFileName;
            bool                m_bExistsFlag;
            bool                m_bCached;
            bool                m_bLoaded;
            long long           m_llFileStamp;      // size and time last read
            long long           m_llFileSize;
            std::vector<SSection> m_aSections;
            std::map<std::string, CString> m_mapReturned;

            void                Refresh( void );
            bool                Load( void );
            bool                Save( void );
            SSection           *FindSection( const char *pszSectionName );
            SLine              *FindEntry( const char *pszSectionName, const char *pszEntryName );
            bool                Stamp( long long &llTime, long long &llSize ) const;

                                CProfile( const CProfile & );
            CProfile           &operator =( const CProfile & );
    };

        // Allows for global profile object.
    extern CProfile *g_Profile;
} // namespace IASLib

#endif // IASLIB_PROFILE_H__
