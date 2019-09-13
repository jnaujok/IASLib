/***********************************************************************
**
**  CStringStub Class
**
**      This class encodes a "string stub" which is where actual string
** data is stored for the CString class. This allows us to have strings
** that are "copied" merely by pointing at the same string stub. If fifty
** copies are made of a string, then they merely all point at the same
** stub in memory. Only when a string is changed does it get its own copy
** of the string stub. This will cut down on a large amount of un-needed
** (and extremely slow) memory allocation and copying.
**
**  Created:
**      8/19/2000
**  Creator:
**      Jeffrey R. Naujok
**  Revision History:
**      ($LOG$)
**
************************************************************************/

#include <memory>
#include <string>

#include "StringStub.h"
#include "StringException.h"
#include "LogSink.h"

#ifdef IASLIB_MEMORY_MANAGER__
#include "MemoryManager.h"
#endif

// using namespace IASLib
namespace IASLib
{
    CStringStub::CStringStub( void )
    {
        m_bFixedStub = false;
        m_bDeletable = false;
        m_strData = NULL;
        m_nLength = 0;
        m_nSize = 0;
        m_nReferences = 0;
    }

    CStringStub::CStringStub( int nLength ) // throw (CStringException)
    {
        m_bFixedStub = false;
        m_bDeletable = true;
        if ( nLength <= 0 )
        {
            ERROR_LOG( "Sized String Stub set to zero (or negative) length!" );
            throw CStringException( "Sized String Stub set to zero (or negative) length!", CException::NORMAL );
        }

#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
        m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
        m_strData = (IASLibChar__ *)CMemoryManager::Allocate( nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
        m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
        m_strData = (IASLibChar__ *)malloc( (size_t)(nLength * sizeof( IASLibChar__ ) + 1) );
        m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
        m_nLength = nLength;
        memset( m_strData, 0, m_nLength * sizeof( IASLibChar__ ) + 1 );
        m_nReferences = 0;
    }

    CStringStub::CStringStub( size_t nLength ) // throw (CStringException)
    {
        m_bFixedStub = false;
        m_bDeletable = true;
        if ( nLength <= 0 )
        {
            ERROR_LOG( "Sized String Stub set to zero (or negative) length!" );
            throw CStringException( "Sized String Stub set to zero (or negative) length!", CException::NORMAL );
        }

#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
        m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
        m_strData = (IASLibChar__ *)CMemoryManager::Allocate( nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
        m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
        m_strData = (IASLibChar__ *)malloc( (size_t)(nLength * sizeof( IASLibChar__ ) + 1) );
        m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
        m_nLength = nLength;
        memset( m_strData, 0, m_nLength * sizeof( IASLibChar__ ) + 1 );
        m_nReferences = 0;
    }

    CStringStub::CStringStub( const IASLibChar__ *strData, int nLength ) // throw (...)
    {
        m_bFixedStub = false;
        m_bDeletable = true;
        if ( ( NULL != strData ) && ( nLength != 0 ) )
        {
            if ( nLength < 0 )
            {
                nLength = (int)strlen( strData );
            }
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
            m_strData = (IASLibChar__ *)CMemoryManager::Allocate( nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
            m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
            m_strData = (IASLibChar__ *)::malloc( nLength * sizeof( IASLibChar__ ) + 1 );
            m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
                // And save the length of the string.
            m_nLength = nLength;
                // And the size of the memory block allocated

            if ( m_strData )
            {
                    // Copy the string.
                memcpy( m_strData, strData, nLength );
                    // Apply the nul terminator
                m_strData[ m_nLength ] = 0;
            }
            else
            {
                ERROR_LOG( "Out of memory while allocating String Stub of length %d", nLength );
                throw CException( "Out of memory while allocating String Stub", CException::FATAL );
            }
        }
        else
        {
            m_nLength = 0;
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( 1, __FILE__, __LINE__ );
    #else
            m_strData = (IASLibChar__ *)CMemoryManager::Allocate( 1 );
    #endif
            m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
            m_strData = (IASLibChar__ *)::malloc( 1 );
            m_nSize = 1;
#endif
            m_strData[0] = 0;
        }
        m_nReferences = 0;
    }

    CStringStub::CStringStub( const IASLibChar__ *strData, size_t nLength ) // throw (...)
    {
        m_bFixedStub = false;
        m_bDeletable = true;
        if ( ( NULL != strData ) && ( nLength != 0 ) )
        {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
            m_strData = (IASLibChar__ *)CMemoryManager::Allocate( nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
            m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
            m_strData = (IASLibChar__ *)::malloc( nLength * sizeof( IASLibChar__ ) + 1 );
            m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
                // And save the length of the string.
            m_nLength = nLength;
                // And the size of the memory block allocated

            if ( m_strData )
            {
                    // Copy the string
                memcpy( m_strData, strData, m_nLength );
                    // Apply the nul terminator
                m_strData[ m_nLength ] = 0;
            }
            else
            {
                ERROR_LOG( "Out of memory while allocating String Stub of length %d", nLength );
                throw CException( "Out of memory while allocating String Stub", CException::FATAL );
            }
        }
        else
        {
            m_nLength = 0;
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( 1, __FILE__, __LINE__ );
    #else
            m_strData = (IASLibChar__ *)CMemoryManager::Allocate( 1 );
    #endif
            m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
            m_strData = (IASLibChar__ *)::malloc( 1 );
            m_nSize = 1;
#endif
            m_strData[0] = 0;
        }
        m_nReferences = 0;
    }

        // Fixed location String Stub. Cannot be resized or moved.
    CStringStub::CStringStub( IASLibChar__ *strData, int nLength, int nSize, bool bDeletable )
    {
        if ( strData == NULL )
        {
            ERROR_LOG( "Cannot set a fixed string stub to NULL!" );
            throw CStringException( "Cannot set a fixed string stub to NULL!", CException::NORMAL );
        }

        if ( nLength < 0 )
        {
            nLength = (int)strlen( strData );
        }

        m_strData = strData;
        m_nLength = nLength;
        m_nSize = nSize;
        m_bFixedStub = true;
        m_bDeletable = bDeletable;
        m_nReferences = 0;
    }

    CStringStub::CStringStub( const CStringStub &oSource ) // throw (CException)
    {
        m_bFixedStub = false;
        m_bDeletable = true;
        if ( oSource.m_nLength )
        {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            m_strData = CMemoryManager::AllocateDebug( oSource.m_nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
            m_strData = CMemoryManager::Allocate( oSource.m_nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
            m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
            m_strData = (IASLibChar__ *)malloc( oSource.m_nLength * sizeof( IASLibChar__ ) + 1 );
            m_nSize = oSource.m_nLength * sizeof( IASLibChar__ ) + 1;
#endif
            if ( m_strData )
            {
                memcpy( m_strData, oSource.m_strData, oSource.m_nLength * sizeof( IASLibChar__ ) + 1 );
            }
            else
            {
                ERROR_LOG( "Out of memory while allocating String Stub of length %d.", oSource.m_nLength );
                throw CException( "Out of memory while allocating String Stub", CException::FATAL );
            }
            m_nLength = oSource.m_nLength;
        }
        else
        {
            m_bDeletable = false;
            m_strData = NULL;
            m_nLength = 0;
            m_nSize = 0;
        }
        m_nReferences = 0;
    }

    CStringStub::~CStringStub( void ) // throw (CStringException)
    {
        if ( m_nReferences != 0 )
        {
            ERROR_LOG( "String Stub deleted while still referenced!" );
            //throw CStringException( "String Stub deleted while still referenced!", CException::NORMAL );
        }

        if ( ( m_bDeletable ) && ( m_strData != NULL ) )
        {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            CMemoryManager::ReleaseDebug( m_strData, __FILE__, __LINE__ );
    #else
            CMemoryManager::Release( m_strData );
    #endif
#else
            free( m_strData );
#endif
        }
        m_strData = NULL;
        m_nSize = 0;
        m_nLength = 0;
    }

    void CStringStub::AddRef( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        m_nReferences++;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
    }

    void CStringStub::RemoveRef( void ) // throw (CStringException)
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif

        if ( m_nReferences > 0 )
        {
            m_nReferences--;
        }
        else
        {
            ERROR_LOG( "String stub containing data %s dropped below zero references!", this->m_strData );
            throw CStringException( "String Stub dropped below zero references!", CException::NORMAL );
        }

        if ( ( m_nReferences == 0 ) && ( ! m_bFixedStub ) )
        {
#ifdef IASLIB_MULTI_THREADED__
            m_mutex.Unlock();
#endif
            delete this;
        }
#ifdef IASLIB_MULTI_THREADED__
        else
        {
            m_mutex.Unlock();
        }
#endif
    }

    void CStringStub::ChangeSize( size_t nLength ) // throw (CStringException)
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( ( m_nReferences > 1 ) && ( ! m_bFixedStub ) )
        {
            ERROR_LOG( "Cannot change size of String Stub with multiple references!" );
#ifdef IASLIB_MULTI_THREADED__
            m_mutex.Unlock();
#endif
            throw CStringException( "Cannot change size of String Stub with multiple references!", CException::NORMAL );
        }

        if ( m_bFixedStub )
        {
            if ( m_nLength <= m_nSize )
            {
                if ( nLength < m_nLength )
                {
                    m_strData[ nLength ] = 0;
                }

                m_nLength = nLength;
            }
            else
            {
                m_nLength = m_nSize;
            }
        }
        else
        {
            if ( nLength == 0 )
            {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
                CMemoryManager::ReleaseDebug( m_strData, __FILE__, __LINE__ );
    #else
                CMemoryManager::Release( m_strData );
    #endif
                m_nSize = 0;
#else
                free( m_strData );
                m_nSize = 0;
#endif
                m_bDeletable = false;
            }
            else
            {
                if ( m_strData )
                {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
                    m_strData = (IASLibChar__ *)CMemoryManager::ReallocateDebug( m_strData, nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
                    m_strData = (IASLibChar__ *)CMemoryManager::Reallocate( m_strData, nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
                    m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
                    m_strData = (IASLibChar__ *)realloc( m_strData, nLength * sizeof( IASLibChar__ ) + 1 );
                    m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
                    m_bDeletable = true;
                }
                else
                {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
                    m_strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nLength * sizeof( IASLibChar__ ) + 1, __FILE__, __LINE__ );
    #else
                    m_strData = (IASLibChar__ *)CMemoryManager::Allocate( nLength * sizeof( IASLibChar__ ) + 1 );
    #endif
                    m_nSize = CMemoryManager::GetBlockSize( m_strData );
#else
                    m_strData = (IASLibChar__ *)malloc( nLength * sizeof( IASLibChar__ ) + 1 );
                    m_nSize = nLength * sizeof( IASLibChar__ ) + 1;
#endif
                    m_bDeletable = true;
                }
                if ( ! m_strData )
                {
#ifdef IASLIB_MULTI_THREADED__
                    m_mutex.Unlock();
#endif
                    ERROR_LOG( "Out of memory while allocating String Stub of length %d", nLength );
                    throw CException( "Could not allocate memory for String Stub!", CException::FATAL );
                }
            }
            m_nLength = nLength;
            m_strData[ m_nLength ] = 0;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
    }

    int CStringStub::GetRefCount( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
        int retVal = m_nReferences;
        m_mutex.Unlock();
        return retVal;
#else
        return m_nReferences;
#endif

    }


}   // namespace IASLib

