/**
 * SIP Headers class
 *
 * This class encapsulates a set of SIP header fields in a SIP request or response.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#include "HeaderList.h"
#include "Header.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
    IMPLEMENT_OBJECT( CHeaderList, CObject );

    CHeaderList::CHeaderList( CStream &headerStream ) : m_hashHeaders( CHash::SMALL )
    {
        m_bModifiable = false;
        CStream &localStream = headerStream;

        if ( ! localStream.IsEOS() )
        {
            CString line = localStream.GetLine();

            // Headers are ended by a blank line.
            while ( line.GetLength() > 0 )
            {
                size_t nSeparator = line.IndexOf( ':' );

                if ( nSeparator != NOT_FOUND )
                {
                    CString name = line.Substring( 0, nSeparator );
                    CString hashName = name;
                    hashName.ToLowerCase();
                    CHeader *temp;

                    // Under standard rules, two lines with the same header name become an addative list.
                    if ( m_hashHeaders.HasKey( hashName ) )
                    {
                        temp = (CHeader *)m_hashHeaders.Get( hashName );
                    }
                    else
                    {
                        temp = newHeader(name);
                        m_hashHeaders.Push( hashName, temp );
                    }

                    // Deal with multi-tokens
                    temp->addValue( line.Substring( nSeparator + 1 ).Trim() );
                }
                if ( ! localStream.IsEOS() )
                {
                    line = localStream.GetLine();
                }
                else
                {
                    // Stream has ended.
                    break;
                }
            }

        }
    }

    CHeaderList::CHeaderList( CHeaderList &oSource ) : m_hashHeaders( CHash::SMALL )
    {
        m_bModifiable = true;
        m_hashHeaders = oSource.m_hashHeaders;
    }

    CHeaderList::CHeaderList( void )  : m_hashHeaders( CHash::SMALL )
    {
        m_bModifiable = true;
    }


    CHeaderList::~CHeaderList( void )
    {
        m_hashHeaders.DeleteAll();
    }

    CHeader *CHeaderList::newHeader( CString &name )
    {
        return new CHeader( name );
    }

    void CHeaderList::addHeader( const char *key, const char *value )
    {
        CString hashKey = key;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *temp = (CHeader *)m_hashHeaders.Get(hashKey);
            temp->setValue( value );
        }
        else
        {
            CHeader *temp = new CHeader( key );
            temp->setValue( value );
            m_hashHeaders.Push( hashKey, temp );
        }
    }

    void CHeaderList::addHeader( const char *key, IASLib::CStringArray &values )
    {
        CString hashKey = key;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *temp = (CHeader *)m_hashHeaders.Get(hashKey);
            for ( size_t nX = 0; nX < values.Count(); nX++ )
            {
                temp->addValue( values[nX]);
            }
        }
        else
        {
            CHeader *temp = new CHeader( key );
            for ( size_t nX = 0; nX < values.Count(); nX++ )
            {
                temp->addValue( values[nX]);
            }
            m_hashHeaders.Push(hashKey, temp );
        }
    }

    bool CHeaderList::hasHeader( const char *name )
    {
        CString hashKey = name;
        hashKey.ToLowerCase();
        return m_hashHeaders.HasKey( hashKey );
    }

    // Returns an unmodifiable List of all of the header string values
    // of the given named header.
    const CStringArray CHeaderList::allValues( const char *name )
    {
        CString hashKey = name;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *header = (CHeader *)m_hashHeaders.Get(hashKey);
            return header->getValues();
        }
        return CStringArray();
    }

    // Returns the first header string value of the given named (and
    // possibly multi-valued) header.
    CString CHeaderList::firstValue( const char *name )
    {
        CString hashKey = name;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *header = (CHeader *)m_hashHeaders.Get( hashKey );
            if ( header != NULL )
            {
                return header->getValues()[0];
            }
        }
        return NULL;
    }

    // Returns the first header string value of the given named (and
    // possibly multi-valued) header.
    long CHeaderList::firstValueAsLong( const char *name )
    {
        CString hashKey = name;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *header = (CHeader *)m_hashHeaders.Get(hashKey);
            if ( header != NULL )
            {
                CString val = header->getValue();
                if ( val.IsDigits() )
                    return atol(val);
            }
        }
        return 0L;
    }

    // Returns an unmodifiable multi Map view of the entire set of
    // headers in the form Key: CString Value: CStringArray. Note that
    // this hash map must be deleted by the caller.
    CHash *CHeaderList::map( void )
    {
        CIterator *iter = m_hashHeaders.Enumerate();
        CHash *retVal = new CHash( CHash::SMALL );

        while ( iter->HasMore() )
        {
            CHeader *header = (CHeader *)iter->Next();
            CString name = header->getName();
            CStringArray *values = new CStringArray();
            *values = header->getValues();
            retVal->Push( name, (CObject *)values );
        }
        delete iter;

        return retVal;
    }

    CString CHeaderList::toString( void )
    {
        CIterator *iter = m_hashHeaders.Enumerate();
        CString retVal;

        while ( iter->HasMore() )
        {
            CHeader *header = (CHeader *)iter->Next();
            CString name = header->getName();
            CStringArray *values = new CStringArray();
            *values = header->getValues();
            for ( size_t nX = 0; nX < values->GetCount(); nX++ )
            {
                retVal += name;
                retVal += ": ";
                retVal += (*values)[nX];
                retVal += "\r\n";
            }
            delete values;
        }
        delete iter;

        return retVal;
    }

    CHeader *CHeaderList::getHeader( const char *key )
    {
        CString hashKey = key;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *header = (CHeader *)m_hashHeaders.Get( hashKey );
            if ( header != NULL )
            {
                return header;
            }
        }
        return NULL;
    }

    void CHeaderList::removeHeader( const char *key )
    {
        CString hashKey = key;
        hashKey.ToLowerCase();

        if ( m_hashHeaders.HasKey( hashKey ) )
        {
            CHeader *header = (CHeader *)m_hashHeaders.Remove( hashKey );
            if ( header != NULL )
            {
                delete header;
            }
        }
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__