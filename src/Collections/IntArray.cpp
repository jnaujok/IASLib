/***********************************************************************
**  CIntArray
**
**  Description:
**      This class is an array class for storing pointers derived from
** CObject. It can dynamically re-allocate to store data.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include <stdlib.h>
#include "IntArray.h"

namespace IASLib
{
    class CIntArrayIterator : public CIterator
    {
        protected:
            CIntArray          *m_pArray;
            size_t              m_nCurrentPos;
        public:
                                CIntArrayIterator( CIntArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CIntArrayIterator, CIterator )
            virtual            ~CIntArrayIterator( void ) {}

            virtual int         NextInt( void );
            virtual CObject    *Next( void ) { return NULL; }
            virtual CObject    *Prev( void ) { return NULL; }

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

    int CIntArrayIterator::NextInt( void )
    {
        int pRetVal = 0;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            pRetVal = (*m_pArray)[ m_nCurrentPos ];
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    bool CIntArrayIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pArray->GetLength() ) )
        {
            bRetVal = true;
        }
        
        return bRetVal;
    }

    IMPLEMENT_OBJECT( CIntArray, CObject );

    CIntArray::CIntArray( size_t nScale )
    {
        if ( nScale > 0 )
            m_nScale = nScale;
        else
            m_nScale = 4;
        m_nSize = 0;
        m_nElements = 0;
        m_apElements = NULL;
    }

    CIntArray::~CIntArray( void )
    {
        DeleteAll();
    }

    void CIntArray::Resize( bool bIncrease )
    {
        if ( bIncrease )
        {
            if ( m_nElements == m_nSize )
            {
                int *aTemp = new int[ m_nSize + m_nScale ];

                for ( size_t nCount = 0; nCount < m_nSize ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete m_apElements;
                m_apElements = aTemp;
                m_nSize += m_nScale;
            }
        }
        else
        {
            if ( ( m_nSize - m_nElements ) > m_nScale )
            {
                int *aTemp = new int[ m_nSize - m_nScale ];

                for ( size_t nCount = 0; nCount < m_nElements ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete m_apElements;
                m_apElements = aTemp;
                m_nSize -= m_nScale;
            }
        }
    }

    size_t CIntArray::Push( int pNew )
    {
        Resize( true );
        m_apElements[ m_nElements ] = pNew;
        m_nElements++;

        return ( m_nElements - 1 );
    }

    int CIntArray::Get( size_t nCount )
    {
        if ( ( nCount >= 0 ) && ( nCount < m_nElements ) )
        {
            return m_apElements[ nCount ];
        }
        return 0;
    }

    int &CIntArray::operator []( size_t nCount )
    {
        static int nRet = -1;

        if ( ( nCount >= 0 ) && ( nCount < m_nElements ) )
        {
            return m_apElements[ nCount ];
        }
        return nRet;
    }

    size_t CIntArray::Insert( size_t nCount, int pNew )
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
        m_apElements[ nCount ] = pNew;
        m_nElements++;

        return nCount;
    }

    bool CIntArray::Delete( size_t nCount )
    {
        if ( ( nCount >= m_nElements ) || ( nCount < 0 ) )
            return false;

        for ( size_t nIndex = nCount; nIndex < (m_nElements - 1) ; nIndex++ )
        {
            m_apElements[ nIndex ] = m_apElements[ nIndex + 1 ];
        }
        m_nElements--;
        Resize( false );
        return true;
    }

    void CIntArray::DeleteAll( void )
    {
        if ( m_nSize )
        {
            delete m_apElements;
            m_nElements = 0;
            m_nSize = 0;
            m_apElements = NULL;
        }
    }

    void CIntArray::EmptyAll( void )
    {
        DeleteAll();
    }

    CIterator *CIntArray::Enumerate( void )
    {
        return new CIntArrayIterator( this );
    }
} // namespace IASLib
