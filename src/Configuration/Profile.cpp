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

#include "Profile.h"
#include "FileStat.h"
#include <stdlib.h>
#include <string.h>

#define ROOT_NAME "[ROOT]"

namespace IASLib
{
        // Allows for a global profile object.
    CProfile *g_Profile = NULL;

    /*************************************************************************
    *                     P R I V A T E   C L A S S E S                      *
    *************************************************************************/

    /*************************************************************************
    *                       C E n t r y S t o r a g e                        *
    *************************************************************************/
    /*************************************************************************
    *   CEntryStorage                                                        *
    *                                                                        *
    *       Description:  This class stores individual entries in the        *
    *                     sections of the profile file. Entries can also     *
    *                     contain comment lines.                             *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/
      
    class CEntryStorage : public CObject
    {
        protected:
            CString         m_strEntryName;
            CString         m_strValue;
            CEntryStorage  *m_listNextEntry;
            bool            m_bCommentFlag;
            int             m_nLeadingWhitespace;
        public:
                            CEntryStorage( const char *strName, const char *strValue, bool bComment = false, int nLeadSpace = 0 ) { m_strEntryName = strName; m_strValue = strValue; m_bCommentFlag = bComment; m_listNextEntry = NULL; m_nLeadingWhitespace = nLeadSpace; }
            virtual        ~CEntryStorage( void ) { delete m_listNextEntry; }

                            DEFINE_OBJECT( CEntryStorage )

            CString        &GetName( void ) { return m_strEntryName; }
            const char     *GetValue( void ) { return (const char *)m_strValue; }
            bool            SetValue( const char *strValue ) { m_strValue = strValue; return true; }

            CEntryStorage  *GetNext( void ) { return m_listNextEntry; }
            CEntryStorage  *SetNext( CEntryStorage *pNext ) { CEntryStorage *pTemp = m_listNextEntry; m_listNextEntry = pNext; return pTemp; }

            bool            IsComment( void ) { return m_bCommentFlag; }

            void            Write( CFile &oFile );
    };

    IMPLEMENT_OBJECT( CEntryStorage, CObject );

    /*************************************************************************
    *   CEntryStorage::Write                                                 *
    *                                                                        *
    *       Description:  This method writes the entry out to the passed in  *
    *                     file.                                              *
    *           Returns:  None.                                              *
    *            Throws:  None                                               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    void CEntryStorage::Write( CFile &oFile )
    {
        if ( m_bCommentFlag )
        {
            oFile.WriteString( m_strValue );
            oFile.Write( "\n", 1 );
        }
        else
        {
            CString strTemp;
            strTemp.Format( "%s = %s\n", (const char *)m_strEntryName, (const char *)m_strValue );
            if ( m_nLeadingWhitespace )
            {
                strTemp = strTemp.Pad( (int)strTemp.GetLength() + m_nLeadingWhitespace, ' ', false );
            }
            oFile.WriteString( strTemp );
        }
    }

    /*************************************************************************
    *                     C S e c t i o n S t o r a g e                      *
    *************************************************************************/
    /*************************************************************************
    *   CSectionStorage                                                      *
    *                                                                        *
    *       Description:  This class stores individual sections in the       *
    *                     profile file.                                      *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    class CSectionStorage : public CObject
    {
        protected:
            CString             m_strSectionName;
            CEntryStorage      *m_listEntries;
            CSectionStorage    *m_listNextSection;

        public:
                                CSectionStorage( const char *strName ) { m_strSectionName = strName; m_listEntries = NULL; m_listNextSection = NULL; }
            virtual            ~CSectionStorage( void ) { delete m_listEntries; delete m_listNextSection; }

                                DEFINE_OBJECT( CSectionStorage )

            CString            &GetName( void ) { return m_strSectionName; }

            CEntryStorage      *FindEntry( const char *strEntryName );
            void                Write( CFile &oFile );

            CEntryStorage      *AddEntry( const char *strName, const char *strValue, bool bComment = false, int nLeadSpace = 0 );

            CSectionStorage    *GetNext( void ) { return m_listNextSection; }
            CSectionStorage    *SetNext( CSectionStorage *pNext ) { CSectionStorage *pTemp = m_listNextSection; m_listNextSection = pNext; return pTemp; }
    };

    IMPLEMENT_OBJECT( CSectionStorage, CObject );

    /*************************************************************************
    *   CSectionStorage::FindEntry                                           *
    *                                                                        *
    *       Description:  This method returns an entry object given the      *
    *                     corresponding entry name.                          *
    *           Returns:  The Entry Storage Object.                          *
    *            Throws:  None                                               *
    *            Errors:  Returns NULL if the entry can't be found.          *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    CEntryStorage *CSectionStorage::FindEntry( const char *strEntryName )
    {
        CEntryStorage *pRetVal = NULL;
        CEntryStorage *pTemp = m_listEntries;

        while ( ( pTemp ) && ( ! pRetVal ) )
        {
            if ( ( pTemp->IsComment() ) || ( pTemp->GetName().Compare( strEntryName, false ) != 0 ) )
            {
                pTemp = pTemp->GetNext();
            }
            else
            {
                pRetVal = pTemp;
            }
        }

        return pRetVal;
    }

    /*************************************************************************
    *   CSectionStorage::AddEntry                                            *
    *                                                                        *
    *       Description:  This method adds an entry to a section.            *
    *           Returns:  The added CEntryStorage Object.                    *
    *            Throws:  None                                               *
    *            Errors:  Returns NULL if the entry can't be added.          *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    CEntryStorage *CSectionStorage::AddEntry( const char *strName, const char *strValue, bool bComment, int nLeadSpace )
    {
        CEntryStorage *pLast = NULL;
        CEntryStorage *pTemp = m_listEntries;

        while ( pTemp )
        {
            pLast = pTemp;
            pTemp = pTemp->GetNext();
        }

        pTemp = new CEntryStorage( strName, strValue, bComment, nLeadSpace );

        if ( ! pLast )
        {
            m_listEntries = pTemp;
        }
        else
        {
            pLast->SetNext( pTemp );
        }

        return pTemp;
    }

    /*************************************************************************
    *   CEntryStorage::Write                                                 *
    *                                                                        *
    *       Description:  This method writes the section out to the passed   *
    *                     in file.                                           *
    *           Returns:  None.                                              *
    *            Throws:  None                                               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    void CSectionStorage::Write( CFile &oFile )
    {
        CString strTemp;
        CEntryStorage *pTemp = m_listEntries;
        
        if ( m_strSectionName != ROOT_NAME )
        {
            strTemp.Format( "[%s]\n", (const char *)m_strSectionName );
            oFile.WriteString( strTemp );
        }

        while ( pTemp )
        {
            pTemp->Write( oFile );
            pTemp = pTemp->GetNext();
        }
    }
        

    /*************************************************************************
    *                       C L A S S   M E T H O D S                        *
    *************************************************************************/

