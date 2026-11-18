/*
 * CHash Class
 *
 *  A hash table of CObject pointers by string key.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Hash.h"

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    #define IASLIB_HASH_LOCK__      CMutexLock oLock__( m_mutexProtect )
#else
    #define IASLIB_HASH_LOCK__      do { } while ( 0 )
#endif

    namespace
    {
            // Walks the table's entries in order. Like every collection
            // iterator, it is undone by changes to the table.
        class CHashIterator : public CIterator
        {
            protected:
                const CHash::Table     *m_pTable;
                CHash::Table::const_iterator m_itCurrent;
                size_t                  m_nPosition;
            public:
                                    CHashIterator( const CHash::Table *pTable ) : m_pTable( pTable ), m_itCurrent( pTable->begin() ), m_nPosition( 0 ) {}
                                    DECLARE_OBJECT( CHashIterator, CIterator )
                virtual            ~CHashIterator( void ) {}
                virtual CObject    *Next( void )
                {
                    if ( m_itCurrent == m_pTable->end() )
                        return NULL;
                    CObject *pValue = m_itCurrent->Value;
                    ++m_itCurrent;
                    m_nPosition++;
                    return pValue;
                }
                virtual CObject    *Prev( void )
                {
                    if ( m_nPosition == 0 )
                        return NULL;
                    m_nPosition--;
                    m_itCurrent = m_pTable->begin();
                    for ( size_t n = 0; n < m_nPosition; n++ )
                        ++m_itCurrent;
                    return m_itCurrent->Value;
                }
                virtual void        Reset( void ) { m_itCurrent = m_pTable->begin(); m_nPosition = 0; }
                virtual bool        HasMore( void ) const { return m_itCurrent != m_pTable->end(); }
        };
    }

    IMPLEMENT_OBJECT( CHash, CCollection );

    CHash::CHash( HASH_SIZE eSize )
    {
        static const char *SIZES[] = { "TINY", "SMALL", "NORMAL", "LARGE", "HUGE", "GARGANTUAN" };
        Init( ( eSize >= TINY && eSize <= GARGANTUAN ) ? SIZES[ eSize ] : "NORMAL" );
    }

    CHash::CHash( const char *strSize )
    {
        Init( strSize );
    }

    void CHash::Init( const char *strSize )
    {
            // The table grows as entries are added, so the size is only a
            // name now (it used to fix the number of buckets).
        m_strSize = ( strSize != NULL ) ? strSize : "NORMAL";
        m_strSize.Trim();
        m_strSize.ToUpperCase();
        if ( m_strSize != "TINY" && m_strSize != "SMALL" && m_strSize != "NORMAL" && m_strSize != "LARGE" &&
             m_strSize != "HUGE" && m_strSize != "GARGANTUAN" )
        {
            m_strSize = "NORMAL";
        }
    }

#ifdef IASLIB_DATABASE__
    CHash::CHash( CCursor *pData, const char *strKey, const char *strData )
    {
        Init( "NORMAL" );
        if ( pData != NULL )
        {
            m_hashTable.Reserve( pData->Rows() );
            size_t nRows = pData->Rows();
            for ( size_t nRow = 0; nRow < nRows; nRow++ )
            {
                Push( pData->GetColumn( strKey ), new CString( pData->GetColumn( strData ) ), true );
                pData->Next();
            }
        }
    }
#endif // IASLIB_DATABASE__

    CHash::~CHash( void )
    {
        DeleteAll();
    }

    CString CHash::NormalizeKey( const char *strKey )
    {
        CString strNormal( strKey );
        strNormal.Trim( " \t\r\n" );
        return strNormal;
    }

    CString CHash::NormalizeKey( int nKey )
    {
        CString strNormal;
        strNormal = nKey;
        return strNormal;
    }

    size_t CHash::GetLength( void ) const
    {
        IASLIB_HASH_LOCK__;
        return m_hashTable.Length();
    }

    void CHash::Push( const char *strKey, CObject *pElement, bool bDeleteCurrent )
    {
        CString strNormal = NormalizeKey( strKey );
        CObject *pReplaced = NULL;
        {
            IASLIB_HASH_LOCK__;
            CObject **ppCurrent = m_hashTable.Find( strNormal );
            if ( ppCurrent != NULL )
            {
                if ( *ppCurrent != pElement && bDeleteCurrent )
                    pReplaced = *ppCurrent;
                *ppCurrent = pElement;
            }
            else
            {
                m_hashTable.Put( strNormal, pElement );
            }
        }
        delete pReplaced;
    }

    void CHash::Push( int nKey, CObject *pElement, bool bDeleteCurrent )
    {
        Push( NormalizeKey( nKey ), pElement, bDeleteCurrent );
    }

    CObject *CHash::Get( const char *strKey ) const
    {
        CString strNormal = NormalizeKey( strKey );
        IASLIB_HASH_LOCK__;
        return m_hashTable.Get( strNormal, NULL );
    }

    CObject *CHash::Get( int nKey ) const
    {
        return Get( NormalizeKey( nKey ) );
    }

    bool CHash::HasKey( const char *strKey ) const
    {
        CString strNormal = NormalizeKey( strKey );
        IASLIB_HASH_LOCK__;
        return m_hashTable.HasKey( strNormal );
    }

    bool CHash::HasKey( int nKey ) const
    {
        return HasKey( NormalizeKey( nKey ) );
    }

    CObject *CHash::Remove( const char *strKey )
    {
        CString strNormal = NormalizeKey( strKey );
        IASLIB_HASH_LOCK__;
        CObject *pValue = NULL;
        m_hashTable.Remove( strNormal, pValue );
        return pValue;
    }

    CObject *CHash::Remove( int nKey )
    {
        return Remove( NormalizeKey( nKey ) );
    }

    void CHash::Delete( const char *strKey )
    {
        delete Remove( strKey );
    }

    void CHash::Delete( int nKey )
    {
        delete Remove( nKey );
    }

    void CHash::DeleteAll( void )
    {
        Table oTaken;
        {
            IASLIB_HASH_LOCK__;
            oTaken.Swap( m_hashTable );
        }
        oTaken.DeleteAll();
    }

    void CHash::EmptyAll( void )
    {
        IASLIB_HASH_LOCK__;
        Table oEmpty;
        oEmpty.Swap( m_hashTable );
    }

    CObject *CHash::Enum( size_t nIndex ) const
    {
        IASLIB_HASH_LOCK__;
        if ( nIndex >= m_hashTable.Length() )
            return NULL;
        Table::const_iterator it = m_hashTable.begin();
        while ( nIndex-- > 0 )
            ++it;
        return it->Value;
    }

    CIterator *CHash::Enumerate( void )
    {
        return new CHashIterator( &m_hashTable );
    }

    CStringArray CHash::keySet( void ) const
    {
        CStringArray aKeys;
        IASLIB_HASH_LOCK__;
        for ( Table::const_iterator it = m_hashTable.begin(); it != m_hashTable.end(); ++it )
            aKeys.Push( it->Key );
        return aKeys;
    }
} // namespace IASLib
