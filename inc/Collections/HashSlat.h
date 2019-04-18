/*
 * Hash Slat
 *
 *  This class defines and implments the hash slat which stores the
 * individual data elements stored in a hash.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1995
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HASHSLAT_H__
#define IASLIB_HASHSLAT_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CHashSlat : public CObject
    {
        //protected:
        public:
            CObject    *m_pElement;
            CString     m_strKey;
        public:
                        CHashSlat( const char *strKey, CObject *pElement );
            virtual    ~CHashSlat( void );

                        DEFINE_OBJECT( CHashSlat )

            CObject    *GetElement( void ) { return m_pElement; }
            void        SetElement( CObject *pElement, bool bDeleteCurrent )
                        {
                            if ( ( pElement ) && ( bDeleteCurrent ) && ( m_pElement != pElement ) )
                                delete m_pElement;
                            m_pElement = pElement;
                        }
            CString     GetKey( void ) { return m_strKey; }
            void        Empty( void ) { m_pElement = NULL; }
    };
} // namespace IASLib

#endif // IASLIB_HASHSLAT_H__

