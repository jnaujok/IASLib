/*
 * XML Chunk Class
 *
 *      This class forms the base type for elements within an XML
 * element. In other words, either another tag (XML Element) or 
 * a piece of data (XML Data). By using a single base class, we 
 * assure that the array uses a single type, but can hold both 
 * types of data.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLCHUNK_H__
#define IASLIB_XMLCHUNK_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CXMLChunk : public CObject
    {
        public:
                            CXMLChunk( void );
            virtual        ~CXMLChunk( void );

                            DECLARE_OBJECT(CXMLChunk, CObject);

            virtual bool    IsElement( void ) const = 0;
            virtual bool    IsData( void ) const = 0;
    };
} // namespace IASLib

#endif // IASLIB_XMLCHUNK_H__
