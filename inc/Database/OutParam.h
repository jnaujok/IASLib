/**
 *  @file
 *  @brief COutParam, one output parameter of a stored procedure.
 *
 *  Database Class
 *
 *  Base class to store output parameters from stored procedures.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/18/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_OUTPARAM_H__
#define IASLIB_OUTPARAM_H__

#include "../BaseTypes/String_.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        /// One output parameter of a stored procedure: a name and its value as text.
    class COutParam : public CObject
    {
        protected:
            CString             m_strParamName;     ///< The parameter's name.
            CString             m_strParamValue;    ///< Its value.
        public:
                                /// Creates a parameter (both strings are copied).
                                COutParam( const char *strName, const char *strValue ) { m_strParamName = strName; m_strParamValue = strValue; }
                                /// Destroys the parameter.
            virtual            ~COutParam( void ) {}

                                DECLARE_OBJECT( COutParam, CObject )

            /// The parameter's name.
            virtual const char *GetName( void ) { return (const char *)m_strParamName; }
            /// The parameter's value.
            virtual const char *GetValue( void ) { return (const char *)m_strParamValue; }
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_OUTPARAM_H__
