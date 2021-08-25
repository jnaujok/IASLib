/**
 * Missing Node Class
 *
 *  This class defines a Node that is missing -- not null, not empty, but
 * a node that doesn't exist at all.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 5/06/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MISSINGNODE_H__
#define IASLIB_MISSINGNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"

namespace IASLib
{
    class CMissingNode : public CJsonNode
    {
        private:
            static CMissingNode *instance;

        public:
            DEFINE_OBJECT( CMissingNode );
            
            static CMissingNode *getInstance( void );

            /**
             * MissingNode only has one singleton instance (there's no need for any more
             * that this.) Cloning, therefore, only returns the "this" pointer because
             * there's no reason to return anything else. All missing nodes are the same
             * singleton.
             */
            virtual CJsonNode *clone() { return this; }

            /**
             * A missing node is only equal to another missing node, which would be the
             * same object, so this comparison is easy.
             */
            virtual bool equals(CJsonNode *o) { if ( o == this ) return true; return false; }

            //Method similar to findValue(java.lang.String), but that will return a "missing node" instead of null if no field is found.
            virtual CJsonNode *findPath(CString fieldName)  { return NULL; }
            // Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
            virtual CJsonNode *findValue(CString fieldName)  { return NULL; }

            // Method for accessing value of the specified element of an array node.
            virtual CJsonNode *get(int index)   { return NULL; }

            // Method for accessing value of the specified field of an object node.
            virtual CJsonNode *get(CString fieldName)   { return NULL; }

            // Return the type of this node
            virtual JsonNodeType getNodeType()
            {
                return MISSING;
            }

            virtual bool	isMissingNode()   { return true; }

            virtual CIterator *iterator()   { return NULL; }

            // This method is similar to get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode	*path(int index)   { return NULL; }

            // This method is similar to get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for isMissingNode()) will be returned.
            virtual CJsonNode *path(CString fieldName)   { return NULL; }

            // Note: marked as abstract to ensure all implementation classes define it properly.
            virtual CString	toString()   { return CString( "" ); }

            virtual CString asText() { return CString(""); }

            virtual int getType() {}
        private:
            // Private constructors
            CMissingNode( void ) {}
            CMissingNode( CMissingNode &oSource ) { oSource.isMissingNode(); }
    };
}

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_MISSINGNODE_H__

