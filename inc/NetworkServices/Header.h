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
            CString key;            // Lower-case version of the name, used for lookups.
            CStringArray values;

        public:
            DEFINE_OBJECT(CHeader);

            CHeader( const CString &name );
            CHeader( void );
            CHeader( const CHeader &oSource );
            CHeader &operator =( const CHeader &oSource );

            virtual ~CHeader( void );

            virtual CString getName( void ) const;
            virtual void setName( const CString &name );

                // The lower-case form of the name. Header names are case-insensitive.
            virtual CString getKey( void ) const;

                // Returns the first value, or an empty string if there are none.
            virtual CString getValue( void ) const;
            virtual void setValue( const CString &value );
            virtual void addValue( const CString &value );
            virtual CStringArray getValues( void ) const;
            virtual size_t getValueCount( void ) const;

                // Used by parsers to append a folded (continuation) line to the
                // most recently added value.
            virtual void appendToLastValue( const CString &text );

            static CString makeKey( const CString &name );
    };
} // namespace IASLib

#endif // IASLIB_HEADER_H__

#endif // IASLIB_NETWORKING__
