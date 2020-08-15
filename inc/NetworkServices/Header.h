/**
 * Header class
 *
 * This class encapsulates a single generic header for HTTP or 
 * SIP style transactions. It stores one or more values attached
 * to a named header.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 * 
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HEADER_H__
#define IASLIB_HEADER_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "Collections/StringArray.h"

namespace IASLib
{
    class CHeader : public CObject
    {
        private:
            CString name;
            CStringArray values;

        public:
            DEFINE_OBJECT(CHeader);

            CHeader( CString name );
            CHeader( void );
            CHeader( const CHeader &oSource );

            virtual ~CHeader( void );

            virtual CString getName( void );
            virtual void setName( CString name );

            virtual CString getValue( void );
            virtual void setValue( CString value );

            virtual void addValue( CString value );
            virtual CStringArray getValues( void );
    };
} // namespace IASLib

#endif // IASLIB_HEADER_H__

#endif // IASLIB_NETWORKING__
