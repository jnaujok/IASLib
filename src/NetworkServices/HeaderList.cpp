/**
 * Header List class
 *
 * This class encapsulates a set of header fields in an HTTP or SIP
 * style request or response.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "HeaderList.h"
#include "Header.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
    IMPLEMENT_OBJECT( CHeaderList, CObject );

    CHeaderList::CHeaderList( CStream &headerStream ) : m_aHeaders()
    {
        m_bModifiable = false;
        m_bComplete = false;
        m_bMalformed = false;
        parse( headerStream );
    }

    CHeaderList::CHeaderList( const CHeaderList &oSource ) : CObject(), m_aHeaders()
    {
        m_bModifiable = true;
        m_bComplete = false;
        m_bMalformed = false;
        copyFrom( oSource );
    }

    CHeaderList::CHeaderList( void ) : m_aHeaders()
    {
        m_bModifiable = true;
        m_bComplete = false;
        m_bMalformed = false;
    }

    CHeaderList &CHeaderList::operator =( const CHeaderList &oSource )
    {
        if ( this != &oSource )
        {
            clear();
            copyFrom( oSource );
        }
        return *this;
    }

    CHeaderList::~CHeaderList( void )
    {
        clear();
    }

    void CHeaderList::copyFrom( const CHeaderList &oSource )
    {
        for ( size_t nX = 0; nX < oSource.m_aHeaders.Count(); nX++ )
        {
            CHeader *pSource = (CHeader *)oSource.m_aHeaders.Get( nX );
            CHeader *pCopy = newHeader( pSource->getName() );
            CStringArray values = pSource->getValues();
            for ( size_t nY = 0; nY < values.Count(); nY++ )
            {
                pCopy->addValue( values[ nY ] );
            }
            m_aHeaders.Append( pCopy );
        }
        m_bComplete = oSource.m_bComplete;
        m_bMalformed = oSource.m_bMalformed;
    }

    void CHeaderList::clear( void )
    {
        m_aHeaders.DeleteAll();
    }

    CString CHeaderList::canonicalName( const CString &name )
    {
        return name;
    }

    CHeader *CHeaderList::newHeader( const CString &name )
    {
        return new CHeader( name );
    }

    int CHeaderList::findIndex( const CString &key )
    {
        for ( size_t nX = 0; nX < m_aHeaders.Count(); nX++ )
        {
            CHeader *pHeader = (CHeader *)m_aHeaders.Get( nX );
            if ( pHeader->getKey() == key )
            {
                return (int)nX;
            }
        }
        return -1;
    }

    bool CHeaderList::parse( CStream &headerStream )
    {
        CHeader *pLastHeader = NULL;

        m_bComplete = false;

        while ( ! headerStream.IsEOS() )
        {
            CString line = headerStream.GetLine();

                // Accept CRLF (the standard) as well as bare LF line endings.
            if ( ( line.GetLength() > 0 ) && ( line[ line.GetLength() - 1 ] == '\r' ) )
            {
                line = line.Substring( 0, (int)line.GetLength() - 1 );
            }

                // A blank line ends the headers.
            if ( line.GetLength() == 0 )
            {
                m_bComplete = true;
                break;
            }

                // A CR anywhere but at the end is not a line ending this parser
                // splits on, so other software may see a header break there;
                // such a line is refused rather than guessed at.
            if ( line.IndexOf( '\r' ) != NOT_FOUND )
            {
                m_bMalformed = true;
                continue;
            }

                // Lines starting with whitespace continue the previous header
                // (RFC 3261 section 7.3.1, RFC 7230 obs-fold).
            if ( ( line[0] == ' ' ) || ( line[0] == '\t' ) )
            {
                if ( pLastHeader )
                {
                    CString continuation = line;
                    continuation.Trim();
                    pLastHeader->appendToLastValue( continuation );
                }
                else
                {
                    m_bMalformed = true;
                }
                continue;
            }

            size_t nSeparator = line.IndexOf( ':' );

            if ( nSeparator == NOT_FOUND || nSeparator == 0 )
            {
                m_bMalformed = true;
                continue;
            }

                // Whitespace is permitted before the colon.
            CString name = line.Substring( 0, (int)nSeparator );
            name.Trim();
            if ( ! CHeader::isValidName( name ) )
            {
                    // (Only an empty name was refused; one with spaces or
                    // control characters in it was kept.)
                m_bMalformed = true;
                pLastHeader = NULL;
                continue;
            }
            name = canonicalName( name );

            CString value = line.Substring( nSeparator + 1 );
            value.Trim();

            CString key = CHeader::makeKey( name );
            int nIndex = findIndex( key );
            if ( nIndex >= 0 )
            {
                pLastHeader = (CHeader *)m_aHeaders.Get( (size_t)nIndex );
            }
            else
            {
                pLastHeader = newHeader( name );
                m_aHeaders.Append( pLastHeader );
            }

            pLastHeader->addValue( value );
        }

        return m_bComplete;
    }

    bool CHeaderList::validName( const char *key, CString &name )
    {
        name = CString( key ? key : "" );
        name.Trim();
        return CHeader::isValidName( name );
    }

    bool CHeaderList::addHeader( const char *key, const char *value )
    {
        CString name;
        CString strValue( value ? value : "" );
            // (Names and values were stored as given, CR LF included, so
            // text from a client could add headers to a response.)
        if ( ! validName( key, name ) || ! CHeader::isValidValue( strValue ) )
        {
            return false;
        }
        name = canonicalName( name );
        int nIndex = findIndex( CHeader::makeKey( name ) );

        if ( nIndex >= 0 )
        {
            ((CHeader *)m_aHeaders.Get( (size_t)nIndex ))->setValue( strValue );
        }
        else
        {
            CHeader *temp = newHeader( name );
            temp->setValue( strValue );
            m_aHeaders.Append( temp );
        }
        return true;
    }

    bool CHeaderList::addHeader( const char *key, const CStringArray &values )
    {
        CString name;
        if ( ! validName( key, name ) )
        {
            return false;
        }
        for ( size_t nX = 0; nX < values.Count(); nX++ )
        {
            if ( ! CHeader::isValidValue( values[ nX ] ) )
            {
                return false;
            }
        }
        for ( size_t nX = 0; nX < values.Count(); nX++ )
        {
            appendHeader( key, values[ nX ] );
        }
        return true;
    }

    bool CHeaderList::appendHeader( const char *key, const char *value )
    {
        CString name;
        CString strValue( value ? value : "" );
        if ( ! validName( key, name ) || ! CHeader::isValidValue( strValue ) )
        {
            return false;
        }
        name = canonicalName( name );
        int nIndex = findIndex( CHeader::makeKey( name ) );

        if ( nIndex >= 0 )
        {
            ((CHeader *)m_aHeaders.Get( (size_t)nIndex ))->addValue( strValue );
        }
        else
        {
            CHeader *temp = newHeader( name );
            temp->addValue( strValue );
            m_aHeaders.Append( temp );
        }
        return true;
    }

    bool CHeaderList::hasHeader( const char *name )
    {
        return ( findIndex( CHeader::makeKey( canonicalName( CString( name ) ) ) ) >= 0 );
    }

    const CStringArray CHeaderList::allValues( const char *name )
    {
        CHeader *header = getHeader( name );

        if ( header )
        {
            return header->getValues();
        }
        return CStringArray();
    }

    CString CHeaderList::firstValue( const char *name )
    {
        CHeader *header = getHeader( name );

        if ( header )
        {
            return header->getValue();
        }
        return CString( "" );
    }

    long CHeaderList::firstValueAsLong( const char *name )
    {
        CString val = firstValue( name );
        val.Trim();

        if ( ( val.GetLength() > 0 ) && ( val.IsDigits() ) )
        {
            return atol( val );
        }
        return 0L;
    }

    CHash *CHeaderList::map( void )
    {
        CHash *retVal = new CHash( CHash::SMALL );

        for ( size_t nX = 0; nX < m_aHeaders.Count(); nX++ )
        {
            CHeader *header = (CHeader *)m_aHeaders.Get( nX );
            CStringArray *values = new CStringArray();
            *values = header->getValues();
            retVal->Push( header->getName(), (CObject *)values );
        }

        return retVal;
    }

    CIterator *CHeaderList::Enumerate( void )
    {
        return m_aHeaders.Enumerate();
    }

    CString CHeaderList::toString( void )
    {
        CString retVal;

        for ( size_t nX = 0; nX < m_aHeaders.Count(); nX++ )
        {
            CHeader *header = (CHeader *)m_aHeaders.Get( nX );
            CString name = header->getName();
            CStringArray values = header->getValues();

                // The setters refuse these; this keeps a header that was
                // changed some other way (a derived class) from being written
                // out as more than one line.
            if ( ! CHeader::isValidName( name ) )
            {
                continue;
            }

            for ( size_t nY = 0; nY < values.Count(); nY++ )
            {
                if ( ! CHeader::isValidValue( values[ nY ] ) )
                {
                    continue;
                }
                retVal += name;
                retVal += ": ";
                retVal += values[ nY ];
                retVal += "\r\n";
            }
        }

        return retVal;
    }

    CHeader *CHeaderList::getHeader( const char *key )
    {
        int nIndex = findIndex( CHeader::makeKey( canonicalName( CString( key ) ) ) );

        if ( nIndex >= 0 )
        {
            return (CHeader *)m_aHeaders.Get( (size_t)nIndex );
        }
        return NULL;
    }

    void CHeaderList::removeHeader( const char *key )
    {
        int nIndex = findIndex( CHeader::makeKey( canonicalName( CString( key ) ) ) );

        if ( nIndex >= 0 )
        {
            CHeader *header = (CHeader *)m_aHeaders.Remove( (size_t)nIndex );
            delete header;
        }
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
