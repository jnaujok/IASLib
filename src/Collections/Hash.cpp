/*
 * Hash
 *
 *  This object provides a string keyed hash for looking up keyed items
 * very, very rapidly. It makes use of one other class, the Hash Bucket,
 * which is placed at each array entry to store data that has been
 * hashed. Array entries that do not have any data are simply set to
 * null rather than waste the memory for a hash bucket.
 *
 *  The default hash table is either 107, 347, or 1019 elements, with
 * hash keys of either 191, 421, or 1607 respectively. The initilizer
 * values of "small", "normal", or "large" are passed to the constructor
 * to determine what sizes are being used. The default size is "small".
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1995
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#include <stdio.h>
#include <stdlib.h>

#ifdef IASLIB_DATABASE__
#include "Cursor.h"
#endif

#include "../../inc/Collections/Hash.h"

namespace IASLib
{
    class CHashIterator : public CIterator
    {
        protected:
            CHash              *m_pHash;
            size_t              m_nCurrentPos;
        public:
                                CHashIterator( CHash *pHash ) { m_pHash = pHash; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CHashIterator, CIterator )
            virtual            ~CHashIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

    CObject *CHashIterator::Next( void )
    {
        CObject *pRetVal = NULL;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pHash->GetLength() ) )
        {
            pRetVal = m_pHash->Enum( m_nCurrentPos );
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    CObject *CHashIterator::Prev( void )
    {
        CObject *pRetVal = NULL;

        if ( ( m_nCurrentPos > 0 ) && ( m_nCurrentPos <= m_pHash->GetLength() ) )
        {
            m_nCurrentPos--;
            pRetVal = m_pHash->Enum( m_nCurrentPos );
        }

        return pRetVal;
    }

    bool CHashIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if ( ( m_nCurrentPos >= 0 ) && ( m_nCurrentPos < m_pHash->GetLength() ) )
        {
            bRetVal = true;
        }
        
        return bRetVal;
    }

    IMPLEMENT_OBJECT( CHash, CObject );

    CHash::CHash( const char *strSize )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        m_strSize = strSize;
        m_strSize.Trim();
        m_strSize.ToUpperCase();

        // The length value should reflect the number of properties in the object, including
        // the length property.

        if ( m_strSize == "GARGANTUAN" )
        {
            m_nArraySize = 509623;
            m_nHashKey = 834299 ;
        }
        else
        {
            if ( m_strSize == "HUGE" )
            {
                m_nArraySize = 20333;
                m_nHashKey = 32713;
            }
            else
            {
                if ( m_strSize == "LARGE" )
                {
                    m_nArraySize = 1019;
                    m_nHashKey = 1607;
                }
                else
                {
                    if ( m_strSize == "NORMAL" )
                    {
                        m_nArraySize = 347;
                        m_nHashKey = 421;
                    }
                    else
                    {
                        m_nArraySize = 107;
                        m_nHashKey = 191;
                    }
                }
            }
        }
        
        m_aHashTable = new CHashBucket *[ m_nArraySize ];
        for ( size_t nCount = 0; nCount < m_nArraySize; nCount ++ )
            m_aHashTable[ nCount ] = NULL;

        m_nElements = 0;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

#ifdef IASLIB_DATABASE__
    CHash::CHash( CCursor *pData, const char *strKey, const char *strData )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        size_t nRows = 0;
        if ( pData )
            nRows = pData->Rows();

        // The length value should reflect the number of properties in the object, including
        // the length property.

        if ( nRows > 1500 )
        {
            m_strSize = "HUGE";
            m_nArraySize = 20333;
            m_nHashKey = 32713;
        }
        else
        {
            if ( nRows > 500 )
            {
                m_strSize = "LARGE";
                m_nArraySize = 1019;
                m_nHashKey = 1607;
            }
            else
            {
                if ( nRows > 150 )
                {
                    m_strSize = "NORMAL";
                    m_nArraySize = 347;
                    m_nHashKey = 421;
                }
                else
                {
                    m_strSize = "SMALL";
                    m_nArraySize = 107;
                    m_nHashKey = 191;
                }
            }
        }

        m_aHashTable = new CHashBucket *[ m_nArraySize ];
        for ( size_t nCount = 0; nCount < m_nArraySize; nCount ++ )
            m_aHashTable[ nCount ] = NULL;

        m_nElements = 0;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        while ( ( pData ) && ( pData->Rows() > m_nElements ) )
        {
            CString *pstrData = new CString( pData->GetColumn( strData ) );
            Push( pData->GetColumn( strKey ), pstrData, true );
            pData->Next();
        }
    }
#endif // IASLIB_DATABASE__

    CHash::~CHash( void )
    {
        DeleteAll();
        for ( size_t nCount = 0; nCount < m_nArraySize; nCount ++ )
            m_aHashTable[ nCount ] = NULL;

        delete m_aHashTable;
        m_nElements = 0;
        m_nArraySize = 0;
    }

    /************************************************************************************
    ** Push
    **
    **  This function adds an element to the hash.
    **
    **************************************************************************************/
    void CHash::Push( const char *strKey, CObject *pElement, bool bDeleteCurrent )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        try
        {
            int nKey = BuildKey( strKey );

            if ( m_aHashTable[ nKey ] == NULL )
            {
                m_aHashTable[ nKey ] = new CHashBucket();
            }

            if ( m_aHashTable[ nKey ]->Push( strKey, pElement, bDeleteCurrent ) )
                m_nElements++;
        }
        catch(...)
        {
            // Do nothing, this is just to make sure we always unlock the mutex
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** Push
    **
    **  This function adds an element to the hash.
    **
    **************************************************************************************/
    void CHash::Push( int nKey, CObject *pElement, bool bDeleteCurrent )
    {
        int nSaveKey = nKey;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif

        nKey = BuildKey( nKey );

        if ( m_aHashTable[ nKey ] == NULL )
        {
            m_aHashTable[ nKey ] = new CHashBucket();
        }

        if ( m_aHashTable[ nKey ]->Push( nSaveKey, pElement, bDeleteCurrent ) )
            m_nElements++;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** Get
    **
    **  This function retrieves an element from the hash.
    **
    **************************************************************************************/
    CObject *CHash::Get( const char *strKey )
    {
        CObject *pRetVal = NULL;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        int nKey = BuildKey( strKey );

        if ( m_aHashTable[ nKey ] )
        {
            pRetVal = m_aHashTable[ nKey ]->Get( strKey );
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        return pRetVal;
    }

    /*************************************************************************************
    ** HasKey
    **
    **  This function Checks if a key is represented in the hash.
    **
    **************************************************************************************/
    bool CHash::HasKey( const char *strKey )
    {
        bool bRetVal = false;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        int nKey = BuildKey( strKey );

        if ( m_aHashTable[ nKey ] )
        {
            bRetVal = m_aHashTable[ nKey ]->HasKey( strKey );
        }

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        return bRetVal;
    }

    /*************************************************************************************
    ** Get
    **
    **  This function retrieves an element from the hash.
    **
    **************************************************************************************/
    CObject *CHash::Get( int nKey )
    {
        int nSaveKey = nKey;
        CObject *pRetVal = NULL;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif

        nKey = BuildKey( nKey );

        if ( m_aHashTable[ nKey ] )
        {
            pRetVal = m_aHashTable[ nKey ]->Get( nSaveKey );
        }

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        return pRetVal;
    }

    /*************************************************************************************
    ** HasKey
    **
    **  This function Checks if a key is represented in the hash.
    **
    **************************************************************************************/
    bool CHash::HasKey( int nKey )
    {
        int nSaveKey = nKey;
        bool bRetVal = false;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif

        nKey = BuildKey( nKey );

        if ( m_aHashTable[ nKey ] )
        {
            bRetVal = m_aHashTable[ nKey ]->HasKey( nSaveKey );
        }

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        return  bRetVal;
    }

    /*************************************************************************************
    ** BuildKey
    **
    **  This function builds the hash key from the string key and the two hash constants.
    **
    **************************************************************************************/
    int CHash::BuildKey( const char *strKey )
    {
        if ( strKey == NULL )
            return 0;

        int nCount = 0;
        int nWork = 0;
        int nLength = 0;


        while ( ( *strKey ) && ( ( *strKey == ' ' ) || ( *strKey == '\t' ) || ( *strKey == '\n' ) || ( *strKey == '\r' ) ) )
        {
            strKey++;
        }

        while ( strKey[ nCount ] )
        {
            if ( ( strKey[ nCount ] != ' ' ) && ( strKey[ nCount ] != '\t' ) && ( strKey[ nCount ] != '\n' ) && ( strKey[ nCount ] != '\r' ) )
            {
                nLength = nCount;
            }
            nCount++;
        }

        nCount = 0;

        while ( ( *strKey ) && ( nLength > 0 ) )
        {
            nWork += (unsigned int)( *strKey ) * ( nCount + 1 );
            strKey++;
            nCount++;
            nLength--;
        }

        return BuildKey( nWork );
    }

    int CHash::BuildKey( int nKey )
    {
        int nWork;
        unsigned long nTemp = (unsigned long)nKey * (unsigned long)m_nHashKey;
        unsigned long nTemp2 = nTemp % (unsigned long)m_nArraySize;
        nWork = (int)nTemp2;
        return nWork;
    }

    /*************************************************************************************
    ** DeleteAll
    **
    **  This function removes all of the elements from the hash.
    **
    **************************************************************************************/
    void CHash::DeleteAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        size_t nKey = 0;

        while ( nKey < m_nArraySize )
        {
            if ( m_aHashTable[ nKey ] != NULL )
            {
                m_aHashTable[ nKey ]->DeleteAll();
                delete m_aHashTable[ nKey ];
            }
            m_aHashTable[ nKey ] = NULL;
            nKey++;
        }
        m_nElements = 0;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** EmptyAll
    **
    **  This function empties the hash but does not delete the elements.
    **
    **************************************************************************************/
    void CHash::EmptyAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        size_t nKey = 0;

        while ( nKey < m_nArraySize )
        {
            if ( m_aHashTable[ nKey ] != NULL )
            {
                m_aHashTable[ nKey ]->Empty();
                delete m_aHashTable[ nKey ];
            }
            m_aHashTable[ nKey ] = NULL;
            nKey++;
        }
        m_nElements = 0;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** Enum
    **
    **  This function returns an enumerated item from the hash. It is terribly inefficient
    ** and should only be used as a last resort or to ensure a walk of all elements.
    **
    **************************************************************************************/
    CObject *CHash::Enum( size_t nIndex )
    {
        size_t nWork = nIndex;
        size_t nKey = 0;

        if ( nIndex < m_nElements )
        {
#ifdef IASLIB_MULTI_THREADED__
            m_mutexProtect.Lock();
#endif
            while ( nKey < m_nArraySize )
            {
                if ( m_aHashTable[ nKey ] != NULL )
                {
                    if ( m_aHashTable[ nKey ]->GetLength() > nWork )
                    {
#ifdef IASLIB_MULTI_THREADED__
                        m_mutexProtect.Unlock();
#endif
                        return m_aHashTable[ nKey ]->Enum( nWork );
                    }
                    nWork -= m_aHashTable[nKey]->GetLength();
                }
                nKey++;
            }

#ifdef IASLIB_MULTI_THREADED__
            m_mutexProtect.Unlock();
#endif
        }
        return NULL;
    }

    /*************************************************************************************
    ** Delete
    **
    **  This function deletes an element from the hash.
    **
    **************************************************************************************/
    void CHash::Delete( const char *strKey )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        int nKey = BuildKey( strKey );

        if ( m_aHashTable[ nKey ] == NULL )
        {
#ifdef IASLIB_MULTI_THREADED__
            m_mutexProtect.Unlock();
#endif
            return;
        }

            // If the deletion results in an empty bucket, delete the bucket.
        if ( m_aHashTable[ nKey ]->Delete( strKey ) )
        {
            delete m_aHashTable[ nKey ];
            m_aHashTable[ nKey ] = NULL;
            m_nElements--;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** Delete
    **
    **  This function deletes an element from the hash.
    **
    **************************************************************************************/
    void CHash::Delete( int nKey )
    {
        int nSaveKey = nKey;

#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif

        nKey = ( nKey * m_nHashKey ) % (int)m_nArraySize;

        if ( m_aHashTable[ nKey ] == NULL )
        {
#ifdef IASLIB_MULTI_THREADED__
            m_mutexProtect.Unlock();
#endif
            return;
        }

            // If the deletion results in an empty bucket, delete the bucket.
        if ( m_aHashTable[ nKey ]->Delete( nSaveKey ) )
        {
            delete m_aHashTable[ nKey ];
            m_aHashTable[ nKey ] = NULL;
            m_nElements--;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    /*************************************************************************************
    ** Empty
    **
    **  This function removes all of the elements from the hash without deleting them.
    **
    **************************************************************************************/
    void CHash::Empty( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        size_t nKey = 0;

        while ( nKey < m_nArraySize )
        {
            if ( m_aHashTable[ nKey ] != NULL )
            {
                m_aHashTable[ nKey ]->Empty();
                delete m_aHashTable[ nKey ];
            }
            m_aHashTable[ nKey ] = NULL;
            nKey++;
        }
        m_nElements = 0;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
    }

    CIterator *CHash::Enumerate( void )
    {
        return new CHashIterator( this );
    }

} // namespace IASLib
