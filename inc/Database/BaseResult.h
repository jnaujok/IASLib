/*
 *  Base Result Class
 *
 *  This abstract class provides a common interface from the
 * results from a Stored Procedure (on databases that support
 * table results from a stored procedure) and results from 
 * a select statement.
 *  This class implements the common elements between these
 * two result types.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/28/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BASERESULT_H__
#define IASLIB_BASERESULT_H__

#include "../Collections/StringArray.h"
#include <stdlib.h>

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CBaseResult : public CObject
    {
        protected:
            size_t              m_nColumns;
            size_t              m_nRows;
            size_t             *m_anColumnWidths;
            CStringArray        m_astrHeaders;
            CStringArray      **m_aastrData;
            size_t              m_nCurrentRow;
            bool                m_bValid;

        public:
                                CBaseResult( void );
            virtual            ~CBaseResult( void );

                                DEFINE_OBJECT( CBaseResult )

            virtual int         Close( void ) = 0;

            virtual const char *ColumnName( size_t nColumn );

            virtual size_t      Columns( void ) { return m_nColumns; }
            virtual size_t      Rows( void ) { return m_nRows; }

            virtual bool        IsValid( void ) { return m_bValid; }

            virtual bool        Next( void );
            virtual bool        Prev( void );

            virtual CString     GetColumn( const char *pszColumn );
            virtual CString     GetColumn( size_t nColumn );

            virtual CString     operator []( size_t nColumn );

            virtual bool        IsColumn( const char *pszColumn );

            virtual bool        SetRow( size_t nRow );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_BASERESULT_H__

