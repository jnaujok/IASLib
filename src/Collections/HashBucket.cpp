/*************************************************************************************
** HashBucket Object
**
**  This file defines and implments the hash bucket object. Hash buckets are used as
** the individual entries in a hash table and may contain an array of elements, keyed
** on a string.
**
**************************************************************************************/

#include <stdlib.h>
#include "HashBucket.h"
#include "String_.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHashBucket, CObject );

    CHashBucket::CHashBucket( void )
    {
        m_aSlats = NULL;
        m_nSlats = 0;
    }

    CHashBucket::~CHashBucket( void )
    {
        DeleteAll();
    }

    /*************************************************************************************
    ** GetLength
    **
    **  This function returns the number of items in the current hash bucket.
    **
    **************************************************************************************/
    size_t CHashBucket::GetLength( void )
    {
        return m_nSlats;
    }

    /*************************************************************************************
    ** Push
    **
    **  This function adds an element or slat to the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::Push( const char *strKey, CObject *pElement, bool bDeleteCurrent )
    {
        CObject *pTemp = Get( strKey );

        if ( pTemp == NULL )
        {
            CHashSlat *pSlat = new CHashSlat( strKey, pElement );
            if ( ( m_nSlats % 4 ) == 0 )
            {
                CHashSlat **aTemp = new CHashSlat *[ m_nSlats + 4 ];
                for ( size_t nStep = 0; nStep < m_nSlats ; nStep++ )
                {
                    aTemp[ nStep ] = m_aSlats[ nStep ];
                }
                delete m_aSlats;
                m_aSlats = aTemp;
            }
            m_aSlats[ m_nSlats ] = pSlat;
            m_nSlats++;
            return true;
        }
        else
        {
            size_t nCount = 0;

            while ( nCount < m_nSlats )
            {
                if ( m_aSlats[ nCount ]->GetKey() == strKey )
                {
                    m_aSlats[ nCount ]->SetElement( pElement, bDeleteCurrent );
                    break;
                }
                nCount++;
            }
        }
        return false;
    }

    /*************************************************************************************
    ** Push
    **
    **  This function adds an element or slat to the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::Push( int nKey, CObject *pElement, bool bDeleteCurrent )
    {
        CString strKey;
        strKey = nKey;

        return Push( strKey, pElement, bDeleteCurrent );
    }

    /*************************************************************************************
    ** Get
    **
    **  This function retrieves an element or slat from the hash bucket.
    **
    **************************************************************************************/
    CObject *CHashBucket::Get( const char *strKey )
    {
        if ( m_nSlats == 0 )
        {
            return NULL;
        }

        CString strWork1 = strKey;
        strWork1.Trim();
        CString strWork2;
        size_t nCount = 0;
        CObject *pRetVal = NULL;

        while ( nCount < m_nSlats )
        {
            strWork2 = m_aSlats[ nCount ]->GetKey();
            strWork2.Trim();
            if ( strWork2 == strWork1 )
            {
                pRetVal = m_aSlats[ nCount ]->GetElement();
                break;
            }
            nCount++;
        }

        return pRetVal;
    }

    /*************************************************************************************
    ** HasKey
    **
    **  This function checks if a key is filled in the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::HasKey( const char *strKey )
    {
        if ( m_nSlats == 0 )
        {
            return false;
        }

        CString strWork1 = strKey;
        strWork1.Trim();
        CString strWork2;
        size_t nCount = 0;

        while ( nCount < m_nSlats )
        {
            strWork2 = m_aSlats[ nCount ]->GetKey();
            strWork2.Trim();
            if ( strWork2 == strWork1 )
            {
                return true;
            }
            nCount++;
        }

        return false;
    }

    /*************************************************************************************
    ** Get
    **
    **  This function retrieves an element or slat from the hash bucket.
    **
    **************************************************************************************/
    CObject *CHashBucket::Get( int nKey )
    {
        CString strKey;
        strKey = nKey;

        return Get( strKey );
    }

    /*************************************************************************************
    ** HasKey
    **
    **  This function checks if a key is filled in the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::HasKey( int nKey )
    {
        CString strKey;
        strKey = nKey;

        return HasKey( strKey );
    }


    /*************************************************************************************
    ** Enum
    **
    **  This function retrieves an element or slat from the hash bucket based on its index.
    **
    **************************************************************************************/
    CObject *CHashBucket::Enum( size_t nIndex )
    {
        if ( nIndex >= m_nSlats )
        {
            return NULL;
        }

        return m_aSlats[ nIndex ]->GetElement();
    }

    /*************************************************************************************
    ** DeleteAll
    **
    **  This function removes all of the elements from the hash bucket.
    **
    **************************************************************************************/
    void CHashBucket::DeleteAll( void )
    {
        for ( size_t nCount = 0; nCount < m_nSlats ; nCount++ )
        {
            delete m_aSlats[ nCount ];
        }
        delete [] m_aSlats;
        m_aSlats = NULL;
        m_nSlats = 0;
    }

    /*************************************************************************************
    ** Delete
    **
    **  This function deletes an element or slat from the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::Delete( const char *strKey )
    {
        CString strWork1 = strKey;
        strWork1.Trim();
        CString strWork2;
        size_t nCount = 0;
        CObject *pRetVal = NULL;

        while ( nCount < m_nSlats )
        {
            strWork2 = m_aSlats[ nCount ]->GetKey();
            strWork2.Trim();
            if ( strWork2 == strWork1 )
            {
                pRetVal = m_aSlats[ nCount ]->GetElement();
                delete pRetVal;
                size_t nStep = nCount + 1;
                while ( nStep < m_nSlats )
                {
                    m_aSlats[ nStep - 1 ] = m_aSlats[ nStep ];
                    nStep++;
                }
                m_nSlats--;
                if ( m_nSlats == 0 )
                {
                    delete m_aSlats;
                    m_aSlats = NULL;
                    return true;
                }
                return false;
            }
            nCount++;
        }

        return false;
    }

    /*************************************************************************************
    ** Delete
    **
    **  This function deletes an element or slat from the hash bucket.
    **
    **************************************************************************************/
    bool CHashBucket::Delete( int nKey )
    {
        CString strKey;
        strKey = nKey;

        return Delete( strKey );
    }

    /*************************************************************************************
    ** Remove
    **
    **  This function removes (returns and deletes) an element or slat from the hash bucket.
    **
    **************************************************************************************/
    CObject *CHashBucket::Remove( const char *strKey )
    {
        CString strWork1 = strKey;
        strWork1.Trim();
        CString strWork2;
        size_t nCount = 0;
        CObject *pRetVal = NULL;

        while ( nCount < m_nSlats )
        {
            strWork2 = m_aSlats[ nCount ]->GetKey();
            strWork2.Trim();
            if ( strWork2 == strWork1 )
            {
                pRetVal = m_aSlats[ nCount ]->GetElement();
                size_t nStep = nCount + 1;
                while ( nStep < m_nSlats )
                {
                    m_aSlats[ nStep - 1 ] = m_aSlats[ nStep ];
                    nStep++;
                }
                m_nSlats--;
                if ( m_nSlats == 0 )
                {
                    delete m_aSlats;
                    m_aSlats = NULL;
                }
            }
            nCount++;
        }

        return pRetVal;
    }

    /*************************************************************************************
    ** Delete
    **
    **  This function deletes an element or slat from the hash bucket.
    **
    **************************************************************************************/
    CObject *CHashBucket::Remove( int nKey )
    {
        CString strKey;
        strKey = nKey;

        return Remove( strKey );
    }

    /*************************************************************************************
    ** Empty
    **
    **  This function removes all of the elements from the hash bucket without deleting
    ** them.
    **
    **************************************************************************************/
    void CHashBucket::Empty( void )
    {
        for ( size_t nCount = 0; nCount < m_nSlats ; nCount++ )
        {
            m_aSlats[nCount]->Empty();
            delete m_aSlats[ nCount ];
        }
        delete m_aSlats;
        m_aSlats = NULL;
        m_nSlats = 0;
    }

    void CHashBucket::keySet( CStringArray &oKeyArray )
    {
        for ( size_t nCount = 0; nCount < m_nSlats ; nCount++ )
        {
            oKeyArray.Append( m_aSlats[nCount]->GetKey() );
        }
    }
} // namespace IASLib

