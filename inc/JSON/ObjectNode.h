/**
 * Object Node Class
 *
 *  A JSON object: named fields, kept in the order they were added. Adding a
 * field that already exists replaces its value in place. The object owns its
 * values; set() takes ownership of the node it is given, which must not
 * belong to any other tree. NULL, or a missing node, is stored as a JSON null.
 *
 *  Finding a field by name takes logarithmic time; removing one takes time
 * in proportion to the fields after it (moving them up), with no change to
 * the name index.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_OBJECTNODE_H__
#define IASLIB_OBJECTNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "CollectiveNode.h"

#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

namespace IASLib
{
    class CObjectNode : public CCollectiveNode
    {
        public:
            DEFINE_OBJECT( CObjectNode )

                                    CObjectNode( void );
            virtual                ~CObjectNode( void );

            virtual JsonNodeType    getNodeType( void ) const { return JSON_OBJECT; }
            virtual int             size( void ) const { return (int)m_aFields.size(); }

                // Fields by position, in order.
            CString                 fieldName( int nIndex ) const;
            const std::string      &fieldNameString( int nIndex ) const;
            CJsonNode              *fieldValue( int nIndex ) const;
            CStringArray            fieldNames( void ) const;

                // Sets a field, taking ownership of the value (NULL means a JSON
                // null). Returns this object, so calls can be chained.
            CObjectNode            *set( const CJsonKey &key, CJsonNode *pValue );

            CObjectNode            *put( const CJsonKey &key, const char *pszValue );
            CObjectNode            *put( const CJsonKey &key, const std::string &strValue );
            CObjectNode            *put( const CJsonKey &key, bool bValue );
            CObjectNode            *put( const CJsonKey &key, int nValue );
            CObjectNode            *put( const CJsonKey &key, unsigned int nValue );
            CObjectNode            *put( const CJsonKey &key, long lValue );
            CObjectNode            *put( const CJsonKey &key, unsigned long lValue );
            CObjectNode            *put( const CJsonKey &key, long long llValue );
            CObjectNode            *put( const CJsonKey &key, unsigned long long llValue );
            CObjectNode            *put( const CJsonKey &key, double dValue );
            CObjectNode            *putNull( const CJsonKey &key );
                // Bytes, written as base64.
            CObjectNode            *putBinary( const CJsonKey &key, const std::vector<unsigned char> &aData );
                // Sets a new, empty object or array and returns it.
            CObjectNode            *putObject( const CJsonKey &key );
            CArrayNode             *putArray( const CJsonKey &key );

                // The object or array in a field, created if the field is not
                // there; NULL if the field holds something else.
            CObjectNode            *with( const CJsonKey &key );
            CArrayNode             *withArray( const CJsonKey &key );

                // Deletes a field's value (remove), or hands it to the caller
                // (release). False or NULL if there is no such field. The fields
                // after it move up one place. (This used to rebuild the whole
                // name index, so removing many fields took quadratic time.)
            bool                    remove( const CJsonKey &key );
            CJsonNode              *release( const CJsonKey &key );
            virtual void            removeAll( void );

            virtual CString         asText( void ) const { return CString( "" ); }
            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const;
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        protected:
            virtual CJsonNode      *field( const std::string &strField );
            virtual void            detachChildren( std::vector<CJsonNode *> &aChildren );

        private:
                // The fields in order, each with a number given when it was
                // added (m_aOrder, ascending, alongside m_aFields). The index
                // maps a name to that number, which a binary search of
                // m_aOrder turns into a position, so removing a field leaves
                // the index of the others as it was.
            std::vector< std::pair<std::string, CJsonNode *> >  m_aFields;
            std::vector<unsigned long long>                     m_aOrder;
            std::unordered_map<std::string, unsigned long long> m_mapIndex;
            unsigned long long                                  m_nNextOrder;

            int                     find( const std::string &strField ) const;

                                    CObjectNode( const CObjectNode & );
            CObjectNode            &operator =( const CObjectNode & );
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_OBJECTNODE_H__
