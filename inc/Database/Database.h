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

#ifndef IASLIB_DATABASE_H__
#define IASLIB_DATABASE_H__

#include "../BaseTypes/String_.h"

#include "ConnectionArray.h"
#include "Connection.h"
#include "BulkCopyConnection.h"

#ifdef IASLIB_DATABASE__

#define IASLIB_DB_OK                     0
#define IASLIB_DB_ERROR_OK               0
#define IASLIB_DB_ERROR_BAD_DATABASE    -1
#define IASLIB_DB_ERROR_BAD_QUERY       -2
#define IASLIB_DB_ERROR_CONNECTION_DEAD -3
#define IASLIB_DB_ERROR_UNSUPPORTED     -4

#define IASLIB_DB_CURSOR_READ_ONLY      -101
#define IASLIB_DB_CURSOR_INVALID_ROW    -102

namespace IASLib
{
    class CDatabase : public CObject
    {
        protected:
            int                             m_bConnected;
            int                             m_nConnections;
            CConnectionArray                m_aConnections;
            CString                         m_strVendor;
        public:
                                            CDatabase( void );
                                            CDatabase( const char *strVendor );
            virtual                        ~CDatabase( void ) {};

                                            DEFINE_OBJECT( CDatabase )

            int                             Connect( const char *strVendor );
            virtual int                     Connect( void ) = 0;

            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" ) = 0;

            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" ) = 0;

            virtual int                     Disconnect( void ) = 0;

            virtual int                     MajorErrorCode( void ) = 0;

            virtual int                     MinorErrorCode( void ) = 0;

            virtual const char             *MajorErrorMessage( void ) = 0;

            virtual const char             *MinorErrorMessage( void ) = 0;

            virtual int                     ReleaseConnection( CConnection *pCon ) = 0;

            virtual bool                    SetTimeout( int nTimeout ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATABASE_H__
