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

/**
 * @file
 * @brief CSybaseStoredProc, the results of a Sybase stored procedure call.
 */

#ifndef __SYB_STOREDPROC_H__
#define __SYB_STOREDPROC_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "ResultSet.h"
#include "StoredProc.h"
#include "OutParam.h"
#include <ctpublic.h>

        /// The results of a Sybase stored procedure call: result sets, output parameters and return status.
        ///
        /// Made by CSybaseConnection::StoredProc(); the caller deletes it.
        /// Every result set is read when it is made.
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
        /// (Close(), OutParamCount() and OutParameters() still have the old
        /// int signatures.)
class CSybaseStoredProc : public CStoredProc
{
    protected:
        CS_COMMAND         *m_pCommand;             ///< The command that ran the call.
        int                 m_nResultSets;          ///< How many result sets.
        int                 m_nCurrentResultSet;    ///< The next one ResultSet() returns.
        CResultSet        **m_aResultSets;          ///< The result sets (owned).
        int                 m_nOutParams;           ///< How many output parameters.
        COutParam         **m_aOutParams;           ///< The output parameters (owned).
        int                 m_nReturnValue;         ///< The return status.
        bool                m_bValid;               ///< True if the call succeeded.

    public:
                            /// Reads every result of the command.
                            CSybaseStoredProc( CS_COMMAND *pCommand );
                            /// Frees the results.
        virtual            ~CSybaseStoredProc( void );

        /// Frees the results. @return 0.
        virtual int         Close( void );

        /// How many output parameters there are.
        virtual int         OutParamCount( void );
        /// An output parameter's value (NULL if there is no such parameter).
        virtual const char *OutParameters( int nParam );

        /// The next result set (NULL when there are no more).
        virtual CResultSet *ResultSet( void );

        /// The procedure's return status.
        virtual int         ReturnValue( void );

        /// True if the call succeeded.
        virtual bool        IsValid( void ) { return m_bValid; }

    private:
        /// Reads the output parameters from the current result.
        void                GetOutputParams( void );
        /// Reads the return status from the current result.
        void                GetStatusParam( void );
};

#endif
#endif
