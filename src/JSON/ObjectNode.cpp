/**
 * Object Node Class Implementation
 *
 *  This Class implements a JSON Object node which contains named Json nodes
 * in a non-order-dependent manner.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ObjectNode.h"

namespace IASLib
{
    /**
     * Calling "asText" on an ObjectNode will return the JSON representation of the entire
     * object node -- in short, the JSON text.
     */
    CString	CObjectNode::asText()
    {
        CString retVal;

    }


    /**
     * Method that can be called to get a node that is guaranteed not to allow changing of
     * this node through mutators on this node or any of its children.
     */
    CJsonNode *CObjectNode::clone()
    {

    }

    /**
     * Method for accessing all value nodes of this Node, iff this node is a JSON Array or Object node.
     */
    CIterator *CObjectNode::elements()
    {

    }

    // Equality for node objects is defined as full (deep) value equality.
    bool CObjectNode::equals(CJsonNode *o)
    {
        if ( o->getNodeType() == JsonNodeType::OBJECT )
        {

        }
        return false;
    }

    CIterator *CObjectNode::fieldNames()
    {

    }

    CIterator *CObjectNode::fields()
    {

    }

    // Method for finding a JSON Object that contains specified field, within this node or its descendants.
    CJsonNode *CObjectNode::findParent(CString fieldName)
    {

    }

    // Method for finding a JSON Object that contains specified field, within this node or its descendants.
    CArray *CObjectNode::FindParents(CString fieldName)
    {

    }

    CArray *CObjectNode::FindParents(CString fieldName, CArray *foundSoFar)
    {

    }

    //Method similar to findValue(CString), but that will return a "missing node" instead of null if no field is found.
    CJsonNode *CObjectNode::findPath(CString fieldName)
    {

    }
    // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
    CJsonNode *CObjectNode::findValue(CString fieldName)
    {}

    // Method for finding JSON Object fields with specified name, and returning found ones as a List.
    CArray *CObjectNode::FindValues(CString fieldName)
    {}

    // Similar to findValues(java.lang.String), but will additionally convert values into Strings, calling asText().
    CArray *CObjectNode::FindValuesAsText(CString fieldName)
    {

    }

    // Method for accessing value of the specified element of an array node.
    CJsonNode *CObjectNode::get(int index)
    {

    }

    // Method for accessing value of the specified field of an object node.
    CJsonNode *CObjectNode::get(CString fieldName) {}

    // Method that allows checking whether this node is JSON Array node and contains a value for specified index If this is the case (including case of specified indexing having null as value), returns true; otherwise returns false.
    bool CObjectNode::has(int index)
    {}

    // Method that allows checking whether this node is JSON Object node and contains value for specified property.
    bool CObjectNode::has(CString fieldName)
    {}

    // Method that is similar to has(int), but that will return false for explicitly added nulls.
    bool CObjectNode::hasNonNull(int index)
    {}

    // Method that is similar to has(String), but that will return false for explicitly added nulls.
    bool CObjectNode::hasNonNull(CString fieldName)
    {}

    bool CObjectNode::isContainerNode()
    {}

    bool CObjectNode::isNull()
    {

    }

    bool	CObjectNode::isPojo()
    {}

    CIterator *CObjectNode::iterator()
    {

    }

    // This method is similar to get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for isMissingNode()) will be returned.
    CJsonNode	*CObjectNode::path(int index)
    {

    }

    // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
    CJsonNode *CObjectNode::path(CString fieldName)
    {

    }

    int	CObjectNode::size()
    {

    }

    // Note: marked as abstract to ensure all implementation classes define it properly.
    CString	CObjectNode::toString()
    {
        return asText();
    }

    // Method that can be called on Object nodes, to access a property that has Object value; or if no such property exists, to create, add and return such Object node.
    CJsonNode   *CObjectNode::with(CString propertyName)
    {

    }

    // Method that can be called on Object nodes, to access a property that has Array value; or if no such property exists, to create, add and return such Array node.
    CJsonNode *CObjectNode::withArray(CString propertyName)
    {

    }
};


