/**
 * SDP Entity class
 *
 * This class encapsulates the Session Description Protocol as an entity
 * within the Request or Response object. 
 *
 * Author: Jeffrey R. Naujok
 * Created: July 30, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/SdpEntity.h"
#include "BaseTypes/StringTokenizer.h"
#include "Logging/LogSink.h"

namespace IASLib
{

IASLIB_REGISTER_ENTITY( CSDPEntity );

class CSDPAttribute : public CObject
{
    private:
        CString name;
        CString value;
    public:
        DECLARE_OBJECT( CSDPAttribue, CObject );

        CSDPAttribute( CString line )
        {
            CStringTokenizer tokens( line, "=" );

            if ( tokens.CountTokens() == 2 )
            {
                name = tokens.NextToken();
                value = tokens.NextToken();
            }
        }

        CSDPAttribute( void )
        {
        }

        void setName( const char *name )
        {
            this->name = name;
        }

        CString getName( void )
        {
            return name;
        }

        void setValue( const char *value )
        {
            this->value = value;
        }

        CString getValue( void )
        {
            return value;
        }
};

CSDPEntity::CSDPEntity( CStream &oStream ) : m_SessionValues( )
{
    while ( ! oStream.IsEOS() )
    {
        CString line = oStream.GetLine();
        if ( line.GetLength() > 0 )
        {
            CSDPAttribute *pNew = new CSDPAttribute( line );
            m_SessionValues.Append( pNew );
        }
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
    pNew->setName(name);
    pNew->setValue(value);
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
        retVal += CString::FormatString( "%s=%s\r\n", (const char *)pTemp->getName(), (const char *)pTemp->getValue() );
    }

    return retVal;
}

CEntity *CSDPEntity::generateEntity( CStream &stream, size_t nContentLength )
{
    DEBUG_LOG( "Creating SDP Entity with a size of %d bytes.", nContentLength );
    return new CSDPEntity( stream );
}

}; // namespace IASLib

#endif // IASLIB_NETWORKING__
