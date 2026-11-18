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

/**
 * @file
 * @brief CSybaseResultSet, a result set from a Sybase stored procedure.
 */

#ifndef __SYB_RESULTSET_H__
#define __SYB_RESULTSET_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "ResultSet.h"
#include <ctpublic.h>

        /// One result set returned by a Sybase stored procedure, read in full.
        ///
        /// Owned by its CSybaseStoredProc.
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
class CSybaseResultSet : public CResultSet
{
    protected:
        CS_INT             *m_apnTypes;     ///< Each column's Client-Library data type.

    public:
                            /// Reads the current result set's rows from the command.
                            CSybaseResultSet( CS_COMMAND *pCommand );
                            /// Frees the rows.
        virtual            ~CSybaseResultSet( void );

        /// Frees the rows. @return 0.
        virtual int         Close( void );
};

#endif
#endif
