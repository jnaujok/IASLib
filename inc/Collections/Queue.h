/*
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
    class CQueue : public CCollection
    {
        protected:
            TQueue<CObject *>   m_qElements;
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex      m_mutex;
#endif
        public:
                                CQueue( size_t nScale = 4 );
            virtual            ~CQueue( void );

                                DEFINE_OBJECT( CQueue )

            virtual size_t      GetLength( void ) const;

                // Adds at the back, returning the new length.
            virtual size_t      Push( CObject *pNew );
            virtual CObject    *Pop( void );
            virtual CObject    *Peek( void ) const;
                // The object nPosition from the front, or NULL.
            virtual CObject    *Get( size_t nPosition ) const;

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );
            virtual void        SetScale( size_t nScale );

            virtual CIterator  *Enumerate( void );

        private:
                                CQueue( const CQueue & );
            CQueue             &operator =( const CQueue & );
    }; // End of class CQueue
} // End of Namespace IASLib

#endif // IASLIB_QUEUE_H__
