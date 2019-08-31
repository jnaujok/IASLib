/**
 * Cache Item Class
 *
 *  This class provides a base class for cachable items. An item that is
 * cachable must provide a hashing function, an equivalence function and
 * a recasting function for extracting the data returned. Thus, the cache
 * item class is an abstract base class that must be inherited and then
 * implemented by a cachable item.
 *
 *  This file also defines the IMPLEMENT_CACHE_ITEM macro that must be
 * defined by any class that implements a cache item.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 1/15/2004
 *  Log:
 *    $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CACHE_ITEM_H__
#define IASLIB_CACHE_ITEM_H__

#include "../BaseTypes/String_.h"
#include "SortedArray.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CCache; // Forward Reference 

    class CCacheItem : public CObject
    {
        friend class CCache; 
                       // We give friend access to the CCache class so that the
                       // cache can maintain the linked list nature of the cache
                       // item, thereby allowing us to have the node links in the
                       // CCacheItem, but modify them in the CCache class.

        protected:
            CDate                   m_dttCreated;
            CDate                   m_dttLastAccess;
            CDate                   m_dttExpires;
            unsigned long           m_nHitCount;
            CCacheItem             *m_pNextItem; // Used in LRU/LFU caching
            CCacheItem             *m_pPrevItem; // doubly linked list
        public:

                                    CCacheItem( void );
            virtual                ~CCacheItem( void );
                
                                    DEFINE_OBJECT(CCacheItem);

            virtual CCacheItem     *GetItem( void ) = 0;
            virtual CString         GetKey( void ) const = 0;
            
            virtual int             Compare( const CCacheItem *pCompare ) const = 0;

            inline bool             operator ==( const CCacheItem &oCompare )
                                    {
                                        return (bool)(Compare( (const CCacheItem *)&oCompare ) == 0 );
                                    }

            static int              SortCompare( const CObject *pItem1, const CObject *pItem2 )
                                    {
                                        return ((CCacheItem *)pItem1)->Compare( (CCacheItem *)pItem2 );
                                    }

            virtual void            Hit( CCache *pParentCache );

            virtual bool            IsExpired( void );
            virtual bool            IsExpired( CDate &dttCompare );
    };
} // Namespace IASLib

#endif // IASLIB_CACHE_ITEM_H__
