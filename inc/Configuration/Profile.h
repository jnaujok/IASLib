/*
 * Profile Class
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


    class CProfile : public CObject
    {
        private:
            CString             m_strFileName;
            CFile               m_ProfileFile;
            CDate               m_dttLastChanged;
            bool                m_bExistsFlag;
            CSectionStorage    *m_listSections;
            bool                m_bCached;
            
        public:
                                CProfile( const char *pszFileName, bool bCacheFile = false );
            virtual            ~CProfile( void );

                                DEFINE_OBJECT( CProfile )

                                    // Get functions return data from the file
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferLen );
            size_t              GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strBuffer );
            const char         *GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault );

            int                 GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault );
                                    // Write functions add data to the file.
            bool                WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer );
            bool                WriteInt( const char *pszSectionName, const char *pszEntryName, const int nValue );
            
            const char         *GetFileName( void ) const { return (const char *)m_strFileName; }

            bool                IsOpen( void ) const { return m_ProfileFile.IsOpen(); }
            bool                DoesExist( void ) const { return m_bExistsFlag; }
            bool                IsCached( void ) const { return m_bCached; }
            
        private:
            CSectionStorage    *AddSection( const char *strName );
            CSectionStorage    *FindSection( const char *strSection );
            bool                IsModified( void );
            bool                WriteFile( void );

            bool                CloseFile( void );
            bool                OpenFile( void );
            short               ReadSection( long lOffset, char *pszBuffer, short nBuffSize );

    };

        // Allows for global profile object.
    extern CProfile *g_Profile;
} // namespace IASLib

#endif // IASLIB_PROFILE_H__
