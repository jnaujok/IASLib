/**
 * Header class
 *
 * This class encapsulates a single generic header for HTTP or
 * SIP style transactions. It stores one or more values attached
 * to a named header.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "Header.h"

#include <cstring>

namespace IASLib
{
    IMPLEMENT_OBJECT(CHeader, CObject)

    CHeader::CHeader( const CString &name )
    {
        this->name = name;
        this->name.Trim();
        this->key = makeKey( this->name );
    }

    CHeader::CHeader( void )
    {
        name = "";
        key = "";
    }

    CHeader::CHeader( const CHeader &oSource )
    {
        this->name = oSource.name;
        this->key = oSource.key;
        this->values = oSource.values;
    }

    CHeader &CHeader::operator =( const CHeader &oSource )
    {
        if ( this != &oSource )
        {
            this->name = oSource.name;
            this->key = oSource.key;
            this->values = oSource.values;
        }
        return *this;
    }

    CHeader::~CHeader( void )
    {
    }

    CString CHeader::getName( void ) const
    {
        return name;
    }

    void CHeader::setName( const CString &name )
    {
        this->name = name;
        this->name.Trim();
        this->key = makeKey( this->name );
    }

    CString CHeader::getKey( void ) const
    {
        return key;
    }

    CString CHeader::getValue( void ) const
    {
        if ( values.Count() > 0 )
        {
            return values[0];
        }
        return CString("");
    }

    bool CHeader::setValue( const CString &value )
    {
            // (Any value was stored, so text passed on from a client could
            // carry a CR LF and further headers into the output.)
        if ( ! isValidValue( value ) )
        {
            return false;
        }
        values.EmptyAll();
        values.Append( value );
        return true;
    }

    bool CHeader::addValue( const CString &value )
    {
        if ( ! isValidValue( value ) )
        {
            return false;
        }
        values.Append( value );
        return true;
    }

    CStringArray CHeader::getValues( void ) const
    {
        return values;
    }

    size_t CHeader::getValueCount( void ) const
    {
        return values.Count();
    }

    void CHeader::appendToLastValue( const CString &text )
    {
        if ( ! isValidValue( text ) )
        {
            return;
        }
        if ( values.Count() == 0 )
        {
            values.Append( text );
        }
        else
        {
            CString &last = values[ values.Count() - 1 ];
            if ( last.GetLength() > 0 )
            {
                last += " ";
            }
            last += text;
        }
    }

    CString CHeader::makeKey( const CString &name )
    {
        CString retVal = name;
        retVal.Trim();
        retVal.ToLowerCase();
        return retVal;
    }

    bool CHeader::isTokenChar( char ch )
    {
        unsigned char uch = (unsigned char)ch;
        if ( ( uch >= '0' && uch <= '9' ) || ( uch >= 'a' && uch <= 'z' ) || ( uch >= 'A' && uch <= 'Z' ) )
        {
            return true;
        }
        return ( uch != 0 ) && ( strchr( "!#$%&'*+-.^_`|~", (int)uch ) != NULL );
    }

    bool CHeader::hasLineBreakOrNul( const CString &text )
    {
        const char *pText = (const char *)text;
        size_t nLength = text.GetLength();
        for ( size_t nX = 0; nX < nLength; nX++ )
        {
            if ( pText[ nX ] == '\r' || pText[ nX ] == '\n' || pText[ nX ] == '\0' )
            {
                return true;
            }
        }
        return false;
    }

    bool CHeader::isValidName( const CString &name )
    {
        const char *pName = (const char *)name;
        size_t nLength = name.GetLength();
        if ( nLength == 0 )
        {
            return false;
        }
        for ( size_t nX = 0; nX < nLength; nX++ )
        {
            if ( ! isTokenChar( pName[ nX ] ) )
            {
                return false;
            }
        }
        return true;
    }

    bool CHeader::isValidValue( const CString &value )
    {
        return ! hasLineBreakOrNul( value );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
