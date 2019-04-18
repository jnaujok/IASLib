/***********************************************************************
**  CSybaseConection
**
**  Description:
**      This class defines and implements a connection to a Sybase
** database.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef __SYB_CONNECTION_H__
#define __SYB_CONNECTION_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "String_.h"
#include "Date.h"
#include "Connection.h"
#include "Database.h"
#include "Mutex.h"
#include <ctpublic.h>

class CSybaseConnection : public CConnection
{
    protected:
        CDatabase              *m_pDatabase;
        CS_CONNECTION          *m_pConnection;
        CS_COMMAND             *m_pCommand;
        bool                    m_bValid;
        bool                    m_bDeadlock;
        bool                    m_bDead;
        long                    m_lAffectedRows;
        static CMutex           m_mutexSynchro;
    public:
                                CSybaseConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName = "NoName" );
        virtual                ~CSybaseConnection( void );

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

		virtual int				GetDBType( void ) { return DB_SYBASE; }
};

#endif
#endif
