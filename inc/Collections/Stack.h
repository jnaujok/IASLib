/**
 * @file Stack.h
 * @brief CStack, an owning last-in, first-out stack of CObject pointers.
 *
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
    /**
     * Last in, first out, of CObject pointers (owned, as in CArray).
     *
     * Pop() and Peek() give NULL when the stack is empty. Iterating runs
     * from the bottom to the top. Each call locks the underlying CArray, but
     * Pop() and Peek() are two calls, so share a stack between threads only
     * behind a lock. For a type safe stack, use TStack.
     *
     * @code
     * CStack stack;
     * stack.Push( new CString( "bottom" ) );
     * stack.Push( new CString( "top" ) );
     * delete stack.Pop();         // "top"; popping hands it to the caller
     * @endcode
     */
    class CStack : public CCollection
    {
        protected:
            CArray              m_aElements;    ///< The elements, bottom first.
        public:
                                /// An empty stack.
                                /// @param nScale The least it grows by (0 or less means 4).
                                CStack( int nScale = 4 );
                                /// Deletes every element.
            virtual            ~CStack( void );

                                DEFINE_OBJECT( CStack )

                /// The number of elements.
            virtual size_t      GetLength( void ) const { return m_aElements.GetLength(); }

                /// Puts pNew on top (the stack owns it). Returns the object's
                /// index (its depth from the bottom).
            virtual size_t      Push( CObject *pNew );
                /// Takes the top element off (the caller owns it), or NULL.
            virtual CObject    *Pop( void );
                /// The top element (still owned by the stack), or NULL.
            virtual CObject    *Peek( void ) const;

                /// Removes and deletes every element.
            virtual void        DeleteAll( void );
                /// Removes every element without deleting it.
            virtual void        EmptyAll( void );
                /// Sets the least the stack grows by (ignored unless positive).
            virtual void        SetScale( int nScale );

                /// A new iterator, bottom to top (the caller deletes it).
            virtual CIterator  *Enumerate( void );
    }; // End of class CStack
} // End of Namespace IASLib

#endif // IASLIB_STACK_H__
