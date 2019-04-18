/**
 * Object Node Class
 *
 *  This Class implements a JSON Object node which contains named Json nodes
 * in a non-order-dependent manner. This is equivalent to a map-type 
 * representation.
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

#include "JsonNode.h"
#include "../Collections/Hash.h"

namespace IASLib
{
    class CObjectNode : public CJsonNode
    {
        private:
            CHash objectNodes;
        public:
            /**
             * Calling "asText" on an ObjectNode will return the JSON representation of the entire 
             * object node -- in short, the JSON text.
             */
            virtual CString	asText();


            /**
             * Method that can be called to get a node that is guaranteed not to allow changing of 
             * this node through mutators on this node or any of its children.
             */
            virtual CJsonNode clone() = 0;

            /**
             * Method for accessing all value nodes of this Node, iff this node is a JSON Array or Object node.
             */
            virtual TIterator<CJsonNode>	elements();

            // Equality for node objects is defined as full (deep) value equality.
            virtual bool equals(CJsonNode *o);

            virtual TIterator<CString>	fieldNames();
            //TIterator<Map.Entry<String,JsonNode>>	fields() 
            
            // Method for finding a JSON Object that contains specified field, within this node or its descendants.
            virtual CJsonNode *findParent(CString fieldName);

            // Method for finding a JSON Object that contains specified field, within this node or its descendants.
            // List<JsonNode>	findParents(String fieldName)
            // abstract List<JsonNode>	findParents(String fieldName, List<JsonNode> foundSoFar) 
        
            //Method similar to findValue(java.lang.String), but that will return a "missing node" instead of null if no field is found.
            virtual CJsonNode *findPath(CString fieldName);
            // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
            virtual CJsonNode *findValue(CString fieldName);

            // Method for finding JSON Object fields with specified name, and returning found ones as a List.
            // List<JsonNode>	findValues(String fieldName)
            // abstract List<JsonNode>	findValues(String fieldName, List<JsonNode> foundSoFar) 
        
            // Similar to findValues(java.lang.String), but will additionally convert values into Strings, calling asText().
            // List<String>	findValuesAsText(String fieldName)
        
            // abstract List<String>	findValuesAsText(String fieldName, List<String> foundSoFar) 
        
            // Method for accessing value of the specified element of an array node.
            virtual CJsonNode *get(int index);
        
            // Method for accessing value of the specified field of an object node.
            virtual CJsonNode *get(CString fieldName);
        
            // Return the type of this node
            // abstract JsonNodeType	getNodeType()

            // Method that allows checking whether this node is JSON Array node and contains a value for specified index If this is the case (including case of specified indexing having null as value), returns true; otherwise returns false.
            virtual bool has(int index);
        
            // Method that allows checking whether this node is JSON Object node and contains value for specified property.
            virtual bool has(CString fieldName);
        
            // Method that is similar to has(int), but that will return false for explicitly added nulls.
            virtual bool hasNonNull(int index);
            // Method that is similar to has(String), but that will return false for explicitly added nulls.
            virtual bool hasNonNull(CString fieldName);
        
            virtual bool	isContainerNode();
            virtual bool	isNull();
            virtual bool	isObject(); 
            virtual bool	isPojo();

            virtual TIterator<CJsonNode>	iterator();

            // This method is similar to get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode	*path(int index);
        
            // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode *path(CString fieldName);
        
            virtual int	size();

            // Note: marked as abstract to ensure all implementation classes define it properly.
            virtual CString	toString();

            // Method that can be called on Object nodes, to access a property that has Object value; or if no such property exists, to create, add and return such Object node.
            virtual CJsonNode   *with(CString propertyName);

            // Method that can be called on Object nodes, to access a property that has Array value; or if no such property exists, to create, add and return such Array node.        
            virtual CJsonNode *withArray(CString propertyName);
    };
}

#endif // IASLIB_OBJECTNODE_H__