    /*************************************************************************
    *                   P U B L I C   F U N C T I O N S                      *
    *************************************************************************/
    IMPLEMENT_OBJECT( CProfile, CObject );

    /*************************************************************************
    *   CProfile                                                             *
    *                                                                        *
    *       Description:  This is the constructor method for the Profile     *
    *                     object. It attempts to locate the given filename   *
    *                     in the correct path group, and then set up its     *
    *                     internal flags accordingly.                        *
    *           Returns:  None                                               *
    *            Throws:  None                                               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/
      
    CProfile::CProfile( const char *pszFileName, bool bCacheFile )
    {
        m_listSections = NULL;

        m_strFileName = pszFileName;
        m_ProfileFile.Open( m_strFileName, CFile::READ );
        
        if ( m_ProfileFile.IsOpen() )
        {
            m_bExistsFlag = true;
            m_bCached = false;

            if ( bCacheFile )
            {
                FindSection( "]" );
            }
        }  
        else
        {
            m_bCached = true;
            m_bExistsFlag = false;
        }
    }

    /*************************************************************************
    *   ~CProfile                                                            *
    *                                                                        *
    *       Description:  This is the destructor method for objects of the   *
    *                     CProfile class. It cleans up the section chain and *
    *                     makes sure that the file is closed.                *
    *           Returns:  None                                               *
    *            Throws:  None                                               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/

    CProfile::~CProfile( void )
    { 
        if ( m_listSections )
        {
            delete m_listSections;
        }

        if ( IsOpen() )
            CloseFile();
    }

    /*************************************************************************
    *   CProfile::GetString                                                  *
    *                                                                        *
    *       Description:  This function retrieves a single string from the   *
    *                     INI file by finding its section, and then finding  *
    *                     its exact entry, and then using an fgets to get    *
    *                     the actual entry value. The string is then trimmed *
    *                     of leading and trailing spaces before being        *
    *                     returned to the calling code.                      *
    *           Returns:  int - Length of String                             *
    *            Throws:  None                                               *
    *            Errors:  Returns Default value on failure                   *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/
                                                                              
    size_t CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferSize )
    {
        CSectionStorage    *pSection;
        CEntryStorage      *pEntry;
        int                 nRetVal = 0;
        bool                bFound = false;

        pSection = FindSection( pszSectionName );

        if ( pSection )
        {
            pEntry = pSection->FindEntry( pszEntryName );

            if ( pEntry )
            {
#ifdef IASLIB_WIN32__
                strncpy_s( pszBuffer, nBufferSize, pEntry->GetValue(), nBufferSize );
#else
                strncpy( pszBuffer, pEntry->GetValue(), nBufferSize );
#endif
                bFound = true;
            }
        }

        if (! bFound )
        {
#ifdef IASLIB_WIN32__
            strncpy_s( pszBuffer, nBufferSize, pszDefault, nBufferSize );
#else
            strncpy( pszBuffer, pszDefault, nBufferSize );
#endif
        }

        pszBuffer[ nBufferSize - 1 ] = 0;
        nRetVal = (int)strlen( pszBuffer );

        return nRetVal;
    }

    /*************************************************************************
    *   CProfile::GetString                                                  *
    *                                                                        *
    *       Description:  This function retrieves a single string from the   *
    *                     INI file by finding its section, and then finding  *
    *                     its exact entry, and then copying it into a        *
    *                     CString object for return to the caller.           *
    *           Returns:  size_t - Length of String                             *
    *            Throws:  None                                               *
    *            Errors:  Returns Default value on failure                   *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/
                                                                              
    size_t CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strReturn )
    {
        CSectionStorage    *pSection;
        CEntryStorage      *pEntry;
        size_t              nRetVal = 0;
        bool                bFound = false;

        pSection = FindSection( pszSectionName );

        if ( pSection )
        {
            pEntry = pSection->FindEntry( pszEntryName );

            if ( pEntry )
            {
                strReturn = pEntry->GetValue();
                bFound = true;
            }
        }

        if ( ! bFound )
        {
            strReturn = pszDefault;
        }

        nRetVal = strReturn.GetLength();

        return nRetVal;
    }

    /*************************************************************************
    *   CProfile::GetString                                                  *
    *                                                                        *
    *       Description:  This function retrieves a single string from the   *
    *                     INI file by finding its section, and then finding  *
    *                     its exact entry, and then copying it into a        *
    *                     CString object for return to the caller.           *
    *           Returns:  int - Length of String                             *
    *            Throws:  None                                               *
    *            Errors:  Returns Default value on failure                   *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/
                                                                              
    const char *CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault )
    {
        CSectionStorage    *pSection;
        CEntryStorage      *pEntry;
        const char         *pRetVal = NULL;
        bool                bFound = false;

        pSection = FindSection( pszSectionName );

        if ( pSection )
        {
            pEntry = pSection->FindEntry( pszEntryName );

            if ( pEntry )
            {
                pRetVal = pEntry->GetValue();
                bFound = true;
            }
        }

        if ( ! bFound )
        {
            pRetVal = pszDefault;
        }

        return pRetVal;
    }

    /*************************************************************************
    *   CProfile::GetInt                                                     *
    *                                                                        *
    *       Description:  This method returns the integer value of the given *
    *                     entry. It does this by getting the string and then *
    *                     converting that return value to an integer.        *
    *           Returns:  int - The value of the entry                       *
    *            Throws:  None                                               *
    *            Errors:  Returns Default value on failure                   *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/

    int CProfile::GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault )
    {
        CSectionStorage    *pSection;
        CEntryStorage      *pEntry;
        int                 nRetVal = nDefault;

        pSection = FindSection( pszSectionName );

        if ( pSection )
        {
            pEntry = pSection->FindEntry( pszEntryName );

            if ( pEntry )
            {
                nRetVal = atoi( pEntry->GetValue() );
            }
        }

        return nRetVal;
    }

    /*************************************************************************
    *   CProfile::WriteString                                                *
    *                                                                        *
    *       Description:  This method would write the given string at the    *
    *                     correct section and entry, replacing the current   *
    *                     value or adding a new entry, or even a new         *
    *                     section. If the file does not exist, the file is   *
    *                     created. All other data in the file remains in its *
    *                     original state.                                    *
    *                     All this sounds good, but for now, the function is *
    *                     just stubbed out to save time.                     *
    *           Returns:  True - Item was written.                           *
    *                     False - Item could not be written.                 *
    *            Throws:  CFileException from FindSection.                   *
    *            Errors:  None.                                              *
    *                                                                        *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/

    bool CProfile::WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer )
    {
        CSectionStorage    *pSection;
        CEntryStorage      *pEntry;

        if ( ! DoesExist() )
        {
            CString strTemp;
            CDate   dttNow;

            pSection = AddSection( ROOT_NAME );
            strTemp.Format( "; Profile created by CProfile class: %s\n", (const char *)dttNow.PrettyDate() );
            pSection->AddEntry( "", strTemp, true );
        }

        if ( ( pSection = FindSection( pszSectionName ) ) == NULL )
            pSection = AddSection( pszSectionName );

        if ( pSection )
        {
            if ( ( pEntry = pSection->FindEntry( pszEntryName ) ) == NULL )
                pEntry = pSection->AddEntry( pszEntryName, pszBuffer );
            else
                pEntry->SetValue( pszBuffer );

            WriteFile();
        }

        return true;
    }


    /*************************************************************************
    *                  P R I V A T E   F U N C T I O N S                     *
    *************************************************************************/


