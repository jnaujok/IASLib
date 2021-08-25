//
// Created by jnaujok on 8/26/20.
//

#ifndef IASLIB_TARRAY_H
#define IASLIB_TARRAY_H

#include "TCollection.h"

namespace IASLib {

    template<class T>
    class TArray : public TCollection<T> {
    protected:
        T                 **m_apElements;
        size_t              m_nSize;
        size_t              m_nScale;
#ifdef IASLIB_MULTI_THREADED__
        mutable CMutex      m_mutex;
#endif
    public:
        DEFINE_OBJECT(TArray)

        explicit TArray( size_t size=16, size_t scale = 16 ) : TCollection<T>() {
            m_apElements = new T[size];
            m_nSize = size;
            m_nScale = scale;
        }

        virtual TIterator<T>  *enumerate( );

        virtual void        deleteAll(  );
        virtual void        emptyAll(  );

        virtual size_t      append( T *pNew ) { return Push( pNew ); }
        virtual size_t      prepend( T *pNew ) { return Insert( 0, pNew ); }

        virtual size_t      insert( size_t nIndex, T *pNew );

        virtual size_t      push( T *pNew );

        virtual size_t      set( size_t nIndex, T *pSet );

        virtual T          *get( size_t nIndex ) const;
        virtual T          *operator []( size_t nIndex ) const;

        virtual T          *remove( size_t nIndex );

        virtual void        sort( int (*fnCompare)(const T *, const T *, void *), void *pCallbackData );

        virtual void        setScale( size_t nScale ) {
            if ( nScale > 0 ) m_nScale = nScale;
        }

        virtual size_t      getScale() { return m_nScale; }

        virtual bool		swap( size_t nSwap1, size_t nSwap2 );
    protected:
        void                resize( bool bIncrease = true );

        void                quickSort( size_t nLow, size_t nHigh, int (*fnCompare)(const T *, const T *, void *), void *pCallback );
    };
} // namespace IASLib

#endif //IASLIB_TARRAY_H
