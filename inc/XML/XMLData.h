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
    class CXMLData : public CXMLChunk
    {
        protected:
            CString         m_strData;
            bool            m_bCData;

        public:
                            CXMLData( void );
                            CXMLData( const CString &strData, bool bCData = false );
            virtual        ~CXMLData( void );

                            DECLARE_OBJECT(CXMLData,CXMLChunk);

            virtual const CString &GetData( void ) const { return m_strData; }
            virtual void    SetData( const CString &strData ) { m_strData = strData; }

            bool            IsCData( void ) const { return m_bCData; }
            void            SetCData( bool bCData ) { m_bCData = bCData; }
                // Whether the text is only spaces, tabs and line breaks.
            bool            IsWhitespace( void ) const;

            virtual bool    IsElement( void ) const { return false; }
            virtual bool    IsData( void ) const { return true; }

            virtual CString toString( int offset = 0, int indent = 0 ) const;
            virtual CXMLChunk *Clone( void ) const { return new CXMLData( m_strData, m_bCData ); }
    };
} // namespace IASLib

#endif // IASLIB_XMLDATA_H__
