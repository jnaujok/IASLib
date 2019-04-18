/*
 * XML Index Class
 *
 *      This class is used to convert an XML file into an XML Index. 
 * In general, this class is simply a container for XML Elements within
 * an XML file. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLIndex.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CXMLIndex, CObject )

    CXMLIndex::CXMLIndex( void )
    {
    }

    CXMLIndex::CXMLIndex( const char *strFileName, CFile *pFile )
    {
        CFile *pXMLFile;

        m_strXMLFileName = strFileName;

        if ( ! pFile )
        {
            pXMLFile = new CFile( m_strXMLFileName, CFile::READ );
        }
        else
        {
            pXMLFile = pFile;
        }

        if ( ! pXMLFile->IsOpen() )
        {
            if ( ! pFile )
                delete pXMLFile;
            return;
        }

        CString strData;
        char    achBuffer[65537];

        while ( ! pXMLFile->IsEOF() )
        {
            int nRead;
            nRead = pXMLFile->Read( achBuffer, 65536 );
            achBuffer[ nRead ] = 0;
            strData += achBuffer;
        }

        pXMLFile->Close();
        delete pXMLFile;

        while ( ( strData.GetLength() ) && ( strData.IndexOf( '<' ) != -1 ) )
        {
            CXMLElement *pElement = new CXMLElement( strData );
            m_aElements.Push( pElement );
        }
    }


    CXMLIndex::~CXMLIndex( void )
    {
        m_aElements.DeleteAll();
    }

    CXMLChunk *CXMLIndex::GetChunk( size_t nCount ) const
    {
        if ( nCount < GetChunkCount() ) 
        {
            return (CXMLChunk *)m_aElements[ nCount ];
        }
        return NULL;
    }

    CXMLElement *CXMLIndex::GetElement( size_t nCount ) const
    {
        CXMLChunk  *pChunk = NULL;

        if ( nCount < GetElementCount() )
        {
            size_t      nStep = 0;
            size_t      nFound = 0;

            while ( nFound < ( nCount + 1 ) )
            {
               pChunk = (CXMLChunk *)m_aElements[ nStep ];
               if ( pChunk->IsElement() )
               {
                  nFound++;
               }
               nStep++;
            }
        }
        return (CXMLElement *)pChunk;
    }

    CXMLElement *CXMLIndex::GetElement( const char *strElementType, size_t nCount ) const
    {
        size_t          nIndex = 0;
        CString         strEleType = strElementType;
        CXMLElement    *pCompare;
        CXMLChunk      *pChunk;

        strEleType.Trim();

        while ( nIndex < m_aElements.Length() )
        {
            pChunk = (CXMLChunk *)m_aElements[ nIndex ];
            if ( pChunk->IsElement() )
            {
                pCompare = (CXMLElement *)pChunk;
                if ( strEleType == pCompare->GetName() )
                {
                     if ( nCount > 0 )
                     {
                         nCount--;
                     }
                     else
                     {
                         return pCompare;
                     }
                }
            }
            nIndex++;
        }

        return NULL;
    }

    void CXMLIndex::AddElement( CXMLElement *pNew, size_t nCount )
    {
        if ( ( nCount == IASLib::NOT_FOUND ) || ( nCount > m_aElements.GetCount() ) )
        {
            m_aElements.Append( pNew );
        }
        else
        {
            m_aElements.Insert( nCount, pNew );
        }
    }

    bool CXMLIndex::DeleteElement( size_t nCount )
    {
        if ( nCount < m_aElements.GetCount() )
        {
            m_aElements.Delete( nCount );
            return true;
        }
        return false;
    }

    size_t CXMLIndex::GetElementCount( void ) const
    {
        size_t nCount = 0;
        size_t nTotal = 0;

        while ( nCount < m_aElements.Length() )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[nCount];
            if ( pChunk->IsElement() ) 
            {
                nTotal++;
            }
            nCount++;
        }
        return nTotal;
    }

    size_t CXMLIndex::GetChunkCount( void ) const
    {
        return m_aElements.Length();
    }

    size_t CXMLIndex::GetElementCount( const char *strElementType ) const
    {
        size_t          nCount = 0;
        size_t          nRetVal = 0;
        CString         strEleType = strElementType;
        CXMLElement    *pCompare;
        CXMLChunk      *pChunk;

        strEleType.Trim();

        while ( nCount < m_aElements.Length() )
        {
            pChunk = (CXMLChunk *)m_aElements[ nCount ];
            if ( pChunk->IsElement() )
            {
                pCompare = (CXMLElement *)pChunk;
                if ( strEleType == pCompare->GetName() )
                {
                    nRetVal++;
                }
            }
            nCount++;
        }

        return nRetVal;
    }

    void CXMLIndex::AddData( CXMLData *pData )
    {
        m_aElements.Append( pData );
    }

    size_t CXMLIndex::GetDataCount( void ) const
    {
        size_t nCount = 0;
        size_t nTotal = 0;

        while ( nCount < m_aElements.Length() )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[nCount];
            if ( pChunk->IsData() ) 
            {
                nTotal++;
            }
            nCount++;
        }
        return nTotal;
    }

    CXMLData *CXMLIndex::GetData( size_t nCount ) const
    {
        CXMLChunk  *pChunk = NULL;

        if ( nCount < GetDataCount() )
        {
            size_t      nStep = 0;
            size_t      nFound = 0;

            while ( nFound < ( nCount + 1 ) )
            {
               pChunk = (CXMLChunk *)m_aElements[ nStep ];
               if ( pChunk->IsData() )
               {
                  nFound++;
               }
               nStep++;
            }
        }
        return (CXMLData *)pChunk;
    }



} // namespace IASLib

