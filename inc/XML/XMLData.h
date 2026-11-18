/**
 * @file XMLData.h
 * @brief CXMLData, a piece of text in an XML document.
 *
 * The original header comment follows.
 */
/*
 * XML Data Class
 *
 *      The text of an XML document or element, as the text itself (with
 * entities and character references already replaced). Text that came
 * from a CDATA section remembers it, and is written back as one.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLDATA_H__
#define IASLIB_XMLDATA_H__

#include "../BaseTypes/String_.h"
#include "XMLChunk.h"

namespace IASLib
{
    /// The text of an XML document or element, as the text itself (with
    /// entities and character references already replaced).
    ///
    /// Text that came from a CDATA section remembers it, and is written
    /// back as one.
    class CXMLData : public CXMLChunk
    {
        protected:
            CString         m_strData;      ///< The text, unescaped.
            bool            m_bCData;       ///< Whether it is written as a CDATA section.

        public:
                /// Empty text.
                            CXMLData( void );
                /// Some text.
                /// @param strData  The text, unescaped.
                /// @param bCData  Write it as a CDATA section.
                            CXMLData( const CString &strData, bool bCData = false );
                /// Destroys the text.
            virtual        ~CXMLData( void );

                            DECLARE_OBJECT(CXMLData,CXMLChunk);

                /// The text, unescaped.
            virtual const CString &GetData( void ) const { return m_strData; }
                /// Replaces the text.
                /// @param strData  The new text, unescaped.
            virtual void    SetData( const CString &strData ) { m_strData = strData; }

                /// Whether it is written as a CDATA section.
            bool            IsCData( void ) const { return m_bCData; }
                /// Sets whether it is written as a CDATA section.
                /// @param bCData  true for CDATA.
            void            SetCData( bool bCData ) { m_bCData = bCData; }
                /// Whether the text is only spaces, tabs and line breaks (true if empty).
            bool            IsWhitespace( void ) const;

                /// false: this is text.
            virtual bool    IsElement( void ) const { return false; }
                /// true: this is text.
            virtual bool    IsData( void ) const { return true; }

                /// The text as XML, escaped (or as a CDATA section, with any
                /// "]]>" split across two). See CXMLChunk::toString().
            virtual CString toString( int offset = 0, int indent = 0 ) const;
                /// A copy (the caller owns it).
            virtual CXMLChunk *Clone( void ) const { return new CXMLData( m_strData, m_bCData ); }
    };
} // namespace IASLib

#endif // IASLIB_XMLDATA_H__
