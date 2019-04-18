/**
 * Queue Class
 *
 *  This class provides a dynamically re-sizable queue for use in
 * storing CObject derived pointers. Any object that has been
 * derived from the CObject type can be stored in this queue.
 *  The queue itself will dynamically re-size itself as used. The
 * rate at which it is re-sized can be adjusted by changing the
 * "Scale" value passed to the constructor, or by using the SetScale
 * function to change it dynamically.
 *  The queue structure only allows insertion at the back of the queue
 * and removal/deletion from the front of the queue. The Push function 
 * adds an element, the Pop function removes an element. The Peek 
 * function allows the viewing of the front element without its removal.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/28/2005
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_QUEUE_H__
#define IASLIB_QUEUE_H__

#include "Array.h"

namespace IASLib
{
    class CQueue : public CCollection
    {
        protected:
            CArray              m_aElements;
        public:
                                CQueue( size_t nScale = 4 );
            virtual            ~CQueue( void );

                                DEFINE_OBJECT( CQueue )

            virtual size_t      Push( CObject *pNew );
            virtual CObject    *Pop( void );
            virtual CObject    *Peek( void );

            virtual size_t      Length( void ) const { return m_aElements.Length(); }
            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }
            virtual size_t      GetCount( void ) const { return m_aElements.Length(); }
            virtual size_t      Count( void ) const { return m_aElements.Length(); }

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual void        SetScale( size_t nScale );

            virtual CIterator  *Enumerate( void );
    }; // End of class CQueue
} // End of Namespace IASLib

#endif // IASLIB_QUEUE_H__
