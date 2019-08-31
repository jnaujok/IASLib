/***********************************************************************
**  CArray
**
**  Description:
**      This class is an array class for storing pointers derived from
** CObject. It can dynamically re-allocate to store data.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include <memory>
#include "Array.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CArray,CCollection);

    CObject *CArrayIterator::Next( void )
    {
        CObject *pRetVal = NULL;

        if (  m_nCurrentPos < m_pArray->GetLength() )
        {
            pRetVal = (*m_pArray)[ m_nCurrentPos ];
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    CObject *CArrayIterator::Prev( void )
    {
        CObject *pRetVal = NULL;

        if ( ( m_nCurrentPos > 0 ) && ( m_nCurrentPos <= m_pArray->GetLength() ) )
        {
            m_nCurrentPos--;
            pRetVal = (*m_pArray)[ m_nCurrentPos ];
        }

        return pRetVal;
    }

    bool CArrayIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if (  m_nCurrentPos < m_pArray->GetLength() )
        {
            bRetVal = true;
        }

        return bRetVal;
    }

    CArray::CArray( size_t nScale )
    {
        if ( nScale > 0 )
            m_nScale = nScale;
        else
            m_nScale = 4;
        m_nSize = 0;
        m_nElements = 0;
        m_apElements = NULL;
    }

    CArray::CArray( const CArray &oSource )
    {
        m_nScale = oSource.m_nScale;
        m_nSize = oSource.m_nSize;
        m_nElements = oSource.m_nElements;
        m_apElements = new CObject *[ m_nSize ];
        for ( size_t nX = 0; nX < m_nElements; nX ++ )
        {
            m_apElements[nX] = oSource.m_apElements[nX];
        }
    }

    CArray::~CArray( void )
    {
        DeleteAll();
    }

    void CArray::Resize( bool bIncrease )
    {
        if ( bIncrease )
        {
            if ( m_nElements == m_nSize )
            {
                CObject **aTemp = new CObject *[ m_nSize + m_nScale ];

                for ( size_t nCount = 0; nCount < m_nSize ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete [] m_apElements;
                m_apElements = aTemp;
                m_nSize += m_nScale;
            }
        }
        else
        {
            if ( ( m_nSize - m_nElements ) > m_nScale )
            {
                CObject **aTemp = new CObject *[ m_nSize - m_nScale ];

                for ( size_t nCount = 0; nCount < m_nElements ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete [] m_apElements;
                m_apElements = aTemp;
                m_nSize -= m_nScale;
            }
        }
    }

    size_t CArray::Push( CObject *pNew )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        Resize( true );
        m_apElements[ m_nElements ] = pNew;
        m_nElements++;
        size_t retVal = m_nElements - 1;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
        return retVal;
    }

    size_t CArray::Set( size_t nIndex, CObject *pSet )
    {
        size_t retVal = nIndex;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( nIndex >= m_nElements )
        {
            Resize( true );
            m_apElements[ m_nElements ] = pSet;
            m_nElements++;
            retVal = m_nElements - 1;
        }
        m_apElements[ nIndex ] = pSet;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
        return retVal;
    }

    CObject *CArray::Get( size_t nCount ) const
    {
        CObject *retVal = NULL;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if (  nCount < m_nElements )
        {
            retVal = m_apElements[ nCount ];
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
        return retVal;
    }

    CObject *CArray::operator []( size_t nCount ) const
    {
        return Get( nCount );
    }

    CObject *CArray::Remove( size_t nCount )
    {
        CObject *pRetVal = NULL;

#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( nCount < m_nElements )
        {
            pRetVal = m_apElements[ nCount ];

            for ( size_t nIndex = nCount; nIndex < (m_nElements - 1) ; nIndex++ )
            {
                m_apElements[ nIndex ] = m_apElements[ nIndex + 1 ];
            }
            m_nElements--;
            Resize( false );
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif

        return pRetVal;
    }

    size_t CArray::Insert( size_t nCount, CObject *pNew )
    {
        size_t retVal = nCount;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( nCount >= m_nElements )
        {
            Resize( true );
            m_apElements[ m_nElements ] = pNew;
            m_nElements++;
            retVal = m_nElements - 1;
        }
        else
        {
            Resize( true );

            for ( size_t nIndex = m_nElements; nIndex > nCount ; nIndex-- )
            {
                m_apElements[ nIndex ] = m_apElements[ nIndex - 1 ];
            }
            m_apElements[ nCount ] = pNew;
            m_nElements++;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
        return retVal;
    }

    bool CArray::Delete( size_t nCount )
    {
        bool retVal = false;
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if (! ( nCount >= m_nElements ) )
        {
            delete m_apElements[ nCount ];
            for ( size_t nIndex = nCount; nIndex < (m_nElements - 1) ; nIndex++ )
            {
                m_apElements[ nIndex ] = m_apElements[ nIndex + 1 ];
            }
            m_nElements--;
            Resize( false );
            retVal = true;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
        return retVal;
    }

    void CArray::DeleteAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( m_nSize )
        {
            for ( size_t nCount = 0; nCount < m_nElements ; nCount++ )
            {
                CObject *pData = m_apElements[ nCount ];
                delete pData;
                m_apElements[ nCount ] = NULL;
            }
            delete [] m_apElements;
            m_nElements = 0;
            m_nSize = 0;
            m_apElements = NULL;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
    }

    void CArray::EmptyAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        if ( m_nSize )
        {
            for ( size_t nCount = 0; nCount < m_nElements ; nCount++ )
            {
                m_apElements[ nCount ] = NULL;
            }            
            delete [] m_apElements;
            m_nSize = 0;
            m_apElements = NULL;
            m_nElements = 0;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
    }

    void CArray::Sort( int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallback )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Lock();
#endif
        QuickSort( 0, m_nElements - 1, fnCompare, pCallback );
#ifdef IASLIB_MULTI_THREADED__
        m_mutex.Unlock();
#endif
    }


    void CArray::QuickSort( size_t nLow, size_t nHigh, int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallback )
    {
        CObject    *pPivot;
        size_t      nLo = nLow;
        size_t      nHi = nHigh;

        if ( nHigh > nLow)
        {
                // Arbitrarily choose the pivot point of the range
            pPivot = m_apElements[ (nLow + nHigh) / 2 ];

                // loop through the array until indices cross
               // Watch out for the unsigned wrap-around on "nHi"
            while ( ( nLo <= nHi ) && ( nHi <= nHigh ) )
            {
                    // find the first element that is greater than or equal to
                    // the partition element starting from the left Index.
                while( ( nLo < nHigh ) && ( (*fnCompare)( m_apElements[ nLo ], pPivot, pCallback ) < 0 ) )
                    ++nLo;

                    // find an element that is smaller than or equal to
                    // the partition element starting from the right Index.
                while( ( nHi > nLow ) && ( nHigh < m_nElements ) && ( (*fnCompare)( m_apElements[ nHi ], pPivot, pCallback ) > 0 ) )
                    --nHi;

                // if the indexes have not crossed, swap the elements
                if ( ( nLo <= nHi ) && ( nHi <= nHigh ) )
                {
                    CObject *pTemp = m_apElements[ nLo ];
                    m_apElements[ nLo ] = m_apElements[ nHi ];
                    m_apElements[ nHi ] = pTemp;

                    ++nLo;
                    --nHi;
                }
            }

                // If the right index has not reached the left side of array
                // must now sort the left partition.
            if ( ( nLow < nHi ) && ( nHi <= nHigh ) )
                QuickSort( nLow, nHi, fnCompare, pCallback );

                // If the left index has not reached the right side of array
                // must now sort the right partition.
            if ( nLo < nHigh )
                QuickSort( nLo, nHigh, fnCompare, pCallback );
        }
    }

	bool CArray::Swap( size_t nSwap1, size_t nSwap2 )
	{
		bool bRetVal = false;

		if ( ( nSwap1 < GetLength() ) && ( nSwap2 < GetLength() ) && ( nSwap1 != nSwap2 ) )
		{
			CObject *pTemp = m_apElements[ nSwap1 ];
			m_apElements[ nSwap1 ] = m_apElements[ nSwap2 ];
			m_apElements[ nSwap2 ] = pTemp;

			bRetVal = true;
		}
		return bRetVal;
	}

    CIterator *CArray::Enumerate( void )
    {
        return new CArrayIterator( this );
    }
} // namespace IASLib

