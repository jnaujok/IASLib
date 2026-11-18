/***********************************************************************
**
**  CStringStub Class
**
**      This class encodes a "string stub" which is where actual string
** data is stored for the CString class. See StringStub.h.
**
**  Created:
**      8/19/2000
**  Creator:
**      Jeffrey R. Naujok
**  Revision History:
**      ($LOG$)
**
************************************************************************/

#include "StringStub.h"

#include <new>

namespace IASLib
{
    namespace
    {
        IASLibChar__ *AllocateCharacters( size_t nCapacity )
        {
            if ( nCapacity >= ( (size_t)-1 ) / sizeof( IASLibChar__ ) - 1 )
            {
                throw std::bad_alloc();
            }
            IASLibChar__ *pData = (IASLibChar__ *)malloc( ( nCapacity + 1 ) * sizeof( IASLibChar__ ) );
            if ( pData == NULL )
            {
                throw std::bad_alloc();
            }
            return pData;
        }
    }

    CStringStub::CStringStub( const IASLibChar__ *strData, size_t nLength, size_t nCapacity )
        : m_nReferences( 0 )
    {
        if ( nCapacity < nLength )
        {
            nCapacity = nLength;
        }
        m_strData = AllocateCharacters( nCapacity );
        m_nCapacity = nCapacity;
        m_nLength = nLength;
        m_bFixedStub = false;
        m_bDeletable = true;

        if ( strData != NULL )
        {
            memcpy( m_strData, strData, nLength * sizeof( IASLibChar__ ) );
        }
        else
        {
            memset( m_strData, 0, nLength * sizeof( IASLibChar__ ) );
        }
        m_strData[ nLength ] = 0;
    }

    CStringStub::CStringStub( IASLibChar__ *strBuffer, size_t nLength, size_t nSize, bool bDeletable )
        : m_nReferences( 0 )
    {
        m_strData = strBuffer;
        m_nCapacity = ( nSize > 0 ) ? nSize - 1 : 0;
        m_nLength = ( nLength < m_nCapacity ) ? nLength : m_nCapacity;
        m_bFixedStub = true;
        m_bDeletable = bDeletable;
        if ( nSize > 0 )
        {
            m_strData[ m_nLength ] = 0;
        }
    }

    CStringStub::~CStringStub( void )
    {
        if ( m_bDeletable && m_strData != NULL )
        {
            free( m_strData );
        }
        m_strData = NULL;
        m_nCapacity = 0;
        m_nLength = 0;
    }

    void CStringStub::RemoveRef( void )
    {
        if ( m_nReferences.fetch_sub( 1, std::memory_order_acq_rel ) == 1 )
        {
            delete this;
        }
    }

    size_t CStringStub::ChangeSize( size_t nLength )
    {
        if ( nLength > m_nCapacity )
        {
            if ( m_bFixedStub )
            {
                nLength = m_nCapacity;
            }
            else
            {
                Reallocate( nLength + GrowthSlack( nLength ) );
            }
        }

        if ( m_strData != NULL && ( m_nCapacity > 0 || ! m_bFixedStub ) )
        {
            m_strData[ nLength ] = 0;
        }
        m_nLength = nLength;
        return nLength;
    }

    size_t CStringStub::GrowthSlack( size_t nLength )
    {
        size_t nSlack = nLength / 2;
        return ( nSlack < MAX_GROWTH_SLACK ) ? nSlack : MAX_GROWTH_SLACK;
    }

    void CStringStub::Reserve( size_t nCapacity )
    {
        if ( ! m_bFixedStub && nCapacity > m_nCapacity )
        {
            Reallocate( nCapacity );
        }
    }

    void CStringStub::Compact( void )
    {
        if ( ! m_bFixedStub && m_nCapacity > m_nLength )
        {
            Reallocate( m_nLength );
        }
    }

    void CStringStub::Reallocate( size_t nCapacity )
    {
        if ( nCapacity < m_nLength )
        {
            nCapacity = m_nLength;
        }
        IASLibChar__ *pData;
        if ( m_bDeletable )
        {
                // realloc can often grow (or shrink) the block in place.
            if ( nCapacity >= ( (size_t)-1 ) / sizeof( IASLibChar__ ) - 1 )
            {
                throw std::bad_alloc();
            }
            pData = (IASLibChar__ *)realloc( m_strData, ( nCapacity + 1 ) * sizeof( IASLibChar__ ) );
            if ( pData == NULL )
            {
                throw std::bad_alloc();     // m_strData is still valid
            }
        }
        else
        {
            pData = AllocateCharacters( nCapacity );
            memcpy( pData, m_strData, ( m_nLength + 1 ) * sizeof( IASLibChar__ ) );
        }
        m_strData = pData;
        m_nCapacity = nCapacity;
        m_bDeletable = true;
    }
}   // namespace IASLib
