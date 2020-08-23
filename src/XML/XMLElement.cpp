/*
 * XML Element Class
 *
 *      This class stores an XML Element within an XML index. Note
 * that element relationships can be recursive, so the entire XML file
 * is considered as the first XML Element. Further elements are then
 * added to this initial element. Each of those elements may have other
 * elements and so forth. XML is highly receptive of exactly this format.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLElement.h"
#include "XMLData.h"
#include "XMLException.h"
#include "StringTokenizer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CXMLElement, CXMLChunk )

    CXMLElement::CXMLElement( )
    {
        m_bCommentTag = false;
        m_bMetaTag = false;
        m_bHasClosingTag = true;
        m_bSelfClosingTag = false;
    }

    CXMLElement::CXMLElement( CString &strTag )
    {
        CString strWork = strTag;
        CString strWholeTag;
        CString strStartTag;
        CString strTagData;
        CString strEndTag;
        m_bCommentTag = false;
        m_bMetaTag = false;
        m_bHasClosingTag = true;
        m_bSelfClosingTag = false;

        strWork.Trim();

        if ( ( strWork[0] != '<' ) || ( strWork.IndexOf( '>' ) == NOT_FOUND ) )
        {
                // There are no tags within this block. That's bad, because we never should
                // have called this constructor if there were no tags...
            // To Do: Really cool error handling.
            IASLIB_THROW_XML_EXCEPTION( "Unclosed tag! No closing brace found in data!" )
            return;
        }

            // Well, there's some kind of tag in here, so let's try and find the highest
            // level tag that we can find. Ideally, it will wrap the whole string, but
            // we can't really be sure of that, so we'll have to be careful about how
            // we do this. If it doesn't wrap the whole thing, we're going to have to
            // return a modified string.


            // In any case, the first thing we have to do is pull out the first tag, this should
            // be the tag we're working on. We only care about what's inside the tag markers
            // though.
        strStartTag = strWork.Substring( 1, (int)strWork.IndexOf( '>' ) - 1  );

		    // Okay, we've found the first tag part, let's take it off of the working string
		    // as a whole, so we're only working with the remainder of the tag.
        strWork = strWork.Substring( strWork.IndexOf( '>' ) + 1 );

            // Let's trim leading/trailing white space off the tag contents.
        strStartTag.Trim();

        if ( ( strStartTag[0] == '?' ) && ( strStartTag[ strStartTag.GetLength() - 1 ] == '?' ) )
        {
                // This is a Processing Instruction (PI) tag like <?xml version="1.0"?>. The most important part is, it
                // has no matching end tag.
                // We remove the question marks, and set the single tag flag.
            strStartTag = strStartTag.Substring( 1, (int)strStartTag.GetLength() - 2 );
            strStartTag.Trim();
            m_bSelfClosingTag = true;
            m_bMetaTag = true;
        }

        if ( strStartTag[ strStartTag.GetLength() - 1 ] == '/' )
        {
                // This is an "Empty" tag. In other words, it contains no data.
            strStartTag = strStartTag.Substring( 0, (int)strStartTag.GetLength() - 1 );
            strStartTag.Trim();
            m_bSelfClosingTag = true;
            m_bHasClosingTag = false;
        }

        if ( strStartTag.Substring( 0, 3 ) == "!--" )
        {
                // This is a comment tag, and it has no structure whatsoever.
            m_bCommentTag = true;
            m_bSelfClosingTag = true;
            m_bHasClosingTag = false;
            m_strElementName = "!--";
            if ( strStartTag.IndexOf( "--", 3 ) == NOT_FOUND )
            {
                CString strTempData;
                    // The end tag wasn't grabbed, let's find it.
                strTempData = strStartTag.Substring( 3 );
                strTempData += ">";
                size_t nEndComment = strWork.IndexOf( "-->" );
                if ( nEndComment != NOT_FOUND )
                {
                    strTempData += strWork.Substring( 0, nEndComment );
                    strWork = strWork.Substring( nEndComment + 3 );
                    strTag = strWork;
                }
                else
                {
                    strTempData += strWork;
                    strTag = "";
                }
                AddData( strTempData );
            }
            else
            {
                CString strTempData;
                strTempData = strStartTag.Substring( 3, (int)strStartTag.GetLength() - 5 );
                AddData( strTempData );
                strTag = strWork;
            }
            return;
        }

        if ( strStartTag[0] == '!' )
        {
                // This is an element type declaration. It'd be nice if we actually understood this and enforced it,
                // but there's always later...
                // We remove the exclamation point (bang)
            strStartTag = strStartTag.Substring( 1 );
            strStartTag.Trim();
                // Now, let's tokenize our starting tag just so we can grab the tag name.
            CStringTokenizer    stDeclares( strStartTag );

                // Whether we have white space or not, the first token has to be the name of the tag.
                // let's grab it.
            m_strElementName = stDeclares.NextToken();
            m_strElementName.Trim();

                // Then we just store the rest of it in the data element.
            strStartTag = strStartTag.Substring( m_strElementName.GetLength() );

            AddData( strStartTag );

            strTag = strWork;

            return;
        }

            // Now, let's tokenize our starting tag just so we can grab the tag name.
        CStringTokenizer    stTokens( strStartTag );

            // Whether we have white space or not, the first token has to be the name of the tag.
            // let's grab it.
        m_strElementName = stTokens.NextToken();
        m_strElementName.Trim();

		    // Let's remove the tag name from the tag, what's left should be either white-space,
		    // property-value tags, or nothing at all.
	    strStartTag = strStartTag.Substring( m_strElementName.GetLength() );

        if ( ! m_bSelfClosingTag )
        {
                // Now, let's find the closing tag of this element. Remembering of course,
                // that XML is case-sensitive. We also have to remember that we may have
                // embedded elements of the same name as this one.
            strEndTag.Format( "</%s>", (const char *)m_strElementName );
            CString	strStartTag1;
            CString	strStartTag2;
            strStartTag1.Format( "<%s>", (const char *)m_strElementName );
            strStartTag2.Format( "<%s ", (const char *)m_strElementName );

            size_t nFound = 0;
            size_t  nStart = 0;
            size_t 	nStart1;
            size_t  nStart2;
            bool bDone = false;

            do
            {
                nFound = strWork.IndexOf( strEndTag, nFound, false );
                nStart1 = strWork.IndexOf( strStartTag1, nStart, false );
                nStart2 = strWork.IndexOf( strStartTag2, nStart, false );

                while ( nStart2 != NOT_FOUND )
                {
                        // Make sure it's not an "Empty Tag" start tag.
                    if (( strWork.IndexOf( "/>", nStart2 ) != NOT_FOUND ) &&
                        strWork.IndexOf('>', nStart2) >= strWork.IndexOf("/>", nStart2))
                    {
                            // It is an empty tag, ignore it and look for the next occurrence.
                        nStart2 = strWork.IndexOf( strStartTag2, nStart2 + strStartTag2.GetLength() , false );
                    }
                    else
                    {
                            // It's not an empty tag, break out of the loop.
                        break;
                    }
                }

                if ( nStart1 == NOT_FOUND )
                    nStart1 = strWork.GetLength() + 1;
                if ( nStart2 == NOT_FOUND )
                    nStart2 = strWork.GetLength() + 1;

                nStart = ( nStart1 < nStart2 ) ? nStart1 : nStart2;

                if ( nStart < nFound )
                {
                    nFound += strEndTag.GetLength();
                    nStart += strStartTag2.GetLength();
                }
                else
                    bDone = true;
            } while ( ! bDone );

            if ( nFound == NOT_FOUND )
            {
                    // Error! No end tag found for this tag. That's bad.
                CString strTemp;
                strTemp.Format( "Bad tag, no ending tag found for tag <%s>.\n", (const char *)m_strElementName );
                strTag = "";
                IASLIB_THROW_XML_EXCEPTION( strTemp )
            }

                // Okay, we have the end tag for this element. Whatever is left must not be a part
                // of this particular element. Let's shorten the "strTag" and put what's between
                // the tags into the TagData element.
            strTag = strWork.Substring( nFound + strEndTag.GetLength() );
            strTagData = strWork.Substring( 0, nFound );
        }
        else
        {
            strTag = strWork;
        }

            // Now that we've got all the parts of our XML tag pulled apart, let's fill in
            // the properties of our element.
        while ( strStartTag.GetLength() )
        {
			    // Start by trimming to make sure there's no white-space at the start.
		    strStartTag.Trim();

			    // We should always be starting with a property name.
			    // What we need to look for is the gap between the value and the next property
			    // to end it.
            bool    bFoundEquals = false;
            bool    bFoundValue = false;
		    bool	bFoundEnd = false;
		    bool	bInQuotes = false;
            bool    bHadQuotes = false;
            int     nIndex = 0;
            int     nValueStart = 0;
            CString strPropName;
            CString strPropValue;

            while ( nIndex < (int)strStartTag.GetLength() )
            {
                if ( ! bFoundEquals )
                {
                    if ( strStartTag[ nIndex] == '=' )
                    {
                        bFoundEquals = true;
                        strPropName = strStartTag.Substring( 0, nIndex );
                    }
                }
                else
                {
                    if ( strStartTag[ nIndex ] == '"' )
                    {
                        bInQuotes = ! bInQuotes;
                        bHadQuotes = true;
						    // If this is the end quote, then we've found our value.
					    if ( ! bInQuotes )
					    {
						    bFoundEnd = true;
						    strPropValue = strStartTag.Substring( nValueStart, nIndex - nValueStart );
					    }
                    }
                    else
                    {
                        if ( ( strStartTag[ nIndex ] == ' ' ) || ( strStartTag[nIndex] == '\t' ) || ( strStartTag[nIndex] == '\n' ) )
                        {
                            if ( ( bFoundValue ) && ( ! bInQuotes ) )
                            {
                                if ( bHadQuotes )
                                {
                                    strPropValue = strStartTag.Substring( nValueStart, nIndex - nValueStart - 1 );
                                }
                                else
                                {
                                    strPropValue = strStartTag.Substring( nValueStart, nIndex - nValueStart );
                                }
                                bFoundEnd = true;
                                break;
                            }
                        }
                        else
                        {
                            if ( ! bFoundValue )
                            {
                                bFoundValue = true;
                                nValueStart = nIndex;
                            }
                        }
                    }
                }
                nIndex++;
            }

            if ( ( bFoundValue ) && ( bFoundEquals ) )
            {
                    // Check if we ran out of tag before we ran out of value (i.e. the value ran to
                    // the end of the tag.
                if ( ! bFoundEnd )
                {
                    strPropValue = strStartTag.Substring( nValueStart );
                }

                auto *pNewProp = new CXMLProperty( strPropName, strPropValue );
                m_aProperties.Push( pNewProp );
            }

                // Cut the stuff we looked at off the start tag.
            strStartTag = strStartTag.Substring( nIndex + 1 );
        }

        if ( ! m_bSelfClosingTag )
        {
                // Okay, now that we've analyzed the start tag properties, it's time to
                // tear apart the data in the tag.
            while ( strTagData.GetLength() )
            {
                strTagData.Trim();

                if ( strTagData.IndexOf( '<' ) != NOT_FOUND )
                {
                    CString strTemp = strTagData.Substring( 0, (int)strTagData.IndexOf( '<' ) );
                    strTagData = strTagData.Substring( strTagData.IndexOf( '<' ) );
                    strTemp.Trim();

                    if ( strTemp.GetLength() > 0 )
                    {
                        AddData( strTemp );
                    }

                    auto *pNewSubElement = new CXMLElement( strTagData );
                    m_aSubElements.Push( pNewSubElement );
                }
                else
                {
                    if ( strTagData.GetLength() > 0 )
                    {
                        AddData( strTagData );
                        strTagData = "";
                    }
                }
            }
        }
    }

    CXMLElement::~CXMLElement( )
    {
        m_aProperties.DeleteAll();
        m_aSubElements.DeleteAll();
    }

    CXMLProperty *CXMLElement::GetProperty( size_t nCount ) const
    {
        if  ( nCount < m_aProperties.Length() )
        {
            return (CXMLProperty *)m_aProperties[ nCount ];
        }
        return nullptr;
    }

    const char *CXMLElement::GetProperty( const char *strPropertyName ) const
    {
        size_t nIndex = 0;
        CString strPropName = strPropertyName;

        strPropName.Trim();

        while ( nIndex < m_aProperties.Length() )
        {
            auto *pCompare = (CXMLProperty *)m_aProperties[ nIndex ];
            CString strSearch = pCompare->GetName();

            if ( strPropName == strSearch )
            {
                return pCompare->GetValue();
            }
            nIndex++;
        }

        return nullptr;
    }

    const char *CXMLElement::GetData( size_t nCount ) const
    {
        size_t nStep = 0;
        size_t nFound = 0;

        while ( nStep < m_aSubElements.GetCount() )
        {
            auto *pChunk = (CXMLChunk *)m_aSubElements.Get( nStep );
            if ( pChunk->IsData() )
            {
                if ( nFound == nCount )
                {
                    auto *pData = (CXMLData *)pChunk;
                    return pData->GetData();
                }
                nFound++;
            }
            nStep++;
        }
        return nullptr;
    }

    size_t CXMLElement::GetSubElementCount( ) const
    {
        size_t          nCount = 0;
        size_t          nRetVal = 0;
        CXMLChunk      *pTemp;

        while ( nCount < m_aSubElements.Length() )
        {
            pTemp = (CXMLChunk *)m_aSubElements[ nCount ];
            if ( pTemp->IsElement() )
            {
                nRetVal++;
            }
            nCount++;
        }

        return nRetVal;
    }

    size_t CXMLElement::GetSubElementCount( const char *strElementType ) const
    {
        size_t          nCount = 0;
        size_t          nRetVal = 0;
        CString         strEleType = strElementType;
        CXMLChunk      *pTemp;
        CXMLElement    *pCompare;

        strEleType.Trim();

        while ( nCount < m_aSubElements.Length() )
        {
            pTemp = (CXMLChunk *)m_aSubElements[ nCount ];
            if ( pTemp->IsElement() )
            {
                pCompare = (CXMLElement *)pTemp;
                if ( strEleType == pCompare->m_strElementName )
                {
                    nRetVal++;
                }
            }
            nCount++;
        }

        return nRetVal;
    }

    const CXMLElement *CXMLElement::GetSubElement( size_t nElement ) const
    {
        size_t          nCount = 0;
        size_t          nFound = 0;
        CXMLChunk      *pTemp;
        CXMLElement    *pCompare;

        while ( nCount < m_aSubElements.Length() )
        {
            pTemp = (CXMLChunk *)m_aSubElements[ nCount ];
            if ( pTemp->IsElement() )
            {
                if ( nFound == nElement )
                {
                    pCompare = (CXMLElement *)pTemp;
                    return pCompare;
                }
                nFound++;
            }
            nCount++;
        }

        return nullptr;
    }

    const CXMLElement *CXMLElement::GetSubElement( const char *strElementType, size_t nIndex ) const
    {
        size_t          nCount = 0;
        size_t          nFound = 0;
        CString         strEleType = strElementType;
        CXMLChunk      *pTemp;
        CXMLElement    *pCompare;

        strEleType.Trim();

        while ( nCount < m_aSubElements.Length() )
        {
            pTemp = (CXMLChunk *)m_aSubElements[ nCount ];
            if ( pTemp->IsElement() )
            {
                pCompare = (CXMLElement *)pTemp;
                if ( strEleType == pCompare->m_strElementName )
                {
                    if ( nFound == nIndex )
                    {
                        return pCompare;
                    }
                    nFound++;
                }
            }
            nCount++;
        }

        return nullptr;
    }

    void CXMLElement::SetName( const CString& strTagName )
    {
        m_strElementName = strTagName;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    void CXMLElement::SetData( const CString& strData, size_t nDataElement )
    {
        size_t  nCount = 0;
        size_t  nFound = 0;

        while ( nCount < m_aSubElements.GetCount() )
        {
            auto *pChunk = (CXMLChunk *)m_aSubElements.Get( nCount );

            if ( pChunk->IsData() )
            {
                nFound++;
                if ( nFound > nDataElement )
                {
                    auto *pData = (CXMLData *)pChunk;

                    pData->SetData( strData );

                    return;
                }
            }
            nCount++;
        }

        auto *pData = new CXMLData( strData );
        m_aSubElements.Append( pData );
    }
#pragma clang diagnostic pop

    void CXMLElement::AddProperty( CXMLProperty *pNewProperty )
    {
        m_aProperties.Append( pNewProperty );
    }

    void CXMLElement::AddSubElement( CXMLElement *pNewElement )
    {
        m_aSubElements.Append( pNewElement );
    }

    void CXMLElement::AddData( const CString& strData )
    {
        auto *pDataChunk = new CXMLData( strData );
        m_aSubElements.Append( pDataChunk );
    }

    size_t CXMLElement::GetDataCount( ) const
    {
        size_t          nCount = 0;
        size_t          nRetVal = 0;
        CXMLChunk      *pTemp;

        while ( nCount < m_aSubElements.Length() )
        {
            pTemp = (CXMLChunk *)m_aSubElements[ nCount ];
            if ( pTemp->IsData() )
            {
                nRetVal++;
            }
            nCount++;
        }

        return nRetVal;
    }

    CString CXMLElement::toString( int offset, int indent) const {
        bool hadInternalData = false;
        CString element;
        element = element.Pad( offset, ' ', false );

        if ( m_bCommentTag )
        {
            element += "<!--";
            element += GetData();
            element += "-->";
            if ( indent > 0 ){
                element += "\n";
            }
        }
        else if ( m_bMetaTag )
        {
            element += "<?";
            element += GetName();
            for ( int nProp = 0; nProp < GetPropertyCount(); nProp ++ )
            {
                auto prop = GetProperty(nProp);
                element += " ";
                element += prop->GetName();
                element += "=\"";
                element += prop->GetValue();
                element += "\"";
            }
            element += "?>";
            if ( indent > 0 ){
                element += "\n";
            }
        }
        else
        {
            element += "<";
            element += GetName();
            for ( int nProp = 0; nProp < GetPropertyCount(); nProp ++ )
            {
                auto prop = GetProperty(nProp);
                element += " ";
                element += prop->GetName();
                element += "=\"";
                element += prop->GetValue();
                element += "\"";
            }
            if ( m_bSelfClosingTag ){
                element += "/>";
                if ( indent > 0 ){
                    element += "\n";
                }
            }
            else {
                element += ">";
            }

            if ( ( ! m_bSelfClosingTag ) && ( m_aSubElements.Length() > 0 )) {
                for (int nSub = 0; nSub < m_aSubElements.Length(); nSub++ )
                {
                    auto chunk = (CXMLChunk *)m_aSubElements[nSub];
                    CString temp = chunk->toString(offset + indent, indent);
                    if ( (indent != 0 ) && ( chunk->IsElement() || ( chunk->IsData() && temp.GetLength() > 0 && !hadInternalData ) ) )
                    {
                        element += "\n";
                        if ( chunk->IsData() ) {
                            hadInternalData = true;
                        }
                    }
                    element += temp;
                }
            }
            if ( m_bHasClosingTag )
            {
                if ( indent > 0 && hadInternalData ) {
                    element += "\n";
                }
                element += CString("").Pad( offset );
                element += "</";
                element += GetName();
                element += ">";
                if ( indent > 0 ){
                    element += "\n";
                }
            }
        }

        if ( offset > 0 ) {
            element = element.Pad(offset, ' ', false);
        }
        return element;
    }

} // namespace IASLib

