/**
 * JSON Node Class Implementation
 *
 *  This abstract base class is the root type of all JSON nodes in a JSON
 * tree. All derived classes must implement the base interface.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */
#include "JsonNode.h"

namespace IASLib
{

bool CJsonNode::asBoolean()
{
    return asBoolean(false);
}

bool CJsonNode::asBoolean(bool defaultValue)
{
    return defaultValue;
}

double CJsonNode::asDouble()
{
    return asDouble((double)0.0);
}

double CJsonNode::asDouble(double defaultValue)
{
    return defaultValue;
}

int CJsonNode::asInt()
{
    return asInt(0);
}
int CJsonNode::asInt(int defaultValue)
{
    return defaultValue;
}

long CJsonNode::asLong()
{
    return asLong((long)0);
}

long CJsonNode::asLong(long defaultValue)
{
    return defaultValue;
}

char *CJsonNode::binaryValue()
{
    return NULL;
}

bool CJsonNode::booleanValue()
{
    return asBoolean();
}

bool CJsonNode::canConvertToInt()
{
    return false;
}

bool CJsonNode::canConvertToLong()
{
    return false;
}

double CJsonNode::doubleValue()
{
    return asDouble();
}

CIterator *CJsonNode::fieldNames()
{
    return NULL;
}

CIterator *CJsonNode::fields()
{
    return NULL;
}

/**
 * Method for finding a JSON Object that contains specified field, within this
 * node or its descendants.
 */
CJsonNode *CJsonNode::findParent(CString fieldName)
{
    return NULL;
}

/**
 * Method for finding a JSON Object that contains specified field, within this
 * node or its descendants.
 */
CArray *CJsonNode::findParents(CString fieldName)
{
    return NULL;
}

// Method for finding JSON Object fields with specified name, and returning found ones as a List.
CArray *CJsonNode::findValues(CString fieldName)
{
    return NULL;
}

// Similar to findValues(java.lang.String), but will additionally convert values into Strings, calling asText().
CStringArray *CJsonNode::findValuesAsText(CString fieldName)
{
    return NULL;
}

float CJsonNode::floatValue()
{
    return (float)asDouble();
}

bool CJsonNode::has(int index)
{
    return false;
}

bool CJsonNode::has(CString fieldName)
{
    return false;
}

bool CJsonNode::hasNonNull(int index)
{
    return false;
}

bool CJsonNode::hasNonNull(CString fieldName)
{
    return false;
}

int CJsonNode::intValue()
{
    return asInt();
}

bool CJsonNode::isArray()
{
    return false;
}

bool CJsonNode::isBinary()
{
    return false;
}

bool CJsonNode::isBoolean()
{
    return false;
}
bool CJsonNode::isContainerNode()
{
    return false;
}
bool CJsonNode::isDouble()
{
    return false;
}
bool CJsonNode::isFloat()
{
    return false;
}
bool CJsonNode::isFloatingPointNumber()
{
    return false;
}
bool CJsonNode::isInt()
{
    return false;
}
bool CJsonNode::isIntegralNumber()
{
    return false;
}
bool CJsonNode::isLong()
{
    return false;
}
bool CJsonNode::isMissingNode()
{
    return false;
}
bool CJsonNode::isNull()
{
    return true;
}
bool CJsonNode::isNumber()
{
    return false;
}
bool CJsonNode::isObject()
{
    return false;
}
bool CJsonNode::isPojo()
{
    return false;
}
bool CJsonNode::isShort()
{
    return false;
}
bool CJsonNode::isTextual()
{
    return false;
}
bool CJsonNode::isValueNode()
{
    return false;
}

long CJsonNode::longValue()
{
    return asLong();
}

short CJsonNode::shortValue()
{
    return (short)asInt();
}

int CJsonNode::size()
{
    return 0;
}

CString CJsonNode::textValue()
{
    return asText();
}

CJsonNode *CJsonNode::with(CString propertyName)
{
    return NULL;
}

CJsonNode *CJsonNode::withArray(CString propertyName)
{
    return NULL;
}

} // namespace IASLib
