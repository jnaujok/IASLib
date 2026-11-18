/**
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

    class CCacheItem : public CObject
    {
        friend class CCache;

        protected:
            CDate                   m_dttCreated;
            CDate                   m_dttLastAccess;
            CDate                   m_dttExpires;
            unsigned long           m_nHitCount;

        public:
                                    CCacheItem( void );
            virtual                ~CCacheItem( void );

                                    DEFINE_OBJECT(CCacheItem);

            virtual CCacheItem     *GetItem( void ) = 0;
            virtual CString         GetKey( void ) const = 0;
            virtual int             Compare( const CCacheItem *pCompare ) const = 0;

            inline bool             operator ==( const CCacheItem &oCompare ) const
                                    {
                                        return Compare( &oCompare ) == 0;
                                    }

            static int              SortCompare( const CObject *pItem1, const CObject *pItem2 )
                                    {
                                        return ((const CCacheItem *)pItem1)->Compare( (const CCacheItem *)pItem2 );
                                    }

                // Records a use (the cache calls this from Get() and Touch()).
            virtual void            Hit( CCache *pParentCache );

            void                    SetExpiration( const CDate &dttExpires ) { m_dttExpires = dttExpires; }
            const CDate            &GetExpiration( void ) const { return m_dttExpires; }
            const CDate            &GetCreated( void ) const { return m_dttCreated; }
            const CDate            &GetLastAccess( void ) const { return m_dttLastAccess; }
            unsigned long           GetHitCount( void ) const { return m_nHitCount; }

            virtual bool            IsExpired( void ) const;
            virtual bool            IsExpired( const CDate &dttCompare ) const;
    };
} // Namespace IASLib

#endif // IASLIB_CACHE_ITEM_H__
