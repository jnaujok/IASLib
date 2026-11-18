/*
 * XML Property Class
 *
 *      An attribute of an XML element: name="value", with the value as
 * text (entities already replaced). Names are case sensitive.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLPROPERTY_H__
#define IASLIB_XMLPROPERTY_H__

#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CXMLProperty : public CObject
    {
        protected:
            CString     m_strName;
            CString     m_strValue;

        public:
                        CXMLProperty( const char *strName, const char *strValue );
            virtual    ~CXMLProperty( void );

                        DEFINE_OBJECT( CXMLProperty )

            const char *GetName( void ) const { return (const char *)m_strName; }
            const char *GetValue( void ) const { return (const char *)m_strValue; }
            void        SetValue( const char *strValue ) { m_strValue = strValue; }
    };
} // namespace IASLib

#endif // IASLIB_XMLPROPERTY_H__
