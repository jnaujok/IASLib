/*
 *  Database Class
 *
 *  Abstract base class from which all of the vendor-specific
 * database classes are derived.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/24/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Database.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    IMPLEMENT_OBJECT( CDatabase, CObject );

    CDatabase::CDatabase( void )
    {
        m_bConnected = 0;
        m_nConnections = 0;
        m_strVendor = "";
    }

    CDatabase::CDatabase( const char *strVendor )
    {
        m_bConnected = 0;
        m_nConnections = 0;
        m_strVendor = strVendor;
    }

    int CDatabase::Connect( const char *strVendor )
    {
        m_bConnected = 0;
        m_nConnections = 0;
        m_aConnections.DeleteAll();
        m_strVendor = strVendor;

        return 0;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
