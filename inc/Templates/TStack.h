/*
 * TStack Template
 *
 *      Last in, first out. Pop() and Peek() on an empty stack throw a
 * CCollectionException. Iterating runs from the bottom of the stack to
 * the top; Get( 0 ) is the top.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TSTACK_H__
#define IASLIB_TSTACK_H__

#include "TArray.h"

namespace IASLib
{
    template <class T>
    class TStack : public CObject
    {
        protected:
            TArray<T>       m_aItems;

        public:
            typedef typename TArray<T>::iterator        iterator;
            typedef typename TArray<T>::const_iterator  const_iterator;

            DECLARE_OBJECT( TStack, CObject )

            explicit TStack( size_t nScale = 16 ) : m_aItems( nScale ) {}

            size_t      Length( void ) const { return m_aItems.Length(); }
            size_t      GetLength( void ) const { return m_aItems.Length(); }
            size_t      Count( void ) const { return m_aItems.Length(); }
            size_t      GetCount( void ) const { return m_aItems.Length(); }
            bool        IsEmpty( void ) const { return m_aItems.IsEmpty(); }
            void        Reserve( size_t nCapacity ) { m_aItems.Reserve( nCapacity ); }
            void        Compact( void ) { m_aItems.Compact(); }
            void        SetScale( size_t nScale ) { m_aItems.SetScale( nScale ); }

                // Returns the new depth.
            size_t      Push( const T &value ) { return m_aItems.Push( value ) + 1; }
            size_t      Push( T &&value ) { return m_aItems.Push( std::move( value ) ) + 1; }

            T Pop( void )
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Pop();
            }

            T &Peek( void )
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Last();
            }

            const T &Peek( void ) const
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Last();
            }

                // nDepth from the top (0 is the top), or oDefault.
            T Get( size_t nDepth, const T &oDefault = T() ) const
            {
                return ( nDepth < m_aItems.Length() ) ? m_aItems[ m_aItems.Length() - 1 - nDepth ] : oDefault;
            }

            void        Clear( void ) { m_aItems.Clear(); }
            void        DeleteAll( void ) { m_aItems.DeleteAll(); }

            iterator        begin( void ) { return m_aItems.begin(); }
            iterator        end( void ) { return m_aItems.end(); }
            const_iterator  begin( void ) const { return m_aItems.begin(); }
            const_iterator  end( void ) const { return m_aItems.end(); }
    };
} // namespace IASLib

#endif // IASLIB_TSTACK_H__
