/**
 * Sparse Array Class
 *
 *  This class provides an array for storing sparse items in an array,
 * in other words, items that may be highly separated in array space. 
 * This class does this in a memory efficient means. There is a slight,
 * four byte, overhead per element for storage, but there is no penalty
 * no matter how large the array becomes. So, you could have ten elements
 * populated over a range of 10,000,000 elements, and this class would
 * only have an overhead of 40 bytes.
 *  Access to elements is O(log n) where n is the number of currently 
 * inserted elements.
 *  This class stores pointers to any object derived from CObject.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2/16/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SparseArray.h"
#include "CollectionException.h"

namespace IASLib
{
    class CSparseArrayElement : public CObject
    {
        protected:
            size_t      m_nIndex;
            CObject    *m_pDataItem;

        public:
                        DECLARE_OBJECT( CSparseArrayElement, CObject );

                        CSparseArrayElement( size_t nIndex, CObject *pData )
                        {
                            m_nIndex = nIndex;
                            m_pDataItem = pData;
                        }

            virtual    ~CSparseArrayElement( void )
                        {
                            delete m_pDataItem;
                        }

            size_t      GetIndex( void ) { return m_nIndex; }
            CObject    *GetData( void ) { return m_pDataItem; }
            void        SetData( CObject *pData ) { m_pDataItem = pData; }

            int         Compare( CSparseArrayElement *pCompare )
                        {
                            if ( m_nIndex < pCompare->m_nIndex )
                            {
                                return -1;
                            }
                            if ( m_nIndex > pCompare->m_nIndex )
                            {
                                return 1;
                            }
                            return 0;
                        }
    };

    CObject *CSparseArrayIterator::Next( void )
    {
        CObject *pRetVal = NULL;
        CSparseArrayElement *pElement;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            pElement = (CSparseArrayElement *)m_pArray->m_apElements[ m_nCurrentPos ];
            pRetVal = pElement->GetData();
            m_nSparseNumber = pElement->GetIndex();
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    CObject *CSparseArrayIterator::Prev( void )
    {
        CObject *pRetVal = NULL;
        CSparseArrayElement *pElement;

        if ( ( m_nCurrentPos > 0 ) && ( m_nCurrentPos <= m_pArray->GetLength() ) )
        {
            m_nCurrentPos--;
            pElement = (CSparseArrayElement *)m_pArray->m_apElements[ m_nCurrentPos ];
            m_nSparseNumber = pElement->GetIndex();
            pRetVal = pElement->GetData();
        }

        return pRetVal;
    }

    bool CSparseArrayIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            bRetVal = true;
        }
        
        return bRetVal;
    }



    CSparseArray::CSparseArray( size_t nGrowBy )
        : CSortedArray( CSparseArray::Compare, nGrowBy )
    {
    }

    CSparseArray::~CSparseArray( void )
    {
    }

    int CSparseArray::Compare( const CObject *pLeft, const CObject *pRight )
    {
        CSparseArrayElement *pLeftElement = (CSparseArrayElement *)pLeft;
        CSparseArrayElement *pRightElement = (CSparseArrayElement *)pRight;

        return pLeftElement->Compare( pRightElement );
    }

    size_t CSparseArray::Push( CObject *pNew )
    {
        pNew = pNew; // Avoids unused parameter exception
        IASLIB_THROW_COLLECTION_EXCEPTION( "Sparse Array does not support Append/Prepend/Push operations." );

        return 0;
    }

    size_t CSparseArray::Insert( size_t nCount, CObject *pNew )
    {
        CSparseArrayElement *pValue = new CSparseArrayElement( nCount, pNew );
        size_t  nFound = Find( pValue );

        if ( nFound > GetLength() )
        {
            nFound = (size_t) (-((int)nFound + 1));
            CSortedArray::Insert( nFound, pValue );
        }
        else
        {
            if ( ((CSparseArrayElement *)m_apElements[ nFound ])->GetData() != pNew )
            {
                delete m_apElements[ nFound ];
                m_apElements[ nFound ] = pValue;
            }
        }
        return nFound;
    }

    CObject *CSparseArray::Get( size_t nCount ) const
    {
        CObject *pRetVal = NULL;
        CSparseArrayElement oValue( nCount, NULL );

        size_t nFound = CSortedArray::Find( &oValue );

        if ( nFound < GetLength() )
        {
            CSparseArrayElement *pElement = (CSparseArrayElement *)m_apElements[ nFound ];
            pRetVal = pElement->GetData();
        }

        return pRetVal;
    }

    CObject *CSparseArray::operator []( size_t nCount ) const
    {
        return Get( nCount );
    }

    CObject *CSparseArray::Remove( size_t nCount )
    {
        CObject *pRetVal = NULL;
        CSparseArrayElement oValue( nCount, NULL );

        size_t nFound = Find( &oValue );

        if ( nFound < GetLength() )
        {
            CSparseArrayElement *pElement = (CSparseArrayElement *)m_apElements[ nFound ];
            pRetVal = pElement->GetData();
            CArray::Remove( nFound );
        }

        return pRetVal;
    }

    bool CSparseArray::Delete( size_t nCount )
    {
        bool bRetVal = false;
        CSparseArrayElement oValue( nCount, NULL );

        size_t nFound = Find( &oValue );

        if ( nFound < GetLength() )
        {
            CArray::Delete( nFound );
            bRetVal = true;
        }

        return bRetVal;
    }

    bool CSparseArray::Swap( size_t nSwap1, size_t nSwap2 )
    {
        CSparseArrayElement oFind1( nSwap1, NULL );
        CSparseArrayElement oFind2( nSwap2, NULL );

        size_t nFound1 = Find( &oFind1 );
        size_t nFound2 = Find( &oFind2 );

        if ( ( nFound1 > GetLength() ) || ( nFound2 > GetLength() ) )
        {
            return false;
        }

        CSparseArrayElement *pItem1 = (CSparseArrayElement *)m_apElements[ nFound1 ];
        CSparseArrayElement *pItem2 = (CSparseArrayElement *)m_apElements[ nFound2 ];
        CObject *pTemp = pItem1->GetData();
        pItem1->SetData( pItem2->GetData() );
        pItem2->SetData( pTemp );

        return true;
    }

    CIterator *CSparseArray::Enumerate( void )
    {
        return new CSparseArrayIterator( this );
    }

    bool CSparseArray::Exists( size_t nIndex )
    {
        CSparseArrayElement oFind( nIndex, NULL );
        size_t nFound = Find( &oFind );

        if ( nFound > GetLength() )
        {
            return false;
        }
        return true;
    }

} // namespace IASLib

