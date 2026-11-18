/*
 * XML Chunk Class
 *
 *      The base type of what an XML element (or document) contains:
 * another element (CXMLElement, which also stands for comments,
 * processing instructions and declarations) or text (CXMLData). One base
 * class lets a single array hold both, in document order.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
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

                // The chunk as XML. With indent 0, exactly as it is (so a
                // parsed document writes back as it was read, apart from
                // attribute quoting and the entities chosen for escaping).
                // With indent > 0, pretty-printed: each element on its own
                // line, offset spaces in, children indent further, text
                // trimmed onto its own lines, and whitespace-only text left out.
            virtual CString toString( int offset = 0, int indent = 0 ) const = 0;

                // A deep copy (the caller owns it).
            virtual CXMLChunk *Clone( void ) const = 0;

                // Text with &, < and > escaped (for element content), and
                // with ", & and < and tabs/newlines escaped (for attribute
                // values, which are written in double quotes).
            static CString  EscapeText( const char *strText );
            static CString  EscapeAttribute( const char *strText );
    };
} // namespace IASLib

#endif // IASLIB_XMLCHUNK_H__
