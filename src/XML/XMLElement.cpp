/*
 * XML Element Class
 *
 *      An XML element, comment, processing instruction or declaration.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLElement.h"
#include "XMLData.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CXMLElement, CXMLChunk )

    namespace
    {
        CString Normalized( const char *strName )
        {
            CString strNormal( strName );
            strNormal.Trim( " \t\r\n" );
            return strNormal;
        }
    }

    void CXMLElement::Init( void )
    {
        m_bCommentTag = false;
        m_bMetaTag = false;
        m_bDeclaration = false;
        m_bHasClosingTag = true;
        m_bSelfClosingTag = false;
    }

    CXMLElement::CXMLElement( void )
    {
        Init();
    }

    CXMLElement::CXMLElement( const char *strTagName )
    {
        Init();
        m_strElementName = strTagName;
    }

    CXMLElement::CXMLElement( const CString &strTagName, const CString &strData )
    {
        Init();
        m_strElementName = strTagName;
        if ( ! strData.IsEmpty() )
            AddData( strData );
    }

    CXMLElement::~CXMLElement( void )
    {
        m_aProperties.DeleteAll();
        m_aSubElements.DeleteAll();
    }

    CXMLChunk *CXMLElement::Clone( void ) const
    {
        CXMLElement *pCopy = new CXMLElement( (const char *)m_strElementName );
        pCopy->m_strRawContent = m_strRawContent;
        pCopy->m_bCommentTag = m_bCommentTag;
        pCopy->m_bMetaTag = m_bMetaTag;
        pCopy->m_bDeclaration = m_bDeclaration;
        pCopy->m_bHasClosingTag = m_bHasClosingTag;
        pCopy->m_bSelfClosingTag = m_bSelfClosingTag;
        for ( size_t n = 0; n < m_aProperties.Length(); n++ )
        {
            const CXMLProperty *pProperty = (const CXMLProperty *)m_aProperties[n];
            pCopy->m_aProperties.Append( new CXMLProperty( pProperty->GetName(), pProperty->GetValue() ) );
        }
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
            pCopy->m_aSubElements.Append( ((const CXMLChunk *)m_aSubElements[n])->Clone() );
        return pCopy;
    }

    void CXMLElement::SetName( const CString& strTagName )
    {
        m_strElementName = strTagName;
    }

        // Properties

    CXMLProperty *CXMLElement::GetProperty( size_t nCount ) const
    {
        return (CXMLProperty *)m_aProperties.Get( nCount );
    }

    const char *CXMLElement::GetProperty( const char *strPropertyName ) const
    {
        CString strName = Normalized( strPropertyName );
        for ( size_t n = 0; n < m_aProperties.Length(); n++ )
        {
            const CXMLProperty *pProperty = (const CXMLProperty *)m_aProperties[n];
            if ( strName == pProperty->GetName() )
                return pProperty->GetValue();
        }
        return NULL;
    }

    void CXMLElement::SetProperty( const char *strPropertyName, const char *strValue )
    {
        CString strName = Normalized( strPropertyName );
        for ( size_t n = 0; n < m_aProperties.Length(); n++ )
        {
            CXMLProperty *pProperty = (CXMLProperty *)m_aProperties[n];
            if ( strName == pProperty->GetName() )
            {
                pProperty->SetValue( strValue );
                m_strRawContent.Clear();    // a meta tag is now written from its properties
                return;
            }
        }
        AddProperty( new CXMLProperty( strName, strValue ) );
    }

    void CXMLElement::AddProperty( CXMLProperty *pNewProperty )
    {
        if ( pNewProperty == NULL )
            return;
        m_aProperties.Append( pNewProperty );
            // A meta tag is now written from its properties.
        m_strRawContent.Clear();
    }

    bool CXMLElement::DeleteProperty( const char *strPropertyName )
    {
        CString strName = Normalized( strPropertyName );
        for ( size_t n = 0; n < m_aProperties.Length(); n++ )
        {
            if ( strName == ((const CXMLProperty *)m_aProperties[n])->GetName() )
            {
                m_aProperties.Delete( n );
                m_strRawContent.Clear();
                return true;
            }
        }
        return false;
    }

        // Sub elements

    size_t CXMLElement::GetSubElementCount( void ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
            if ( ((const CXMLChunk *)m_aSubElements[n])->IsElement() )
                nFound++;
        return nFound;
    }

    size_t CXMLElement::GetSubElementCount( const char *strElementType ) const
    {
        CString strName = Normalized( strElementType );
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            const CXMLChunk *pChunk = (const CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsElement() && strName == ((const CXMLElement *)pChunk)->GetName() )
                nFound++;
        }
        return nFound;
    }

    CXMLElement *CXMLElement::GetSubElement( size_t nElement )
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsElement() && nFound++ == nElement )
                return (CXMLElement *)pChunk;
        }
        return NULL;
    }

    CXMLElement *CXMLElement::GetSubElement( const char *strElementType, size_t nIndex )
    {
        CString strName = Normalized( strElementType );
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsElement() && strName == ((CXMLElement *)pChunk)->GetName() && nFound++ == nIndex )
                return (CXMLElement *)pChunk;
        }
        return NULL;
    }

    const CXMLElement *CXMLElement::GetSubElement( size_t nElement ) const
    {
        return const_cast<CXMLElement *>( this )->GetSubElement( nElement );
    }

    const CXMLElement *CXMLElement::GetSubElement( const char *strElementType, size_t nIndex ) const
    {
        return const_cast<CXMLElement *>( this )->GetSubElement( strElementType, nIndex );
    }

    void CXMLElement::AddSubElement( CXMLElement *pNewElement )
    {
        if ( pNewElement != NULL )
            m_aSubElements.Append( pNewElement );
    }

    bool CXMLElement::DeleteSubElement( size_t nElement )
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            if ( ((const CXMLChunk *)m_aSubElements[n])->IsElement() && nFound++ == nElement )
                return m_aSubElements.Delete( n );
        }
        return false;
    }

        // Text

    const char *CXMLElement::GetData( size_t nCount ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            const CXMLChunk *pChunk = (const CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsData() && nFound++ == nCount )
                return ((const CXMLData *)pChunk)->GetData();
        }
        return NULL;
    }

    size_t CXMLElement::GetDataCount( void ) const
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
            if ( ((const CXMLChunk *)m_aSubElements[n])->IsData() )
                nFound++;
        return nFound;
    }

    CString CXMLElement::GetText( void ) const
    {
        CString strText;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            const CXMLChunk *pChunk = (const CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsData() )
                strText += ((const CXMLData *)pChunk)->GetData();
        }
        return strText;
    }

    CString CXMLElement::GetChildText( const char *strElementType ) const
    {
        const CXMLElement *pChild = GetSubElement( strElementType );
        if ( pChild == NULL )
            return CString();
        CString strText = pChild->GetText();
        strText.Trim( " \t\r\n" );
        return strText;
    }

    void CXMLElement::SetData( const CString& strData, size_t nDataElement )
    {
        size_t nFound = 0;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            CXMLChunk *pChunk = (CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsData() && nFound++ == nDataElement )
            {
                ((CXMLData *)pChunk)->SetData( strData );
                return;
            }
        }
        AddData( strData );
    }

    void CXMLElement::AddData( const CString& strData, bool bCData )
    {
        m_aSubElements.Append( new CXMLData( strData, bCData ) );
    }

        // Writing

    CString CXMLElement::StartTag( void ) const
    {
        CString strTag;
        for ( size_t n = 0; n < m_aProperties.Length(); n++ )
        {
            const CXMLProperty *pProperty = (const CXMLProperty *)m_aProperties[n];
            strTag += " ";
            strTag += pProperty->GetName();
            strTag += "=\"";
            strTag += EscapeAttribute( pProperty->GetValue() );
            strTag += "\"";
        }
        return strTag;
    }

    void CXMLElement::Write( CString &strOut, int offset, int indent, int nDepth ) const
    {
        CString strPad;
        if ( indent > 0 && offset > 0 )
            strPad = CString().Pad( (size_t)offset );
        const char *pszEnd = ( indent > 0 ) ? "\n" : "";

        if ( m_bCommentTag )
        {
            strOut += strPad;
            strOut += "<!--";
            strOut += GetText();
            strOut += "-->";
            strOut += pszEnd;
            return;
        }
        if ( m_bMetaTag )
        {
            strOut += strPad;
            strOut += "<?";
            strOut += m_strElementName;
            strOut += m_strRawContent.IsEmpty() ? StartTag() : m_strRawContent;
            strOut += "?>";
            strOut += pszEnd;
            return;
        }
        if ( m_bDeclaration )
        {
            strOut += strPad;
            strOut += "<!";
            strOut += m_strElementName;
            strOut += m_strRawContent;
            strOut += ">";
            strOut += pszEnd;
            return;
        }

        strOut += strPad;
        strOut += "<";
        strOut += m_strElementName;
        strOut += StartTag();

        bool bHasContent = false;
        for ( size_t n = 0; n < m_aSubElements.Length() && ! bHasContent; n++ )
        {
            const CXMLChunk *pChunk = (const CXMLChunk *)m_aSubElements[n];
                // When pretty-printing, white space alone is not content.
            bHasContent = pChunk->IsElement() || indent == 0 || ((const CXMLData *)pChunk)->IsCData() || ! ((const CXMLData *)pChunk)->IsWhitespace();
        }

        if ( ! bHasContent && ( m_bSelfClosingTag || indent > 0 ) )
        {
            if ( m_bSelfClosingTag || m_aSubElements.Length() == 0 )
            {
                strOut += "/>";
            }
            else
            {
                strOut += "></";
                strOut += m_strElementName;
                strOut += ">";
            }
            strOut += pszEnd;
            return;
        }

        strOut += ">";
        strOut += pszEnd;
        for ( size_t n = 0; n < m_aSubElements.Length(); n++ )
        {
            const CXMLChunk *pChunk = (const CXMLChunk *)m_aSubElements[n];
            if ( pChunk->IsElement() )
                ((const CXMLElement *)pChunk)->Write( strOut, offset + indent, indent, nDepth + 1 );
            else
                strOut += pChunk->toString( offset + indent, indent );
        }
        strOut += strPad;
        strOut += "</";
        strOut += m_strElementName;
        strOut += ">";
        strOut += pszEnd;
    }

    CString CXMLElement::toString( int offset, int indent ) const
    {
        CString strOut;
        Write( strOut, ( offset > 0 ) ? offset : 0, ( indent > 0 ) ? indent : 0, 0 );
        return strOut;
    }
} // namespace IASLib
