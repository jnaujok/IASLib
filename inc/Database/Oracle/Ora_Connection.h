/*
 *  COracleConection
 *
 *  This class defines an Oracle 7.3 database connection. This means
 * that this can be used to connect to any database after 7.3. However
 * Oracle versions after 9i are very inefficent when accessed with this
 * code, and the Oracle10g connection should be used instead.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/15/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_CONNECTION_H__
#define IASLIB_ORA_CONNECTION_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "../../BaseTypes/String_.h"
#include "../../BaseTypes/Date.h"
#include "../Connection.h"
#include "../Database.h"

#ifdef IASLIB_MULTI_THREADED__
#include "../../Threads/Mutex.h"
#endif

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
    class COracleConnection : public CConnection
    {
        protected:
            CDatabase              *m_pDatabase;
            Lda_Def                *m_pLDA;
            ub1                    *m_pHDA;
            bool                    m_bValid;
            bool                    m_bDeadlock;
            bool                    m_bDead;
            long                    m_lAffectedRows;
#ifdef IASLIB_MULTI_THREADED__
            static CMutex           m_mutexSynchro;
#endif
        public:
                                    COracleConnection( Lda_Def *pLDA, ub1 *pHDA, const char *strName = "NoName" );
            virtual                ~COracleConnection( void );

                                    DEFINE_OBJECT( COracleConnection )

            void                    SetDatabase( CDatabase *pDatabase );

            virtual bool            BeginTransaction( void );
            virtual bool            CommitTransaction( void );
            virtual bool            RollbackTransaction( void );

            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = 0 );
            virtual bool            Execute( const char *strExecString );

            virtual int             MajorErrorCode( void );
            virtual int             MinorErrorCode( void );

            virtual const char     *MajorErrorMessage( void );
            virtual const char     *MinorErrorMessage( void );

            virtual bool            Release( void );

            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

            virtual CStoredProc    *StoredProc( const char *strProcName, const char *strParamList = NULL );

            void                    SetDeadlock( bool bLocked ) { m_bDeadlock = bLocked; }

            bool                    IsDead( void ) { return m_bDead; }

		    virtual int				GetDBType( void ) { return DB_ORACLE; }
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_CONNECTION_H__
