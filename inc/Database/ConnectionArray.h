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
#include "../Templates/TArray.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        // The connections a database holds (it owns them: DeleteAll()
        // deletes them; Delete() only takes one out of the array).
    class CConnectionArray : public CObject
    {
        protected:
            TArray<CConnection *> m_aElements;
        public:
                                CConnectionArray( int nScale = 4 );
            virtual            ~CConnectionArray( void );

                                DEFINE_OBJECT( CConnectionArray )

            virtual int         Push( CConnection *pNew );
            virtual CConnection *Get( int nCount ) const;
            virtual CConnection *operator []( int nCount ) const { return Get( nCount ); }
            virtual int         Insert( int nCount, CConnection *pNew );
                // Takes the connection out of the array, without deleting it.
            virtual bool        Delete( int nCount );
            virtual int         Length( void ) const { return (int)m_aElements.Length(); }
                // Takes a connection out of the array (false if not there).
            bool                Remove( CConnection *pConnection );
            int                 Find( CConnection *pConnection ) const;
                // Deletes every connection. Each is taken out before it is
                // deleted, so a destructor that removes itself is safe.
            virtual void        DeleteAll( void );

        private:
                                CConnectionArray( const CConnectionArray & );
            CConnectionArray   &operator =( const CConnectionArray & );
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTIONARRAY_H__
