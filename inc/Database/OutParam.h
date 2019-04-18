/*
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
    class COutParam : public CObject
    {
        protected:
            CString             m_strParamName;
            CString             m_strParamValue;
        public:
                                COutParam( const char *strName, const char *strValue ) { m_strParamName = strName; m_strParamValue = strValue; }
            virtual            ~COutParam( void ) {}

                                DECLARE_OBJECT( COutParam, CObject )

            virtual const char *GetName( void ) { return (const char *)m_strParamName; }
            virtual const char *GetValue( void ) { return (const char *)m_strParamValue; }
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_OUTPARAM_H__
