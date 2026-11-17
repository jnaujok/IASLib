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

    void CHeader::setValue( const CString &value )
    {
        values.EmptyAll();
        values.Append( value );
    }

    void CHeader::addValue( const CString &value )
    {
        values.Append( value );
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
} // namespace IASLib

#endif // IASLIB_NETWORKING__
