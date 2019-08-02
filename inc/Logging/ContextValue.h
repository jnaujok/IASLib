/**
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
    class CContextValue : public CObject
    {
        protected:
            CString     m_strName;
            CVariant    m_varValue; 

        public:
                        DEFINE_OBJECT( CContextValue );
                        CContextValue( void );
            virtual    ~CContextValue( void );

            CContextValue( const char *name, CString value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, CDate value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, int value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, long value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, float value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, double value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, bool value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, CArray *value ) { m_strName = name; m_varValue = CVariant( value ); }
            CContextValue( const char *name, CObject *value ) { m_strName = name; m_varValue = CVariant( value ); }

            virtual CString getStringValue( void ) { return m_varValue.getString(); }
            virtual CDate   getDateValue( void )  { return m_varValue.getDate(); }
            virtual int     getIntValue( void )  { return m_varValue.getInt(); }
            virtual long    getLongValue( void )  { return m_varValue.getLong(); }
            virtual float   getFloatValue( void )  { return m_varValue.getFloat(); }
            virtual double  getDoubleValue( void )  { return m_varValue.getDouble(); }
            virtual bool    getBoolValue( void )  { return m_varValue.getBoolean(); }
            virtual CArray *getArrayValue( void )  { return m_varValue.getArray(); }
            virtual CObject *getObjectValue( void )  { return m_varValue.getObject(); }

            virtual CVariant &get( void )  { return m_varValue; }

            
    };
} // namespace IASLib

#endif // IASLIB_LOG_CONTEXT_H__
