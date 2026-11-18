/**
 * @file XMLParser.h
 * @brief CXMLParser, which reads XML 1.0 text into a document.
 *
 * The original header comment follows.
 */
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
    /// Reads XML 1.0 into a CXMLIndex (or a new CXMLDocument).
    ///
    /// It reads elements, attributes, text, CDATA sections, comments,
    /// processing instructions, the XML declaration and a DOCTYPE. Text and
    /// attribute values have the predefined entities and character
    /// references replaced, along with general entities declared in the
    /// DOCTYPE's internal subset, whose text is used as text (not parsed for
    /// markup). External entities are refused, and entity expansion and
    /// nesting are limited, so a document cannot read files or grow without
    /// bound. Line breaks are read as "\n", and attribute values have tabs
    /// and line breaks turned into spaces, as XML requires. Input is UTF-8
    /// (a UTF-8 byte-order mark is skipped; UTF-16 is refused).
    ///
    /// An error throws a CXMLException whose message gives the line and
    /// column ("line 3, column 7: ..."). A parser holds only its settings,
    /// so one may be used from several threads at once.
    ///
    /// @code
    /// CXMLDocument *pDocument = CXMLParser::parseFile( CString( "config.xml" ) );
    /// CXMLElement *pRoot = pDocument->GetRoot();
    /// ...
    /// delete pDocument;
    /// @endcode
    class CXMLParser : public CObject
    {
        public:
            static const size_t DEFAULT_MAX_DEPTH = 256;                        ///< The default nesting limit.
            static const size_t DEFAULT_MAX_ENTITY_EXPANSION = 1024 * 1024;     ///< The default entity expansion limit, in characters.

        protected:
            bool            m_bSingleRoot;          ///< Require exactly one root element.
            size_t          m_nMaxDepth;            ///< How deeply elements may nest.
            size_t          m_nMaxEntityExpansion;  ///< How many characters entities may produce in all.

        public:
                /// A parser with the default settings: several roots allowed,
                /// DEFAULT_MAX_DEPTH and DEFAULT_MAX_ENTITY_EXPANSION.
                            CXMLParser( void );
                /// Destroys the parser.
            virtual        ~CXMLParser( void );

                            DECLARE_OBJECT(CXMLParser,CObject);

                /// Whether a document must have exactly one root element (the
                /// XML rule; off by default, for files of several).
            void            SetSingleRoot( bool bSingleRoot ) { m_bSingleRoot = bSingleRoot; }
                /// How deeply elements may nest.
            void            SetMaxDepth( size_t nMaxDepth ) { m_nMaxDepth = nMaxDepth; }
                /// How many characters entity references may produce in all.
            void            SetMaxEntityExpansion( size_t nMax ) { m_nMaxEntityExpansion = nMax; }

                /// Parses into oIndex, replacing what it held (it is left as
                /// it was if the text is not well formed).
                /// @param pData  The XML text (NULL counts as empty).
                /// @param nLength  Its length in bytes.
                /// @param oIndex  Where to put the result.
                /// @throws CXMLException if the text is not well formed.
            void            Parse( const char *pData, size_t nLength, CXMLIndex &oIndex ) const;

                /// Reads a new document from an open file, to its end.
                ///
                /// New documents (the caller deletes them), throwing a
                /// CXMLException if the input is not well formed or, for
                /// files, cannot be read. These use the default settings.
                /// @param file  An open file (left open).
                /// @return The document, which the caller deletes.
                /// @throws CXMLException if the file is not open, holds a NUL
                ///         byte or is not well formed.
            static CXMLDocument *parseFile( CFile *file );
                /// Reads a new document from a file.
                /// @param fileName  The file's path.
                /// @return The document, which the caller deletes.
                /// @throws CXMLException if the file cannot be opened or is not well formed.
            static CXMLDocument *parseFile( CString fileName );
                /// Reads a new document from text.
                /// @param xmlData  The XML text.
                /// @return The document, which the caller deletes.
                /// @throws CXMLException if it is not well formed.
            static CXMLDocument *parseString( CString xmlData );
#ifdef IASLIB_NETWORKING__
                /// Reads from the socket until it closes.
                /// @param socket  The socket (not deleted).
                /// @return The document, which the caller deletes.
                /// @throws CXMLException if socket is NULL, the data holds a NUL
                ///         byte or is not well formed. (The socket's Read() may
                ///         throw a CSocketException.)
            static CXMLDocument *parseSocket( CSocket *socket );
#endif
                /// Reads a new document from a stream, to its end.
                /// @param stream  The stream.
                /// @return The document, which the caller deletes.
                /// @throws CXMLException if stream is NULL, or the data holds a
                ///         NUL byte or is not well formed.
            static CXMLDocument *parseStream( CStream *stream );

                /// Everything left in a stream.
                /// @param pStream  The stream (NULL gives "").
                /// @return The text.
                /// @throws CXMLException if it holds a NUL byte.
            static CString  ReadAll( CStream *pStream );
    };
}; // end of Namespace IASLib

#endif // IASLIB_XMLPARSER_H__
