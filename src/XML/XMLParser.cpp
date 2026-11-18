/*
 * XML Parser Class
 *
 *      Reads XML 1.0 into a CXMLIndex. See XMLParser.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000 (rewritten 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLParser.h"
#include "XMLException.h"
#include "FileException.h"
#include "../Streams/FileStream.h"
#include "../Streams/StringStream.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IASLib
{
    namespace
    {
        bool IsSpace( char ch )
        {
            return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
        }

        bool IsNameStart( unsigned char ch )
        {
            return ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ch == '_' || ch == ':' || ch >= 0x80;
        }

        bool IsNameChar( unsigned char ch )
        {
            return IsNameStart( ch ) || ( ch >= '0' && ch <= '9' ) || ch == '-' || ch == '.';
        }

        void AppendUtf8( std::string &strOut, unsigned long nCode )
        {
            if ( nCode < 0x80 )
            {
                strOut += (char)nCode;
            }
            else if ( nCode < 0x800 )
            {
                strOut += (char)( 0xC0 | ( nCode >> 6 ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
            else if ( nCode < 0x10000 )
            {
                strOut += (char)( 0xE0 | ( nCode >> 12 ) );
                strOut += (char)( 0x80 | ( ( nCode >> 6 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
            else
            {
                strOut += (char)( 0xF0 | ( nCode >> 18 ) );
                strOut += (char)( 0x80 | ( ( nCode >> 12 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( ( nCode >> 6 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
        }

            // The characters XML allows (XML 1.0 section 2.2).
        bool IsXmlChar( unsigned long nCode )
        {
            return nCode == 0x9 || nCode == 0xA || nCode == 0xD || ( nCode >= 0x20 && nCode <= 0xD7FF ) ||
                   ( nCode >= 0xE000 && nCode <= 0xFFFD ) || ( nCode >= 0x10000 && nCode <= 0x10FFFF );
        }

        class CParse
        {
            public:
                CParse( const std::string &strText, bool bSingleRoot, size_t nMaxDepth, size_t nMaxExpansion )
                    : m_strText( strText ), m_nPos( 0 ), m_bSingleRoot( bSingleRoot ), m_nMaxDepth( nMaxDepth ),
                      m_nExpansionLeft( nMaxExpansion ), m_bHadRoot( false ), m_bHadDoctype( false )
                {
                }

                void Run( CXMLIndex &oIndex );

            private:
                const std::string                  &m_strText;
                size_t                              m_nPos;
                bool                                m_bSingleRoot;
                size_t                              m_nMaxDepth;
                size_t                              m_nExpansionLeft;
                bool                                m_bHadRoot;
                bool                                m_bHadDoctype;
                std::vector<CXMLElement *>          m_aOpen;        // not owned: each is in its parent
                std::vector<size_t>                 m_aOpenAt;      // where each began
                std::map<std::string, std::string>  m_mapEntities;  // from the internal subset

                bool AtEnd( void ) const { return m_nPos >= m_strText.size(); }
                char Peek( size_t nAhead = 0 ) const { return ( m_nPos + nAhead < m_strText.size() ) ? m_strText[ m_nPos + nAhead ] : '\0'; }
                bool LookingAt( const char *pszText ) const { return m_strText.compare( m_nPos, strlen( pszText ), pszText ) == 0; }
                void SkipSpace( void ) { while ( ! AtEnd() && IsSpace( Peek() ) ) m_nPos++; }

                void Fail( const std::string &strMessage, size_t nAt = NOT_FOUND ) const;
                void Expect( char ch, const char *pszWhere );

                std::string ReadName( const char *pszWhat );
                void ReadReference( std::string &strOut, int nDepth );
                void Expand( const std::string &strName, std::string &strOut, int nDepth );
                std::string ReadQuoted( bool bAttribute );

                void Add( CXMLIndex &oIndex, CXMLChunk *pChunk );
                void ReadText( CXMLIndex &oIndex );
                void ReadComment( CXMLIndex &oIndex );
                void ReadCData( CXMLIndex &oIndex );
                void ReadProcessingInstruction( CXMLIndex &oIndex );
                void ReadDoctype( CXMLIndex &oIndex );
                void ReadInternalSubset( const std::string &strSubset, size_t nSubsetStart );
                void ReadStartTag( CXMLIndex &oIndex );
                void ReadEndTag( void );
        };

        void CParse::Fail( const std::string &strMessage, size_t nAt ) const
        {
            size_t nPos = ( nAt == NOT_FOUND ) ? m_nPos : nAt;
            if ( nPos > m_strText.size() )
                nPos = m_strText.size();
            size_t nLine = 1, nColumn = 1;
            for ( size_t n = 0; n < nPos; n++ )
            {
                if ( m_strText[n] == '\n' )
                {
                    nLine++;
                    nColumn = 1;
                }
                else if ( ( (unsigned char)m_strText[n] & 0xC0 ) != 0x80 )
                {
                    nColumn++;              // count characters, not UTF-8 bytes
                }
            }
            CString strFull = CString::FormatString( "line %lu, column %lu: %s", (unsigned long)nLine, (unsigned long)nColumn, strMessage.c_str() );
            IASLIB_THROW_XML_EXCEPTION( (const char *)strFull );
        }

        void CParse::Expect( char ch, const char *pszWhere )
        {
            if ( Peek() != ch || AtEnd() )
                Fail( std::string( "expected '" ) + ch + "' " + pszWhere );
            m_nPos++;
        }

        std::string CParse::ReadName( const char *pszWhat )
        {
            size_t nStart = m_nPos;
            if ( AtEnd() || ! IsNameStart( (unsigned char)Peek() ) )
                Fail( std::string( "expected " ) + pszWhat );
            while ( ! AtEnd() && IsNameChar( (unsigned char)Peek() ) )
                m_nPos++;
            return m_strText.substr( nStart, m_nPos - nStart );
        }

            // At '&': appends what the reference stands for.
        void CParse::ReadReference( std::string &strOut, int nDepth )
        {
            size_t nStart = m_nPos;
            m_nPos++;   // '&'
            if ( Peek() == '#' )
            {
                m_nPos++;
                bool bHex = ( Peek() == 'x' );
                if ( bHex )
                    m_nPos++;
                unsigned long nCode = 0;
                size_t nDigits = 0;
                while ( ! AtEnd() && Peek() != ';' )
                {
                    char ch = Peek();
                    int nDigit = ( ch >= '0' && ch <= '9' ) ? ch - '0' :
                                 ( bHex && ch >= 'a' && ch <= 'f' ) ? 10 + ch - 'a' :
                                 ( bHex && ch >= 'A' && ch <= 'F' ) ? 10 + ch - 'A' : -1;
                    if ( nDigit < 0 )
                        Fail( "bad character reference", nStart );
                    nCode = nCode * ( bHex ? 16 : 10 ) + (unsigned long)nDigit;
                    if ( nCode > 0x10FFFF )
                        Fail( "character reference out of range", nStart );
                    nDigits++;
                    m_nPos++;
                }
                if ( AtEnd() || nDigits == 0 )
                    Fail( "unterminated character reference", nStart );
                m_nPos++;   // ';'
                if ( ! IsXmlChar( nCode ) || nCode == 0 )
                    Fail( "character reference to a character XML does not allow", nStart );
                AppendUtf8( strOut, nCode );
                return;
            }

            std::string strName = ReadName( "an entity name after '&'" );
            if ( Peek() != ';' )
                Fail( "expected ';' after the entity name", nStart );
            m_nPos++;
            Expand( strName, strOut, nDepth );
        }

        void CParse::Expand( const std::string &strName, std::string &strOut, int nDepth )
        {
            if ( strName == "lt" ) { strOut += '<'; return; }
            if ( strName == "gt" ) { strOut += '>'; return; }
            if ( strName == "amp" ) { strOut += '&'; return; }
            if ( strName == "quot" ) { strOut += '"'; return; }
            if ( strName == "apos" ) { strOut += '\''; return; }

            std::map<std::string, std::string>::const_iterator it = m_mapEntities.find( strName );
            if ( it == m_mapEntities.end() )
                Fail( "undefined entity &" + strName + ";" );
            if ( nDepth >= 16 )
                Fail( "entities nest too deeply (or refer to themselves): &" + strName + ";" );

                // The replacement text may itself hold references.
            const std::string &strValue = it->second;
            for ( size_t n = 0; n < strValue.size(); n++ )
            {
                if ( strValue[n] == '&' )
                {
                    size_t nEnd = strValue.find( ';', n );
                    if ( nEnd == std::string::npos )
                        Fail( "bad reference in the replacement text of &" + strName + ";" );
                    std::string strReference = strValue.substr( n, nEnd - n + 1 );
                    if ( strReference.size() > 2 && strReference[1] == '#' )
                    {
                            // Character references were replaced when the
                            // entity was declared.
                        strOut += strReference;
                    }
                    else
                    {
                        Expand( strReference.substr( 1, strReference.size() - 2 ), strOut, nDepth + 1 );
                    }
                    n = nEnd;
                }
                else
                {
                    strOut += strValue[n];
                }
                if ( m_nExpansionLeft == 0 )
                    Fail( "entities expand to too much text" );
                m_nExpansionLeft--;
            }
        }

            // At a quote: the quoted text, with references replaced (and,
            // for attribute values, white space characters made spaces).
        std::string CParse::ReadQuoted( bool bAttribute )
        {
            char chQuote = Peek();
            if ( chQuote != '"' && chQuote != '\'' )
                Fail( bAttribute ? "expected a quoted attribute value" : "expected a quoted value" );
            size_t nStart = m_nPos;
            m_nPos++;
            std::string strValue;
            while ( ! AtEnd() && Peek() != chQuote )
            {
                char ch = Peek();
                if ( bAttribute && ch == '<' )
                    Fail( "'<' in an attribute value" );
                if ( bAttribute && ch == '&' )
                {
                        // A reference's text is kept as it is: &#10; stays a
                        // line break, where a literal one becomes a space.
                    ReadReference( strValue, 0 );
                    continue;
                }
                if ( bAttribute && ( ch == '\t' || ch == '\n' ) )
                    ch = ' ';
                strValue += ch;
                m_nPos++;
            }
            if ( AtEnd() )
                Fail( "unterminated quoted value", nStart );
            m_nPos++;
            return strValue;
        }

        void CParse::Add( CXMLIndex &oIndex, CXMLChunk *pChunk )
        {
            if ( m_aOpen.empty() )
                oIndex.AddChunk( pChunk );
            else
                m_aOpen.back()->AddChunk( pChunk );
        }

        void CParse::ReadText( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            std::string strText;
            while ( ! AtEnd() && Peek() != '<' )
            {
                if ( Peek() == '&' )
                {
                    ReadReference( strText, 0 );
                    continue;
                }
                if ( LookingAt( "]]>" ) )
                    Fail( "']]>' in text" );
                strText += Peek();
                m_nPos++;
            }

            bool bWhitespace = true;
            for ( size_t n = 0; n < strText.size() && bWhitespace; n++ )
                bWhitespace = IsSpace( strText[n] );
            if ( m_aOpen.empty() && ! bWhitespace && m_bSingleRoot )
                Fail( "text outside the root element", nStart );
            if ( strText.find( '\0' ) != std::string::npos )
                Fail( "NUL character in text", nStart );
            if ( ! strText.empty() )
                Add( oIndex, new CXMLData( CString( strText.data(), strText.size() ) ) );
        }

        void CParse::ReadComment( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            m_nPos += 4;   // "<!--"
            size_t nEnd = m_strText.find( "--", m_nPos );
            if ( nEnd == std::string::npos )
                Fail( "unterminated comment", nStart );
            if ( nEnd + 2 >= m_strText.size() || m_strText[ nEnd + 2 ] != '>' )
                Fail( "'--' inside a comment", nEnd );

            CXMLElement *pComment = new CXMLElement( "!--" );
            pComment->SetComment();
            if ( nEnd > m_nPos )
                pComment->AddData( CString( m_strText.data() + m_nPos, nEnd - m_nPos ) );
            m_nPos = nEnd + 3;
            Add( oIndex, pComment );
        }

        void CParse::ReadCData( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            if ( m_aOpen.empty() )
                Fail( "CDATA section outside an element" );
            m_nPos += 9;   // "<![CDATA["
            size_t nEnd = m_strText.find( "]]>", m_nPos );
            if ( nEnd == std::string::npos )
                Fail( "unterminated CDATA section", nStart );
            std::string strText = m_strText.substr( m_nPos, nEnd - m_nPos );
            if ( strText.find( '\0' ) != std::string::npos )
                Fail( "NUL character in a CDATA section", nStart );
            m_nPos = nEnd + 3;
            Add( oIndex, new CXMLData( CString( strText.data(), strText.size() ), true ) );
        }

        void CParse::ReadProcessingInstruction( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            m_nPos += 2;   // "<?"
            std::string strTarget = ReadName( "a processing instruction target after '<?'" );
            bool bXmlDeclaration = ( strTarget.size() == 3 && ( strTarget[0] | 0x20 ) == 'x' && ( strTarget[1] | 0x20 ) == 'm' && ( strTarget[2] | 0x20 ) == 'l' );
            if ( bXmlDeclaration && ( strTarget != "xml" || nStart != 0 ) )
                Fail( "the XML declaration must come first, as <?xml ...?>", nStart );

            size_t nEnd = m_strText.find( "?>", m_nPos );
            if ( nEnd == std::string::npos )
                Fail( "unterminated processing instruction", nStart );
            if ( nEnd > m_nPos && ! IsSpace( Peek() ) )
                Fail( "expected white space after the processing instruction target" );
            std::string strContent = m_strText.substr( m_nPos, nEnd - m_nPos );
            m_nPos = nEnd + 2;

            CXMLElement *pInstruction = new CXMLElement( strTarget.c_str() );
            pInstruction->SetMetaTag();

                // Pseudo-attributes, if the content is written as attributes
                // (as the XML declaration and stylesheet links are).
            std::vector< std::pair<std::string, std::string> > aPairs;
            bool bAttributes = true;
            size_t n = 0;
            while ( bAttributes )
            {
                while ( n < strContent.size() && IsSpace( strContent[n] ) )
                    n++;
                if ( n >= strContent.size() )
                    break;
                size_t nName = n;
                while ( n < strContent.size() && IsNameChar( (unsigned char)strContent[n] ) )
                    n++;
                std::string strName = strContent.substr( nName, n - nName );
                while ( n < strContent.size() && IsSpace( strContent[n] ) )
                    n++;
                if ( strName.empty() || n >= strContent.size() || strContent[n] != '=' )
                {
                    bAttributes = false;
                    break;
                }
                n++;
                while ( n < strContent.size() && IsSpace( strContent[n] ) )
                    n++;
                if ( n >= strContent.size() || ( strContent[n] != '"' && strContent[n] != '\'' ) )
                {
                    bAttributes = false;
                    break;
                }
                size_t nClose = strContent.find( strContent[n], n + 1 );
                if ( nClose == std::string::npos )
                {
                    bAttributes = false;
                    break;
                }
                aPairs.push_back( std::make_pair( strName, strContent.substr( n + 1, nClose - n - 1 ) ) );
                n = nClose + 1;
            }
            if ( bAttributes )
            {
                for ( size_t i = 0; i < aPairs.size(); i++ )
                    pInstruction->AddProperty( new CXMLProperty( aPairs[i].first.c_str(), aPairs[i].second.c_str() ) );
            }
            else if ( bXmlDeclaration )
            {
                delete pInstruction;
                Fail( "malformed XML declaration", nStart );
            }
                // After the properties, so it is written exactly as it was.
            pInstruction->SetRawContent( CString( strContent.data(), strContent.size() ) );
            Add( oIndex, pInstruction );
        }

        void CParse::ReadDoctype( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            if ( ! m_aOpen.empty() || m_bHadRoot || m_bHadDoctype )
                Fail( "a DOCTYPE must come once, before the root element" );
            m_bHadDoctype = true;
            m_nPos += 9;   // "<!DOCTYPE"
            size_t nContent = m_nPos;
            if ( ! IsSpace( Peek() ) )
                Fail( "expected white space after <!DOCTYPE" );

                // To the '>' that ends it: past quoted strings and the
                // internal subset in brackets.
            size_t nSubsetStart = NOT_FOUND, nSubsetEnd = NOT_FOUND;
            char chQuote = 0;
            while ( ! AtEnd() )
            {
                char ch = Peek();
                if ( chQuote != 0 )
                {
                    if ( ch == chQuote )
                        chQuote = 0;
                }
                else if ( nSubsetStart != NOT_FOUND && nSubsetEnd == NOT_FOUND )
                {
                        // Inside the subset: quotes and comments are skipped as
                        // they come, so ']' in them does not end it.
                    if ( ch == '"' || ch == '\'' )
                        chQuote = ch;
                    else if ( LookingAt( "<!--" ) )
                    {
                        size_t nEnd = m_strText.find( "-->", m_nPos + 4 );
                        if ( nEnd == std::string::npos )
                            Fail( "unterminated comment in the DOCTYPE" );
                        m_nPos = nEnd + 2;
                    }
                    else if ( ch == ']' )
                        nSubsetEnd = m_nPos;
                }
                else if ( ch == '"' || ch == '\'' )
                    chQuote = ch;
                else if ( ch == '[' && nSubsetStart == NOT_FOUND )
                    nSubsetStart = m_nPos + 1;
                else if ( ch == '>' )
                    break;
                m_nPos++;
            }
            if ( AtEnd() )
                Fail( "unterminated DOCTYPE", nStart );

            std::string strContent = m_strText.substr( nContent, m_nPos - nContent );
            m_nPos++;   // '>'

            if ( nSubsetStart != NOT_FOUND && nSubsetEnd != NOT_FOUND )
                ReadInternalSubset( m_strText.substr( nSubsetStart, nSubsetEnd - nSubsetStart ), nSubsetStart );

            CXMLElement *pDoctype = new CXMLElement( "DOCTYPE" );
            pDoctype->SetDeclaration();
            pDoctype->SetRawContent( CString( strContent.data(), strContent.size() ) );
            Add( oIndex, pDoctype );
        }

            // Records the general entities declared in the subset; other
            // declarations are skipped.
        void CParse::ReadInternalSubset( const std::string &strSubset, size_t nSubsetStart )
        {
            size_t n = 0;
            while ( n < strSubset.size() )
            {
                if ( IsSpace( strSubset[n] ) )
                {
                    n++;
                    continue;
                }
                if ( strSubset.compare( n, 4, "<!--" ) == 0 )
                {
                    size_t nEnd = strSubset.find( "-->", n + 4 );
                    n = ( nEnd == std::string::npos ) ? strSubset.size() : nEnd + 3;
                    continue;
                }
                if ( strSubset.compare( n, 2, "<?" ) == 0 )
                {
                    size_t nEnd = strSubset.find( "?>", n + 2 );
                    n = ( nEnd == std::string::npos ) ? strSubset.size() : nEnd + 2;
                    continue;
                }
                if ( strSubset[n] == '%' )
                {
                        // A parameter entity reference: not followed.
                    size_t nEnd = strSubset.find( ';', n );
                    n = ( nEnd == std::string::npos ) ? strSubset.size() : nEnd + 1;
                    continue;
                }
                if ( strSubset.compare( n, 2, "<!" ) != 0 )
                    Fail( "unexpected text in the DOCTYPE's internal subset", nSubsetStart + n );

                    // To the end of this declaration.
                size_t nEnd = n + 2;
                char chQuote = 0;
                for ( ; nEnd < strSubset.size(); nEnd++ )
                {
                    if ( chQuote != 0 )
                    {
                        if ( strSubset[ nEnd ] == chQuote )
                            chQuote = 0;
                    }
                    else if ( strSubset[ nEnd ] == '"' || strSubset[ nEnd ] == '\'' )
                        chQuote = strSubset[ nEnd ];
                    else if ( strSubset[ nEnd ] == '>' )
                        break;
                }
                if ( nEnd >= strSubset.size() )
                    Fail( "unterminated declaration in the DOCTYPE", nSubsetStart + n );

                if ( strSubset.compare( n, 8, "<!ENTITY" ) == 0 )
                {
                    size_t p = n + 8;
                    while ( p < nEnd && IsSpace( strSubset[p] ) )
                        p++;
                    bool bParameter = ( p < nEnd && strSubset[p] == '%' );
                    if ( bParameter )
                    {
                        p++;
                        while ( p < nEnd && IsSpace( strSubset[p] ) )
                            p++;
                    }
                    size_t nName = p;
                    while ( p < nEnd && IsNameChar( (unsigned char)strSubset[p] ) )
                        p++;
                    std::string strName = strSubset.substr( nName, p - nName );
                    while ( p < nEnd && IsSpace( strSubset[p] ) )
                        p++;
                    if ( strName.empty() )
                        Fail( "entity declaration without a name", nSubsetStart + n );
                    if ( p < nEnd && ( strSubset[p] == '"' || strSubset[p] == '\'' ) )
                    {
                        size_t nClose = strSubset.find( strSubset[p], p + 1 );
                        std::string strLiteral = strSubset.substr( p + 1, nClose - p - 1 );
                        if ( ! bParameter && m_mapEntities.find( strName ) == m_mapEntities.end() )
                        {
                                // Character references are replaced now, entity
                                // references when the entity is used.
                            std::string strValue;
                            for ( size_t c = 0; c < strLiteral.size(); c++ )
                            {
                                if ( strLiteral.compare( c, 2, "&#" ) == 0 )
                                {
                                    size_t nSemi = strLiteral.find( ';', c );
                                    if ( nSemi == std::string::npos )
                                        Fail( "bad character reference in an entity value", nSubsetStart + n );
                                    std::string strReference = strLiteral.substr( c, nSemi - c + 1 );
                                    std::string strExpanded;
                                    CParse oInner( strReference, false, 1, 16 );
                                    oInner.ReadReference( strExpanded, 0 );
                                    strValue += strExpanded;
                                    c = nSemi;
                                }
                                else
                                {
                                    strValue += strLiteral[c];
                                }
                            }
                            m_mapEntities[ strName ] = strValue;
                        }
                    }
                    else if ( ! bParameter )
                    {
                            // SYSTEM or PUBLIC: an external entity, which could
                            // read files or the network.
                        Fail( "external entity &" + strName + "; is not supported", nSubsetStart + n );
                    }
                }
                n = nEnd + 1;
            }
        }

        void CParse::ReadStartTag( CXMLIndex &oIndex )
        {
            size_t nStart = m_nPos;
            if ( m_aOpen.empty() && m_bHadRoot && m_bSingleRoot )
                Fail( "a second root element" );
            if ( m_aOpen.size() >= m_nMaxDepth )
                Fail( "elements nest too deeply" );

            m_nPos++;   // '<'
            std::string strName = ReadName( "an element name after '<'" );
            CXMLElement *pElement = new CXMLElement( strName.c_str() );
            Add( oIndex, pElement );
            if ( m_aOpen.empty() )
                m_bHadRoot = true;

            for ( ;; )
            {
                bool bHadSpace = IsSpace( Peek() );
                SkipSpace();
                if ( AtEnd() )
                    Fail( "unterminated start tag <" + strName + ">", nStart );
                if ( Peek() == '>' )
                {
                    m_nPos++;
                    pElement->SetHasClosingTag( true );
                    m_aOpen.push_back( pElement );
                    m_aOpenAt.push_back( nStart );
                    return;
                }
                if ( LookingAt( "/>" ) )
                {
                    m_nPos += 2;
                    pElement->SetSelfClose( true );
                    return;
                }
                if ( ! bHadSpace )
                    Fail( "expected white space before an attribute" );

                size_t nAttribute = m_nPos;
                std::string strAttribute = ReadName( "an attribute name, '>' or '/>'" );
                SkipSpace();
                Expect( '=', ( "after the attribute name " + strAttribute ).c_str() );
                SkipSpace();
                std::string strValue = ReadQuoted( true );
                if ( pElement->HasProperty( strAttribute.c_str() ) )
                    Fail( "attribute " + strAttribute + " appears twice", nAttribute );
                if ( strValue.find( '\0' ) != std::string::npos )
                    Fail( "NUL character in an attribute value", nAttribute );
                pElement->AddProperty( new CXMLProperty( strAttribute.c_str(), CString( strValue.data(), strValue.size() ) ) );
            }
        }

        void CParse::ReadEndTag( void )
        {
            size_t nStart = m_nPos;
            m_nPos += 2;   // "</"
            std::string strName = ReadName( "an element name after '</'" );
            SkipSpace();
            Expect( '>', ( "to end </" + strName ).c_str() );
            if ( m_aOpen.empty() )
                Fail( "</" + strName + "> without a start tag", nStart );
            if ( strName != m_aOpen.back()->GetName() )
                Fail( "</" + strName + "> does not match <" + m_aOpen.back()->GetName() + ">", nStart );
            m_aOpen.pop_back();
            m_aOpenAt.pop_back();
        }

        void CParse::Run( CXMLIndex &oIndex )
        {
            while ( ! AtEnd() )
            {
                if ( Peek() != '<' )
                    ReadText( oIndex );
                else if ( LookingAt( "<!--" ) )
                    ReadComment( oIndex );
                else if ( LookingAt( "<![CDATA[" ) )
                    ReadCData( oIndex );
                else if ( LookingAt( "<!DOCTYPE" ) )
                    ReadDoctype( oIndex );
                else if ( LookingAt( "<!" ) )
                    Fail( "unknown markup declaration" );
                else if ( LookingAt( "<?" ) )
                    ReadProcessingInstruction( oIndex );
                else if ( LookingAt( "</" ) )
                    ReadEndTag();
                else
                    ReadStartTag( oIndex );
            }

            if ( ! m_aOpen.empty() )
                Fail( std::string( "<" ) + m_aOpen.back()->GetName() + "> is never closed", m_aOpenAt.back() );
            if ( m_bSingleRoot && ! m_bHadRoot )
                Fail( "no root element" );
        }
    }


    CXMLParser::CXMLParser( void )
    {
        m_bSingleRoot = false;
        m_nMaxDepth = DEFAULT_MAX_DEPTH;
        m_nMaxEntityExpansion = DEFAULT_MAX_ENTITY_EXPANSION;
    }

    CXMLParser::~CXMLParser( void )
    {
    }

    void CXMLParser::Parse( const char *pData, size_t nLength, CXMLIndex &oIndex ) const
    {
        if ( pData == NULL )
            nLength = 0;

            // Line breaks are "\n" (XML 1.0 section 2.11), and the UTF-8
            // byte-order mark is not part of the document.
        size_t nStart = 0;
        if ( nLength >= 3 && (unsigned char)pData[0] == 0xEF && (unsigned char)pData[1] == 0xBB && (unsigned char)pData[2] == 0xBF )
            nStart = 3;
        else if ( nLength >= 2 && ( ( (unsigned char)pData[0] == 0xFE && (unsigned char)pData[1] == 0xFF ) || ( (unsigned char)pData[0] == 0xFF && (unsigned char)pData[1] == 0xFE ) ) )
            IASLIB_THROW_XML_EXCEPTION( "line 1, column 1: UTF-16 documents are not supported (only UTF-8)" );

        std::string strText;
        strText.reserve( nLength - nStart );
        for ( size_t n = nStart; n < nLength; n++ )
        {
            if ( pData[n] == '\r' )
            {
                strText += '\n';
                if ( n + 1 < nLength && pData[ n + 1 ] == '\n' )
                    n++;
            }
            else
            {
                strText += pData[n];
            }
        }

        CXMLIndex oParsed;
        CParse oParse( strText, m_bSingleRoot, m_nMaxDepth, m_nMaxEntityExpansion );
        oParse.Run( oParsed );
        oIndex.Clear();
        oIndex.TakeFrom( oParsed );
    }

    CString CXMLParser::ReadAll( CStream *pStream )
    {
        CString strAll;
        if ( pStream == NULL )
            return strAll;
        char achBuffer[ 65536 ];
        while ( ! pStream->IsEOS() )
        {
            int nRead = pStream->GetBuffer( achBuffer, (int)sizeof( achBuffer ) );
            if ( nRead <= 0 )
                break;
                // CString stops at a NUL; XML text cannot hold one anyway.
            strAll += CString( achBuffer, (size_t)nRead );
            if ( memchr( achBuffer, 0, (size_t)nRead ) != NULL )
                IASLIB_THROW_XML_EXCEPTION( "NUL byte in the XML input" );
        }
        return strAll;
    }

    CXMLDocument *CXMLParser::parseStream( CStream *stream )
    {
        if ( stream == NULL )
            IASLIB_THROW_XML_EXCEPTION( "No stream to read XML from." );
        std::unique_ptr<CXMLDocument> pDocument( new CXMLDocument() );
        pDocument->Read( stream );
        return pDocument.release();
    }

    CXMLDocument *CXMLParser::parseString( CString xmlData )
    {
        std::unique_ptr<CXMLDocument> pDocument( new CXMLDocument() );
        pDocument->Parse( xmlData, xmlData.GetLength() );
        return pDocument.release();
    }

    CXMLDocument *CXMLParser::parseFile( CFile *file )
    {
        if ( file == NULL || ! file->IsOpen() )
            IASLIB_THROW_XML_EXCEPTION( "XML file is not open." );
        CString strAll;
        char achBuffer[ 65536 ];
        while ( ! file->IsEOF() )
        {
            int nRead = file->Read( achBuffer, (int)sizeof( achBuffer ) );
            if ( nRead <= 0 )
                break;
            if ( memchr( achBuffer, 0, (size_t)nRead ) != NULL )
                IASLIB_THROW_XML_EXCEPTION( "NUL byte in the XML input" );
            strAll += CString( achBuffer, (size_t)nRead );
        }
        return parseString( strAll );
    }

    CXMLDocument *CXMLParser::parseFile( CString fileName )
    {
        std::unique_ptr<CFile> pFile;
        try
        {
            pFile.reset( new CFile( fileName, CFile::READ ) );
        }
        catch ( CFileException & )
        {
        }
        if ( ! pFile || ! pFile->IsOpen() )
        {
            CString strMessage = "Cannot open XML file ";
            strMessage += fileName;
            IASLIB_THROW_XML_EXCEPTION( (const char *)strMessage );
        }
        CXMLDocument *pDocument = parseFile( pFile.get() );
        pFile->Close();
        return pDocument;
    }

#ifdef IASLIB_NETWORKING__
    CXMLDocument *CXMLParser::parseSocket( CSocket *socket )
    {
        if ( socket == NULL )
            IASLIB_THROW_XML_EXCEPTION( "No socket to read XML from." );
            // Read directly: a CSocketStream would delete the socket.
        CString strAll;
        char achBuffer[ 65536 ];
        for ( ;; )
        {
            int nRead = socket->Read( achBuffer, (int)sizeof( achBuffer ) );
            if ( nRead <= 0 )
                break;
            if ( memchr( achBuffer, 0, (size_t)nRead ) != NULL )
                IASLIB_THROW_XML_EXCEPTION( "NUL byte in the XML input" );
            strAll += CString( achBuffer, (size_t)nRead );
        }
        return parseString( strAll );
    }
#endif
} // end of Namespace IASLib
