/**
 * @file XMLChunk.h
 * @brief CXMLChunk, the base of what an XML element or document contains.
 *
 * The original header comment follows.
 */
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
    /// The base type of what an XML element (or document) contains:
    /// another element (CXMLElement, which also stands for comments,
    /// processing instructions and declarations) or text (CXMLData).
    ///
    /// One base class lets a single array hold both, in document order.
    /// Whoever holds a chunk in its content (an element or a CXMLIndex)
    /// owns it.
    class CXMLChunk : public CObject
    {
        public:
                /// Creates the chunk.
                            CXMLChunk( void );
                /// Destroys the chunk.
            virtual        ~CXMLChunk( void );

                            DECLARE_OBJECT(CXMLChunk, CObject);

                /// Whether this is a CXMLElement (including comments, meta tags
                /// and declarations).
            virtual bool    IsElement( void ) const = 0;
                /// Whether this is text (a CXMLData).
            virtual bool    IsData( void ) const = 0;

                /// The chunk as XML.
                ///
                /// With indent 0, exactly as it is (so a parsed document writes
                /// back as it was read, apart from attribute quoting and the
                /// entities chosen for escaping). With indent > 0,
                /// pretty-printed: each element on its own line, offset spaces
                /// in, children indent further, text trimmed onto its own
                /// lines, and whitespace-only text left out.
                /// @param offset  How many spaces to put before the first line.
                /// @param indent  How many more spaces each level of children
                ///                gets; 0 writes the chunk unchanged.
                /// @return The XML text.
            virtual CString toString( int offset = 0, int indent = 0 ) const = 0;

                /// A deep copy (the caller owns it).
            virtual CXMLChunk *Clone( void ) const = 0;

                /// Text escaped for element content: & and < as entities, > only
                /// where it ends "]]>", and a carriage return as a character
                /// reference (a reader would turn it into a line break).
                /// @param strText  The text (NULL counts as "").
                /// @return The escaped text.
            static CString  EscapeText( const char *strText );
                /// Text escaped for an attribute value, which is written in
                /// double quotes: ", & and < as entities, and tabs and line
                /// breaks as character references (a reader would turn them
                /// into spaces).
                /// @param strText  The text (NULL counts as "").
                /// @return The escaped text.
            static CString  EscapeAttribute( const char *strText );
    };
} // namespace IASLib

#endif // IASLIB_XMLCHUNK_H__
