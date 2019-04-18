/***********************************************************************
**  CSybaseCursor
**
**  Description:
**      This class defines and implements a Sybase cursor which is used
** to store the results from a SQL select statement.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef __SYB_CURSOR_H__
#define __SYB_CURSOR_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "Cursor.h"

#include <ctpublic.h>

class CSybaseConnection;

class CSybaseCursor : public CCursor
{
    protected:
        CSybaseConnection  *m_pConnection;
        CS_INT             *m_apnTypes;

    public:
                            CSybaseCursor( CSybaseConnection *pConnection, CS_COMMAND *pCommand, bool bUpdatable );
        virtual            ~CSybaseCursor( void );

        virtual int         Close( void );
        virtual int         DeleteRow( const char *strTable );
        virtual int         InsertRow( const char *strTable );
        virtual int         UpdateRow( const char *strTable );

        virtual bool        Next( void );
        virtual bool        Prev( void );
};

#endif

#endif
