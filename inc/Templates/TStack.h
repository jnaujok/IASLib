/**
 * @file
 * @brief TStack Template.
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
    /// A last in, first out stack, built on a TArray.
    ///
    /// Pop() and Peek() on an empty stack throw a CCollectionException.
    /// Iteration runs from the bottom of the stack to the top. Not thread-safe.
    ///
    /// @tparam T The element type.
    ///
    /// @code
    /// TStack<int> oStack;
    /// oStack.Push( 1 );
    /// oStack.Push( 2 );
    /// int nTop = oStack.Pop();        // 2
    /// @endcode
    template <class T>
    class TStack : public CObject
    {
        protected:
            TArray<T>       m_aItems;   ///< The elements, bottom first.

        public:
            typedef typename TArray<T>::iterator        iterator;   ///< Iterator, bottom to top.
            typedef typename TArray<T>::const_iterator  const_iterator;   ///< Const iterator, bottom to top.

            DECLARE_OBJECT( TStack, CObject )

            /// Makes an empty stack.
            /// @param nScale The least the storage grows by.
            explicit TStack( size_t nScale = 16 ) : m_aItems( nScale ) {}

            size_t      Length( void ) const { return m_aItems.Length(); }   ///< The depth.
            size_t      GetLength( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            size_t      Count( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_aItems.IsEmpty(); }   ///< True if the stack is empty.
            void        Reserve( size_t nCapacity ) { m_aItems.Reserve( nCapacity ); }   ///< Room for nCapacity elements without growing.
            void        Compact( void ) { m_aItems.Compact(); }   ///< Gives back unused room.
            void        SetScale( size_t nScale ) { m_aItems.SetScale( nScale ); }   ///< Sets the least the storage grows by.

                /// Pushes a copy on top. Returns the new depth.
            size_t      Push( const T &value ) { return m_aItems.Push( value ) + 1; }
            /// Same, moving the value in.
            size_t      Push( T &&value ) { return m_aItems.Push( std::move( value ) ) + 1; }

            /// Removes and returns the top value.
            /// @throws CCollectionException if the stack is empty.
            T Pop( void )
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Pop();
            }

            /// The top value, left in place.
            /// @throws CCollectionException if the stack is empty.
            T &Peek( void )
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Last();
            }

            /// The top value, left in place.
            /// @throws CCollectionException if the stack is empty.
            const T &Peek( void ) const
            {
                if ( m_aItems.IsEmpty() )
                    TemplateDetail::ThrowOutOfRange( "TStack is empty" );
                return m_aItems.Last();
            }

                /// A copy of the value nDepth from the top (0 is the top), or oDefault.
            T Get( size_t nDepth, const T &oDefault = T() ) const
            {
                return ( nDepth < m_aItems.Length() ) ? m_aItems[ m_aItems.Length() - 1 - nDepth ] : oDefault;
            }

            void        Clear( void ) { m_aItems.Clear(); }   ///< Removes every value.
            void        DeleteAll( void ) { m_aItems.DeleteAll(); }   ///< For pointers: deletes what each points to, then empties the stack.

            iterator        begin( void ) { return m_aItems.begin(); }   ///< The bottom value.
            iterator        end( void ) { return m_aItems.end(); }   ///< Past the top value.
            const_iterator  begin( void ) const { return m_aItems.begin(); }   ///< The bottom value.
            const_iterator  end( void ) const { return m_aItems.end(); }   ///< Past the top value.
    };
} // namespace IASLib

#endif // IASLIB_TSTACK_H__