    /*************************************************************************
    *   CProfile::OpenFile                                                   *
    *                                                                        *
    *       Description:  This method opens the profile file. If it does not *
    *                     exist, then it creates the file. If it does exist, *
    *                     it opens the file for reading and writing.         *
    *           Returns:  True - File is open.                               *
    *                     False - File did not open.                         *
    *            Throws:  None.                                              *
    *            Errors:  None.                                              *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/

    bool CProfile::OpenFile( void )
    {
        CString     strTemp;
        CDate       dttNow;

        if ( m_ProfileFile.IsOpen() )
            return true;
        
        if ( m_bExistsFlag )
        {
            m_ProfileFile.Open( m_strFileName, CFile::READ );
        }
        else
        {
            m_ProfileFile.Open( m_strFileName, CFile::WRITE );
            strTemp.Format( "; Profile created by CProfile class: %s\n", (const char *)dttNow.PrettyDate() );
            m_ProfileFile.WriteString( strTemp );
            m_ProfileFile.Close();
            m_ProfileFile.Open( m_strFileName, CFile::READ );
            m_bCached = false;
            m_bExistsFlag = true;
        }  
        return m_ProfileFile.IsOpen();
    }

    /*************************************************************************
    *   CProfile::CloseFile                                                  *
    *                                                                        *
    *       Description:  This method closes and flushes the object's open   *
    *                     INI file. This allows other profile files to be    *
    *                     opened without a major loss of file handles.       *
    *           Returns:  None.                                              *
    *            Throws:  None.                                              *
    *            Errors:  None.                                              *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, November 9, 1994                        *
    *                                                                        *
    *     Revision Date:  Wednesday, March 6, 2002                           *
    *        Revised By:  Jeffrey R. Naujok                                  *
    *           Changes:  Changed entire style and formatting to match the   *
    *                     genlib formatting to include all of the actual OOP *
    *                     features that genlib includes. Basically a total   *
    *                     rewrite of all the code.                           *
    *                                                                        *
    *************************************************************************/

