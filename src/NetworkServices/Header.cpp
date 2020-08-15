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

    CHeader::CHeader( CString name )
    {
        this->name = name;
    }

    CHeader::CHeader()
    {
        name = "";
    }

    CHeader::CHeader( const CHeader &oSource )
    {
        this->name = oSource.name;
        this->values = oSource.values;
    }

    CHeader::~CHeader( void )
    {
        // nothing to do, but ensuring an override of default non-virtual destructor.
    }

    CString CHeader::getName()
    {
        return name;
    }

    void CHeader::setName( CString name )
    {
        this->name = name;
    }

    CString CHeader::getValue( void )
    {
        if ( values.Count() > 0 )
        {
            return values[0];
        }
        return CString("");
    }

    void CHeader::setValue( CString value )
    {
        values.EmptyAll();
        values.Append( value );
    }

    void CHeader::addValue( CString value )
    {
        values.Append( value );
    }

    CStringArray CHeader::getValues( void )
    {
        return values;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
