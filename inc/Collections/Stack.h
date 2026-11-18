/*
 * CStack Class
 *
 *  Last in, first out, of CObject pointers (owned, as in CArray). Pop()
 * and Peek() give NULL when the stack is empty. Iterating runs from the
 * bottom to the top. For a type safe stack, use TStack.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STACK_H__
#define IASLIB_STACK_H__

#include "Array.h"

namespace IASLib
{
    class CStack : public CCollection
    {
        protected:
            CArray              m_aElements;
        public:
                                CStack( int nScale = 4 );
            virtual            ~CStack( void );

                                DEFINE_OBJECT( CStack )

            virtual size_t      GetLength( void ) const { return m_aElements.GetLength(); }

                // Returns the object's index (its depth from the bottom).
            virtual size_t      Push( CObject *pNew );
            virtual CObject    *Pop( void );
            virtual CObject    *Peek( void ) const;

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );
            virtual void        SetScale( int nScale );

            virtual CIterator  *Enumerate( void );
    }; // End of class CStack
} // End of Namespace IASLib

#endif // IASLIB_STACK_H__
