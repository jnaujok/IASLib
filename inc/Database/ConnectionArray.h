/*
 *  Connection Array Class
 *
 *  This class is a specialization of the generic CArray class that is used
 * to store and maintain the list of connections on a database.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/26/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CONNECTIONARRAY_H__
#define IASLIB_CONNECTIONARRAY_H__

#include "Connection.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CConnectionArray : public CObject
    {
        protected:
            CConnection       **m_apElements;
            int                 m_nElements;
            int                 m_nSize;
            int                 m_nScale;
        public:
                                CConnectionArray( int nScale = 4 );
            virtual            ~CConnectionArray( void );

                                DEFINE_OBJECT( CConnectionArray )

            virtual int         Push( CConnection *pNew );
            virtual CConnection *Get( int nCount );
            virtual CConnection *operator []( int nCount );
            virtual int         Insert( int nCount, CConnection *pNew );
            virtual bool        Delete( int nCount );
            virtual int         Length( void ) { return m_nElements; }
            virtual void        DeleteAll( void );
        private:
            void                Resize( bool bIncrease = true );
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTIONARRAY_H__

