/***********************************************************************
**  CSybaseResultSet
**
**  Description:
**      Provides an interface for a result set from a select/Curosr
** statement.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef __SYB_RESULTSET_H__
#define __SYB_RESULTSET_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "ResultSet.h"
#include <ctpublic.h>

class CSybaseResultSet : public CResultSet
{
    protected:
        CS_INT             *m_apnTypes;

    public:
                            CSybaseResultSet( CS_COMMAND *pCommand );
        virtual            ~CSybaseResultSet( void );

        virtual int         Close( void );
};

#endif
#endif
