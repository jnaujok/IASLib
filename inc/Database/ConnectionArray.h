/**
 * @file
 * @brief Connection Array Class.
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
        /// The connections a database holds (it owns them: DeleteAll()
        /// deletes them; Delete() only takes one out of the array).
    class CConnectionArray : public CObject
    {
        protected:
            TArray<CConnection *> m_aElements;      ///< The connections.
        public:
                                /// Makes an empty array; @p nScale is the initial room (4 if not positive).
                                CConnectionArray( int nScale = 4 );
                                /// Destructor. Deletes every connection (DeleteAll()).
            virtual            ~CConnectionArray( void );

                                DEFINE_OBJECT( CConnectionArray )

                /// Adds a connection at the end (the array takes ownership).
                /// Returns its index.
            virtual int         Push( CConnection *pNew );
                /// The connection at @p nCount, or NULL.
            virtual CConnection *Get( int nCount ) const;
            virtual CConnection *operator []( int nCount ) const { return Get( nCount ); }  ///< Same as Get().
                /// Inserts a connection before @p nCount (at the start if it is
                /// negative, at the end if past it). Returns its index.
            virtual int         Insert( int nCount, CConnection *pNew );
                /// Takes the connection out of the array, without deleting it.
                /// False if there is no such index.
            virtual bool        Delete( int nCount );
            virtual int         Length( void ) const { return (int)m_aElements.Length(); }  ///< The number of connections.
                /// Takes a connection out of the array (false if not there).
            bool                Remove( CConnection *pConnection );
                /// The index of a connection, or -1.
            int                 Find( CConnection *pConnection ) const;
                /// Deletes every connection. Each is taken out before it is
                /// deleted, so a destructor that removes itself is safe.
                /// @note Deleting in place used to skip every other one, and
                /// those leaked.
            virtual void        DeleteAll( void );

        private:
                                CConnectionArray( const CConnectionArray & );   ///< Not copyable.
            CConnectionArray   &operator =( const CConnectionArray & );       ///< Not copyable.
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTIONARRAY_H__
