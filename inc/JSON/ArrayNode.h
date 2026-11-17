/**
 * Array Node Class
 *
 *  A JSON array: an ordered list of nodes, which it owns. add(), insert()
 * and set() take ownership of the nodes they are given, which must not belong
 * to any other tree.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ARRAYNODE_H__
#define IASLIB_ARRAYNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "CollectiveNode.h"

#include <string>
#include <vector>

namespace IASLib
{
    class CArrayNode : public CCollectiveNode
    {
        public:
            DEFINE_OBJECT( CArrayNode )

                                    CArrayNode( void );
            virtual                ~CArrayNode( void );

            virtual JsonNodeType    getNodeType( void ) const { return ARRAY; }
            virtual int             size( void ) const { return (int)m_aElements.size(); }

                // Appends an element, taking ownership (NULL means a JSON null).
                // Returns this array, so calls can be chained.
            CArrayNode             *add( CJsonNode *pValue );
            CArrayNode             *add( const char *pszValue );
            CArrayNode             *add( const std::string &strValue );
            CArrayNode             *add( bool bValue );
            CArrayNode             *add( int nValue );
            CArrayNode             *add( unsigned int nValue );
            CArrayNode             *add( long lValue );
            CArrayNode             *add( unsigned long lValue );
            CArrayNode             *add( long long llValue );
            CArrayNode             *add( unsigned long long llValue );
            CArrayNode             *add( double dValue );
            CArrayNode             *addNull( void );
            CArrayNode             *addBinary( const std::vector<unsigned char> &aData );
                // Appends a new, empty object or array and returns it.
            CObjectNode            *addObject( void );
            CArrayNode             *addArray( void );

                // Inserts before an index (size() appends), or replaces the
                // element at an index, taking ownership. False, with the value
                // deleted, for an index out of range.
            bool                    insert( int nIndex, CJsonNode *pValue );
            bool                    set( int nIndex, CJsonNode *pValue );

                // Deletes an element (remove), or hands it to the caller
                // (release). False or NULL for an index out of range.
            bool                    remove( int nIndex );
            CJsonNode              *release( int nIndex );
            virtual void            removeAll( void );

            virtual CString         asText( void ) const { return CString( "" ); }
            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const;
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        protected:
            virtual CJsonNode      *element( int nIndex );
            virtual bool            collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const;

        private:
            std::vector<CJsonNode *>    m_aElements;

                                    CArrayNode( const CArrayNode & );
            CArrayNode             &operator =( const CArrayNode & );
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_ARRAYNODE_H__
