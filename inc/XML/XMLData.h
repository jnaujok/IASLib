/*
 * XML Data Class
 *
 *      This class derives from the XMLChunk type in order to store the
 * non-formatted data inside a 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
 *	Log:
 *	  $LOG$
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
        public:
                            CXMLData( void );
                            CXMLData( const CString &strData );
            virtual        ~CXMLData( void );

                            DECLARE_OBJECT(CXMLData,CXMLChunk);

            virtual CString GetData( void ) { return m_strData; }
            virtual void    SetData( const CString &strData ) { m_strData = strData; }

            virtual bool    IsElement( void ) const { return false; }
            virtual bool    IsData( void ) const { return true; }

            virtual CString toString(int offset = 0, int indent = 0 ) const;
    };
} // namespace IASLib

#endif // IASLIB_XMLDATA_H__
