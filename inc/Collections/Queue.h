/**
 * @file Queue.h
 * @brief CQueue, an owning first-in, first-out queue of CObject pointers.
 *
 * CQueue Class
 *
 *  First in, first out, of CObject pointers (owned, as in CArray), in a
 * ring buffer, so Push() and Pop() take constant time. Pop() and Peek()
 * give NULL when the queue is empty. Iterating runs from the front (the
 * next to be popped). It is thread-safe: each call locks the queue. For
 * a type safe queue, use TQueue.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_QUEUE_H__
#define IASLIB_QUEUE_H__

#include "Collection.h"
#include "../Templates/TQueue.h"
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

namespace IASLib
{
    /**
     * First in, first out, of CObject pointers (owned, as in CArray).
     *
     * A ring buffer, so Push() and Pop() take constant time. Pop() and
     * Peek() give NULL when the queue is empty. Iterating runs from the front
     * (the next to be popped). It is thread-safe: each call locks the queue.
     * It cannot be copied. For a type safe queue, use TQueue.
     *
     * @code
     * CQueue q;
     * q.Push( new CString( "first" ) );
     * q.Push( new CString( "second" ) );
     * CObject *p = q.Pop();       // "first"; the caller owns it now
     * delete p;
     * @endcode
     */
    class CQueue : public CCollection
    {
        protected:
            TQueue<CObject *>   m_qElements;    ///< The elements, front first.
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex      m_mutex;        ///< Locked by every call.
#endif
        public:
                                /// An empty queue.
                                /// @param nScale The least it grows by (0 means 4).
                                CQueue( size_t nScale = 4 );
                                /// Deletes every element.
            virtual            ~CQueue( void );

                                DEFINE_OBJECT( CQueue )

                /// The number of elements.
            virtual size_t      GetLength( void ) const;

                /// Adds at the back, returning the new length. The queue owns pNew.
            virtual size_t      Push( CObject *pNew );
                /// Takes the front element out (the caller owns it), or NULL.
            virtual CObject    *Pop( void );
                /// The front element (still owned by the queue), or NULL.
            virtual CObject    *Peek( void ) const;
                /// The object nPosition from the front, or NULL.
            virtual CObject    *Get( size_t nPosition ) const;

                /// Removes and deletes every element.
            virtual void        DeleteAll( void );
                /// Removes every element without deleting it.
            virtual void        EmptyAll( void );
                /// Sets the least the queue grows by.
            virtual void        SetScale( size_t nScale );

                /// A new iterator, front to back (the caller deletes it).
            virtual CIterator  *Enumerate( void );

        private:
                                /// Not copyable (the queue owns its elements).
                                CQueue( const CQueue & );
                                /// Not assignable.
            CQueue             &operator =( const CQueue & );
    }; // End of class CQueue
} // End of Namespace IASLib

#endif // IASLIB_QUEUE_H__
