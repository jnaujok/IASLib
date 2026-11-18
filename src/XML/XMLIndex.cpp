/*
 * XML Index Class
 *
 *      The top level of an XML document.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLIndex.h"
#include "XMLParser.h"
#include "FileException.h"

#include <memory>

namespace IASLib
{
    IMPLEMENT_OBJECT( CXMLIndex, CObject )

    CXMLIndex::CXMLIndex( void )
    {
    }

    CXMLIndex::CXMLIndex( const char *strFileName, CFile *pFile )
    {
        m_strXMLFileName = strFileName;

        std::unique_ptr<CFile> pOwned;
        if ( pFile == NULL )
        {
            try
            {
                pOwned.reset( new CFile( m_strXMLFileName, CFile::READ ) );
            }
            catch ( CFileException & )
            {
                return;     // a file that cannot be opened gives an empty index
            }
            pFile = pOwned.get();
        }
        if ( ! pFile->IsOpen() )
            return;

        std::unique_ptr<CXMLDocument> pDocument( CXMLParser::parseFile( pFile ) );
        pFile->Close();
        TakeFrom( *pDocument->GetIndex() );
    }

    CXMLIndex::~CXMLIndex( void )
    {
        m_aElements.DeleteAll();
    }

    void CXMLIndex::Clear( void )
    {
        m_aElements.DeleteAll();
    }

    void CXMLIndex::TakeFrom( CXMLIndex &oOther )
    {
        if ( &oOther == this )
            return;
        for ( size_t n = 0; n < oOther.m_aElements.Length(); n++ )
            m_aElements.Append( oOther.m_aElements.Get( n ) );
        oOther.m_aElements.EmptyAll();
    }

    CXMLChunk *CXMLIndex::GetChunk( size_t nCount ) const
    {
        return (CXMLChunk *)m_aElements.Get( nCount );
    }

    size_t CXMLIndex::GetChunkCount( void ) const
    {
        return m_aElements.Length();
    }

    void CXMLIndex::AddChunk( CXMLChunk *pChunk )
    {
        if ( pChunk != NULL )
            m_aElements.Append( pChunk );
    }

    CXMLElement *CXMLIndex::GetElement( size_t nCount ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[n];
            if ( pChunk->IsElement() && nFound++ == nCount )
                return (CXMLElement *)pChunk;
        }
        return NULL;
    }

    CXMLElement *CXMLIndex::GetElement( const char *strElementType, size_t nCount ) const
    {
        CString strName( strElementType );
        strName.Trim( " \t\r\n" );
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[n];
            if ( pChunk->IsElement() && strName == ((CXMLElement *)pChunk)->GetName() && nFound++ == nCount )
                return (CXMLElement *)pChunk;
        }
        return NULL;
    }

    CXMLElement *CXMLIndex::GetRoot( void ) const
    {
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[n];
            if ( pChunk->IsElement() && ((CXMLElement *)pChunk)->IsPlainElement() )
                return (CXMLElement *)pChunk;
        }
        return NULL;
    }

    size_t CXMLIndex::GetElementCount( void ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
            if ( ((CXMLChunk *)m_aElements[n])->IsElement() )
                nFound++;
        return nFound;
    }

    size_t CXMLIndex::GetElementCount( const char *strElementType ) const
    {
        CString strName( strElementType );
        strName.Trim( " \t\r\n" );
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[n];
            if ( pChunk->IsElement() && strName == ((CXMLElement *)pChunk)->GetName() )
                nFound++;
        }
        return nFound;
    }

    void CXMLIndex::AddElement( CXMLElement *pNew, size_t nCount )
    {
        if ( pNew == NULL )
            return;
        if ( nCount == IASLib::NOT_FOUND || nCount >= m_aElements.GetCount() )
            m_aElements.Append( pNew );
        else
            m_aElements.Insert( nCount, pNew );
    }

    bool CXMLIndex::DeleteElement( size_t nCount )
    {
        return m_aElements.Delete( nCount );
    }

    void CXMLIndex::AddData( CXMLData *pData )
    {
        if ( pData != NULL )
            m_aElements.Append( pData );
    }

    CXMLData *CXMLIndex::GetData( size_t nCount ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aElements[n];
            if ( pChunk->IsData() && nFound++ == nCount )
                return (CXMLData *)pChunk;
        }
        return NULL;
    }

    size_t CXMLIndex::GetDataCount( void ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
            if ( ((CXMLChunk *)m_aElements[n])->IsData() )
                nFound++;
        return nFound;
    }
} // namespace IASLib
