/**
 *  @file
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
        /// The connections a database holds.
        ///
        /// The array owns them: DeleteAll() (and the destructor) deletes
        /// them; Delete() only takes one out of the array. Not thread-safe.
        /// Not copyable.
    class CConnectionArray : public CObject
    {
        protected:
            TArray<CConnection *> m_aElements;  ///< The connections (owned).
        public:
                                /// Creates an empty array.
                                /// @param nScale How many elements to grow by (0 or less: 4).
                                CConnectionArray( int nScale = 4 );
                                /// Deletes every connection.
            virtual            ~CConnectionArray( void );

                                DEFINE_OBJECT( CConnectionArray )

            /// Adds a connection at the end; the array then owns it.
            /// @return Its index.
            virtual int         Push( CConnection *pNew );
            /// The connection at an index (NULL if there is none).
            virtual CConnection *Get( int nCount ) const;
            /// The same as Get().
            virtual CConnection *operator []( int nCount ) const { return Get( nCount ); }
            /// Inserts a connection before an index (at the end if past it; at 0 if negative).
            /// @return The index it went to.
            virtual int         Insert( int nCount, CConnection *pNew );
                /// Takes the connection out of the array, without deleting it.
                /// @return False if there is no such index.
            virtual bool        Delete( int nCount );
            /// How many connections there are.
            virtual int         Length( void ) const { return (int)m_aElements.Length(); }
            /// Deletes every connection and empties the array.
            virtual void        DeleteAll( void );

        private:
                                /// Not copyable.
                                CConnectionArray( const CConnectionArray & );
                                /// Not assignable.
            CConnectionArray   &operator =( const CConnectionArray & );
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTIONARRAY_H__
