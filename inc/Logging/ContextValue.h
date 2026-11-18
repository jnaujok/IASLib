/**
 * @file
 * Context Value Class Header File
 *
 * Context Value objects are objects that contain a named value that might be
 * of different types. This variant value
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/25/2019
 *	Log:
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CONTEXT_VALUE_H__
#define IASLIB_CONTEXT_VALUE_H__

#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "../BaseTypes/Variant.h"
#include "../Collections/Array.h"

namespace IASLib
{
        /// One named value in a CLogContext.
        ///
        /// Holds a name and a CVariant. The getters convert the value the
        /// way CVariant does. An array or object value is held by pointer
        /// only: the context value never deletes it.
    class CContextValue : public CObject
    {
        protected:
            CString     m_strName;      ///< The value's name.
            CVariant    m_varValue;     ///< The value itself.

        public:
                        DEFINE_OBJECT( CContextValue );
                        /// Creates an unnamed, empty value.
                        CContextValue( void );
                        /// Destroys the value (an array or object it points to is not deleted).
            virtual    ~CContextValue( void );

            /// Creates a string value.
            /// @param name  The value's name.
            /// @param value The value.
            CContextValue( const char *name, CString value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates a date value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, CDate value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates an int value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, int value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates a long value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, long value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates a float value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, float value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates a double value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, double value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates a boolean value. @copydetails CContextValue(const char*,CString)
            CContextValue( const char *name, bool value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates an array value that points to value (not a copy, and not owned).
            /// @param name  The value's name.
            /// @param value The array; it must outlive this object.
            CContextValue( const char *name, CArray *value ) { m_strName = name; m_varValue = CVariant( value ); }
            /// Creates an object value that points to value (not owned).
            /// @param name  The value's name.
            /// @param value The object; it must outlive this object.
            CContextValue( const char *name, CObject *value ) { m_strName = name; m_varValue = CVariant( value ); }

            /// The value as a string.
            virtual CString getStringValue( void ) { return m_varValue.getString(); }
            /// The value as a date.
            virtual CDate   getDateValue( void )  { return m_varValue.getDate(); }
            /// The value as an int.
            virtual int     getIntValue( void )  { return m_varValue.getInt(); }
            /// The value as a long.
            virtual long    getLongValue( void )  { return m_varValue.getLong(); }
            /// The value as a float.
            virtual float   getFloatValue( void )  { return m_varValue.getFloat(); }
            /// The value as a double.
            virtual double  getDoubleValue( void )  { return m_varValue.getDouble(); }
            /// The value as a boolean.
            virtual bool    getBoolValue( void )  { return m_varValue.getBoolean(); }
            /// The array the value points to (see CVariant::getArray()).
            virtual CArray *getArrayValue( void )  { return m_varValue.getArray(); }
            /// The object the value points to (see CVariant::getObject()).
            virtual CObject *getObjectValue( void )  { return m_varValue.getObject(); }

            /// The underlying variant, which can be changed in place.
            virtual CVariant &get( void )  { return m_varValue; }


    };
} // namespace IASLib

#endif // IASLIB_LOG_CONTEXT_H__
