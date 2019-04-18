/*
 *  CStringStub Class
 *
 *      This class encodes a "string stub" which is where actual string
 * data is stored for the CString class. This allows us to have strings
 * that are "copied" merely by pointing at the same string stub. If fifty
 * copies are made of a string, then they merely all point at the same
 * stub in memory. Only when a string is changed does it get its own copy
 * of the string stub. This will cut down on a large amount of un-needed
 * (and extremely slow) memory allocation and copying.
 *
 *  Created:
 *      8/19/2004
 *  Creator:
 *      Jeffrey R. Naujok
 *  Revision History:
 *      ($LOG$)
 *
 */

#ifndef IASLIB_STRINGSTUB_H__
#define IASLIB_STRINGSTUB_H__

#ifdef IASLIB_AIX__
#include <stdlib.h>
#include <memory.h>
#endif

#include "Object.h"

namespace IASLib
{
    class CStringStub
    {
        public:
            IASLibChar__  *m_strData;
            size_t      m_nLength;
            size_t      m_nSize;
            int         m_nReferences;
            bool        m_bFixedStub;
            bool        m_bDeletable;

                        CStringStub( void );
                        CStringStub( int nLength );//throw (CStringException);
                        CStringStub( size_t nLength );
                        CStringStub( const IASLibChar__ *strData, int nLength );// throw (CException);
                        CStringStub( const IASLibChar__ *strData, size_t nLength );// throw (CException);
                        CStringStub( IASLibChar__ *strBuffer, int nLength, int nSize, bool bDeletable = false );
                        CStringStub( IASLibChar__ *strBuffer, size_t nLength, size_t nSize, bool bDeletable = false );
                        CStringStub( const CStringStub &oSource );// throw (CException);
            virtual    ~CStringStub( void );

            void        AddRef( void );
            void        RemoveRef( void );// throw (CException);

            int         GetRefCount( void ) { return m_nReferences; }

            void        ChangeSize( size_t nLength );
    };
}

#endif // IASLIB_STRINGSTUB_H__
