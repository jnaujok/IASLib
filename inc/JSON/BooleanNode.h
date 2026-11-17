/**
 * Boolean Node Class
 *
 *  A JSON true or false.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BOOLEANNODE_H__
#define IASLIB_BOOLEANNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
    class CBooleanNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CBooleanNode )

                                    CBooleanNode( bool bValue ) : m_bValue( bValue ) {}

            virtual JsonNodeType    getNodeType( void ) const { return BOOLEAN; }
            virtual bool            booleanValue( void ) const { return m_bValue; }
            virtual CString         asText( void ) const { return CString( m_bValue ? "true" : "false" ); }
            virtual bool            asBoolean( bool bDefault = false ) const { (void)bDefault; return m_bValue; }
            virtual int             asInt( int nDefault = 0 ) const { (void)nDefault; return m_bValue ? 1 : 0; }
            virtual long long       asLong( long long llDefault = 0 ) const { (void)llDefault; return m_bValue ? 1 : 0; }
            virtual double          asDouble( double dDefault = 0.0 ) const { (void)dDefault; return m_bValue ? 1.0 : 0.0; }

            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const { return new CBooleanNode( m_bValue ); }
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            bool                    m_bValue;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_BOOLEANNODE_H__
