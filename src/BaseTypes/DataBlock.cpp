/**
 * CDataBlock Class
 *
 *	One of the most common things to do in programs is to allocate a data
 * block, and then write past the end of it...
 *  This class supports a dynamically allocated data block and the support
 * functions (copy, dereference, etc.) that go with it. The data block also
 * supports guardposting to prevent unauthorized writes before and after
 * the data block, although this can be turned on and off at will to save
 * CPU time and memory space.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/26/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#include "DataBlock.h"
#include "Exception.h"

#include <new>

namespace IASLib
{
    IMPLEMENT_OBJECT( CDataBlock, CObject );

    CDataBlock::CDataBlock( size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize;
        m_nSize = 0;
        Resize( nSize, false );
    }

    CDataBlock::CDataBlock( const unsigned char *pData, size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize;
        m_nSize = 0;
        Resize( nSize, false );
        if ( pData != NULL && nSize > 0 )
            memcpy( Data(), pData, nSize );
    }

    CDataBlock::CDataBlock( const char *pData, size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize;
        m_nSize = 0;
        Resize( nSize, false );
        if ( pData != NULL && nSize > 0 )
            memcpy( Data(), pData, nSize );
    }

    CDataBlock::CDataBlock( const CDataBlock &oSource ) : CObject( oSource )
    {
        oSource.CheckGuardposts();
        m_pBuffer = NULL;
        m_nGuardpostSize = oSource.m_nGuardpostSize;
        m_nSize = 0;
        Resize( oSource.m_nSize, false );
        if ( m_nSize > 0 )
            memcpy( Data(), oSource.Data(), m_nSize );
    }

    CDataBlock::~CDataBlock( void )
    {
        if ( m_pBuffer != NULL && ! GuardpostsIntact() )
        {
            fprintf( stderr, "CDataBlock: guardpost corrupted (block of %lu bytes)\n", (unsigned long)m_nSize );
        }
        free( m_pBuffer );
        m_pBuffer = NULL;
        m_nSize = 0;
    }

    void CDataBlock::Copy( const void *pSource, size_t nLength, size_t nOffset )
    {
        CheckGuardposts();
        if ( nLength == 0 || pSource == NULL )
            return;
        if ( nOffset > (size_t)-1 - nLength )
            throw std::bad_alloc();
        if ( nOffset + nLength > m_nSize )
            Resize( nOffset + nLength, true );
        memmove( Data() + nOffset, pSource, nLength );
    }

    void CDataBlock::Set( unsigned char chValue, size_t nLength, size_t nOffset )
    {
        CheckGuardposts();
        if ( nLength == 0 )
            return;
        if ( nOffset > (size_t)-1 - nLength )
            throw std::bad_alloc();
        if ( nOffset + nLength > m_nSize )
            Resize( nOffset + nLength, true );
        memset( Data() + nOffset, (int)chValue, nLength );
    }

    void CDataBlock::Set( char chValue, size_t nLength, size_t nOffset )
    {
        Set( (unsigned char)chValue, nLength, nOffset );
    }

    CDataBlock::operator const char *() const
    {
        CheckGuardposts();
        return (const char *)Data();
    }

    CDataBlock::operator char *() const
    {
        CheckGuardposts();
        return (char *)Data();
    }

    CDataBlock::operator const unsigned char *() const
    {
        CheckGuardposts();
        return Data();
    }

    CDataBlock::operator unsigned char *() const
    {
        CheckGuardposts();
        return Data();
    }

    CDataBlock::operator const void *() const
    {
        CheckGuardposts();
        return Data();
    }

    CDataBlock::operator void *() const
    {
        CheckGuardposts();
        return Data();
    }

    char &CDataBlock::operator []( size_t nIndex )
    {
        CheckGuardposts();
        if ( nIndex >= m_nSize )
        {
            if ( nIndex == (size_t)-1 )
                throw std::bad_alloc();
            Resize( nIndex + 1, true );
        }
        return (char &)Data()[ nIndex ];
    }

    char CDataBlock::operator []( size_t nIndex ) const
    {
        CheckGuardposts();
        return ( nIndex < m_nSize ) ? (char)Data()[ nIndex ] : '\0';
    }

    CDataBlock &CDataBlock::operator =( const CDataBlock &oSource )
    {
        if ( this != &oSource )
        {
            oSource.CheckGuardposts();
            CheckGuardposts();

                // Build the copy first, so a failure leaves this block as it was.
            CDataBlock oCopy( oSource );
            unsigned char *pSwap = m_pBuffer;
            m_pBuffer = oCopy.m_pBuffer;
            oCopy.m_pBuffer = pSwap;
            size_t nSwap = m_nSize;
            m_nSize = oCopy.m_nSize;
            oCopy.m_nSize = nSwap;
            nSwap = m_nGuardpostSize;
            m_nGuardpostSize = oCopy.m_nGuardpostSize;
            oCopy.m_nGuardpostSize = nSwap;
        }
        return *this;
    }

    void CDataBlock::ChangeSize( size_t nLength, bool bPreserveData )
    {
        CheckGuardposts();
        Resize( nLength, bPreserveData );
    }

    void CDataBlock::Resize( size_t nLength, bool bPreserveData )
    {
        if ( nLength > (size_t)-1 - 2 * m_nGuardpostSize || m_nGuardpostSize > (size_t)-1 / 2 )
            throw std::bad_alloc();

        size_t nTotal = nLength + 2 * m_nGuardpostSize;
        unsigned char *pNew = (unsigned char *)malloc( ( nTotal > 0 ) ? nTotal : 1 );
        if ( pNew == NULL )
            throw std::bad_alloc();

        memset( pNew, GUARDPOST_PATTERN, m_nGuardpostSize );
        memset( pNew + m_nGuardpostSize, 0, nLength );
        memset( pNew + m_nGuardpostSize + nLength, GUARDPOST_PATTERN, m_nGuardpostSize );
        if ( bPreserveData && m_pBuffer != NULL )
        {
            memcpy( pNew + m_nGuardpostSize, Data(), ( nLength < m_nSize ) ? nLength : m_nSize );
        }

        free( m_pBuffer );
        m_pBuffer = pNew;
        m_nSize = nLength;
    }

    bool CDataBlock::GuardpostsIntact( void ) const
    {
        const unsigned char *pBack = Data() + m_nSize;
        for ( size_t nCount = 0; nCount < m_nGuardpostSize; nCount++ )
        {
            if ( m_pBuffer[ nCount ] != GUARDPOST_PATTERN || pBack[ nCount ] != GUARDPOST_PATTERN )
                return false;
        }
        return true;
    }

    void CDataBlock::CheckGuardposts( void ) const
    {
        if ( ! GuardpostsIntact() )
        {
            throw CException( "Guardpost corrupted in DataBlock!" );
        }
    }
} // namespace IASLib
