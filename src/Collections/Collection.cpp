/**
 * Collection Class
 *
 *  This abstract base class is common to all of the collection classes
 * and all of the derived classes must implement the base interface.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Collection.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCollection, CObject );

        /**
         * Collection Constructor
         *
         * This function provides the constructor for the base Collection
         * class. It takes no parameters. As it is an abstract class, this
         * constructor is never called directly. 
         */
    CCollection::CCollection( void )
    {
        m_nElements = 0;
    }

        /**
         * Collection Destructor
         *
         *  This method supplies the destructor for the abstract Collection
         * class. 
         */
    CCollection::~CCollection( void )
    {
        m_nElements = 0;
    }
} // namespace IASLib