    bool CProfile::CloseFile( void )
    {
        if ( m_ProfileFile.IsOpen() )
        {
            m_ProfileFile.Close();
        }

        return ( ! m_ProfileFile.IsOpen() );
    }

    /*************************************************************************
    *   CProfile::AddSection                                                 *
    *                                                                        *
    *       Description:  This method adds a new section to the list of      *
    *                     cached sections in memory. It maintains the linked *
    *                     list chain as well.                                *
    *           Returns:  The CSectionStorage object added to the cache.     *
    *            Throws:  None.                                              *
    *            Errors:  None.                                              *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    CSectionStorage *CProfile::AddSection( const char *strName )
    {
        CSectionStorage *pLast = NULL;
        CSectionStorage *pTemp = m_listSections;

        while ( pTemp )
        {
            pLast = pTemp;
            pTemp = pTemp->GetNext();
        }

        pTemp = new CSectionStorage( strName );

        if ( ! pLast )
        {
            m_listSections = pTemp;
        }
        else
        {
            pLast->SetNext( pTemp );
        }

        return pTemp;
    }

    /*************************************************************************
    *   CProfile::FindSection                                                *
    *                                                                        *
    *       Description:  This method is the real heart of the profile       *
    *                     class. It looks for a matching section in the      *
    *                     cached linked list. If it doesn't find one, it     *
    *                     reads the disk file to cache sections into memory. *
    *                     It only caches up to the section required. It also *
    *                     correctly re-caches the file if the disk file is   *
    *                     modified.                                          *
    *           Returns:  The CSectionStorage object associated with the     *
    *                     given section name.                                *
    *            Throws:  IsModified can throw a CFileException.             *
    *            Errors:  Returns NULL if the section is not found.          *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    CSectionStorage *CProfile::FindSection( const char *strSection )
    {
        CString             strLine;
        CString             strTemp;
        CSectionStorage    *pCurrentSection = NULL;
        CSectionStorage    *pRetVal = NULL;
        int                 nLastOffset = 0;
        int                 nBreak;
        CString             strName;
        CString             strValue;
        int                 nLeadSpace;

            // Check if the file has changed. 
        if ( IsModified() )
        {
                // The disk file was modified, we need to wipe all of the cached 
                // version out of memory and start over.
            m_bCached = false;
            delete m_listSections;
            m_listSections = NULL;

            if ( ! m_ProfileFile.IsOpen() )
            {
                m_ProfileFile.Open( m_strFileName, CFile::READ );
            }
        }

        CSectionStorage *pTemp = m_listSections;

        while ( ( pTemp ) && ( ! pRetVal ) )
        {
            if ( pTemp->GetName().Compare( strSection, false ) != 0 )
            {
                pCurrentSection = pTemp;
                pTemp = pTemp->GetNext();
            }
            else
            {
                pRetVal = pTemp;
            }
        }

            // We haven't found the section, and we haven't cached the file yet.
        if ( ( ! pRetVal ) && ( ! m_bCached ) && ( m_ProfileFile.IsOpen() ) )
        {

            while ( ( ! m_ProfileFile.IsEOF() ) && ( ! pRetVal ) )
            {
                nLastOffset = m_ProfileFile.GetPos();
                strLine = m_ProfileFile.GetLine();
                while ( ( strLine[ strLine.GetLength() - 1 ] == '\n' ) || ( strLine[ strLine.GetLength() - 1 ] == '\r' ) )
                {
                    strLine = strLine.Substring( 0, (int)strLine.GetLength() - 1 );
                }
                strTemp = strLine;
                strTemp.Trim();

                switch ( strTemp[0] )
                {
                    case '[':
                            // This is a new section! Hooray!
                        nBreak = (int)strTemp.IndexOf( ']' );
                        if ( nBreak != -1 )
                        {
                            if ( ( pCurrentSection ) && ( pCurrentSection->GetName().Compare( strSection, false ) == 0 ) )
                            {
                                    // We don't need to cache this section yet. We found the one we need.
                                pRetVal = pCurrentSection;
                                    // We wind the file back to the last section
                                m_ProfileFile.Seek( nLastOffset, CFile::BEGIN );
                            }
                            else
                            {
                                strName = strTemp.Substring( 1, nBreak - 1 );
                                strName.Trim();
                                pCurrentSection = AddSection( strName );
                            }
                        }
                        else
                        {
                                // This is a corrupted section heading. It has an open bracket "[",
                                // but no closing bracket. We'll turn it into a comment...

                                // We have to do the following, in case it's a comment
                                // before the first section. Then we have to create a dummy section to
                                // store the comments in.
                            if ( pCurrentSection == NULL )
                            {
                                pCurrentSection = new CSectionStorage( ROOT_NAME );
                                m_listSections = pCurrentSection;
                            }
                            pCurrentSection->AddEntry( "", strLine, true );
                        }
                        break;
                    
                    case ';':
                    case '#':
                    case '\n':
                    case '\r':
                            // This is a comment! We have to do the following, in case it's a comment
                            // before the first section. Then we have to create a dummy section to
                            // store the comments in.
                        if ( pCurrentSection == NULL )
                        {
                            pCurrentSection = new CSectionStorage( ROOT_NAME );
                            m_listSections = pCurrentSection;
                        }
                        pCurrentSection->AddEntry( "", strLine, true );
                        break;
                   
                    default:
                            // This could be a name/value or a comment.
                            // We have to do the following, in case it's a comment
                            // before the first section. Then we have to create a dummy section to
                            // store the comments in.
                        if ( pCurrentSection == NULL )
                        {
                            pCurrentSection = new CSectionStorage( ROOT_NAME );
                            m_listSections = pCurrentSection;
                        }

                        if ( ( nBreak = (int)strLine.IndexOf( '=' ) ) == -1 )
                        {
                                // This is a comment.
                            pCurrentSection->AddEntry( "", strLine, true );
                        }
                        else
                        {
                            strName = strLine.Substring( 0, nBreak );
                            strValue = strLine.Substring( nBreak + 1 );
                            strName.Trim();
                            strValue.Trim();
                            nLeadSpace = (int)strLine.IndexOf( strName );
                            pCurrentSection->AddEntry( strName, strValue, false, nLeadSpace );
                        }
                        break;
                }
            }

            if ( m_ProfileFile.IsEOF() )
            {
                m_bCached = true;
                CloseFile();    // Saves File Handles.
            }

            if ( ( pCurrentSection ) && ( pCurrentSection->GetName().Compare( strSection, false ) == 0 ) )
            {
                pRetVal = pCurrentSection;
            }
        }

        return pRetVal;
    }

    /*************************************************************************
    *   CProfile::IsModified                                                 *
    *                                                                        *
    *       Description:  This method uses the CFileStat class to check if   *
    *                     the profile file has been modified.                *
    *           Returns:  True - The file has been modified since the last   *
    *                            time it was loaded.                         *
    *                     False - The file has not changed since its last    *
    *                             load time.                                 *
    *            Throws:  FileStat can throw a CFileException.               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    bool CProfile::IsModified( void )
    {
        bool    bRetVal = false;

        CFileStat fsCheckFile( m_strFileName );

        if ( fsCheckFile.GetLastModified() > m_dttLastChanged )
        {
            bRetVal = true;
        }

        return bRetVal;
    }

    /*************************************************************************
    *   CProfile::WriteFile                                                  *
    *                                                                        *
    *       Description:  This method writes the profile file back to disk.  *
    *                     Some formatting can be lost in this process, but   *
    *                     only on section headers and unused whitespace      *
    *                     surrounding the "=" in name/value pairs. Trailing  *
    *                     whitespace is also trimmed from values. Comments   *
    *                     are rendered true to form.                         *
    *           Returns:  True - File was written.                           *
    *                     False - File was not written.                      *
    *            Throws:  None                                               *
    *            Errors:  None                                               *
    *                                                                        *
    *            Author:  Jeffrey R. Naujok                                  *
    *              Date:  Wednesday, March 6, 2002                           *
    *                                                                        *
    *************************************************************************/

    bool CProfile::WriteFile( void )
    {
        if ( ! m_bCached )
        {
            FindSection( "]" );
        }

        CSectionStorage    *pTemp = m_listSections;
        m_ProfileFile.Close();

        m_ProfileFile.Open( m_strFileName, CFile::WRITE );

        while ( pTemp )
        {
            pTemp->Write( m_ProfileFile );
            pTemp = pTemp->GetNext();
        }

        m_ProfileFile.Close();

        m_bCached = true;
        m_dttLastChanged.SetToCurrent();

        return true;
    }
} // namespace IASLib
