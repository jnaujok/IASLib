/*
 *  COracleCursor
 *
 *  This class defines an Oracle 7.3 cursor that is used to store the 
 * results from a SQL select statement.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/19/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_CURSOR_H__
#define IASLIB_ORA_CURSOR_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Cursor.h"

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
    class COracleConnection;

    class COracleCursor : public CCursor
    {
        protected:
            COracleConnection  *m_pConnection;
            ub2                *m_apnTypes;
            Cda_Def            *m_pCDA;

        public:
                                COracleCursor( COracleConnection *pConnection, Lda_Def *pLDA, Cda_Def *pCDA, bool bUpdatable );
            virtual            ~COracleCursor( void );

                                DEFINE_OBJECT( COracleCursor )

            virtual int         Close( void );
            virtual int         DeleteRow( const char *strTable );
            virtual int         InsertRow( const char *strTable );
            virtual int         UpdateRow( const char *strTable );

            virtual bool        Next( void );
            virtual bool        Prev( void );
        private:
            bool                NeedsQuotes( size_t nColumn );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_CURSOR_H__
