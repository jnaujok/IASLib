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


#include <memory>
#include "DataBlock.h"
#include "Exception.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CDataBlock, CObject );

        /**
        * /brief The standard CDataBlock constructor.
        * Constructs a data block object of size <code>nSize</code>. The
        * buffer contains random data by default and is not intialized.
        * <p>
        * For security purposes, data blocks can be built with "Guardposts"
        * that protect the front and back of the data buffer. A guardpost is
        * a continuous set of bytes that are filled with a specified pattern
        * to determine if data has been written beyond the end (or before 
        * the begining) of the data block. The size of the guardposts is set
        * by the <code>nGuardpostSize</code> parameter. Note that there is 
        * overhead associated with maintaining these guardposts, and production
        * code usually should have these guardposts disabled. (A length of zero.)
        *
        * @param   nSize            The size of the buffer to create.
        * @param   nGuardpostSize   The size of the guardposts to use. Debug 
        *                           default = 16 bytes, release = 0 bytes.
        *
        */
    CDataBlock::CDataBlock( size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize; 
        m_nSize = 0;
        Resize( nSize, false );
    }

        /**
        * Constructs a data block object of size <code>nSize</code>. The
        * buffer then has the contents of <code>pData</code> copied into
        * it.
        * <p>
        * For security purposes, data blocks can be built with "Guardposts"
        * that protect the front and back of the data buffer. A guardpost is
        * a continuous set of bytes that are filled with a specified pattern
        * to determine if data has been written beyond the end (or before 
        * the begining) of the data block. The size of the guardposts is set
        * by the <code>nGuardpostSize</code> parameter. Note that there is 
        * overhead associated with maintaining these guardposts, and production
        * code usually should have these guardposts disabled. (A length of zero.)
        *
        * @param   pData            The data to be copied into the buffer. It 
        *                           must be at least <code>nSize</code> bytes in 
        *                           length.
        * @param   nSize            The size of the buffer to create.
        * @param   nGuardpostSize   The size of the guardposts to use. Debug 
        *                           default = 16 bytes, release = 0 bytes.
        *
        */
    CDataBlock::CDataBlock( const unsigned char *pData, size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize; 
        m_nSize = 0;
        Resize( nSize, false );
        Copy( pData, nSize );
    }

        /**
        * Constructs a data block object of size <code>nSize</code>. The
        * buffer then has the contents of <code>pData</code> copied into
        * it.
        * <p>
        * For security purposes, data blocks can be built with "Guardposts"
        * that protect the front and back of the data buffer. A guardpost is
        * a continuous set of bytes that are filled with a specified pattern
        * to determine if data has been written beyond the end (or before 
        * the begining) of the data block. The size of the guardposts is set
        * by the <code>nGuardpostSize</code> parameter. Note that there is 
        * overhead associated with maintaining these guardposts, and production
        * code usually should have these guardposts disabled. (A length of zero.)
        *
        * @param   pData            The data to be copied into the buffer. It 
        *                           must be at least <code>nSize</code> bytes in 
        *                           length.
        * @param   nSize            The size of the buffer to create.
        * @param   nGuardpostSize   The size of the guardposts to use. Debug 
        *                           default = 16 bytes, release = 0 bytes.
        *
        */
    CDataBlock::CDataBlock( const char *pData, size_t nSize, size_t nGuardpostSize )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = nGuardpostSize; 
        m_nSize = 0;
        Resize( nSize, false );
        Copy( pData, nSize );
    }

        /**
        * /brief Copy Constructor
        * Constructs a data block object from the already existing object passed in 
        * as <code>oSource</code>.
        *
        * @param   oSource          The original DataBlock object to be copied
        *                           into this one.
        *
        */
    CDataBlock::CDataBlock( const CDataBlock &oSource )
    {
        m_pBuffer = NULL;
        m_nGuardpostSize = oSource.m_nGuardpostSize; 
        m_nSize = 0;
        Resize( oSource.m_nSize, false );
        Copy( oSource.m_pBuffer, oSource.m_nSize );
    }

        /**
        * /brief CDataBlock Destructor
        * This method frees the memory associated with the DataBlock object
        * at the end of the Data Block's lifetime.
        *
        */
    CDataBlock::~CDataBlock( void )
    {
        CheckGuardposts();
        if ( m_pBuffer )
            delete m_pBuffer;
        m_pBuffer = NULL;
        m_nSize = 0;
        m_nGuardpostSize = 0;
    }

        /**
        * This method copies data from a buffer into the DataBlock. The data 
        * can be copied at any offset into the block and can be of any length.
        * If the new block is longer than the current block, the data buffer 
        * is dynamically resized.
        *
        * @param    pSource         The data buffer to be copied to the Data Block
        * @param    nLength         The size of the data buffer to be copied
        * @param    nOffset         The offset within the DataBlock to begin the copy,
        *                           the default is zero.
        *
        */
    void CDataBlock::Copy( const void *pSource, size_t nLength, size_t nOffset )
    {
        CheckGuardposts();

        if ( (nOffset + nLength) > m_nSize )
        {
            Resize( nOffset + nLength, (bool)(nOffset != 0) );
        }
        memcpy( m_pBuffer + ( m_nGuardpostSize + nOffset ), pSource, nLength );
    }

        /**
        * This method sets a range of data within the DataBlock to the specified
        * value, much as the <code>memset</code> function would be used. The main 
        * difference is that if the <code>nLength</code> value is large enough to
        * cause a write past the end of the block, then the block is dynamically
        * resized.
        *
        * @param    chValue         The <code>unsigned char</code> to set the block of data to.
        * @param    nLength         The size of the data area to be set
        * @param    nOffset         The offset within the DataBlock to begin setting
        *                           the data.
        *
        */
    void CDataBlock::Set( unsigned char chValue, size_t nLength, size_t nOffset )
    {
        CheckGuardposts();

        if ( ( nOffset + nLength ) > m_nSize )
        {
            Resize( nLength, (bool)(nOffset != 0) );
        }
        memset( m_pBuffer + ( m_nGuardpostSize  + nOffset ), (int)chValue, nLength );
    }

        /**
        * This method sets a range of data within the DataBlock to the specified
        * value, much as the <code>memset</code> function would be used. The main 
        * difference is that if the <code>nLength</code> value is large enough to
        * cause a write past the end of the block, then the block is dynamically
        * resized.
        *
        * @param    chValue         The <code>char</code> to set the block of data to.
        * @param    nLength         The size of the data area to be set
        * @param    nOffset         The offset within the DataBlock to begin setting
        *                           the data.
        *
        */
    void CDataBlock::Set( char chValue, size_t nLength, size_t nOffset )
    {
        CheckGuardposts();

        if ( ( nOffset + nLength ) > m_nSize )
        {
            Resize( nLength, (bool)(nOffset != 0) );
        }
        memset( m_pBuffer + ( m_nGuardpostSize  + nOffset ), (int)chValue, nLength );
    }

        /**
        * This operator is an automatic cast to the type "const char *".
        *
        */
    CDataBlock::operator const char *() const
    {
        CheckGuardposts();
        return (const char *)( m_pBuffer + m_nGuardpostSize );
    }

        /**
        * This operator is an automatic cast to the type "char *".
        *
        */
    CDataBlock::operator char *() const
    {
        CheckGuardposts();
        return (char *)( m_pBuffer + m_nGuardpostSize );
    }

        /**
        * This operator is an automatic cast to the type "const unsigned char *".
        *
        */
    CDataBlock::operator const unsigned char *() const
    {
        CheckGuardposts();
        return (const unsigned char *)( m_pBuffer + m_nGuardpostSize );
    }

       /**
        * This operator is an automatic cast to the type "unsigned char *".
        *
        */
    CDataBlock::operator unsigned char *() const
    {
        CheckGuardposts();
        return (unsigned char *)( m_pBuffer + m_nGuardpostSize );
    }

       /**
        * This operator is an automatic cast to the type "const void *".
        *
        */
    CDataBlock::operator const void *() const
    {
        CheckGuardposts();
        return (const void *)( m_pBuffer + m_nGuardpostSize );
    }

       /**
        * This operator is an automatic cast to the type "void *".
        *
        */
    CDataBlock::operator void *() const
    {
        CheckGuardposts();
        return (void *)( m_pBuffer + m_nGuardpostSize );
    }

       /**
        * This operator allows access to the underlying data of the 
        * buffer by exposing the individual bytes (<code>char</code>) to 
        * the calling class. Note that if the index is out of range, then
        * the buffer is expanded to meet the range.
        *
        */
    char &CDataBlock::operator []( size_t nIndex )
    {
        if ( m_nSize < nIndex )
        {
            Resize( nIndex, true );
        }
        return (char &)m_pBuffer[ nIndex ];
    }

        /**
        * Copies a data block object from the already existing object passed in 
        * as <code>oSource</code>.
        *
        * @param   oSource          The original DataBlock object to be copied
        *                           into this one.
        *
        */
    CDataBlock &CDataBlock::operator =( const CDataBlock &oSource )
    {
        if ( this != &oSource )
        {
            m_nGuardpostSize = oSource.m_nGuardpostSize; 
            Resize( oSource.m_nSize, false );
            Copy( oSource.m_pBuffer, oSource.m_nSize );
        }
        return *this;
    }

        /**
        * This method is called to change the current size of the DataBlock object.
        * If the new size is smaller, the data blck is sized down. If the new size 
        * (plus the offset) is larget, than the buffer is reallocated.
        *
        * @param   nLength          the new size of the Datablock. 
        * @param   bPreserveData    if <code>bPreserveData</code> is set to true,
        *                           the default, then data stored in the memory is
        *                           maintained (ie. copied) when the size is changed.
        *
        */
    void CDataBlock::ChangeSize( size_t nLength, bool bPreserveData )
    {
        CheckGuardposts();
        Resize( nLength, bPreserveData );
    }

        /**
        * This private method is actually responsible for dynamically resizing
        * the buffer used for storing data.
        *
        * @param   nLength          the new size of the Datablock. 
        * @param   bCopy            if <code>bCopy</code> is set to true,
        *                           the default, then data stored in the memory is
        *                           maintained (ie. copied) when the size is changed.
        *                           Note: This is for future use only, as currently
        *                           the data is always copied by the memory re-allocation
        *                           routines.
        *
        */
    void CDataBlock::Resize( size_t nLength, bool bCopy  )
    {
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
        m_pBuffer = (unsigned char *)CMemoryManager::ReallocateDebug( m_pBuffer, nLength + (m_nGuarpostSize * 2), __FILE__, __LINE__ );
    #else
        m_pBuffer = (unsigned char *)CMemoryManager::Reallocate( m_pBuffer, nLength + (m_nGuarpostSize * 2) );
    #endif
#else
        m_pBuffer = (unsigned char *)realloc( m_pBuffer, (size_t)(nLength  + (m_nGuardpostSize * 2)) );
#endif
            // The copy flag is really ignored. It shouldn't be, but since
            // all of the Realloc calls move the data intrinisically, it becomes
            // tricky to make it work. It's really here for the future, in case
            // we ever get clever enough to use it.
        bCopy = bCopy;

        if ( m_nGuardpostSize )
        {
            memset( m_pBuffer, '\xAA', m_nGuardpostSize );
            memset( (m_pBuffer + nLength) - m_nGuardpostSize, '\xAA', m_nGuardpostSize );
        }
    }


        /**
        * This private method tests whether the data Guard-posts 
        * are still correct (containing a fixed data pattern). This method
        * is called before most of the functions are called. It may throw 
        * a CException if the guardpost bytes have been corrupted.
        *
        */
    void CDataBlock::CheckGuardposts( void ) const
    {
        if ( m_nGuardpostSize )
        {
            for ( size_t nCount = 0; nCount < m_nGuardpostSize; nCount++ )
            {
                if ( m_pBuffer[ nCount ] != (unsigned char)'\xAA' )
                {
                    throw CException( "Starting guardpost corrupted in DataBlock!" );
                }

                if ( m_pBuffer[ m_nSize - ( nCount + 1 ) ] != (unsigned char)'\xAA' )
                {
                    throw CException( "Ending guardpost corrupted in DataBlock!" );
                }
            }
        }
    }
} // namespace IASLib

