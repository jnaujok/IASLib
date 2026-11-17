/**
 * Number Node Class
 *
 *  A JSON number. See NumberNode.h.
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

#include "NumberNode.h"

#include <climits>
#include <cmath>
#include <string>

namespace IASLib
{
    IMPLEMENT_OBJECT( CNumberNode, CValueNode );

    namespace
    {
        const double TWO_63 = 9223372036854775808.0;

            // A double truncated and clamped to the 64-bit range (NaN as 0).
        long long toLong( double dValue )
        {
            if ( dValue != dValue )
                return 0;
            if ( dValue >= TWO_63 )
                return LLONG_MAX;
            if ( dValue < -TWO_63 )
                return LLONG_MIN;
            return (long long)dValue;
        }
    }

    CNumberNode::CNumberNode( int nValue ) : m_eKind( KIND_LONG ), m_llValue( nValue ), m_dValue( 0.0 )
    {
    }

    CNumberNode::CNumberNode( long long llValue ) : m_eKind( KIND_LONG ), m_llValue( llValue ), m_dValue( 0.0 )
    {
    }

    CNumberNode::CNumberNode( unsigned long long llValue ) : m_eKind( KIND_LONG ), m_llValue( 0 ), m_dValue( 0.0 )
    {
        if ( llValue <= (unsigned long long)LLONG_MAX )
        {
            m_llValue = (long long)llValue;
        }
        else
        {
            m_eKind = KIND_BIG;
            m_dValue = (double)llValue;
            m_strText = std::to_string( llValue );
        }
    }

    CNumberNode::CNumberNode( double dValue ) : m_eKind( KIND_DOUBLE ), m_llValue( 0 ), m_dValue( dValue )
    {
    }

    CNumberNode *CNumberNode::fromText( const std::string &strText )
    {
        if ( strText.empty() )
            return NULL;
        bool bIntegral = ( strText.find_first_of( ".eE" ) == std::string::npos );
        if ( bIntegral )
        {
                // Exactly, if it fits in 64 bits.
            bool bNegative = ( strText[0] == '-' );
            unsigned long long ullValue = 0;
            bool bOverflow = false;
            for ( size_t n = bNegative ? 1 : 0; n < strText.size(); n++ )
            {
                unsigned int nDigit = (unsigned int)( strText[ n ] - '0' );
                if ( nDigit > 9 )
                    return NULL;
                if ( ullValue > ( ~0ULL - nDigit ) / 10 )
                {
                    bOverflow = true;
                    break;
                }
                ullValue = ullValue * 10 + nDigit;
            }
            if ( ! bOverflow )
            {
                if ( ! bNegative && ullValue <= (unsigned long long)LLONG_MAX )
                    return new CNumberNode( (long long)ullValue );
                if ( bNegative && ullValue <= (unsigned long long)LLONG_MAX + 1ULL )
                    return new CNumberNode( ullValue == (unsigned long long)LLONG_MAX + 1ULL ? LLONG_MIN : -(long long)ullValue );
            }
        }

        double dValue = 0.0;
        if ( ! parseDouble( strText, dValue ) )
            return NULL;
        CNumberNode *pNode = new CNumberNode( dValue );
        pNode->m_eKind = bIntegral ? KIND_BIG : KIND_DOUBLE;
        pNode->m_strText = strText;
        return pNode;
    }

    bool CNumberNode::isInt( void ) const
    {
        return m_eKind == KIND_LONG && m_llValue >= INT_MIN && m_llValue <= INT_MAX;
    }

    bool CNumberNode::canConvertToInt( void ) const
    {
        if ( m_eKind == KIND_LONG )
            return m_llValue >= INT_MIN && m_llValue <= INT_MAX;
        return std::isfinite( m_dValue ) && m_dValue > (double)INT_MIN - 1.0 && m_dValue < (double)INT_MAX + 1.0;
    }

    bool CNumberNode::canConvertToLong( void ) const
    {
        if ( m_eKind == KIND_LONG )
            return true;
        return std::isfinite( m_dValue ) && m_dValue >= -TWO_63 && m_dValue < TWO_63;
    }

    int CNumberNode::intValue( void ) const
    {
        long long llValue = longValue();
        if ( llValue > INT_MAX )
            return INT_MAX;
        if ( llValue < INT_MIN )
            return INT_MIN;
        return (int)llValue;
    }

    long long CNumberNode::longValue( void ) const
    {
        return ( m_eKind == KIND_LONG ) ? m_llValue : toLong( m_dValue );
    }

    double CNumberNode::doubleValue( void ) const
    {
        return ( m_eKind == KIND_LONG ) ? (double)m_llValue : m_dValue;
    }

    std::string CNumberNode::text( void ) const
    {
        if ( m_eKind == KIND_LONG )
            return std::to_string( m_llValue );
        if ( ! m_strText.empty() )
            return m_strText;
        return formatDouble( m_dValue );
    }

    CString CNumberNode::asText( void ) const
    {
        std::string strText = text();
        return CString( strText.c_str(), strText.size() );
    }

        // Integers compare exactly, other numbers by value; an integer equals a
        // double only if the double is exactly that integer.
    bool CNumberNode::equals( const CJsonNode *pOther ) const
    {
        if ( pOther == NULL || ! pOther->isNumber() )
            return false;
        const CNumberNode *pNumber = static_cast<const CNumberNode *>( pOther );
        if ( m_eKind == KIND_LONG && pNumber->m_eKind == KIND_LONG )
            return m_llValue == pNumber->m_llValue;
        if ( m_eKind == KIND_LONG || pNumber->m_eKind == KIND_LONG )
        {
            const CNumberNode *pLong = ( m_eKind == KIND_LONG ) ? this : pNumber;
            const CNumberNode *pOtherKind = ( m_eKind == KIND_LONG ) ? pNumber : this;
            double dValue = pOtherKind->m_dValue;
            if ( ! std::isfinite( dValue ) || dValue != std::floor( dValue ) || dValue < -TWO_63 || dValue >= TWO_63 )
                return false;
            return (long long)dValue == pLong->m_llValue;
        }
        if ( m_eKind == KIND_BIG && pNumber->m_eKind == KIND_BIG )
            return m_strText == pNumber->m_strText;
        return m_dValue == pNumber->m_dValue;
    }

    CJsonNode *CNumberNode::clone( void ) const
    {
        CNumberNode *pCopy = new CNumberNode( m_dValue );
        pCopy->m_eKind = m_eKind;
        pCopy->m_llValue = m_llValue;
        pCopy->m_strText = m_strText;
        return pCopy;
    }

    void CNumberNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        (void)nIndent; (void)nLevel; (void)bAsciiOnly;
        strOut += text();
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
