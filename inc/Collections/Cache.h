/**
 * Cache Class
 *
 *  This class provides an LRU cache for storing objects derived 
 * from CCacheItem. As new items are cached, those that were used 
 * last are dropped from the cache. 
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 1/15/2004
 *  Log:
 *    $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#ifndef IASLIB_CACHE_H__
#define IASLIB_CACHE_H__

#include "../BaseTypes/String_.h"
#include "Hash.h"
#include "CacheItem.h"

namespace IASLib
{
    class CCache : public CCollection
    {
        friend class CCacheItem;

        protected:
            CCacheItem             *m_pListFirst;   // Used for LRU determination
            CCacheItem             *m_pListLast;
            CHash                   m_hashEntries;
            
            size_t                  m_nQueueMax;
			bool					m_bUseExpiration;
#ifdef IASLIB_MULTI_THREADED__
            CMutex                  cacheMutex;
#endif


        public:
                                    CCache( size_t nMaxEntries, bool bUseExpiration = false );
            virtual                ~CCache( void );

                                    DEFINE_OBJECT(CCache);

            virtual bool            AddItem( CCacheItem *pItem );

            virtual bool            RemoveItem( CCacheItem *pKey );

            virtual CCacheItem     *Get( CCacheItem *pKey );

            virtual bool            Touch( CCacheItem *pKey );

            virtual bool            Discard( CCacheItem *pKey );

            virtual void            DeleteAll( void );
            virtual void            EmptyAll( void );

            virtual CIterator      *Enumerate( void );

            virtual void            ClearExpired( void );
            virtual void            ClearExpired( CDate &dttCompare );

        private:
            void                    MoveToHead( CCacheItem *pItem );
            void                    MoveToTail( CCacheItem *pItem );
            void                    DetatchList( void );
            void                    RemoveFromList( CCacheItem *pItem );
    };

} // namespace IASLib

#endif // IASLIB_CACHE_H__
