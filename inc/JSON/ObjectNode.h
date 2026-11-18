/**
 * @file ObjectNode.h
 * @brief CObjectNode, a JSON object.
 *
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
        /// A JSON object: named fields, kept in the order they were added.
        ///
        /// Adding a field that already exists replaces its value in place. The
        /// object owns its values; set() takes ownership of the node it is
        /// given, which must not belong to any other tree. NULL, or a missing
        /// node, is stored as a JSON null. Field names are given as a
        /// CJsonKey, so any of the library's string types will do.
        ///
        /// Finding a field by name takes logarithmic time; removing one takes
        /// time in proportion to the fields after it. Objects cannot be copied
        /// with a copy constructor; use clone().
        ///
        /// @code
        ///     CObjectNode *pUser = new CObjectNode();
        ///     pUser->put( "name", "Ada" )->put( "age", 36 );
        ///     pUser->putArray( "tags" )->add( "admin" );
        ///     pUser->with( "address" )->put( "city", "London" );
        ///     CString strJson = pUser->toPrettyString();
        ///     delete pUser;
        /// @endcode
    class CObjectNode : public CCollectiveNode
    {
        public:
            DEFINE_OBJECT( CObjectNode )

                /// An empty object.
                                    CObjectNode( void );
                /// Deletes the object and every value.
            virtual                ~CObjectNode( void );

                /// #JSON_OBJECT.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_OBJECT; }
                /// The number of fields.
            virtual int             size( void ) const { return (int)m_aFields.size(); }

                /// @name Fields by position
                /// Positions run from 0, in the order the fields were added.
                ///@{

                /// The name of field nIndex; "" out of range. The CString stops
                /// at the first U+0000: see fieldNameString().
            CString                 fieldName( int nIndex ) const;
                /// The name of field nIndex, whole; a reference to an empty
                /// string out of range.
            const std::string      &fieldNameString( int nIndex ) const;
                /// The value of field nIndex (owned by the object); NULL out of
                /// range.
            CJsonNode              *fieldValue( int nIndex ) const;
                /// Every field name, in order.
            CStringArray            fieldNames( void ) const;
                ///@}

                /// Sets a field, taking ownership of the value. A field that
                /// already exists keeps its place, and its old value is deleted.
                /// @param key    The field name.
                /// @param pValue The value (NULL, or a missing node, means a
                ///               JSON null).
                /// @return This object, so calls can be chained.
            CObjectNode            *set( const CJsonKey &key, CJsonNode *pValue );

                /// @name Setting values
                /// Each sets a field to a new value node, as set() does, and
                /// returns this object, so calls can be chained.
                ///@{

                /// Sets text; NULL sets a JSON null.
            CObjectNode            *put( const CJsonKey &key, const char *pszValue );
                /// Sets text.
            CObjectNode            *put( const CJsonKey &key, const std::string &strValue );
                /// Sets true or false.
            CObjectNode            *put( const CJsonKey &key, bool bValue );
                /// Sets a number.
            CObjectNode            *put( const CJsonKey &key, int nValue );
                /// Sets a number.
            CObjectNode            *put( const CJsonKey &key, unsigned int nValue );
                /// Sets a number.
            CObjectNode            *put( const CJsonKey &key, long lValue );
                /// Sets a number.
            CObjectNode            *put( const CJsonKey &key, unsigned long lValue );
                /// Sets a number.
            CObjectNode            *put( const CJsonKey &key, long long llValue );
                /// Sets a number, held exactly up to 2^64 - 1.
            CObjectNode            *put( const CJsonKey &key, unsigned long long llValue );
                /// Sets a floating point number (infinities and NaN are written
                /// as null).
            CObjectNode            *put( const CJsonKey &key, double dValue );
                /// Sets a JSON null.
            CObjectNode            *putNull( const CJsonKey &key );
                /// Sets bytes, written as base64.
            CObjectNode            *putBinary( const CJsonKey &key, const std::vector<unsigned char> &aData );
                ///@}
                /// Sets a field to a new, empty object and returns it (owned by
                /// this object). Any old value is deleted.
            CObjectNode            *putObject( const CJsonKey &key );
                /// Sets a field to a new, empty array and returns it (owned by
                /// this object). Any old value is deleted.
            CArrayNode             *putArray( const CJsonKey &key );

                /// The object in a field, created (empty) if the field is not
                /// there.
                /// @return The object, owned by this one; NULL if the field
                ///         holds something else (including null).
            CObjectNode            *with( const CJsonKey &key );
                /// The array in a field, created (empty) if the field is not
                /// there.
                /// @return The array, owned by this object; NULL if the field
                ///         holds something else (including null).
            CArrayNode             *withArray( const CJsonKey &key );

                /// Deletes a field and its value. The fields after it move up
                /// one place.
                /// @return False if there is no such field.
                /// @note This used to rebuild the whole name index, so removing
                /// many fields took quadratic time.
            bool                    remove( const CJsonKey &key );
                /// Takes a field out of the object and hands its value to the
                /// caller, who then owns it. The fields after it move up one
                /// place.
                /// @return The value, or NULL if there is no such field.
            CJsonNode              *release( const CJsonKey &key );
                /// Deletes every field.
            virtual void            removeAll( void );

                /// "": an object has no text value.
            virtual CString         asText( void ) const { return CString( "" ); }
                /// Deep equality: the same field names with equal values, in
                /// any order.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A deep copy, owned by the caller, with the fields in the same
                /// order.
            virtual CJsonNode      *clone( void ) const;
                /// Appends the object's JSON, fields in order (see
                /// CJsonNode::writeJson()).
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        protected:
                /// The value of the field named strField, or NULL.
            virtual CJsonNode      *field( const std::string &strField );
                /// Moves the values into aChildren, leaving the object empty.
            virtual void            detachChildren( std::vector<CJsonNode *> &aChildren );

        private:
                // The fields in order, each with a number given when it was
                // added (m_aOrder, ascending, alongside m_aFields). The index
                // maps a name to that number, which a binary search of
                // m_aOrder turns into a position, so removing a field leaves
                // the index of the others as it was.
            std::vector< std::pair<std::string, CJsonNode *> >  m_aFields;      ///< The fields in order: name and owned value (never NULL).
            std::vector<unsigned long long>                     m_aOrder;       ///< Each field's number, ascending, alongside m_aFields.
            std::unordered_map<std::string, unsigned long long> m_mapIndex;     ///< A field's name to its number.
            unsigned long long                                  m_nNextOrder;   ///< The number the next new field gets.

                /// The position of the field named strField, or -1.
            int                     find( const std::string &strField ) const;

                /// Not copyable: use clone().
                                    CObjectNode( const CObjectNode & );
                /// Not assignable.
            CObjectNode            &operator =( const CObjectNode & );
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_OBJECTNODE_H__
