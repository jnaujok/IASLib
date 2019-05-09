/**
 * Array Node Class
 *
 *  This concrete class defines a node in JSON that stores an array of nodes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 5/02/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "../../inc/JSON/ArrayNode.h"
#include "../../inc/JSON/MissingNode.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CArrayNode, CJsonNode );

    CArrayNode::CArrayNode( void ) : m_aElements()
    {

    }

    CArrayNode::~CArrayNode()
    {
        m_aElements.DeleteAll();
    }

    /**
     * Method that will return a valid String representation of the container value, if the
     * node is a value node (method isValueNode() returns true), otherwise empty String.
     */
    CString	CArrayNode::asText()
    {

    }

    /**
     * Method that can be called to get a node that is guaranteed not to allow changing of
     * this node through mutators on this node or any of its children.
     */
    CJsonNode *CArrayNode::clone()
    {
        CArrayNode *retVal = new CArrayNode();

        for ( int nX = 0; nX < m_aElements.Length(); nX++ )
        {
            retVal->m_aElements.AddLast( ((CJsonNode *)m_aElements.GetAt(nX))->clone() );
        }

        return retVal;
    }

    /**
     * Method for accessing all value nodes of this Node, iff this node is a JSON Array or Object node.
     */
    CIterator *CArrayNode::elements()
    {
        return m_aElements.Enumerate();
    }

    // Equality for node objects is defined as full (deep) value equality.
    bool CArrayNode::equals(CJsonNode *o)
    {
        if ( o->getNodeType() == JsonNodeType::ARRAY )
        {

        }
        return false;
    }

    CIterator *CArrayNode::fieldNames()
    {
        return NULL;
    }

    /**
     * This function returns named fields -- by definition, the fields in an array are not named.
     */
    CIterator *CArrayNode::fields()
    {
        return NULL;
    }

    // Method for finding a JSON Object that contains specified field, within this node or its descendants.
    CJsonNode *CArrayNode::findParent(CString fieldName)
    {
        CJsonNode *retVal = NULL;

        for ( int nX = 0; nX < this->m_aElements.Length(); nX++ )
        {
            retVal = ((CJsonNode *)m_aElements.GetAt(nX))->findParent(fieldName );
            if ( retVal != NULL )
            {
                break;
            }
        }
        return retVal;
    }

    // Method for finding a JSON Object that contains specified field, within this node or its descendants.
    CArray *CArrayNode::findParents(CString fieldName)
    {

    }

    //Method similar to findValue(java.lang.String), but that will return a "missing node" instead of null if no field is found.
    CJsonNode *CArrayNode::findPath(CString fieldName)
    {

    }

    // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
    CJsonNode *CArrayNode::findValue(CString fieldName)
    {

    }

    // Method for finding JSON Object fields with specified name, and returning found ones as a List.
    CArray *CArrayNode::findValues(CString fieldName)
    {

    }

    CStringArray *CArrayNode::findValuesAsText(CString fieldName)
    {

    }

    // Method for accessing value of the specified element of an array node.
    CJsonNode *CArrayNode::get(int index)
    {
        return (CJsonNode *)m_aElements.GetAt(index);
    }

    // Method for accessing value of the specified field of an object node.
    CJsonNode *CArrayNode::get(CString fieldName)
    {
        // Fields in arrays are not accessable by name.
        return NULL;
    }

    // Method that allows checking whether this node is JSON Array node and contains a value for specified index If this is the case (including case of specified indexing having null as value), returns true; otherwise returns false.
    bool CArrayNode::has(int index)
    {
        if ( index >= 0 && index < m_aElements.Length() )
        {
            return true;
        }
        return false;
    }

    // Method that allows checking whether this node is JSON Object node and contains value for specified property.
    bool CArrayNode::has(CString fieldName)
    {
        return false;
    }

    // Method that is similar to has(int), but that will return false for explicitly added nulls.
    bool CArrayNode::hasNonNull(int index)
    {
        if ( has( index ) )
        {
            if ( ! ((CJsonNode *)m_aElements.GetAt(index))->isNull() )
                return true;
        }
        return false;
    }

    // Method that is similar to has(String), but that will return false for explicitly added nulls.
    bool CArrayNode::hasNonNull(CString fieldName)
    {
        return false;
    }

    CIterator *CArrayNode::iterator()
    {
        return m_aElements.Enumerate();
    }

    /**
     *  This method is similar to get(int), except that instead of returning null
     * if no such element exists (due to index being out of range, or this node
     * not being an array), a "missing node" (node that returns true for
     * isMissingNode()) will be returned.
     */
    CJsonNode *CArrayNode::path(int index)
    {
        CJsonNode *retVal = this->get( index );

        if ( retVal == NULL )
        {
            retVal = CMissingNode::getInstance();
        }

        return retVal;
    }

    // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
    CJsonNode *CArrayNode::path(CString fieldName)
    {
        return NULL;
    }

    int CArrayNode::size()
    {
        return m_aElements.Length();
    }

    // Note: marked as abstract to ensure all implementation classes define it properly.
    CString CArrayNode::toString()
    {
        return asText();
    }

    // Method that can be called on Object nodes, to access a property that has Object value; or if no such property exists, to create, add and return such Object node.
    CJsonNode   *CArrayNode::with(CString propertyName)
    {
        return NULL;
    }

            // Method that can be called on Object nodes, to access a property that has Array value; or if no such property exists, to create, add and return such Array node.
    CJsonNode *CArrayNode::withArray(CString propertyName)
    {
        return NULL;
    }
} // namespace IASLib

