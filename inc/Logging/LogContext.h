/**
 * Log Context Class Header File
 *
 * The Log Context provides a context for storing names and values
 * that can be extracted and used to generate log messages. It also
 * includes some defined macros that build some helpful data into
 * the context each time it's used.
 *
 * A context should be generated on a per-event or per-thread basis
 * and not shared across threads, as it includes certain thread-based
 * information that should not be traded across contexts.
 *
 * A context can be created from another context to inherit certain
 * information from a parent context.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/25/2019
 *	Log:
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_LOG_CONTEXT_H__
#define IASLIB_LOG_CONTEXT_H__

#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "../Collections/Hash.h"
#include "../Collections/Array.h"
#include "ContextValue.h"

namespace IASLib
{
    class CLogContext : public CObject
    {
        protected:
            CHash       m_hashValues;

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
