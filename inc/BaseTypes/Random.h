/**
 * @file Random.h
 * @brief TRandom, the ISAAC and ISAAC-64 pseudo-random number generators.
 *
 * Random Class
 *
 * This class generates pseudo-random numbers with Robert J. Jenkins Jr.'s
 * ISAAC algorithm (or ISAAC-64, for TRandom< 8, uint64_t >). ISAAC has no
 * known practical weakness, but its output is only as unpredictable as its
 * seed: seed it with SeedFromSystem() (the operating system's random
 * source) where that matters, not with the clock.
 *
 * Based on Quinn Tyler Jackson's C++ template port of ISAAC (1998). The
 * results are exactly those of Jenkins' reference code (rand.c, and
 * isaac64.c with ALPHA 8).
 *
 *    The expected period, according to Jenkins, is
 *        f(a,b) = 2**((a+b*(3+2^^a)-1)
 *    (where a is ALPHA and b is bitwidth), so for 32 bits and an ALPHA
 *    of 8, it is 2^^8295.
 *
 * Author: Jeffrey R. Naujok
 * Created: August 26, 2019
 *
 * Copyright (C) 2019, Irene Adler Software. All Rights Reserved.
 */

#ifndef IASLIB_RANDOM_H__
#define IASLIB_RANDOM_H__

#include "Object.h"

#include <stdint.h>
#include <random>

namespace IASLib
{
    /// Implementation details of TRandom.
    namespace RandomDetail
    {
            /// What differs between ISAAC and ISAAC-64.
        template <class T> struct TIsaacTraits;

            /// ISAAC's constants and mixing steps (32 bits).
        template <> struct TIsaacTraits<uint32_t>
        {
                /// The seeding constant.
            static uint32_t GoldenRatio( void ) { return 0x9e3779b9UL; }
                /// One of the four mixing steps (nStep 0-3) of the generator.
            static uint32_t Mix( uint32_t a, int nStep )
            {
                switch ( nStep )
                {
                    case 0:  return a ^ ( a << 13 );
                    case 1:  return a ^ ( a >> 6 );
                    case 2:  return a ^ ( a << 2 );
                    default: return a ^ ( a >> 16 );
                }
            }
                /// The seeding scramble of eight words.
            static void Shuffle( uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, uint32_t &e, uint32_t &f, uint32_t &g, uint32_t &h )
            {
                a ^= b << 11; d += a; b += c;
                b ^= c >> 2;  e += b; c += d;
                c ^= d << 8;  f += c; d += e;
                d ^= e >> 16; g += d; e += f;
                e ^= f << 10; h += e; f += g;
                f ^= g >> 4;  a += f; g += h;
                g ^= h << 8;  b += g; h += a;
                h ^= a >> 9;  c += h; a += b;
            }
        };

            /// ISAAC-64's constants and mixing steps.
        template <> struct TIsaacTraits<uint64_t>
        {
                /// The seeding constant.
            static uint64_t GoldenRatio( void ) { return 0x9e3779b97f4a7c13ULL; }
                /// One of the four mixing steps (nStep 0-3) of the generator.
            static uint64_t Mix( uint64_t a, int nStep )
            {
                switch ( nStep )
                {
                    case 0:  return ~( a ^ ( a << 21 ) );
                    case 1:  return a ^ ( a >> 5 );
                    case 2:  return a ^ ( a << 12 );
                    default: return a ^ ( a >> 33 );
                }
            }
                /// The seeding scramble of eight words.
            static void Shuffle( uint64_t &a, uint64_t &b, uint64_t &c, uint64_t &d, uint64_t &e, uint64_t &f, uint64_t &g, uint64_t &h )
            {
                a -= e; f ^= h >> 9;  h += a;
                b -= f; g ^= a << 9;  a += b;
                c -= g; h ^= b >> 23; b += c;
                d -= h; a ^= c << 15; c += d;
                e -= a; b ^= d >> 14; d += e;
                f -= b; c ^= e << 20; e += f;
                g -= c; d ^= f >> 17; f += g;
                h -= d; e ^= g << 14; g += h;
            }
        };
    }

    /// The word type of the default (32-bit ISAAC) generator.
    typedef uint32_t ISAAC_INT;

        /// A pseudo-random number generator: Jenkins' ISAAC.
        ///
        /// T is uint32_t (ISAAC) or uint64_t (ISAAC-64); ALPHA sets the size
        /// of the state (2^ALPHA words; 8 is the reference). Copies continue
        /// the same sequence independently. Not thread-safe: use one
        /// generator per thread, or lock around it. The class meets the
        /// standard's UniformRandomBitGenerator requirements, so it works
        /// with the &lt;random&gt; distributions.
        ///
        /// @code
        /// TRandom<> rng;
        /// rng.SeedFromSystem();
        /// uint32_t n = rng.rand();
        /// std::uniform_int_distribution<int> die( 1, 6 );
        /// int nRoll = die( rng );
        /// @endcode
        ///
        /// @tparam ALPHA log2 of the number of state words.
        /// @tparam T uint32_t or uint64_t.
    template <int ALPHA = (8), class T = ISAAC_INT>
    class TRandom
    {
        public:
                /// The number of state words (and results per round).
            enum { N = ( 1 << ALPHA ) };

