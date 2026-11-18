/**
 * @file
 * @brief Template collection support.
 *
 *      What the template collections share: the growth policy, the
 * exceptions they throw, and the default comparison and hashing.
 *
 *      Growth: a collection that runs out of room grows by half its size,
 * but never by more than MAX_GROWTH_BYTES at once (and never by less than
 * its scale, SetScale()). So small collections grow geometrically (and
 * appending is cheap), while no collection carries more than about a
 * megabyte of unused room from growing. Reserve() makes room ahead of
 * time, and Compact() gives back what is not used. A collection that
 * shrinks to a quarter of its room gives back half of it.
 *
 *      The template collections are not thread-safe (like the standard
 * library's): lock around a collection that is shared between threads.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TCOMMON_H__
#define IASLIB_TCOMMON_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../Exceptions/CollectionException.h"

#include <stdint.h>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace IASLib
{
    /// Helpers shared by the template collections. Not part of the public interface.
    namespace TemplateDetail
    {
        const size_t MAX_GROWTH_BYTES = 1024 * 1024;   ///< The most a collection grows by at once, in bytes.

            /// The capacity to grow to, to hold at least nNeeded elements.
        /// It grows by half the current capacity, capped at MAX_GROWTH_BYTES and
        /// at least nScale.
        /// @param nCapacity The current capacity.
        /// @param nNeeded The elements that must fit.
        /// @param nScale The least step.
        /// @param nElementSize sizeof the element.
        /// @return The new capacity (nCapacity itself if nNeeded already fits).
        /// @throws std::bad_alloc if nNeeded elements could not be addressed.
        inline size_t GrowCapacity( size_t nCapacity, size_t nNeeded, size_t nScale, size_t nElementSize )
        {
            if ( nNeeded <= nCapacity )
            {
                return nCapacity;
            }

            size_t nMaxStep = MAX_GROWTH_BYTES / ( ( nElementSize > 0 ) ? nElementSize : 1 );
            size_t nStep = nCapacity / 2;
            if ( nStep > nMaxStep )
                nStep = nMaxStep;
            if ( nStep < nScale )
                nStep = nScale;

            size_t nLimit = ( (size_t)-1 ) / ( ( nElementSize > 0 ) ? nElementSize : 1 );
            if ( nCapacity > nLimit - nStep )
            {
                if ( nNeeded > nLimit )
                    throw std::bad_alloc();
                return nNeeded;
            }
            size_t nNew = nCapacity + nStep;
            return ( nNew < nNeeded ) ? nNeeded : nNew;
        }

            /// The capacity to shrink to once only nLength elements are used
            /// (or nCapacity, to keep it). It halves once a quarter or less is used, but not below nScale.
        inline size_t ShrinkCapacity( size_t nCapacity, size_t nLength, size_t nScale )
        {
            if ( nCapacity <= nScale || nLength > nCapacity / 4 )
            {
                return nCapacity;
            }
            size_t nNew = nCapacity / 2;
            return ( nNew < nScale ) ? nScale : nNew;
        }

        /// Throws a CCollectionException with message pszWhat.
        /// @throws CCollectionException always.
        inline void ThrowOutOfRange( const char *pszWhat )
        {
            throw CCollectionException( pszWhat, CException::NORMAL );
        }

            /// A 64-bit mix (the SplitMix64 finaliser), so hashes of nearby
            /// integers spread over the whole table.
        inline size_t MixHash( uint64_t nValue )
        {
            nValue ^= nValue >> 30;
            nValue *= 0xbf58476d1ce4e5b9ULL;
            nValue ^= nValue >> 27;
            nValue *= 0x94d049bb133111ebULL;
            nValue ^= nValue >> 31;
            return (size_t)nValue;
        }

            /// FNV-1a over bytes, then MixHash().
        inline size_t HashBytes( const void *pData, size_t nLength )
        {
            const unsigned char *p = (const unsigned char *)pData;
            uint64_t nHash = 14695981039346656037ULL;
            for ( size_t n = 0; n < nLength; n++ )
            {
                nHash ^= p[n];
                nHash *= 1099511628211ULL;
            }
            return MixHash( nHash );
        }

            /// Deletes what a pointer collection holds (DeleteAll()), and sets the pointer to NULL.
        template <class T> inline void DeletePointer( T *&p )
        {
            delete p;
            p = NULL;
        }
    }

        /// The default ordering: operator <.
    template <class T> struct TLess
    {
        bool operator ()( const T &a, const T &b ) const { return a < b; }   ///< True if a goes before b.
    };

        /// The default equality: operator ==.
    template <class T> struct TEqualTo
    {
        bool operator ()( const T &a, const T &b ) const { return a == b; }   ///< True if a equals b.
    };

        /// The default hash: integers, enums and pointers are mixed, strings
        /// (CString, std::string) are hashed by their text, and anything else
        /// by its hashCode() (as library objects have).
    template <class T, class Enable = void> struct THashOf
    {
        size_t operator ()( const T &value ) const { return TemplateDetail::MixHash( (uint64_t)(unsigned int)value.hashCode() ); }   ///< The mixed hashCode() of value.
    };

    /// THashOf for integers and enums: the mixed value.
    template <class T> struct THashOf<T, typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
    {
        size_t operator ()( T value ) const { return TemplateDetail::MixHash( (uint64_t)value ); }   ///< The mixed value.
    };

    /// THashOf for pointers: the mixed address.
    template <class T> struct THashOf<T *, void>
    {
        size_t operator ()( const T *value ) const { return TemplateDetail::MixHash( (uint64_t)(uintptr_t)value ); }   ///< The mixed address.
    };

    /// THashOf for CString: a hash of its bytes.
    template <> struct THashOf<CString, void>
    {
        size_t operator ()( const CString &value ) const { return TemplateDetail::HashBytes( value.c_str(), value.GetLength() ); }   ///< A hash of the string's bytes.
    };

    /// THashOf for std::string: a hash of its bytes.
    template <> struct THashOf<std::string, void>
    {
        size_t operator ()( const std::string &value ) const { return TemplateDetail::HashBytes( value.data(), value.size() ); }   ///< A hash of the string's bytes.
    };
} // namespace IASLib

#endif // IASLIB_TCOMMON_H__
