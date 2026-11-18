/**
 * @file ArrayNode.h
 * @brief CArrayNode, a JSON array.
 *
 * Array Node Class
 *
 *  A JSON array: an ordered list of nodes, which it owns. add(), insert()
 * and set() take ownership of the nodes they are given, which must not belong
 * to any other tree. NULL, or a missing node, is stored as a JSON null.
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
        /// A JSON array: an ordered list of nodes, which it owns.
        ///
        /// add(), insert() and set() take ownership of the nodes they are
        /// given, which must not belong to any other tree. NULL, or a missing
        /// node, is stored as a JSON null. Arrays cannot be copied with a copy
        /// constructor; use clone().
        ///
        /// @code
        ///     CArrayNode *pArray = new CArrayNode();
        ///     pArray->add( 1 )->add( "two" )->addNull();
        ///     pArray->addObject()->put( "name", "three" );
        ///     std::string strJson = pArray->toJson();  // [1,"two",null,{"name":"three"}]
        ///     delete pArray;
        /// @endcode
    class CArrayNode : public CCollectiveNode
    {
        public:
            DEFINE_OBJECT( CArrayNode )

                /// An empty array.
                                    CArrayNode( void );
                /// Deletes the array and every element.
            virtual                ~CArrayNode( void );

                /// #JSON_ARRAY.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_ARRAY; }
                /// The number of elements.
            virtual int             size( void ) const { return (int)m_aElements.size(); }

                /// @name Appending
                /// Each appends an element and returns this array, so calls can
                /// be chained.
                ///@{

                /// Appends a node, taking ownership (NULL, or a missing node,
                /// means a JSON null).
            CArrayNode             *add( CJsonNode *pValue );
                /// Appends text; NULL appends a JSON null.
            CArrayNode             *add( const char *pszValue );
                /// Appends text.
            CArrayNode             *add( const std::string &strValue );
                /// Appends true or false.
            CArrayNode             *add( bool bValue );
                /// Appends a number.
            CArrayNode             *add( int nValue );
                /// @copydoc add(int)
            CArrayNode             *add( unsigned int nValue );
                /// @copydoc add(int)
            CArrayNode             *add( long lValue );
                /// @copydoc add(int)
            CArrayNode             *add( unsigned long lValue );
                /// @copydoc add(int)
            CArrayNode             *add( long long llValue );
                /// Appends a number, held exactly up to 2^64 - 1.
            CArrayNode             *add( unsigned long long llValue );
                /// Appends a floating point number (infinities and NaN are
                /// written as null).
            CArrayNode             *add( double dValue );
                /// Appends a JSON null.
            CArrayNode             *addNull( void );
                /// Appends bytes, written as a base64 string.
            CArrayNode             *addBinary( const std::vector<unsigned char> &aData );
                ///@}
                /// Appends a new, empty object and returns it (owned by the array).
            CObjectNode            *addObject( void );
                /// Appends a new, empty array and returns it (owned by this array).
            CArrayNode             *addArray( void );

                /// Inserts a node before an index, taking ownership.
                /// @param nIndex Where it goes, from 0; size() appends.
                /// @param pValue The node (NULL, or a missing node, means a JSON null).
                /// @return False, with the value deleted, for an index out of range.
            bool                    insert( int nIndex, CJsonNode *pValue );
                /// Replaces the element at an index, deleting the old one and
                /// taking ownership of the new.
                /// @param nIndex The element, from 0.
                /// @param pValue The node (NULL, or a missing node, means a JSON null).
                /// @return False, with the value deleted, for an index out of range.
            bool                    set( int nIndex, CJsonNode *pValue );

                /// Deletes an element; the ones after it move down one place.
                /// @return False for an index out of range.
            bool                    remove( int nIndex );
                /// Takes an element out of the array and hands it to the caller,
                /// who then owns it.
                /// @return The element, or NULL for an index out of range.
            CJsonNode              *release( int nIndex );
                /// Deletes every element.
            virtual void            removeAll( void );

                /// "": an array has no text value.
            virtual CString         asText( void ) const { return CString( "" ); }
                /// Deep equality: the same elements, equal, in the same order.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A deep copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const;
                /// Appends the array's JSON (see CJsonNode::writeJson()).
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        protected:
                /// The element at nIndex, or NULL out of range.
            virtual CJsonNode      *element( int nIndex );
                /// Moves the elements into aChildren, leaving the array empty.
            virtual void            detachChildren( std::vector<CJsonNode *> &aChildren );

        private:
            std::vector<CJsonNode *>    m_aElements;    ///< The elements, owned; never NULL.

                /// Not copyable: use clone().
                                    CArrayNode( const CArrayNode & );
                /// Not assignable.
            CArrayNode             &operator =( const CArrayNode & );
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_ARRAYNODE_H__
