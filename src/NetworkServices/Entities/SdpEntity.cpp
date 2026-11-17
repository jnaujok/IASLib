/**
 * SDP Entity class
 *
 * This class encapsulates the Session Description Protocol as an entity
 * within the Request or Response object.
 *
 * Author: Jeffrey R. Naujok
 * Created: July 30, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/SdpEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSDPEntity, CEntity );

    class CSDPAttribute : public CObject
    {
        private:
            CString name;
            CString value;

        public:
            DECLARE_OBJECT( CSDPAttribute, CObject );

                // Splits a line at the first '=' only, since values such as
                // "a=fmtp:101 0-15;mode=20" contain further '=' characters.
            CSDPAttribute( const CString &line )
            {
                size_t nEquals = line.IndexOf( '=' );
                if ( nEquals != NOT_FOUND )
                {
                    name = line.Substring( 0, (int)nEquals );
                    value = line.Substring( nEquals + 1 );
                }
                else
                {
                    name = line;
                    value = "";
                }
            }

            CSDPAttribute( void )
            {
            }

            void setName( const char *name ) { this->name = name; }
            CString getName( void ) { return name; }
            void setValue( const char *value ) { this->value = value; }
            CString getValue( void ) { return value; }
    };

    CSDPEntity::CSDPEntity( const CString &body ) : m_SessionValues( )
    {
        size_t nStart = 0;
        size_t nLength = body.GetLength();

        while ( nStart < nLength )
        {
            size_t nEnd = body.IndexOf( '\n', nStart );
            if ( nEnd == NOT_FOUND )
            {
                nEnd = nLength;
            }

            CString line = body.Substring( nStart, (int)( nEnd - nStart ) );
            if ( ( line.GetLength() > 0 ) && ( line[ line.GetLength() - 1 ] == '\r' ) )
            {
                line = line.Substring( 0, (int)line.GetLength() - 1 );
            }

            if ( line.GetLength() > 0 )
            {
                m_SessionValues.Append( new CSDPAttribute( line ) );
            }

            nStart = nEnd + 1;
        }
    }

    CSDPEntity::CSDPEntity( void ) : m_SessionValues( )
    {
    }

    CSDPEntity::~CSDPEntity( void )
    {
        m_SessionValues.DeleteAll();
    }

    void CSDPEntity::addAttribute( const char *name, const char *value )
    {
        CSDPAttribute *pNew = new CSDPAttribute();
        pNew->setName( name );
        pNew->setValue( value );
        m_SessionValues.Append( pNew );
    }

    CString CSDPEntity::getAttribute( const char *name )
    {
        for ( size_t nX = 0; nX < m_SessionValues.Length(); nX++ )
        {
            CSDPAttribute *pTemp = (CSDPAttribute *)m_SessionValues[nX];
            if ( pTemp->getName() == name )
            {
                return pTemp->getValue();
            }
        }
        return CString("");
    }

    CString CSDPEntity::toString( void )
    {
        CString retVal = "";
        for ( size_t nX = 0; nX < m_SessionValues.Length(); nX++ )
        {
            CSDPAttribute *pTemp = (CSDPAttribute *)m_SessionValues[nX];
            retVal += pTemp->getName();
            retVal += "=";
            retVal += pTemp->getValue();
            retVal += "\r\n";
        }

        return retVal;
    }

    CEntity *CSDPEntity::generateEntity( const CString &body )
    {
        return new CSDPEntity( body );
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
