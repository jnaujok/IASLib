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
                        DEFINE_OBJECT( CLogContext );
                        CLogContext( void );
            virtual    ~CLogContext( void );

            virtual void    addValue( const char *name, CString value );
            virtual void    addValue( const char *name, CDate value );
            virtual void    addValue( const char *name, int value );
            virtual void    addValue( const char *name, long value );
            virtual void    addValue( const char *name, float value );
            virtual void    addValue( const char *name, double value );
            virtual void    addValue( const char *name, bool value );
            virtual void    addValue( const char *name, CArray &value );
            virtual void    addObject( const char *name, CObject *value );

            virtual CString getStringValue( const char *name );
            virtual CDate   getDateValue( const char *name );
            virtual int     getIntValue( const char *name );
            virtual long    getLongValue( const char *name );
            virtual float   getFloatValue( const char *name );
            virtual double  getDoubleValue( const char *name );
            virtual bool    getBoolValue( const char *name );
            virtual CArray *getArrayValue( const char *name );
            virtual CObject *getObjectValue( const char *name );

            virtual CContextValue *get( const char *name );

    };
} // namespace IASLib

#endif // IASLIB_LOG_CONTEXT_H__
