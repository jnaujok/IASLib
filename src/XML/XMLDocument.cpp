/**
 * XML Document class
 *
 *		This class implements a high-speed parser for XML to provide a
 * fast and consistent means of loading an XML document from a stream.
 * Because streams can represent sockets, files, or other items, this will
 * improve the overall capability of the IAS Library versus the XML Index
 * class which can only load files.
 *		However, to retain compatibility, this class will actually load an
 * XML Index/Element/Property hierarchy as it runs. This should make any
 * conversion from the old system much simpler.
 *
 *	@author Jeffrey R. Naujok
 *  @created 02/10/2006
 */

#include "XMLDocument.h"
#include "XMLIndex.h"
#include "XMLElement.h"
#include "XMLProperty.h"
#include "XMLException.h"

namespace IASLib
{
    CXMLDocument::CXMLDocument( )
    {
        m_pInput = nullptr;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"
    CXMLDocument::CXMLDocument(CStream *pInput)
    {
        m_pInput = nullptr;
        if ( ! Read( pInput ) )
        {
            IASLIB_THROW_XML_EXCEPTION( "Could not read input stream." );
        }
    }
#pragma clang diagnostic pop

    CXMLDocument::~CXMLDocument( )
    {
        // Nothing to do here, the XMLIndex is self-cleaning, and
        // we shouldn't delete the stream. So, we just clear the
        // stream's pointer.
        m_pInput = nullptr;
    }

    bool CXMLDocument::Read( CStream *pInput )
    {
        m_pInput = pInput;

        char                        chCurrent;
        CString                     strCurrentData;
        CXMLElement                *pElement = nullptr;
        CXMLData                   *pData = nullptr;
        bool                        bTagOpen = false;
        CXMLDocument::TagReturns    trRetVal = CXMLDocument::OPEN_TAG;

        while ( ! m_pInput->IsEOS( ) )
        {
            chCurrent = m_pInput->GetChar();

            if (chCurrent == '<' ) {
                if (strCurrentData.GetLength() > 0) {
                    if (pElement == nullptr) {
                        pData = new CXMLData(strCurrentData);
                        m_xiIndex.AddData(pData);
                    } else {
                        pElement->AddData(strCurrentData);
                    }
                    strCurrentData = "";
                }

                m_stackTags.Push(pElement);
                if (!bTagOpen) {
                    pElement = new CXMLElement();
                    m_xiIndex.AddElement(pElement);

                    trRetVal = StartTag(*m_pInput, pElement);
                } else {
                    auto *pTemp = new CXMLElement();

                    trRetVal = StartTag(*m_pInput, pTemp);

                    if (trRetVal != CXMLDocument::END_TAG) {
                        pElement->AddSubElement(pTemp);

                        if (trRetVal != CXMLDocument::SELF_CLOSE) {
                            pElement = pTemp;
                        }
                    }
                }


                if (trRetVal == OPEN_TAG) {
                    bTagOpen = true;
                } else {
                    pElement = (CXMLElement *) m_stackTags.Pop();
                    if (pElement != nullptr) {
                        bTagOpen = true;
                    } else {
                        bTagOpen = false;
                    }
                }
            }
            else
            {
                strCurrentData += chCurrent;
                break;
            }
        }

        if ( bTagOpen )
        {
            IASLIB_THROW_XML_EXCEPTION( "Stream ended with an unclosed tag." );
        }

        return true;
    }

    size_t CXMLDocument::Write( CStream *pOutput, int indent )
    {
        size_t nLength = 0;
        int offset = 0;

        CStream &oStream = *pOutput;

        for ( int nTopLevel = 0; nTopLevel < m_xiIndex.GetChunkCount(); nTopLevel++ )
        {
            auto element = m_xiIndex.GetChunk(nTopLevel);
            auto strOut = element->toString( offset, indent );
            oStream << strOut;
        }

        oStream << "\n<!-- Written by IASLib -->\n";
        return nLength;
    }

    CXMLDocument::TagReturns  CXMLDocument::StartTag( CStream &oStream, CXMLElement *pTag )
    {
        CString strTagName;
        char    chChar;
        bool    bIsEndTag = false;
        bool    bTagFound = false;
        bool    bTagClosed = false;
        bool    bIsSelfClose = false;
        CString strPropertyName;

        while ( ( ! bTagFound ) && ( ! bTagClosed ) && ( ! oStream.IsEOS() ) )
        {
            oStream >> chChar;

            switch ( chChar )
            {
                    // stupid <?xml?> tag that doesn't match anything
                    // else in the XML standard.
                case '?':
                    if ( strTagName.GetLength() == 0 )
                    {
                        bIsSelfClose = true;
                        pTag->SetMetaTag();
                    }
                    else
                    {
                        oStream >> chChar;

                        if ( chChar != '>' )
                        {
                            IASLIB_THROW_XML_EXCEPTION( "No matching end tag found for <?" + strTagName );
                        }
                        pTag->SetName( strTagName );
                        bTagFound = true;
                        bTagClosed = true;
                    }
                    break;

                case '/':
                    if ( strTagName.GetLength() > 0 )
                    {
                        bIsSelfClose = true;
                    }
                    bIsEndTag = true;
                    break;

                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if ( strTagName.GetLength() == 0 )
                    {
                        // This is whitespace *berfore* the tag name
                        // horrible coding, but marginally allowed by
                        // the standard. We just ignore it.
                        break;
                    }

                    if ( strTagName.Substring( 0, 3 ) == "!--" )
                    {
                        pTag->SetComment();
                        while ( ( ! bTagClosed ) && ( ! oStream.IsEOS() ) )
                        {
                            oStream >> chChar;

                            if ( chChar == '>' )
                            {
                                if ( strTagName.Substring( strTagName.GetLength() - 2, 2 ) == "--" )
                                {
                                    bTagClosed = true;
                                    bIsSelfClose = true;
                                    bIsEndTag = true;
                                }
                                else
                                {
                                    strTagName += chChar;
                                }
                            }
                            else
                            {
                                strTagName += chChar;
                            }
                        }
                    }
                    if ( strTagName.GetLength() )
                    {
                        if ( ( bIsEndTag ) && ( ! pTag->IsComment() ) )
                        {
                            if ( strTagName != pTag->GetName() )
                            {
                                IASLIB_THROW_XML_EXCEPTION( "Mismatched tag names." );
                            }
                        }
                        else
                        {
                            pTag->SetName( strTagName );
                        }
                        bTagFound = true;
                    }
                    break;

                case '>':
                    bTagClosed = true;
                    if ( ! bTagFound )
                    {
                        if ( strTagName.GetLength() == 0 )
                        {
                            IASLIB_THROW_XML_EXCEPTION( "Tag is missing name!" );
                        }
                        bTagFound = true;
                    }
                    if ( ( bIsEndTag ) && ( ! bIsSelfClose ) )
                    {
                        // This is an end tag, we need to compare it to the last *open*
                        // tag, which should be on the top of the stack
                        CXMLElement *pTemp = (CXMLElement *)m_stackTags.Pop();

                        if ( strTagName != pTemp->GetName() )
                        {
                            CString strException = "Mismatched tag names [";
                            strException += strTagName;
                            strException += "] != [";
                            strException += pTemp->GetName();
                            strException += "]";
                            IASLIB_THROW_XML_EXCEPTION( strException );
                        }

                        delete pTag;
                        pTag = pTemp;
                    }
                    else
                    {
                        pTag->SetName( strTagName );
                    }
                    break;

                default:
                    strTagName += chChar;
                    break;
            }
        }

        if ( ! bTagFound )
        {
            IASLIB_THROW_XML_EXCEPTION( "Unclosed Tag Found." );
        }

        if ( bTagClosed )
        {
            if ( bIsEndTag )
            {
                if ( bIsSelfClose )
                {
                    return CXMLDocument::SELF_CLOSE;
                }
                else
                {
                    return CXMLDocument::END_TAG;
                }
            }
            return CXMLDocument::OPEN_TAG;
        }

        // If we get here, then we have a tag name with whitespace after it, implying that
        // we are about to run into some properties. So, here we start going through the
        // stream looking for the closing brace of the tag or the next property in a loop.
        strPropertyName.Clear();

        while ( ( ! bTagClosed ) && ( ! oStream.IsEOS() ) )
        {
            oStream >> chChar;

            switch ( chChar )
            {
                    // stupid <?xml?> tag that doesn't match anything
                    // else in the XML standard.
                case '?':
                    oStream >> chChar;

                    if ( chChar != '>' )
                    {
                        IASLIB_THROW_XML_EXCEPTION( "No matching end tag found for <?" + strTagName );
                    }
                    bTagFound = true;
                    bTagClosed = true;
                    bIsEndTag = true;
                    if ( strPropertyName.GetLength() != 0 )
                    {
                        // If we close a tag with a property name, then we add the property
                        // name to the tag with a blank value.
                        CXMLProperty *pNewProperty = new CXMLProperty( strPropertyName, "" );
                        pTag->AddProperty( pNewProperty );
                    }
                    break;

                case '=':
                    if ( strPropertyName.GetCount() )
                    {
                        if ( bIsEndTag )
                        {
                            IASLIB_THROW_XML_EXCEPTION( "Property found in XML End Tag." );
                        }
                        TagReturns trPropRet = StartProperty( oStream, pTag, strPropertyName );
                        if ( trPropRet == CXMLDocument::OPEN_TAG )
                        {
                            // We've gotten a property, but we haven't found the ending
                            // bracket for the tag, so we just prep for the next property
                            // name by clearing the current name out.
                            strPropertyName.Clear();
                        }
                        else
                        {
                            // Here, the property ended with the "/>" tag, so we can close
                            // out the tag at the same time. Note that if we get a
                            // self-close in an end tag (for that matter, if we get a
                            // property in an end tag) we have an error.
                            if ( trPropRet == CXMLDocument::SELF_CLOSE )
                            {
                                bTagClosed = true;
                                bIsSelfClose = true;
                                if ( bIsEndTag )
                                {
                                    IASLIB_THROW_XML_EXCEPTION( "Self close (/>) found in XML End Tag." );
                                }
                                // We have to pop our tag off the stack to restore
                                // the correct stack frame on the document.
                                m_stackTags.Pop();
                                bIsEndTag = true;
                            }
                            else
                            {
                                // The only other option is that the tag is closed
                                bTagClosed = true;
                            }
                        }
                    } // if ( strPropertyName.GetLength != 0 )
                    else
                    {
                        IASLIB_THROW_XML_EXCEPTION( "Found \"=\" without a property name in XML Tag." );
                    }
                    break;

                case '/':
                    // This may be a self-close mark. To know for sure, we need to check
                    // if the next character is the close brace. If not, we're going to
                    // need to throw an error, because this is a bad character in an XML
                    // tag.
                    oStream >> chChar;

                    if ( chChar != '>' )
                    {
                        IASLIB_THROW_XML_EXCEPTION( "Found unexpected \"/\" in tag." );
                    }
                    bIsSelfClose = true;
                    if ( bIsEndTag )
                    {
                        IASLIB_THROW_XML_EXCEPTION( "Found self close \"/>\" in XML End Tag." );
                    }
                    bTagClosed = true;
                    bIsEndTag = true;
                    if ( strPropertyName.GetLength() != 0 )
                    {
                        // If we close a tag with a property name, then we add the property
                        // name to the tag with a blank value.
                        CXMLProperty *pNewProperty = new CXMLProperty( strPropertyName, "" );
                        pTag->AddProperty( pNewProperty );
                    }
                    break;

                case '>':
                    bTagClosed = true;
                    if ( strPropertyName.GetLength() != 0 )
                    {
                        // If we close a tag with a property name, then we add the property
                        // name to the tag with a blank value.
                        CXMLProperty *pNewProperty = new CXMLProperty( strPropertyName, "" );
                        pTag->AddProperty( pNewProperty );
                    }
                    break;

                    // Whitespace is ignored, but will end a property name.
                case '\n':
                case '\r':
                case ' ':
                case '\t':
                    if ( strPropertyName.GetLength() != 0 )
                    {
                        // If we close a tag with a property name, then we add the property
                        // name to the tag with a blank value.
                        CXMLProperty *pNewProperty = new CXMLProperty( strPropertyName, "" );
                        pTag->AddProperty( pNewProperty );
                        strPropertyName.Clear();
                    }
                    break;

                default:
                    strPropertyName += chChar;
                    break;
            } // switch chChar
        } // while ( ( ! bTagClosed ) || ( ! oStream.IsEOS() ) )

        if ( bTagClosed )
        {
            if ( bIsEndTag )
            {
                if ( bIsSelfClose )
                {
                    return CXMLDocument::SELF_CLOSE;
                }
                else
                {
                    return CXMLDocument::END_TAG;
                }
            }
            return CXMLDocument::OPEN_TAG;
        }
        CString strException = "No closing brace found for tag within stream. Stream ended without finding closing \">\" for tag named [";
        strException += strTagName;
        strException += "]";
        IASLIB_THROW_XML_EXCEPTION( strException );
        return CXMLDocument::ERROR_TAG;
    }

    CXMLDocument::TagReturns CXMLDocument::StartProperty( CStream &oStream, CXMLElement *pTag, CString strName )
    {
        CString strValue;
        char    chChar;
        bool    bFinished = false;
        bool    bInQuotes = false;
        bool    bSingleQuotes = false;
        bool    bFoundTagEnd = false;
        bool    bFoundSelfClose = false;

        while ( ( ! bFinished ) && ( ! oStream.IsEOS() ) )
        {
            oStream >> chChar;

            switch ( chChar )
            {
                case '?':
                    if ( bInQuotes )
                    {
                        strValue += chChar;
                    }
                    else
                    {
                        oStream >> chChar;
                        if ( chChar != '>' )
                        {
                            IASLIB_THROW_XML_EXCEPTION( "No matching end tag found for <?" );
                        }
                        bFinished = true;
                        bFoundTagEnd = true;
                        bFoundSelfClose = true;
                    }
                    break;

                case '"':
                    if ( ( bInQuotes ) && ( ! bSingleQuotes ) )
                    {
                        bInQuotes = false;
                        bFinished = true;
                        bSingleQuotes = false;
                    }
                    else
                    {
                        if ( ! bInQuotes )
                        {
                            if ( strValue.GetCount() != 0 )
                            {
                                IASLIB_THROW_XML_EXCEPTION( "Quote found in middle of property value!" );
                            }
                            bInQuotes = true;
                            bSingleQuotes = false;
                        }
                        else
                        {
                            strValue += chChar;
                        }
                    }
                    break;

                case '\'':
                    if ( ( bInQuotes ) && ( bSingleQuotes ) )
                    {
                        bInQuotes = false;
                        bFinished = true;
                        bSingleQuotes = false;
                    }
                    else
                    {
                        if ( ! bInQuotes )
                        {
                            if ( strValue.GetCount() != 0 )
                            {
                                IASLIB_THROW_XML_EXCEPTION( "Quote found in middle of property value!" );
                            }
                            bInQuotes = true;
                            bSingleQuotes = true;
                        }
                        else
                        {
                            strValue += chChar;
                        }
                    }
                    break;

                case '\\':
                    strValue += chChar;
                    if ( ! oStream.IsEOS() )
                    {
                        oStream >> chChar;

                        strValue += chChar;
                        if ( chChar == 'x' )
                        {
                            if ( ! oStream.IsEOS() )
                            {
                                oStream >> chChar;
                                strValue += chChar;
                                if ( ! oStream.IsEOS() )
                                {
                                    oStream >> chChar;
                                    strValue += chChar;
                                }
                            }
                        }
                    }
                    break;

                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if ( bInQuotes )
                    {
                        strValue += chChar;
                    }
                    else
                    {
                        bFinished = true;
                    }
                    break;

                case '/':
                    if ( bInQuotes )
                    {
                        strValue += chChar;
                    }
                    else
                    {
                        bFinished = true;
                        bFoundTagEnd = true;
                        bFoundSelfClose = true;
                    }
                    break;

                case '>':
                    if ( bInQuotes )
                    {
                        strValue += chChar;
                    }
                    else
                    {
                        bFinished = true;
                        bFoundTagEnd = true;
                    }
                    break;

                default:
                    strValue += chChar;
                    break;
            }
        }

        if ( bFinished )
        {
            CXMLProperty *pNewProperty = new CXMLProperty( strName, strValue );
            pTag->AddProperty( pNewProperty );
        }
        if ( bFoundTagEnd )
        {
            if ( bFoundSelfClose )
            {
                return CXMLDocument::SELF_CLOSE;
            }
            return CXMLDocument::END_TAG;
        }
        return CXMLDocument::OPEN_TAG;
    }
}; // Namespace IASLib
