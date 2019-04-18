/*
 *  Connection Class
 *
 *  This abstract class provides a base interface for database connections.
 * Database connections are used to provide a conduit for database 
 * communications.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/25/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CONNECTION_H__
#define IASLIB_CONNECTION_H__

#include "../BaseTypes/String_.h"
#include "../Streams/Stream.h"
#include "StoredProc.h"
#include "Cursor.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CConnection : public CObject
    {
        protected:
            CString                 m_strConnectionName;
            bool                    m_bConnected;
            bool                    m_bInTransaction;

        public:
                                    CConnection( const char *strName = "" ) { m_strConnectionName = strName; m_bConnected = 0; m_bInTransaction = 0;}
            virtual                ~CConnection( void ) {};

                                    DECLARE_OBJECT( CConnection, CObject )

            virtual bool            BeginTransaction( void ) = 0;
            virtual bool            CommitTransaction( void ) = 0;
            virtual bool            RollbackTransaction( void ) = 0;

            virtual bool            Connected( void ) { return m_bConnected; }
            virtual bool            IsValid( void ) { return ( ( Connected() ) && ( ! IsDead() ) ); }

            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false ) = 0;
            virtual bool            Execute( const char *strExecString ) = 0;

            virtual int             MajorErrorCode( void ) = 0;
            virtual int             MinorErrorCode( void ) = 0;

            virtual const char     *MajorErrorMessage( void ) = 0;
            virtual const char     *MinorErrorMessage( void ) = 0;

            virtual bool            Release( void ) = 0;

            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL ) = 0;

            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL ) = 0;

            virtual bool            IsDead( void ) = 0;

		    virtual int				GetDBType( void ) = 0;
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTION_H__
