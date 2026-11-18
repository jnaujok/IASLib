/*
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

    class CSparseArrayIterator : public CIterator
    {
        protected:
            const CSparseArray *m_pArray;
            size_t              m_nCurrentPos;
            size_t              m_nSparseNumber;
        public:
                                CSparseArrayIterator( const CSparseArray *pArray ) { m_nSparseNumber = 0; m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CSparseArrayIterator, CIterator )
            virtual            ~CSparseArrayIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );
            virtual void        Reset( void ) { m_nCurrentPos = 0; m_nSparseNumber = 0; }
            virtual bool        HasMore( void ) const;
            virtual size_t      GetSparseNumber( void ) { return m_nSparseNumber; }
    };

    class CSparseArray : public CCollection
    {
        friend class CSparseArrayIterator;
        protected:
            TSparseArray<CObject *> m_aElements;
        public:
                                CSparseArray( size_t nGrowBy = 16 );
            virtual            ~CSparseArray( void );

                                DECLARE_OBJECT( CSparseArray, CCollection );

            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

                // Puts pNew at nIndex, deleting what was there (unless it is
                // pNew); returns nIndex.
            virtual size_t      Insert( size_t nIndex, CObject *pNew );
                // Not supported: throw a CCollectionException.
            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Push( CObject *pNew );

            virtual CObject    *Get( size_t nIndex ) const;
            virtual CObject    *operator []( size_t nIndex ) const { return Get( nIndex ); }
                // Takes the object at nIndex out (the caller owns it), or NULL.
            virtual CObject    *Remove( size_t nIndex );
            virtual bool        Delete( size_t nIndex );
                // Exchanges the objects at two indexes (either may be empty).
            virtual bool        Swap( size_t nSwap1, size_t nSwap2 );
            virtual bool        Exists( size_t nIndex ) const { return m_aElements.Exists( nIndex ); }

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );

        private:
                                CSparseArray( const CSparseArray & );
            CSparseArray       &operator =( const CSparseArray & );
    };
} // namespace IASLib

#endif // IASLIB_SPARSEARRAY_H__
