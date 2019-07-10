/***********************************************************************
**  CStringArray
**
**  Description:
**      This class is an array class for storing pointers derived from
** CObject. It can dynamically re-allocate to store data.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include <string>
#include "StringArray.h"

namespace IASLib
{
    class CStringArrayIterator : public CIterator
    {
        protected:
            CStringArray       *m_pArray;
            size_t              m_nCurrentPos;
        public:
                                CStringArrayIterator( CStringArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CStringArrayIterator, CIterator )
            virtual            ~CStringArrayIterator( void ) {}

            virtual CString     NextString( void );
            virtual CObject    *Next( void ) { return NULL; }
            virtual CObject    *Prev( void ) { return NULL; }

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

    CString CStringArrayIterator::NextString( void )
    {
        CString pRetVal = NULL;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            pRetVal = (*m_pArray)[ m_nCurrentPos ];
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    bool CStringArrayIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            bRetVal = true;
        }

        return bRetVal;
    }

    IMPLEMENT_OBJECT( CStringArray, CObject );

    CStringArray::CStringArray( size_t nScale )
    {
        if ( nScale > 0 )
            m_nScale = nScale;
        else
            m_nScale = 4;
        m_nSize = 0;
        m_nElements = 0;
        m_apElements = NULL;
    }

    CStringArray::CStringArray( const CStringArray &oSource )
    {
        m_nScale = oSource.m_nScale;
        m_nSize = oSource.m_nSize;
        m_nElements = oSource.m_nElements;
        m_apElements = new CString *[ m_nSize ];
        for ( int nX = 0; nX < m_nElements; nX++ )
        {
            m_apElements[nX] = new CString( *(oSource.m_apElements[nX]) );
        }
    }

    CStringArray &CStringArray::operator =( const CStringArray &oSource )
    {
        if ( &oSource != this )
        {
            this->DeleteAll();
            m_nScale = oSource.m_nScale;
            m_nSize = oSource.m_nSize;
            m_nElements = oSource.m_nElements;
            m_apElements = new CString *[ m_nSize ];
            for ( int nX = 0; nX < m_nElements; nX++ )
            {
                m_apElements[nX] = new CString( *(oSource.m_apElements[nX]) );
            }
        }

        return *this;
    }

    CStringArray::~CStringArray( void )
    {
        DeleteAll();
    }

    void CStringArray::Resize( bool bIncrease )
    {
        if ( bIncrease )
        {
            if ( m_nElements == m_nSize )
            {
                CString **aTemp = new CString *[ m_nSize + m_nScale ];

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
                CString **aTemp = new CString *[ m_nSize - m_nScale ];

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

    size_t CStringArray::Push( CString pNew )
    {
        Resize( true );
        m_apElements[ m_nElements ] = new CString( pNew );
        m_nElements++;

        return ( m_nElements - 1 );
    }

    CString CStringArray::Get( size_t nCount ) const
    {
        if ( ( nCount >= 0 ) && ( nCount < m_nElements ) )
        {
            return *(m_apElements[ nCount ]);
        }
        CString strRet;
        return strRet;
    }

    CString &CStringArray::operator []( size_t nCount ) const
    {
        static     CString strRet;


        if ( ( nCount >= 0 ) && ( nCount < m_nElements ) )
        {
            return *(m_apElements[ nCount ]);
        }
        return strRet;
    }

    size_t CStringArray::Insert( size_t nCount, CString pNew )
    {
        if ( nCount >= m_nElements )
        {
            return Push( pNew );
        }

        Resize( true );

        if ( nCount < 0 )
            nCount = 0;

        for ( size_t nIndex = m_nElements; nIndex > nCount ; nIndex-- )
        {
            m_apElements[ nIndex ] = m_apElements[ nIndex - 1 ];
        }
        m_apElements[ nCount ] = new CString( pNew );
        m_nElements++;

        return nCount;
    }

    bool CStringArray::Delete( size_t nCount )
    {
        if ( ( nCount >= m_nElements ) || ( nCount < 0 ) )
            return false;

        CString *pDel = m_apElements[ nCount ];

        for ( size_t nIndex = nCount; nIndex < (m_nElements - 1) ; nIndex++ )
        {
            m_apElements[ nIndex ] = m_apElements[ nIndex + 1 ];
        }
        m_nElements--;

        delete pDel;

        Resize( false );
        return true;
    }

    void CStringArray::DeleteAll( void )
    {
        if ( m_nSize )
        {
            for ( size_t nIndex = 0; nIndex < m_nElements; nIndex++ )
            {
                delete m_apElements[ nIndex ];
            }
            delete [] m_apElements;
            m_nElements = 0;
            m_nSize = 0;
            m_apElements = NULL;
        }
    }

    void CStringArray::EmptyAll( void )
    {
        if ( m_nSize )
        {
            delete [] m_apElements;
            m_nElements = 0;
            m_nSize = 0;
            m_apElements = NULL;
        }
    }

    CIterator *CStringArray::Enumerate( void )
    {
        return new CStringArrayIterator( this );
    }

    bool CStringArray::Sort( bool bAscending )
    {
        QuickSort( 0, m_nElements - 1, bAscending );

        return true;
    }


    void CStringArray::QuickSort( size_t nLow, size_t nHigh, bool bAscending )
    {
        CString    *pPivot;
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
                if ( bAscending )
                {
                   while( ( nLo < nHigh ) && ( *(m_apElements[ nLo ]) < *pPivot ) )
                       ++nLo;
                }
                else
                {
                   while( ( nLo < nHigh ) && ( *(m_apElements[ nLo ]) > *pPivot ) )
                       ++nLo;
                }

                    // find an element that is smaller than or equal to
                    // the partition element starting from the right Index.
                if ( bAscending )
                {
                    while( ( nHi > nLow ) && ( nHigh < m_nElements ) && ( *(m_apElements[ nHi ]) > *pPivot ) )
                        --nHi;
                }
                else
                {
                    while( ( nHi > nLow ) && ( nHigh < m_nElements ) && ( *(m_apElements[ nHi ]) < *pPivot ) )
                        --nHi;
                }

                // if the indexes have not crossed, swap the elements
                if ( ( nLo <= nHi ) && ( nHi <= nHigh ) )
                {
                    CString *pTemp = m_apElements[ nLo ];
                    m_apElements[ nLo ] = m_apElements[ nHi ];
                    m_apElements[ nHi ] = pTemp;

                    ++nLo;
                    --nHi;
                }
            }

                // If the right index has not reached the left side of array
                // must now sort the left partition.
            if ( ( nLow < nHi ) && ( nHi <= nHigh ) )
                QuickSort( nLow, nHi, bAscending );

                // If the left index has not reached the right side of array
                // must now sort the right partition.
            if ( nLo < nHigh )
                QuickSort( nLo, nHigh, bAscending );
        }
    }
} // namespace IASLib