                /// The type of the numbers produced.
            typedef T result_type;

                /// Seeded with a, b, c and nothing else (the same values
                /// always give the same sequence).
            explicit TRandom( T a = 0, T b = 0, T c = 0 )
            {
                srand( a, b, c );
            }
            virtual ~TRandom( void ) {}

                /// The next number, uniform over all values of T.
            T rand( void )
            {
                if ( m_nCount == 0 )
                {
                    isaac();
                    m_nCount = N;
                }
                return m_aResults[ --m_nCount ];
            }

                /// The next number; same as rand().
            T operator ()( void ) { return rand(); }
                /// The smallest number produced: 0.
            static T ( min )( void ) { return 0; }
                /// The largest number produced: every bit set.
            static T ( max )( void ) { return (T)~(T)0; }

                /// Reseeds with a, b, c and the N values of s (zeros if s is
                /// NULL).
            void srand( T a = 0, T b = 0, T c = 0, const T *s = NULL )
            {
                for ( int i = 0; i < N; i++ )
                {
                    m_aResults[i] = ( s != NULL ) ? s[i] : 0;
                }
                m_a = a;
                m_b = b;
                m_c = c;
                randinit( true );
            }

                /// Reseeds from the operating system's random source
                /// (std::random_device).
            void SeedFromSystem( void )
            {
                std::random_device device;
                T aSeed[ N ];
                for ( int i = 0; i < N; i++ )
                {
                    aSeed[i] = 0;
                    for ( size_t nBits = 0; nBits < sizeof( T ) * 8; nBits += 32 )
                    {
                        aSeed[i] = (T)( ( (uint64_t)aSeed[i] << 16 << 16 ) | (uint32_t)device() );
                    }
                }
                srand( 0, 0, 0, aSeed );
            }

        protected:
                /// Initializes the state from the seed in the results array
                /// and m_a, m_b, m_c (bUseSeed), or from the constants alone
                /// (which also zeroes m_a, m_b and m_c), then makes the first
                /// round of results.
            void randinit( bool bUseSeed )
            {
                typedef RandomDetail::TIsaacTraits<T> Traits;
                T a, b, c, d, e, f, g, h;
                a = b = c = d = e = f = g = h = Traits::GoldenRatio();

                if ( ! bUseSeed )
                {
                    m_a = m_b = m_c = 0;
                }

                for ( int i = 0; i < 4; ++i )         // scramble it
                {
                    Traits::Shuffle( a, b, c, d, e, f, g, h );
                }

                    // Two passes over the seed (or, without one, the
                    // scrambled constants alone) so all of it affects all of
                    // the memory.
                for ( int nPass = 0; nPass < ( bUseSeed ? 2 : 1 ); nPass++ )
                {
                    const T *pSource = ( nPass == 0 ) ? m_aResults : m_aMemory;
                    for ( int i = 0; i < N; i += 8 )
                    {
                        if ( bUseSeed )
                        {
                            a += pSource[i  ]; b += pSource[i+1]; c += pSource[i+2]; d += pSource[i+3];
                            e += pSource[i+4]; f += pSource[i+5]; g += pSource[i+6]; h += pSource[i+7];
                        }
                        Traits::Shuffle( a, b, c, d, e, f, g, h );
                        m_aMemory[i  ] = a; m_aMemory[i+1] = b; m_aMemory[i+2] = c; m_aMemory[i+3] = d;
                        m_aMemory[i+4] = e; m_aMemory[i+5] = f; m_aMemory[i+6] = g; m_aMemory[i+7] = h;
                    }
                }

                isaac();            // fill in the first set of results
                m_nCount = N;       // prepare to use the first set of results
            }

                /// Makes the next N results (one round of ISAAC).
            void isaac( void )
            {
                typedef RandomDetail::TIsaacTraits<T> Traits;
                const int SHIFT = ( sizeof( T ) == 8 ) ? 3 : 2;   // log2 of the size of T
                T a = m_a;
                T b = m_b + ( ++m_c );
                for ( int i = 0; i < N; i++ )
                {
                    T x = m_aMemory[i];
                    a = Traits::Mix( a, i % 4 ) + m_aMemory[ ( i + N / 2 ) % N ];
                    T y = m_aMemory[ ( x >> SHIFT ) & ( N - 1 ) ] + a + b;
                    m_aMemory[i] = y;
                    b = m_aMemory[ ( y >> ( ALPHA + SHIFT ) ) & ( N - 1 ) ] + x;
                    m_aResults[i] = b;
                }
                m_a = a;
                m_b = b;
            }

        private:
            T       m_aResults[ N ];    ///< The current round of results (the seed, while seeding).
            T       m_aMemory[ N ];     ///< The generator's internal state.
            T       m_a;                ///< Accumulator.
            T       m_b;                ///< The last result.
            T       m_c;                ///< Counter, incremented each round.
            int     m_nCount;           ///< Results left in m_aResults.

            static_assert( sizeof( T ) == 4 || sizeof( T ) == 8, "TRandom works on 32- or 64-bit unsigned integers" );
            static_assert( (T)-1 > 0, "TRandom works on unsigned integers" );
    };

} // namespace IASLib

#endif // IASLIB_RANDOM_H__
