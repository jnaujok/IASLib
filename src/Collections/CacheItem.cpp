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

#include "CacheItem.h"
#include "Cache.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCacheItem, CObject );

	CCacheItem::CCacheItem( void )
	{
        m_dttCreated.SetToCurrent();
        m_dttLastAccess.SetToCurrent();
        m_dttExpires.SetToCurrent();
        m_dttExpires.Add(1);    // Default expiration is 24 hours.
        m_nHitCount = 0;
        m_pNextItem = NULL;
        m_pPrevItem = NULL;
	}

	CCacheItem::~CCacheItem( void )
	{
 	}

    void CCacheItem::Hit( CCache *pParentCache )
    {
        m_dttLastAccess.SetToCurrent();
        m_nHitCount++;

        pParentCache->MoveToHead( this );
    }


    bool CCacheItem::IsExpired( void )
    {
       CDate dttNow;

       dttNow.SetToCurrent();

       if ( dttNow >= m_dttExpires )
       {
          return true;
       }
       return false;
    }

    bool CCacheItem::IsExpired( CDate &dttDate )
    {
       if ( m_dttExpires <= dttDate )
       {
          return true;
       }
       return false;
    }


} // Namespace IASLib

