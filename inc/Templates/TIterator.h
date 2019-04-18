/**
 * TIterator Class
 *
 *  This abstract base template class is used to define the basic iterator for
 * the template collection classes. The iterator type allows the enumeration of 
 * all the different collections while using the same interface
 * regardless of the collection type.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 20019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TITERATOR_H__
#define IASLIB_TITERATOR_H__

namespace IASLib
{
    template <class T>
    class TIterator : public CObject
    {
        public:
                                    /**
                                     * This is the destructor that ends an iterator.
                                     */
            virtual            ~TIterator( void ) {}
    
                                DECLARE_OBJECT( TIterator, CObject )

                                    /**
                                     * This method returns the next item in the iteration.
                                     */
            virtual T               *Next( void ) = 0;
                                    /**
                                     * This method returns the previous item in the iteration. NOTE: Not all collections
                                     * may implement this method. You must catch the <code>CException</code> that will be
                                     * thrown if this function is not implemented by the collection's iterator.
                                     */
            virtual T         *Prev( void ) = 0;

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
            virtual T    *operator ++( void ) { return Next(); }
                                    /**
                                     * This opertor returns a pointer to the previous object in the collection.
                                     */
            virtual T    *operator --( void ) { return Prev(); }

        protected:
                                    /**
                                     * TIterator constructor
                                     *
                                     * This class is protected so no one tries to create an Iterator
                                     * directly. 
                                     */
                                TIterator( void ) {}
                                    /**
                                     * TIterator Copy Constructor
                                     *
                                     * This class is protected so no one can create a copy of an Iterator.
                                     */
                                TIterator( const TIterator &oSource ) { oSource.HasMore(); }
                                    /**
                                     * TIterator Copy Operator 
                                     *
                                     * This operator is hidden to prevent copying of iterators.
                                     */
            TIterator          &operator =( const TIterator &oSource ) { oSource.HasMore(); return *this; }
    };
} // namespace IASLib

#endif // IASLIB_TITERATOR_H__
