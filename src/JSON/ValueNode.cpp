/**
 * Value Node Classes
 *
 *  The single-value JSON nodes: text, booleans, null, binary data and the
 * missing node. Numbers are in NumberNode.cpp.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"
#include "TextNode.h"
#include "BooleanNode.h"
#include "NullNode.h"
#include "BinaryNode.h"
#include "MissingNode.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>

namespace IASLib
{
    IMPLEMENT_OBJECT( CValueNode, CJsonNode );
    IMPLEMENT_OBJECT( CTextNode, CValueNode );
    IMPLEMENT_OBJECT( CBooleanNode, CValueNode );
    IMPLEMENT_OBJECT( CNullNode, CValueNode );
    IMPLEMENT_OBJECT( CBinaryNode, CValueNode );
    IMPLEMENT_OBJECT( CMissingNode, CJsonNode );

    void CValueNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel; (void)bAsciiOnly;
        std::string strText( (const char *)asText() );
        strOut += strText;
    }

        // ----- Text -----

    namespace
    {
        std::string trimmed( const std::string &strText )
        {
            size_t nStart = strText.find_first_not_of( " \t\r\n" );
            if ( nStart == std::string::npos )
                return std::string();
            size_t nEnd = strText.find_last_not_of( " \t\r\n" );
            return strText.substr( nStart, nEnd - nStart + 1 );
        }

            // Text as a number: a whole integer, or failing that a finite
            // double. False if it is neither.
        bool textNumber( const std::string &strText, bool &bInteger, long long &llValue, double &dValue )
        {
            std::string strNumber = trimmed( strText );
            if ( strNumber.empty() )
                return false;
            if ( strNumber.find_first_not_of( "+-0123456789" ) == std::string::npos )
            {
                char *pszEnd = NULL;
                errno = 0;
                llValue = strtoll( strNumber.c_str(), &pszEnd, 10 );
                if ( *pszEnd == '\0' && errno == 0 )
                {
                    bInteger = true;
                    dValue = (double)llValue;
                    return true;
                }
            }
                // Only number characters: strtod would also read "inf", "nan"
                // and hexadecimal.
            if ( strNumber.find_first_not_of( "+-0123456789.eE" ) != std::string::npos )
                return false;
            if ( ! CJsonNode::parseDouble( strNumber, dValue ) || ! std::isfinite( dValue ) )
                return false;
            bInteger = false;
            return true;
        }

        long long clampToLong( double dValue )
        {
            if ( dValue != dValue )
                return 0;
            if ( dValue >= 9223372036854775807.0 )
                return LLONG_MAX;
            if ( dValue <= -9223372036854775808.0 )
                return LLONG_MIN;
            return (long long)dValue;
        }

        int clampToInt( long long llValue )
        {
            if ( llValue > INT_MAX )
                return INT_MAX;
            if ( llValue < INT_MIN )
                return INT_MIN;
            return (int)llValue;
        }
    }

    CString CTextNode::textValue( void ) const
    {
        return CString( m_strValue.c_str(), m_strValue.size() );
    }

    bool CTextNode::asBoolean( bool bDefault ) const
    {
        std::string strText = trimmed( m_strValue );
        if ( strText == "true" )
            return true;
        if ( strText == "false" )
            return false;
        return bDefault;
    }

    int CTextNode::asInt( int nDefault ) const
    {
        bool bInteger = false;
        long long llValue = 0;
        double dValue = 0;
        if ( ! textNumber( m_strValue, bInteger, llValue, dValue ) )
            return nDefault;
        return clampToInt( bInteger ? llValue : clampToLong( dValue ) );
    }

    long long CTextNode::asLong( long long llDefault ) const
    {
        bool bInteger = false;
        long long llValue = 0;
        double dValue = 0;
        if ( ! textNumber( m_strValue, bInteger, llValue, dValue ) )
            return llDefault;
        return bInteger ? llValue : clampToLong( dValue );
    }

    unsigned long long CTextNode::asUnsignedLong( unsigned long long ullDefault ) const
    {
            // Digits alone are read exactly, up to 2^64 - 1; anything else as
            // the other conversions read it, with negative values as 0.
        std::string strNumber = trimmed( m_strValue );
        size_t nStart = ( ! strNumber.empty() && strNumber[0] == '+' ) ? 1 : 0;
        if ( strNumber.size() > nStart && strNumber.find_first_not_of( "0123456789", nStart ) == std::string::npos )
        {
            char *pszEnd = NULL;
            errno = 0;
            unsigned long long ullValue = strtoull( strNumber.c_str() + nStart, &pszEnd, 10 );
            if ( *pszEnd == '\0' && errno == 0 )
                return ullValue;
            return ULLONG_MAX;      // too large
        }
        bool bInteger = false;
        long long llValue = 0;
        double dValue = 0;
        if ( ! textNumber( m_strValue, bInteger, llValue, dValue ) )
            return ullDefault;
        if ( bInteger )
            return ( llValue < 0 ) ? 0 : (unsigned long long)llValue;
        if ( dValue <= 0.0 )
            return 0;
        if ( dValue >= 18446744073709551616.0 )
            return ULLONG_MAX;
        return (unsigned long long)dValue;
    }

    double CTextNode::asDouble( double dDefault ) const
    {
        bool bInteger = false;
        long long llValue = 0;
        double dValue = 0;
        if ( ! textNumber( m_strValue, bInteger, llValue, dValue ) )
            return dDefault;
        return dValue;
    }

    bool CTextNode::equals( const CJsonNode *pOther ) const
    {
        return pOther != NULL && pOther->isTextual() && pOther->textString() == m_strValue;
    }

    void CTextNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel;
        writeString( strOut, m_strValue, bAsciiOnly );
    }

        // ----- Booleans and null -----

    bool CBooleanNode::equals( const CJsonNode *pOther ) const
    {
        return pOther != NULL && pOther->isBoolean() && pOther->booleanValue() == m_bValue;
    }

    void CBooleanNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel; (void)bAsciiOnly;
        strOut += m_bValue ? "true" : "false";
    }

    void CNullNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel; (void)bAsciiOnly;
        strOut += "null";
    }

        // ----- Binary -----

    CString CBinaryNode::asText( void ) const
    {
        std::string strText = encodeBase64( m_aData );
        return CString( strText.c_str(), strText.size() );
    }

    bool CBinaryNode::equals( const CJsonNode *pOther ) const
    {
        return pOther != NULL && pOther->isBinary() && static_cast<const CBinaryNode *>( pOther )->m_aData == m_aData;
    }

    void CBinaryNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel; (void)bAsciiOnly;
        strOut += '"';
        strOut += encodeBase64( m_aData );
        strOut += '"';
    }

        // ----- Missing -----

    const CMissingNode &CMissingNode::getInstance( void )
    {
            // Created on first use. Its destructor does nothing that could
            // affect a late caller during shutdown.
        static CMissingNode instance;
        return instance;
    }

    void CMissingNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)strOut; (void)nIndent; (void)nLevel; (void)bAsciiOnly;
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
