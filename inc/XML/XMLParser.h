/*
 * XML Parser Class
 *
 *      Reads XML 1.0 into a CXMLIndex (or a new CXMLDocument): elements,
 * attributes, text, CDATA sections, comments, processing instructions,
 * the XML declaration and a DOCTYPE. Text and attribute values have the
 * predefined entities (&lt; &gt; &amp; &quot; &apos;) and character
 * references (&#65; &#x41;) replaced, along with general entities
 * declared in the DOCTYPE's internal subset (<!ENTITY name "text">),
 * whose text is used as text (not parsed for markup). External entities
 * are refused, and entity expansion is limited, so a document cannot
 * read files or grow without bound. Line breaks are read as "\n", and
 * attribute values have tabs and line breaks turned into spaces, as XML
 * requires. Input is UTF-8 (a UTF-8 byte-order mark is skipped).
 *
 *      An error throws a CXMLException whose message gives the line and
 * column ("line 3, column 7: ...").
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000 (rewritten 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLPARSER_H__
#define IASLIB_XMLPARSER_H__

#include "XMLDocument.h"
#include "../Files/File.h"
#ifdef IASLIB_NETWORKING__
#include "../Sockets/Socket.h"
#endif

namespace IASLib
{
    class CXMLParser : public CObject
    {
        public:
            static const size_t DEFAULT_MAX_DEPTH = 256;
            static const size_t DEFAULT_MAX_ENTITY_EXPANSION = 1024 * 1024;

        protected:
            bool            m_bSingleRoot;
            size_t          m_nMaxDepth;
            size_t          m_nMaxEntityExpansion;

        public:
                            CXMLParser( void );
            virtual        ~CXMLParser( void );

                            DECLARE_OBJECT(CXMLParser,CObject);

                // Whether a document must have exactly one root element (the
                // XML rule; off by default, for files of several).
            void            SetSingleRoot( bool bSingleRoot ) { m_bSingleRoot = bSingleRoot; }
                // How deeply elements may nest.
            void            SetMaxDepth( size_t nMaxDepth ) { m_nMaxDepth = nMaxDepth; }
                // How many characters entity references may produce in all.
            void            SetMaxEntityExpansion( size_t nMax ) { m_nMaxEntityExpansion = nMax; }

                // Parses into oIndex, replacing what it held (it is left as
                // it was if the text is not well formed).
            void            Parse( const char *pData, size_t nLength, CXMLIndex &oIndex ) const;

                // New documents (the caller deletes them), throwing a
                // CXMLException if the input is not well formed or, for
                // files, cannot be read.
            static CXMLDocument *parseFile( CFile *file );
            static CXMLDocument *parseFile( CString fileName );
            static CXMLDocument *parseString( CString xmlData );
#ifdef IASLIB_NETWORKING__
                // Reads from the socket until it closes.
            static CXMLDocument *parseSocket( CSocket *socket );
#endif
            static CXMLDocument *parseStream( CStream *stream );

                // Everything left in a stream.
            static CString  ReadAll( CStream *pStream );
    };
}; // end of Namespace IASLib

#endif // IASLIB_XMLPARSER_H__
