/***********************************************************************
**  SybaseStoredProc
**
**  Description:
**      This class encapsulates the functionality of pulling returns
** from a Sybase stored procedure call. It handles multiple result sets
** and return values.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef __SYB_STOREDPROC_H__
#define __SYB_STOREDPROC_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "ResultSet.h"
#include "StoredProc.h"
#include "OutParam.h"
#include <ctpublic.h>

class CSybaseStoredProc : public CStoredProc
{
    protected:
        CS_COMMAND         *m_pCommand;
        int                 m_nResultSets;
        int                 m_nCurrentResultSet;
        CResultSet        **m_aResultSets;
        int                 m_nOutParams;
        COutParam         **m_aOutParams;
        int                 m_nReturnValue;
        bool                m_bValid;

    public:
                            CSybaseStoredProc( CS_COMMAND *pCommand );
        virtual            ~CSybaseStoredProc( void );

        virtual int         Close( void );

        virtual int         OutParamCount( void );
        virtual const char *OutParameters( int nParam );

        virtual CResultSet *ResultSet( void );

        virtual int         ReturnValue( void );

        virtual bool        IsValid( void ) { return m_bValid; }

    private:
        void                GetOutputParams( void );
        void                GetStatusParam( void );
};

#endif
#endif
