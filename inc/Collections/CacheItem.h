/**
 * @file CacheItem.h
 * @brief CCacheItem, the base class of objects kept in a CCache.
 *
 * CacheItem Class
 *
 *  The base class of objects kept in a CCache. A cache item knows its
 * key (GetKey()), when it was created and last used, how often it has
 * been used, and when it expires (24 hours after it is made, unless
 * SetExpiration() says otherwise).
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 1/15/2004
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CACHE_ITEM_H__
#define IASLIB_CACHE_ITEM_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CCache; // Forward Reference

    /**
     * The base class of objects kept in a CCache.
     *
     * A cache item knows its key (GetKey()), when it was created and last
     * used, how often it has been used, and when it expires (24 hours after
     * it is made, unless SetExpiration() says otherwise). A derived class
     * supplies GetItem(), GetKey() and Compare().
     *
     * @code
     * class CUserEntry : public CCacheItem
     * {
     *     public:
     *         CString     m_strName;
     *         CCacheItem *GetItem( void ) { return this; }
     *         CString     GetKey( void ) const { return m_strName; }
     *         int         Compare( const CCacheItem *p ) const
     *                     { return m_strName.Compare( p->GetKey() ); }
     * };
     * @endcode
     */
    class CCacheItem : public CObject
    {
        friend class CCache;

        protected:
            CDate                   m_dttCreated;       ///< When the item was made.
            CDate                   m_dttLastAccess;    ///< When it was last used.
            CDate                   m_dttExpires;       ///< When it expires.
            unsigned long           m_nHitCount;        ///< How many times it has been used.

        public:
                                    /// A new item, expiring in 24 hours.
                                    CCacheItem( void );
            virtual                ~CCacheItem( void );

                                    DEFINE_OBJECT(CCacheItem);

                /// The item (for a derived class, normally itself).
            virtual CCacheItem     *GetItem( void ) = 0;
                /// The key the cache finds the item by.
            virtual CString         GetKey( void ) const = 0;
                /// Orders items: negative, zero or positive, as strcmp().
            virtual int             Compare( const CCacheItem *pCompare ) const = 0;

                /// Whether Compare() says the items are equal.
            inline bool             operator ==( const CCacheItem &oCompare ) const
                                    {
                                        return Compare( &oCompare ) == 0;
                                    }

                /// Compare() for two CObject pointers that are cache items,
                /// as a sort function wants it.
            static int              SortCompare( const CObject *pItem1, const CObject *pItem2 )
                                    {
                                        return ((const CCacheItem *)pItem1)->Compare( (const CCacheItem *)pItem2 );
                                    }

                /// Records a use (the cache calls this from Get() and Touch()):
                /// the last access becomes now, the hit count goes up, and the
                /// item becomes pParentCache's most recently used (if not NULL).
            virtual void            Hit( CCache *pParentCache );

                /// Sets when the item expires.
            void                    SetExpiration( const CDate &dttExpires ) { m_dttExpires = dttExpires; }
                /// When the item expires.
            const CDate            &GetExpiration( void ) const { return m_dttExpires; }
                /// When the item was made.
            const CDate            &GetCreated( void ) const { return m_dttCreated; }
                /// When the item was last used.
            const CDate            &GetLastAccess( void ) const { return m_dttLastAccess; }
                /// How many times the item has been used.
            unsigned long           GetHitCount( void ) const { return m_nHitCount; }

                /// Whether the item has expired by now.
            virtual bool            IsExpired( void ) const;
                /// Whether the item has expired by dttCompare (its expiry is
                /// at or before it).
            virtual bool            IsExpired( const CDate &dttCompare ) const;
    };
} // Namespace IASLib

#endif // IASLIB_CACHE_ITEM_H__
