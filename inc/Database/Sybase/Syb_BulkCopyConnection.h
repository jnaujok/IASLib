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

#ifndef __SYB_BULKCOPYCONNECTION_H__
#define __SYB_BULKCOPYCONNECTION_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "String_.h"
#include "BulkCopyConnection.h"
#include "Database.h"
#include "Mutex.h"
#include <ctpublic.h>

class CSybaseBulkCopyConnection : public CBulkCopyConnection
{
    protected:
        CS_CONNECTION          *m_pConnection;
        CS_BLKDESC             *m_pBulkDescriptor;
        bool                    m_bValid;
        bool                    m_bDead;
        static CMutex           m_mutexSynchro;
        CString                 m_strApplication;
        bool                    m_bInitialized;
    public:
                                CSybaseBulkCopyConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName = "NoName" );
        virtual                ~CSybaseBulkCopyConnection( void );

        virtual bool            SetTable( const char *strTableName );
        virtual bool            BindColumn( int nColumnNumber, int nDataType, void *pDataBuffer, int nBufferLength );
        virtual bool            SendRow( void );
        virtual bool            Flush( void );

        void                    SetDeadlock( bool bLocked ) { m_bDeadlock = bLocked; }

        virtual bool            IsDead( void ) { return m_bDead; }

};

#endif
#endif
