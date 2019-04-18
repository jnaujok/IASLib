/**
 * Entry Template
 *
 *  This template defines a generic entry for use in collections that involve
 * mapping a key to a value. An entry stores a key/value pair. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TENTRY_H__
#define IASLIB_TENTRY_H__

#include "../BaseTypes/Object.h"

namespace IASLib
{
    template<class K, class V>
    class TEntry : public CObject
    {
        protected:
            K   *m_pKey;
            T   *m_pValue;

        public:
            void TEntry( K *pKey, V *pValue )
            {
                m_pKey = pKey;
                m_pValue = pValue;
            }

            virtual K *getKey( void )
            {
                return m_pKey;
            }

            virtual V *getValue( void )
            {
                return m_pValue;
            }

            virtual V *setValue( V *pValue )
            {
                V *retVal = m_pValue;
                m_pValue = pValue;
                return retVal;
            }

            virtual int hashcode( void )
            {
                
            }
    };  
}

#endif // IASLIB_TENTRY_H__

