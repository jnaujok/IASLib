/**
 * @file XMLElement.h
 * @brief CXMLElement, an XML element (or comment, processing instruction or declaration).
 *
 * The original header comment follows.
 */
/*
 * XML Element Class
 *
 *      An XML element: a name, attributes (properties) and content (sub
 * elements and text, in document order). The same class stands for the
 * other kinds of markup a document can hold, told apart by IsComment(),
 * IsMetaTag() (a processing instruction, such as <?xml version="1.0"?>)
 * and IsDeclaration() (such as <!DOCTYPE html>):
 *
 *      comment         the comment's text is GetData()
 *      meta tag        the name is the target ("xml"); pseudo-attributes
 *                      (version="1.0") are its properties, and
 *                      GetRawContent() is everything after the target
 *      declaration     the name is the keyword ("DOCTYPE"), and
 *                      GetRawContent() is the rest
 *
 *      An element owns its properties and sub elements.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XML_ELEMENT_H__
#define IASLIB_XML_ELEMENT_H__

#include "../BaseTypes/String_.h"
#include "../Collections/Array.h"
#include "XMLProperty.h"
#include "XMLChunk.h"
#include "XMLData.h"

namespace IASLib
{
    /// An XML element: a name, attributes (properties) and content (sub
    /// elements and text, in document order).
    ///
    /// The same class stands for the other kinds of markup a document can
    /// hold, told apart by IsComment(), IsMetaTag() (a processing
    /// instruction, such as `<?xml version="1.0"?>`) and IsDeclaration()
    /// (such as `<!DOCTYPE html>`):
    ///
    /// - comment: the comment's text is GetData().
    /// - meta tag: the name is the target ("xml"); pseudo-attributes
    ///   (version="1.0") are its properties, and GetRawContent() is
    ///   everything after the target.
    /// - declaration: the name is the keyword ("DOCTYPE"), and
    ///   GetRawContent() is the rest.
    ///
    /// An element owns its properties and sub elements (and text); the
    /// Add methods take ownership. Names given to the lookup methods are
    /// trimmed of spaces and line breaks, and compared case sensitively.
    ///
    /// @code
    /// CXMLElement *pServer = new CXMLElement( "server" );
    /// pServer->SetProperty( "name", "main" );
    /// pServer->AddSubElement( new CXMLElement( "port", "8080" ) );
    /// CString strXML = pServer->toString( 0, 2 );
    /// @endcode
    class CXMLElement : public CXMLChunk
    {
        protected:
            CString             m_strElementName;   ///< The name (target or keyword for meta tags and declarations).
            CArray              m_aProperties;      ///< The CXMLProperty attributes (owned).
            CArray              m_aSubElements;     ///< CXMLElements and CXMLDatas, in order (owned).
            CString             m_strRawContent;    ///< Meta tags and declarations: what follows the name.
            bool                m_bCommentTag;      ///< A comment.
            bool                m_bMetaTag;         ///< A processing instruction.
            bool                m_bDeclaration;     ///< A declaration such as DOCTYPE.
            bool                m_bHasClosingTag;   ///< Written `<name>...</name>`.
            bool                m_bSelfClosingTag;  ///< Written `<name/>`.

        public:
                /// An element with no name.
                                CXMLElement( void );
                /// An empty element.
                /// @param strTagName  The name.
                                CXMLElement( const char *strTagName );
                                    /// An element holding one piece of text.
                                    /// @param strTagName  The name.
                                    /// @param strData  The text (none is added if it is empty).
                                CXMLElement( const CString &strTagName, const CString &strData );
                /// Deletes the properties and content.
            virtual            ~CXMLElement( void );

                                DEFINE_OBJECT( CXMLElement )

                /// The name.
            const char         *GetName( void ) const { return m_strElementName; }
                /// Renames the element.
                /// @param strTagName  The new name.
            void                SetName( const CString& strTagName );

                /// A property (attribute) by position.
                /// @param nIndex  From 0 to GetPropertyCount() - 1.
                /// @return The property, which the element still owns.
            CXMLProperty       *GetProperty( size_t nIndex ) const;
                /// How many properties the element has.
            size_t              GetPropertyCount( void ) const { return m_aProperties.Length(); }
                /// A property's value by name.
                /// @param strPropertyName  The name.
                /// @return The value, or NULL if there is none.
            const char         *GetProperty( const char *strPropertyName ) const;
                /// Whether the element has a property of this name.
            bool                HasProperty( const char *strPropertyName ) const { return GetProperty( strPropertyName ) != NULL; }
                /// Adds a property, or replaces the value of the one of this name.
                ///
                /// A meta tag is written from its properties from then on (its
                /// raw content is cleared).
                /// @param strPropertyName  The name.
                /// @param strValue  The value, unescaped.
            void                SetProperty( const char *strPropertyName, const char *strValue );
                /// Adds a property (even if one of its name exists), taking
                /// ownership. NULL is ignored. Clears a meta tag's raw content.
                /// @param pNewProperty  The property.
            void                AddProperty( CXMLProperty *pNewProperty );
                /// Removes (and deletes) the property of this name.
                /// @param strPropertyName  The name.
                /// @return false if there was none.
            bool                DeleteProperty( const char *strPropertyName );

                /// How many sub elements there are (not counting text).
            size_t              GetSubElementCount( void ) const;
                /// How many sub elements have this name.
                /// @param strElementType  The name.
            size_t              GetSubElementCount( const char *strElementType ) const;
                /// The nth sub element (not counting text).
                /// @param nElement  From 0.
                /// @return The element, which this one still owns, or NULL.
            const CXMLElement  *GetSubElement( size_t nElement ) const;
                /// The nth sub element of a name.
                /// @param strElementType  The name.
                /// @param nIndex  Which of them, from 0.
                /// @return The element, which this one still owns, or NULL.
            const CXMLElement  *GetSubElement( const char *strElementType, size_t nIndex = 0 ) const;
                /// @copydoc GetSubElement(size_t) const
            CXMLElement        *GetSubElement( size_t nElement );
                /// @copydoc GetSubElement(const char *,size_t) const
            CXMLElement        *GetSubElement( const char *strElementType, size_t nIndex = 0 );
                /// Adds a sub element at the end, taking ownership. NULL is ignored.
                /// @param pNewElement  The element.
            void                AddSubElement( CXMLElement *pNewElement );
                /// Removes (and deletes) the nth sub element.
                /// @param nElement  From 0 (not counting text).
                /// @return false if there is no such sub element.
            bool                DeleteSubElement( size_t nElement );

                /// The nth piece of text.
                /// @param nDataElement  From 0 (not counting sub elements).
                /// @return The text, or NULL if there is none.
            const char         *GetData( size_t nDataElement = 0 ) const;
                /// How many pieces of text there are.
            size_t              GetDataCount( void ) const;
                /// All the element's own text, joined (not the sub elements').
            CString             GetText( void ) const;
                /// The trimmed text of the first sub element with a name.
                /// @param strElementType  The name.
                /// @return The text, or "" if there is no such sub element.
            CString             GetChildText( const char *strElementType ) const;
                /// Replaces the nth piece of text (adding one if there is none).
                /// @param strData  The text, unescaped.
                /// @param nDataElement  Which piece, from 0.
            void                SetData( const CString& strData, size_t nDataElement = 0 );
                /// Adds a piece of text at the end.
                /// @param strData  The text, unescaped.
                /// @param bCData  Write it as a CDATA section.
            void                AddData( const CString& strData, bool bCData = false );

                /// How many chunks of content there are (elements and text).
            size_t              GetChunkCount( void ) const { return m_aSubElements.Length(); }
                /// A chunk of content by position, elements and text in order.
                /// @param nIndex  From 0 to GetChunkCount() - 1.
            CXMLChunk          *GetChunk( size_t nIndex ) const { return (CXMLChunk *)m_aSubElements.Get( nIndex ); }
                /// Adds a chunk at the end, taking ownership. NULL is ignored.
            void                AddChunk( CXMLChunk *pChunk ) { if ( pChunk != NULL ) m_aSubElements.Append( pChunk ); }

                /// Makes this a comment.
            void                SetComment( void ) { m_bCommentTag = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
                /// Makes this a meta tag (processing instruction).
            void                SetMetaTag( void ) { m_bMetaTag = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
                /// Makes this a declaration.
            void                SetDeclaration( void ) { m_bDeclaration = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
                /// Whether this is a comment.
            bool                IsComment( void ) const { return m_bCommentTag; }
                /// Whether this is a meta tag (processing instruction).
            bool                IsMetaTag( void ) const { return m_bMetaTag; }
                /// Whether this is a declaration.
            bool                IsDeclaration( void ) const { return m_bDeclaration; }
                /// An ordinary element (not a comment, meta tag or declaration).
            bool                IsPlainElement( void ) const { return ! m_bCommentTag && ! m_bMetaTag && ! m_bDeclaration; }
                /// Meta tags and declarations: what follows the name.
            const CString      &GetRawContent( void ) const { return m_strRawContent; }
                /// Sets what follows a meta tag's or declaration's name.
                /// @param strContent  The text, written as it is.
            void                SetRawContent( const CString &strContent ) { m_strRawContent = strContent; }

                /// Sets whether the element was written `<name/>` (it is, when
                /// written, if it has no content).
                /// @param selfClose  true for `<name/>`.
            void                SetSelfClose( bool selfClose ) { m_bSelfClosingTag = selfClose; m_bHasClosingTag = !selfClose; }
                /// The opposite of SetSelfClose().
                /// @param hasClose  true for `<name>...</name>`.
            void                SetHasClosingTag( bool hasClose ) { m_bHasClosingTag = hasClose; m_bSelfClosingTag = !hasClose; }
                /// Whether the element was written `<name/>`.
            bool                IsSelfClosing( void ) const { return m_bSelfClosingTag; }

                /// true: this is an element.
            virtual bool        IsElement( void ) const { return true; }
                /// false: this is not text.
            virtual bool        IsData( void ) const { return false; }

                /// The element and its content as XML (see CXMLChunk::toString()).
            virtual CString     toString( int offset = 0, int indent = 0 ) const;
                /// A deep copy, with copies of the properties and content (the
                /// caller owns it).
            virtual CXMLChunk  *Clone( void ) const;

        private:
                /// Sets the flags of a plain element.
            void                Init( void );
                /// Appends the element as XML to strOut.
            void                Write( CString &strOut, int offset, int indent, int nDepth ) const;
                /// The properties as they go in the start tag (` name="value"` each).
            CString             StartTag( void ) const;

                                CXMLElement( const CXMLElement & );        ///< Not copyable (use Clone()).
            CXMLElement        &operator =( const CXMLElement & );        ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_XML_ELEMENT_H__
