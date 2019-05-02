/**
 * JSON Node Class
 *
 *  This abstract base class is the root type of all JSON nodes in a JSON
 * tree. All derived classes must implement the base interface.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_JSONNODE_H__
#define IASLIB_JSONNODE_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "../Collections/Iterator.h"
#include "../Collections/Array.h"
#include "../Collections/StringArray.h"
#include "JsonNodeType.h"

namespace IASLib
{
    class CJsonNode : public CObject
    {
        public:
            /**
             * Method that will try to convert value of this node to a boolean value.
             */
            virtual bool asBoolean();
            virtual bool asBoolean(bool defaultValue);

            /**
             * Method that will try to convert value of this node to a double.
             */
             virtual double asDouble();
             virtual double asDouble(double defaultValue);

            /**
             * Method that will try to convert value of this node to an integer.
             */
            virtual int asInt();
            virtual int asInt(int defaultValue);

            /**
             * Method that will try to convert value of this node to a long.
             */
            virtual long asLong();
            virtual long asLong(long defaultValue);

            /**
             * Method that will return a valid String representation of the container value, if the
             * node is a value node (method isValueNode() returns true), otherwise empty String.
             */
            virtual CString	asText() = 0;

            /**
             * Method to use for accessing binary content of binary nodes (nodes for which isBinary()
             * returns true); or for Text Nodes (ones for which textValue() returns non-null value),
             * to read decoded base64 data.
             */
            virtual char* binaryValue();

            /**
             * Method to use for accessing JSON boolean values (value literals 'true' and 'false').
             */
            virtual bool booleanValue();

            /**
             * Method that can be used to check whether this node is a numeric node (isNumber() would
             * return true) AND its value fits within a 32-bit signed integer type, int.
             */
            virtual bool canConvertToInt();

            /**
             * Method that can be used to check whether this node is a numeric node (isNumber() would
             * return true) AND its value fits within a 64-bit signed integer type, long int.
             */
            virtual bool canConvertToLong();

            /**
             * Method that can be called to get a node that is guaranteed not to allow changing of
             * this node through mutators on this node or any of its children.
             */
            virtual CJsonNode *clone() = 0;

            /**
             * Returns 64-bit floating point (double) value for this node, if and only if this node
             * is numeric (isNumber() returns true).
             */
            virtual double doubleValue();

            /**
             * Method for accessing all value nodes of this Node, iff this node is a JSON
             * Array or Object node.
             */
            virtual CIterator	*elements();

            /**
             * Equality for node objects is defined as full (deep) value equality.
             */
            virtual bool equals(CJsonNode *o)  = 0;

            virtual CIterator *fieldNames();
            virtual CIterator *fields();

            /**
             * Method for finding a JSON Object that contains specified field, within this
             * node or its descendants.
             */
            virtual CJsonNode *findParent(CString fieldName);

            /**
             * Method for finding a JSON Object that contains specified field, within this
             * node or its descendants.
             */
            virtual CArray *findParents(CString fieldName);

            //Method similar to findValue(java.lang.String), but that will return a "missing node" instead of null if no field is found.
            virtual CJsonNode *findPath(CString fieldName) = 0;
            // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
            virtual CJsonNode *findValue(CString fieldName) = 0;

            // Method for finding JSON Object fields with specified name, and returning found ones as a List.
            virtual CArray *findValues(CString fieldName);

            // Similar to findValues(java.lang.String), but will additionally convert values into Strings, calling asText().
            virtual CStringArray *findValuesAsText(CString fieldName);

            // Returns 32-bit floating value for this node, if and only if this node is numeric (isNumber() returns true).
            virtual float   floatValue();

            // Method for accessing value of the specified element of an array node.
            virtual CJsonNode *get(int index) = 0;

            // Method for accessing value of the specified field of an object node.
            virtual CJsonNode *get(CString fieldName) = 0;

            // Return the type of this node
            virtual JsonNodeType getNodeType()
            {
                return MISSING;
            }

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

            virtual bool	isArray() ;
            //Method that can be used to check if this node represents binary data (Base64 encoded).
            virtual bool	isBinary();
            //Method that can be used to check if this node was created from JSON boolean value (literals "true" and "false").
            virtual bool	isBoolean();
            virtual bool	isContainerNode();
            virtual bool	isDouble();
            virtual bool	isFloat();
            virtual bool	isFloatingPointNumber();
            //Method that can be used to check whether contained value is a number represented as Java int.
            virtual bool	isInt();
            virtual bool	isIntegralNumber();
            //Method that can be used to check whether contained value is a number represented as Java long.
            virtual bool	isLong();
            virtual bool	isMissingNode();
            //Method that can be used to check if this node was created from JSON literal null value.
            virtual bool	isNull();
            virtual bool	isNumber();
            virtual bool	isObject();
            //Method that can be used to check if the node is a wrapper for a POJO ("Plain Old Java Object" aka "bean".
            virtual bool	isPojo();
            //Method that can be used to check whether contained value is a number represented as Java short.
            virtual bool	isShort();
            //Method that checks whether this node represents basic JSON String value.
            virtual bool	isTextual();
            //         Same as calling elements(); implemented so that convenience "for-each" loop can be used for looping over elements of JSON Array constructs.
            virtual bool	isValueNode();

            virtual CIterator *iterator() = 0;

            //Returns 64-bit long value for this node, if and only if this node is numeric (isNumber() returns true).
            virtual long longValue();

            // This method is similar to get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode	*path(int index) = 0;

            // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode *path(CString fieldName) = 0;

            // Returns 16-bit short value for this node, if and only if this node is numeric (isNumber() returns true).
            virtual short shortValue();

            virtual int	size();
            // Method to use for accessing String values.
            virtual CString	textValue();

            // Note: marked as abstract to ensure all implementation classes define it properly.
            virtual CString	toString() = 0;

            // Method that can be called on Object nodes, to access a property that has Object value; or if no such property exists, to create, add and return such Object node.
            virtual CJsonNode   *with(CString propertyName);

            // Method that can be called on Object nodes, to access a property that has Array value; or if no such property exists, to create, add and return such Array node.
            virtual CJsonNode *withArray(CString propertyName);
    };
}

#endif // IASLIB_JSONNODE_H__

