/**
 * Iterator Class
 *
 *  This abstract base class is used to define the basic iterator for
 * the collection classes. The iterator type allows the enumeration of 
 * all the different collections while using the same interface
 * regardless of the collection type.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ITERATOR_H__
#define IASLIB_ITERATOR_H__

#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CIterator : public CObject
    {
        public:
                                    /**
                                     * This is the destructor that ends an iterator.
                                     */
            virtual            ~CIterator( void ) {}
    
                                DECLARE_OBJECT( CIterator, CObject )

                                    /**
                                     * This method is for use by the <code>CStringArray</code> class. Namely, it provides
                                     * for the fact that the iterator returned by the <code>CStringArray</code> class 
                                     * should return <code>CStrings</code>, and not <code>CObject</code> pointers.
                                     */
            virtual CString     NextString( void ) { return CString( "" ); }

                                    /**
                                     * This method is for use by the <code>CIntArray</code> class. Namely, it provides
                                     * for the fact that the iterator returned by the <code>CIntArray</code> class 
                                     * should return <code>int</code>s, and not <code>CObject</code> pointers.
                                     */
            virtual int         NextInt( void ) { return 0; }
            
                                    /**
                                     * This method returns the next item in the iteration.
                                     */
            virtual CObject    *Next( void ) = 0;
                                    /**
                                     * This method returns the previous item in the iteration. NOTE: Not all collections
                                     * may implement this method. You must catch the <code>CException</code> that will be
                                     * thrown if this function is not implemented by the collection's iterator.
                                     */
            virtual CObject    *Prev( void ) = 0;

                                    /**
                                     * This method resets the iterator to the begining of the collection. It
                                     * is not required that the iterator allow this function. In that case, you
                                     * must catch the <code>CException</code> that is thrown.
                                     */
            virtual void        Reset( void ) = 0;
                
                                    /**
                                     * This method returns true if there are elements remaining to be iterated
                                     * in the collection.
                                     */
            virtual bool        HasMore( void ) const = 0;

                                    /**
                                     * This operator returns a pointer to the next object in the collection.
                                     */
            virtual CObject    *operator ++( void ) { return Next(); }
                                    /**
                                     * This opertor returns a pointer to the previous object in the collection.
                                     */
            virtual CObject    *operator --( void ) { return Prev(); }

        protected:
                                    /**
                                     * CIterator constructor
                                     *
                                     * This class is protected so no one tries to create an Iterator
                                     * directly. 
                                     */
                                CIterator( void ) {}
                                    /**
                                     * CIterator Copy Constructor
                                     *
                                     * This class is protected so no one can create a copy of an Iterator.
                                     */
                                CIterator( const CIterator &oSource ) { oSource.HasMore(); }
                                    /**
                                     * CIterator Copy Operator 
                                     *
                                     * This operator is hidden to prevent copying of iterators.
                                     */
            CIterator          &operator =( const CIterator &oSource ) { oSource.HasMore(); return *this; }
    };
} // namespace IASLib

#endif // IASLIB_ITERATOR_H__
