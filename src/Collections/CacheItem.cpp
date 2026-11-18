/**
 * CacheItem Class
 *
 *  The base class of objects kept in a CCache.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "CacheItem.h"
#include "Cache.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCacheItem, CObject );

    CCacheItem::CCacheItem( void )
    {
        m_dttCreated.SetToCurrent();
        m_dttLastAccess.SetToCurrent();
        m_dttExpires.SetToCurrent();
        m_dttExpires.Add( 1 );    // Default expiration is 24 hours.
        m_nHitCount = 0;
    }

    CCacheItem::~CCacheItem( void )
    {
    }

    void CCacheItem::Hit( CCache *pParentCache )
    {
        m_dttLastAccess.SetToCurrent();
        m_nHitCount++;
        if ( pParentCache != NULL )
            pParentCache->MoveToHead( this );
    }

    bool CCacheItem::IsExpired( void ) const
    {
        CDate dttNow;
        dttNow.SetToCurrent();
        return IsExpired( dttNow );
    }

    bool CCacheItem::IsExpired( const CDate &dttDate ) const
    {
        return m_dttExpires <= dttDate;
    }
} // Namespace IASLib
