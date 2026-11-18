/*
 * XML Document Class
 *
 *      An XML document, read from a stream (or a string or file, through
 * CXMLParser) and written back out. Its content is in the index
 * (GetIndex()); GetRoot() is the root element.
 *
 *      Reading checks the document is well formed (XML 1.0): tags match
 * and nest, attributes are quoted and not repeated, entities and
 * character references are known, and so on; a document that is not
 * throws a CXMLException giving the line and column. By default a
 * document may have several root elements (as many configuration files
 * do); CXMLParser can require exactly one.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLDOCUMENT_H__
#define IASLIB_XMLDOCUMENT_H__

#include "../BaseTypes/Object.h"
#include "../Streams/Stream.h"
#include "XMLIndex.h"
#include "XMLElement.h"

namespace IASLib
{
    class CXMLDocument : public CObject
    {
        protected:
            CXMLIndex   m_xiIndex;

        public:
                        CXMLDocument( void );
                            // Reads the stream (throws a CXMLException if the
                            // document is not well formed).
                        CXMLDocument( CStream *pInput );
            virtual    ~CXMLDocument( void );

                        DECLARE_OBJECT(CXMLDocument,CObject);

                // Replaces the content with what the stream holds; throws a
                // CXMLException (leaving the document as it was) if it is not
                // well formed. Returns true.
            bool        Read( CStream *pInput );
                // The same from text.
            bool        Parse( const char *strXML, size_t nLength = NOT_FOUND );

                // Writes the document (see CXMLChunk::toString()), returning
                // how many bytes.
            size_t      Write( CStream *pOutput, int indent = 0 ) const;
            CString     toString( int indent = 0 ) const;

            CXMLIndex  *GetIndex( void ) { return &m_xiIndex; }
            const CXMLIndex *GetIndex( void ) const { return &m_xiIndex; }
            CXMLElement *GetRoot( void ) const { return m_xiIndex.GetRoot(); }

        private:
                        CXMLDocument( const CXMLDocument & );
            CXMLDocument &operator =( const CXMLDocument & );
    };
}; // end of Namespace IASLib

#endif // IASLIB_XMLDOCUMENT_H__
