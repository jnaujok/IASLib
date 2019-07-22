/**
 * Array Node Class
 *
 *  This concrete class defines a node in JSON that stores an array of nodes.
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

#ifdef IASLIB_JSONSUPPORT__

#include "JsonNode.h"
#include "../Collections/List.h"

namespace IASLib
{
    class CArrayNode : public CJsonNode
    {
        protected:
            CList   m_aElements;

        public:
            DEFINE_OBJECT( CArrayNode );

            CArrayNode( void );
            ~CArrayNode( void );

            /**
             * Method that will return a valid String representation of the container value, if the
             * node is a value node (method isValueNode() returns true), otherwise empty String.
             */
            virtual CString	asText();

            /**
             * Method that can be called to get a node that is guaranteed not to allow changing of
             * this node through mutators on this node or any of its children.
             */
            virtual CJsonNode *clone();

            /**
             * Method for accessing all value nodes of this Node, iff this node is a JSON Array or Object node.
             */
            virtual CIterator *elements();

            // Equality for node objects is defined as full (deep) value equality.
            virtual bool equals(CJsonNode *o);

            virtual CIterator *fields();

            virtual CIterator *fieldNames();

            // Method for finding a JSON Object that contains specified field, within this node or its descendants.
            virtual CJsonNode *findParent(CString fieldName);

            // Method for finding a JSON Object that contains specified field, within this node or its descendants.
            CArray *findParents(CString fieldName);

            //Method similar to findValue(java.lang.String), but that will return a "missing node" instead of null if no field is found.
            virtual CJsonNode *findPath(CString fieldName);

            // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
            virtual CJsonNode *findValue(CString fieldName);

            // Method for finding JSON Object fields with specified name, and returning found ones as a List.
            virtual CArray *findValues(CString fieldName);

            // Similar to findValues(java.lang.String), but will additionally convert values into Strings, calling asText().
            virtual CStringArray *findValuesAsText(CString fieldName);

            // Returns 32-bit floating value for this node, if and only if this node is numeric (isNumber() returns true).
            virtual float   floatValue();

            // Method for accessing value of the specified element of an array node.
            virtual CJsonNode *get(int index);

            // Method for accessing value of the specified field of an object node.
            virtual CJsonNode *get(CString fieldName);

            virtual JsonNodeType	getNodeType() { return JsonNodeType::ARRAY; }

            // Method that allows checking whether this node is JSON Array node and contains a value for specified index If this is the case (including case of specified indexing having null as value), returns true; otherwise returns false.
            virtual bool has(int index);

            // Method that allows checking whether this node is JSON Object node and contains value for specified property.
            virtual bool has(CString fieldName);

            // Method that is similar to has(int), but that will return false for explicitly added nulls.
            virtual bool hasNonNull(int index);
            // Method that is similar to has(String), but that will return false for explicitly added nulls.
            virtual bool hasNonNull(CString fieldName);

            // Returns integer value for this node, if and only if this node is numeric (isNumber() returns true).
            virtual int intValue();

            virtual bool	isArray() { return true; }
            virtual bool	isContainerNode() { return true; }
            virtual bool	isNull();

            virtual CIterator *iterator();

            // This method is similar to get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode	*path(int index);

            // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode *path(CString fieldName);

            virtual int	size();

            // Method to use for accessing String values.
            virtual CString	textValue();

            // Note: marked as abstract to ensure all implementation classes define it properly.
            virtual CString	toString();

            // Method that can be called on Object nodes, to access a property that has Object value; or if no such property exists, to create, add and return such Object node.
            virtual CJsonNode   *with(CString propertyName);

            // Method that can be called on Object nodes, to access a property that has Array value; or if no such property exists, to create, add and return such Array node.
            virtual CJsonNode *withArray(CString propertyName);
    };
}

#endif // IASLIB_JSONSUPPORT__

#endif // IASLIB_ARRAYNODE_H__

