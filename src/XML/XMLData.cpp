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

#include "XMLData.h"

namespace IASLib
{
    CXMLData::CXMLData( void )
    {
        m_strData = "";
    }

    CXMLData::CXMLData( const CString &strData )
    {
        m_strData = strData;
    }
     
    CXMLData::~CXMLData( void )
    {
    }
} // namespace IASLib
