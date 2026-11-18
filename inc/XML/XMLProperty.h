/**
 * @file XMLProperty.h
 * @brief CXMLProperty, an attribute of an XML element.
 *
 * The original header comment follows.
 */
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
    /// An attribute of an XML element: name="value", with the value as
    /// text (entities already replaced). Names are case sensitive.
    class CXMLProperty : public CObject
    {
        protected:
            CString     m_strName;      ///< The attribute's name.
            CString     m_strValue;     ///< The value, unescaped.

        public:
                /// Creates an attribute.
                /// @param strName  The name.
                /// @param strValue  The value, unescaped.
                        CXMLProperty( const char *strName, const char *strValue );
                /// Destroys the attribute.
            virtual    ~CXMLProperty( void );

                        DEFINE_OBJECT( CXMLProperty )

                /// The attribute's name.
            const char *GetName( void ) const { return (const char *)m_strName; }
                /// The value, unescaped.
            const char *GetValue( void ) const { return (const char *)m_strValue; }
                /// Replaces the value.
                /// @param strValue  The new value, unescaped.
            void        SetValue( const char *strValue ) { m_strValue = strValue; }
    };
} // namespace IASLib

#endif // IASLIB_XMLPROPERTY_H__
