/*
 * Object
 *
 *	This class defines the base (abstract) class that all objects in the
 * IAS Library are based on.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/18/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Object.h"
#include "String_.h"
#ifdef IASLIB_MULTI_THREADED__
#include "Mutex.h"
#endif

namespace IASLib
{
    unsigned long   CObject::m_ulCurrentID = 0;
#ifdef IASLIB_MULTI_THREADED__
    CMutex          CObject::m_mutexCreation( false );
#endif

    CObject::CObject( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexCreation.Lock();
#endif
        m_nObjectID = ++m_ulCurrentID;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexCreation.Unlock();
#endif
    }

    CObject::~CObject( void )
    {
        m_nObjectID = 0xFFFFFFFF;
    }

#ifdef IASLIB_RTTI__
    const char *CObject::GetType( void )
    {
        return "CObject";
    }

    bool CObject::IsType( const char *strName )
    {
        if (strcmp( strName, "CObject" ) == 0)
            return true;
        return false;
    }
#endif

    void CObject::SetAssertHandler( )
    {
        // TODO: Implement Assertion Handlers
    }


    void CObject::CallAssertHandler( const char *strFile,
                                     int nLine,
                                     const char *strCondition )
    {
        // TODO: Implement Assertion Handlers
        printf( "Assertion failed: (%s) - %s:%d", strCondition, strFile, nLine );
    }
} // End of Namespace
