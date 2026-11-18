/**
 * @file SparseArray.h
 * @brief CSparseArray, an owning array where only some indexes have
 * objects, and its iterator.
 *
 * CSparseArray Class
 *
 *  An array where only some indexes have objects (owned, as in CArray):
 * ten objects spread over 10,000,000 indexes take the room of ten.
 * Insert( n, p ) puts p at index n (deleting what was there), and Get()
 * gives NULL where there is nothing. There is no Push: an object needs an
 * index. Iterating gives the objects in index order, with
 * GetSparseNumber() the index of the last one returned.
 *
 *  For a type safe sparse array, or one of values, use TSparseArray.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SPARSEARRAY_H__
#define IASLIB_SPARSEARRAY_H__

#include "Collection.h"
#include "../Templates/TSparseArray.h"

namespace IASLib
{
    class CSparseArray;

    /// The iterator CSparseArray::Enumerate() gives: the objects in index
    /// order, with GetSparseNumber() the index of the last one returned.
    class CSparseArrayIterator : public CIterator
    {
        protected:
            const CSparseArray *m_pArray;           ///< The array (not owned).
            size_t              m_nCurrentPos;      ///< The position of the next entry among those present.
            size_t              m_nSparseNumber;    ///< The index of the last object returned.
        public:
                                /// An iterator at the start of pArray.
                                CSparseArrayIterator( const CSparseArray *pArray ) { m_nSparseNumber = 0; m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CSparseArrayIterator, CIterator )
            virtual            ~CSparseArrayIterator( void ) {}
                /// The next object, or NULL at the end.
            virtual CObject    *Next( void );
                /// Steps back and returns that object, or NULL at the start.
            virtual CObject    *Prev( void );
                /// Back to the start.
            virtual void        Reset( void ) { m_nCurrentPos = 0; m_nSparseNumber = 0; }
                /// Whether there is a next object.
            virtual bool        HasMore( void ) const;
                /// The index of the last object Next() or Prev() returned.
            virtual size_t      GetSparseNumber( void ) { return m_nSparseNumber; }
    };

    /**
     * An array where only some indexes have objects (owned, as in CArray).
     *
     * Ten objects spread over 10,000,000 indexes take the room of ten.
     * Insert( n, p ) puts p at index n (deleting what was there), and Get()
     * gives NULL where there is nothing. There is no Push: an object needs
     * an index. Iterating gives the objects in index order. Not thread-safe,
     * and not copyable. For a type safe sparse array, or one of values, use
     * TSparseArray.
     *
     * @code
     * CSparseArray aSparse;
     * aSparse.Insert( 9000000, new CString( "far" ) );
     * CObject *p = aSparse[ 5 ];               // NULL: nothing there
     * size_t n = aSparse.GetLength();          // 1
     * @endcode
     */
    class CSparseArray : public CCollection
    {
        friend class CSparseArrayIterator;
        protected:
            TSparseArray<CObject *> m_aElements;    ///< The objects, by index.
        public:
                                /// An empty array.
                                /// @param nGrowBy How the storage grows.
                                CSparseArray( size_t nGrowBy = 16 );
                                /// Deletes every object.
            virtual            ~CSparseArray( void );

                                DECLARE_OBJECT( CSparseArray, CCollection );

                /// The number of objects (not the highest index).
            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

                /// Puts pNew at nIndex, deleting what was there (unless it is
                /// pNew); returns nIndex. The array owns pNew.
            virtual size_t      Insert( size_t nIndex, CObject *pNew );
                /// Not supported (an object needs an index).
                /// @throws CCollectionException Always.
            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
                /// Not supported (an object needs an index).
                /// @throws CCollectionException Always.
            virtual size_t      Prepend( CObject *pNew ) { return Push( pNew ); }
                /// Not supported (an object needs an index).
                /// @throws CCollectionException Always.
            virtual size_t      Push( CObject *pNew );

                /// The object at nIndex (still owned by the array), or NULL.
            virtual CObject    *Get( size_t nIndex ) const;
                /// Same as Get().
            virtual CObject    *operator []( size_t nIndex ) const { return Get( nIndex ); }
                /// Takes the object at nIndex out (the caller owns it), or NULL.
            virtual CObject    *Remove( size_t nIndex );
                /// Takes the object at nIndex out and deletes it; false if
                /// there was none.
            virtual bool        Delete( size_t nIndex );
                /// Exchanges the objects at two indexes (either may be empty).
                /// False if they are the same, or both empty.
            virtual bool        Swap( size_t nSwap1, size_t nSwap2 );
                /// Whether there is an object at nIndex.
            virtual bool        Exists( size_t nIndex ) const { return m_aElements.Exists( nIndex ); }

                /// Removes and deletes every object.
            virtual void        DeleteAll( void );
                /// Removes every object without deleting it.
            virtual void        EmptyAll( void );

                /// A new CSparseArrayIterator (the caller deletes it).
            virtual CIterator  *Enumerate( void );

        private:
                                /// Not copyable (the array owns its objects).
                                CSparseArray( const CSparseArray & );
                                /// Not assignable.
            CSparseArray       &operator =( const CSparseArray & );
    };
} // namespace IASLib

#endif // IASLIB_SPARSEARRAY_H__
