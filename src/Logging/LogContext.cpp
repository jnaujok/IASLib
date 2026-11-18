/**
 * Log Context Class File
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

#include "LogContext.h"
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CLogContext, CObject );
    
    CLogContext::CLogContext( void ) : m_hashValues( CHash::SMALL )
    {

    }

    CLogContext::~CLogContext( void )
    {
        m_hashValues.DeleteAll();
    }

    void CLogContext::addValue( const char *name, CString value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, CDate value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, int value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, long value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, float value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, double value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, bool value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addValue( const char *name, CArray &value )
    {
            // The context refers to the caller's array, as it does to
            // objects (it used to copy it, and both copies deleted the
            // same objects).
        CContextValue *pValue = new CContextValue( name, &value );

        m_hashValues.Push( name, pValue );
    }

    void CLogContext::addObject( const char *name, CObject *value )
    {
        CContextValue *pValue = new CContextValue( name, value );

        m_hashValues.Push( name, pValue );
    }

    CString CLogContext::getStringValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getStringValue();
        }
        else
        {
            return CString("");
        }
    }

    CDate CLogContext::getDateValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getDateValue();
        }
        else
        {
            return CDate();
        }
    }

    int CLogContext::getIntValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getIntValue();
        }
        else
        {
            return 0;
        }
    }

    long CLogContext::getLongValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getLongValue();
        }
        else
        {
            return 0L;
        }

    }

    float CLogContext::getFloatValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getFloatValue();
        }
        else
        {
            return 0.0;
        }

    }

    double CLogContext::getDoubleValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getDoubleValue();
        }
        else
        {
            return 0.0;
        }
    }

    bool CLogContext::getBoolValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getBoolValue();
        }
        else
        {
            return false;
        }
    }

        // NULL when there is no such value. (It returned one shared empty
        // array, created without a lock and open to changes by any caller.)
    CArray *CLogContext::getArrayValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        return ( value != NULL ) ? value->getArrayValue() : NULL;
    }

    CObject *CLogContext::getObjectValue( const char *name )
    {
        CContextValue *value = (CContextValue *)m_hashValues.Get( name );
        if ( value != NULL )
        {
            return value->getObjectValue();
        }
        else
        {
            return NULL;
        }
    }

    CContextValue *CLogContext::get( const char *name )
    {
        return (CContextValue *)m_hashValues.Get( name );
    }

    bool CLogContext::has( const char *name ) const
    {
        return name != NULL && m_hashValues.Get( name ) != NULL;
    }

    bool CLogContext::remove( const char *name )
    {
        if ( ! has( name ) )
            return false;
        CObject *pValue = m_hashValues.Remove( name );
        delete pValue;
        return true;
    }

    CString CLogContext::expand( const char *strText ) const
    {
        CString strOut;
        if ( strText == NULL )
            return strOut;

        const char *p = strText;
        while ( *p )
        {
            if ( p[0] == '$' && p[1] == '$' && p[2] == '{' )
            {
                strOut += "${";
                p += 3;
                continue;
            }
            if ( p[0] == '$' && p[1] == '{' )
            {
                const char *pEnd = strchr( p + 2, '}' );
                if ( pEnd )
                {
                    CString strName( p + 2, (size_t)( pEnd - ( p + 2 ) ) );
                    CContextValue *pValue = (CContextValue *)m_hashValues.Get( (const char *)strName );
                    if ( pValue )
                    {
                        strOut += pValue->getStringValue();
                        p = pEnd + 1;
                        continue;
                    }
                }
            }
            strOut += *p;
            p++;
        }
        return strOut;
    }
}; // namespace IASLib

