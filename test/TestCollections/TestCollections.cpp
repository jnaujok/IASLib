/***********************************************************************
**  TestCollections
**
**  Tests for the template collections (TArray, TSortedArray,
**  TSparseArray, TList, TStack, TQueue, THash, TCache), checked against
**  the standard library on random operations, with an element type that
**  counts its constructions and destructions (so leaks and double
**  destruction show), and for the classic collections built on them
**  (CArray, CIntArray, CStringArray, CSortedArray, CSparseArray, CHash,
**  CList, CStack, CQueue, CCache).
***********************************************************************/

#include "IASLib.h"

#include <algorithm>
#include <cstdio>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
        } \
    } while ( 0 )

#define CHECK_THROWS( expression ) \
    do { \
        g_nChecks++; \
        bool _bThrew = false; \
        try { expression; } catch ( CCollectionException & ) { _bThrew = true; } \
        if ( ! _bThrew ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s did not throw\n", __FILE__, __LINE__, #expression ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

    // Counts live instances, so every collection can be checked to destroy
    // exactly what it made.
struct CCounted
{
    static int s_nLive;
    int         nValue;
    bool        bMovedFrom;

    CCounted( int n = 0 ) : nValue( n ), bMovedFrom( false ) { s_nLive++; }
    CCounted( const CCounted &o ) : nValue( o.nValue ), bMovedFrom( false ) { s_nLive++; }
    CCounted( CCounted &&o ) noexcept : nValue( o.nValue ), bMovedFrom( false ) { o.bMovedFrom = true; s_nLive++; }
    ~CCounted( void ) { s_nLive--; }
    CCounted &operator =( const CCounted &o ) { nValue = o.nValue; bMovedFrom = false; return *this; }
    CCounted &operator =( CCounted &&o ) noexcept { nValue = o.nValue; bMovedFrom = false; o.bMovedFrom = true; return *this; }
    bool operator ==( const CCounted &o ) const { return nValue == o.nValue; }
    bool operator <( const CCounted &o ) const { return nValue < o.nValue; }
    int hashCode( void ) const { return nValue; }
};
int CCounted::s_nLive = 0;

    // A library object that counts its deletions (for DeleteAll()).
class CTracked : public CObject
{
    public:
        static int s_nLive;
        int         m_nValue;
        explicit CTracked( int n ) : m_nValue( n ) { s_nLive++; }
        virtual ~CTracked( void ) { s_nLive--; }
#ifdef IASLIB_RTTI__
        virtual const char *GetType( void ) const { return "CTracked"; }
#endif
};
int CTracked::s_nLive = 0;

/***********************************************************************
**  Templates
***********************************************************************/

static void testTArray( void )
{
    section( "TArray" );
    {
        TArray<int> a;
        CHECK( a.IsEmpty() && a.Length() == 0 );
        for ( int n = 0; n < 1000; n++ )
            CHECK( a.Push( n ) == (size_t)n );
        CHECK( a.Length() == 1000 && a[0] == 0 && a[999] == 999 );
        CHECK( a.Prepend( -1 ) == 0 && a[0] == -1 && a[1] == 0 );
        CHECK( a.Insert( 500, 42 ) == 500 && a[500] == 42 );
        size_t nAt = a.Insert( 99999, 7 );
        CHECK( nAt == a.Length() - 1 && a.Last() == 7 );
        CHECK( a.Remove( 500 ) == 42 );
        CHECK( a.Get( 99999, -5 ) == -5 );
        CHECK( a.Find( 10 ) == 11 );
        CHECK( a.Find( 12345 ) == NOT_FOUND );
        CHECK( a.Contains( 999 ) );
        CHECK( a.RemoveValue( 999 ) && ! a.Contains( 999 ) );
        CHECK( a.Set( 0, 100 ) == 0 && a[0] == 100 );
        nAt = a.Set( 99999, 5 );                                  // past the end: appended
        CHECK( nAt == a.Length() - 1 );
        CHECK_THROWS( a[ a.Length() ] );
        CHECK_THROWS( a.Remove( a.Length() ) );
        CHECK_THROWS( a.Swap( 0, a.Length() ) );

        a.Sort();
        CHECK( std::is_sorted( a.begin(), a.end() ) );
        a.Sort( []( int x, int y ) { return x > y; } );
        CHECK( a[0] >= a[1] && a[1] >= a[2] );
        a.Reverse();
        CHECK( std::is_sorted( a.begin(), a.end() ) );

        int nSum = 0;
        for ( int n : a )                                         // range-based for
            nSum += n;
        CHECK( nSum != 0 );

        TArray<int> b( a );
        CHECK( b == a );
        b.Push( 1 );
        CHECK( b != a );
        TArray<int> c = std::move( b );
        CHECK( b.IsEmpty() && c.Length() == a.Length() + 1 );
        b = a;
        CHECK( b == a );

            // Growing by at most a megabyte: a big array has bounded slack.
        TArray<char> big;
        for ( int n = 0; n < 5 * 1024 * 1024; n++ )
            big.Push( 'x' );
        CHECK( big.Capacity() - big.Length() <= 1024 * 1024 );
        big.Compact();
        CHECK( big.Capacity() == big.Length() );
        big.Clear();
        CHECK( big.IsEmpty() );

        TArray<int> r;
        r.Reserve( 100 );
        CHECK( r.Capacity() >= 100 );
        TArray<int> init = { 3, 1, 2 };
        CHECK( init.Length() == 3 && init[0] == 3 );
        CHECK( init.Pop() == 2 && init.Length() == 2 );

            // Inserting one of its own elements while it grows.
        TArray<int> self;
        self.Push( 7 );
        for ( int n = 0; n < 100; n++ )
            self.Push( self[0] );
        CHECK( self.Length() == 101 && self[100] == 7 );

            // Removing gives room back once it is mostly empty.
        TArray<int> shrink( 1 );
        for ( int n = 0; n < 1000; n++ )
            shrink.Push( n );
        size_t nFull = shrink.Capacity();
        while ( shrink.Length() > 10 )
            shrink.Pop();
        CHECK( shrink.Capacity() < nFull / 4 );
    }

    {
        CCounted::s_nLive = 0;
        {
            TArray<CCounted> a;
            for ( int n = 0; n < 100; n++ )
                a.Push( CCounted( n ) );
            for ( int n = 0; n < 50; n++ )
                a.Insert( n * 2, CCounted( -n ) );
            for ( int n = 0; n < 30; n++ )
                a.Remove( n );
            CHECK( CCounted::s_nLive == (int)a.Length() );
            TArray<CCounted> b = a;
            CHECK( CCounted::s_nLive == 2 * (int)a.Length() );
            a = b;
            a.Insert( 0, a[5] );                                  // its own element
            CHECK( a[0] == b[5] );
        }
        CHECK( CCounted::s_nLive == 0 );                          // nothing leaked or freed twice
    }

    {
        TArray< std::unique_ptr<int> > a;                         // move-only elements
        for ( int n = 0; n < 50; n++ )
            a.Push( std::unique_ptr<int>( new int( n ) ) );
        a.Insert( 0, std::unique_ptr<int>( new int( -1 ) ) );
        CHECK( *a[0] == -1 && *a[50] == 49 );
        std::unique_ptr<int> p = a.Remove( 0 );
        CHECK( *p == -1 && a.Length() == 50 );
    }

    {
        CTracked::s_nLive = 0;
        TArray<CTracked *> a;
        for ( int n = 0; n < 10; n++ )
            a.Push( new CTracked( n ) );
        a.DeleteAll();
        CHECK( a.IsEmpty() && CTracked::s_nLive == 0 );
    }

        // Against std::vector, on random operations.
    {
        std::mt19937 rng( 1 );
        TArray<int> a;
        std::vector<int> v;
        bool bSame = true;
        for ( int n = 0; n < 20000 && bSame; n++ )
        {
            int nOp = rng() % 4;
            if ( nOp < 2 || v.empty() )
            {
                size_t nAt = v.empty() ? 0 : rng() % ( v.size() + 1 );
                int nValue = (int)( rng() % 1000 );
                a.Insert( nAt, nValue );
                v.insert( v.begin() + nAt, nValue );
            }
            else
            {
                size_t nAt = rng() % v.size();
                a.Remove( nAt );
                v.erase( v.begin() + nAt );
            }
            bSame = a.Length() == v.size() && std::equal( v.begin(), v.end(), a.begin() );
        }
        CHECK( bSame );
    }
}

static void testTSortedArray( void )
{
    section( "TSortedArray" );

    TSortedArray<int> s;
    std::mt19937 rng( 2 );
    std::multiset<int> ref;
    for ( int n = 0; n < 2000; n++ )
    {
        int nValue = (int)( rng() % 500 );
        s.Push( nValue );
        ref.insert( nValue );
    }
    CHECK( s.Length() == ref.size() );
    CHECK( std::equal( ref.begin(), ref.end(), s.begin() ) );
    CHECK( std::is_sorted( s.begin(), s.end() ) );
    CHECK( s.Find( *ref.begin() ) == 0 );
    CHECK( s.Find( 1000 ) == NOT_FOUND );
    CHECK( s.Contains( *ref.rbegin() ) );

    size_t nBefore = s.Length();
    size_t nAt = s.PushUnique( s[10] );
    CHECK( s.Length() == nBefore && s[nAt] == s[10] );
    size_t nNew = s.PushUnique( 100000 );
    CHECK( nNew == s.Length() - 1 );
    CHECK( s.RemoveValue( 100000 ) && ! s.Contains( 100000 ) );

        // Equal values keep the order they came in.
    struct CByKey
    {
        bool operator ()( const std::pair<int, int> &a, const std::pair<int, int> &b ) const { return a.first < b.first; }
    };
    TSortedArray< std::pair<int, int>, CByKey > stable;
    stable.Push( std::make_pair( 1, 0 ) );
    stable.Push( std::make_pair( 0, 1 ) );
    stable.Push( std::make_pair( 1, 2 ) );
    stable.Push( std::make_pair( 1, 3 ) );
    CHECK( stable[0].second == 1 && stable[1].second == 0 && stable[2].second == 2 && stable[3].second == 3 );

        // A descending order from a comparison object.
    TSortedArray< int, std::greater<int> > down;
    for ( int n = 0; n < 10; n++ )
        down.Push( n );
    CHECK( down[0] == 9 && down[9] == 0 && down.Find( 3 ) == 6 );
}

static void testTSparseArray( void )
{
    section( "TSparseArray" );

    TSparseArray<CString> s;
    s.Set( 5000000, "far" );
    s.Set( 5, "five" );
    s.Set( 50, "fifty" );
    CHECK( s.Length() == 3 );
    CHECK( s.Exists( 5 ) && ! s.Exists( 6 ) );
    CHECK( s.Get( 50 ) == "fifty" );
    CHECK( s.Get( 51, "none" ) == "none" );
    CHECK( s.Find( 51 ) == NULL );
    CHECK( *s.Find( 5000000 ) == "far" );
    s.Set( 5, "FIVE" );
    CHECK( s.Length() == 3 && s.Get( 5 ) == "FIVE" );
    s[7] = "seven";
    CHECK( s.Length() == 4 && s.Get( 7 ) == "seven" );
    CHECK( s.FirstIndex() == 5 && s.LastIndex() == 5000000 );

    std::vector<size_t> aOrder;
    for ( const TSparseArray<CString>::Entry &e : s )
        aOrder.push_back( e.Index );
    CHECK( aOrder.size() == 4 && aOrder[0] == 5 && aOrder[1] == 7 && aOrder[2] == 50 && aOrder[3] == 5000000 );

    CHECK( s.Remove( 7 ) && ! s.Remove( 7 ) );
    CHECK( s.Length() == 3 );

    std::mt19937 rng( 3 );
    TSparseArray<int> r;
    std::map<size_t, int> ref;
    for ( int n = 0; n < 5000; n++ )
    {
        size_t nIndex = rng() % 2000;
        if ( rng() % 3 == 0 )
        {
            CHECK( r.Remove( nIndex ) == ( ref.erase( nIndex ) > 0 ) );
        }
        else
        {
            r.Set( nIndex, n );
            ref[ nIndex ] = n;
        }
    }
    bool bSame = r.Length() == ref.size();
    size_t nPos = 0;
    for ( std::map<size_t, int>::const_iterator it = ref.begin(); bSame && it != ref.end(); ++it, nPos++ )
        bSame = r.GetEntry( nPos ).Index == it->first && r.GetEntry( nPos ).Value == it->second;
    CHECK( bSame );
}

static void testTList( void )
{
    section( "TList" );

    TList<int> l;
    CHECK( l.IsEmpty() );
    CHECK_THROWS( l.RemoveFirst() );
    CHECK_THROWS( l.GetLast() );
    l.AddLast( 2 );
    l.AddFirst( 1 );
    l.AddLast( 4 );
    CHECK( l.AddAt( 2, 3 ) == 2 );
    CHECK( l.Length() == 4 && l.GetFirst() == 1 && l.GetLast() == 4 && l.GetAt( 2 ) == 3 );
    CHECK( l.AddBefore( 1, 0 ) && l.GetFirst() == 0 );
    CHECK( l.AddAfter( 4, 5 ) && l.GetLast() == 5 );
    CHECK( ! l.AddAfter( 99, 6 ) );
    CHECK( l.Find( 3 ) == 3 && l.Find( 99 ) == NOT_FOUND );

    std::vector<int> v( l.begin(), l.end() );
    CHECK( v.size() == 6 && v[0] == 0 && v[5] == 5 );
    TList<int>::iterator it = l.end();
    --it;
    CHECK( *it == 5 );                                        // back from end()
    std::vector<int> back;
    for ( TList<int>::iterator r = l.end(); r != l.begin(); )
        back.push_back( *--r );
    CHECK( back.size() == 6 && back[0] == 5 && back[5] == 0 );

    CHECK( l.RemoveFirst() == 0 && l.RemoveLast() == 5 );
    CHECK( l.RemoveAt( 1 ) == 2 );
    CHECK( l.RemoveValue( 3 ) && ! l.RemoveValue( 3 ) );
    CHECK( l.Length() == 2 && l.GetFirst() == 1 && l.GetLast() == 4 );

        // Removing while iterating, and inserting at an iterator.
    TList<int> e = { 1, 2, 3, 4, 5, 6 };
    for ( TList<int>::iterator i = e.begin(); i != e.end(); )
    {
        if ( *i % 2 == 0 )
            i = e.Remove( i );
        else
            ++i;
    }
    CHECK( e.Length() == 3 && e.GetAt( 1 ) == 3 );
    e.Insert( e.begin(), 0 );
    e.Insert( e.end(), 9 );
    CHECK( e.GetFirst() == 0 && e.GetLast() == 9 );

    TList<int> copy( e );
    CHECK( copy == e );
    copy.AddLast( 1 );
    CHECK( copy != e );

    CCounted::s_nLive = 0;
    {
        TList<CCounted> c;
        std::list<int> ref;
        std::mt19937 rng( 4 );
        bool bSame = true;
        for ( int n = 0; n < 5000 && bSame; n++ )
        {
            int nOp = rng() % 5;
            if ( nOp == 0 ) { c.AddFirst( CCounted( n ) ); ref.push_front( n ); }
            else if ( nOp == 1 ) { c.AddLast( CCounted( n ) ); ref.push_back( n ); }
            else if ( nOp == 2 && ! ref.empty() ) { c.RemoveFirst(); ref.pop_front(); }
            else if ( nOp == 3 && ! ref.empty() ) { c.RemoveLast(); ref.pop_back(); }
            else if ( ! ref.empty() )
            {
                size_t nAt = rng() % ref.size();
                c.RemoveAt( nAt );
                std::list<int>::iterator r = ref.begin();
                std::advance( r, nAt );
                ref.erase( r );
            }
            bSame = c.Length() == ref.size() && ( ref.empty() || ( c.GetFirst().nValue == ref.front() && c.GetLast().nValue == ref.back() ) );
        }
        CHECK( bSame );
        CHECK( CCounted::s_nLive == (int)c.Length() );
    }
    CHECK( CCounted::s_nLive == 0 );
}

static void testTStackQueue( void )
{
    section( "TStack and TQueue" );

    TStack<CString> s;
    CHECK_THROWS( s.Pop() );
    CHECK_THROWS( s.Peek() );
    CHECK( s.Push( "a" ) == 1 && s.Push( "b" ) == 2 && s.Push( "c" ) == 3 );
    CHECK( s.Peek() == "c" && s.Get( 0 ) == "c" && s.Get( 2 ) == "a" && s.Get( 3, "x" ) == "x" );
    CHECK( s.Pop() == "c" && s.Pop() == "b" && s.Length() == 1 );

    TQueue<int> q( 4 );
    CHECK_THROWS( q.Pop() );
    std::deque<int> ref;
    std::mt19937 rng( 5 );
    bool bSame = true;
    for ( int n = 0; n < 20000 && bSame; n++ )
    {
        if ( rng() % 3 != 0 || ref.empty() )
        {
            q.Push( n );
            ref.push_back( n );
        }
        else
        {
            bSame = q.Pop() == ref.front();
            ref.pop_front();
        }
        bSame = bSame && q.Length() == ref.size() && ( ref.empty() || q.Peek() == ref.front() );
    }
    CHECK( bSame );
    CHECK( std::equal( ref.begin(), ref.end(), q.begin() ) );
    TQueue<int> qc( q );
    CHECK( std::equal( q.begin(), q.end(), qc.begin() ) && qc.Length() == q.Length() );

        // Wrapping around the ring and growing while wrapped.
    TQueue<CCounted> w( 2 );
    CCounted::s_nLive = 0;
    {
        TQueue<CCounted> wr( 2 );
        for ( int n = 0; n < 3; n++ )
            wr.Push( CCounted( n ) );
        wr.Pop();
        wr.Pop();
        for ( int n = 3; n < 20; n++ )
            wr.Push( CCounted( n ) );
        CHECK( wr.Length() == 18 && wr.Peek().nValue == 2 && wr.Get( 17 ).nValue == 19 );
        CHECK( CCounted::s_nLive == 18 );
    }
    CHECK( CCounted::s_nLive == 0 );
}

static void testTHash( void )
{
    section( "THash" );

    THash<CString, int> h;
    CHECK( h.IsEmpty() && h.Find( "x" ) == NULL );
    CHECK( h.Put( "one", 1 ) );
    CHECK( h.Put( "two", 2 ) );
    CHECK( ! h.Put( "one", 11 ) );                            // replaced
    CHECK( h.Length() == 2 && h.Get( "one" ) == 11 );
    CHECK( h.Get( "three", -1 ) == -1 );
    int nOut = 0;
    CHECK( h.Get( "two", nOut ) && nOut == 2 );
    CHECK( h.HasKey( "two" ) && ! h.HasKey( "Two" ) );
    h["three"] = 3;
    h["one"] += 100;
    CHECK( h.Get( "three" ) == 3 && h.Get( "one" ) == 111 );

        // Iteration in the order the keys were added.
    TArray<CString> aKeys = h.Keys();
    CHECK( aKeys.Length() == 3 && aKeys[0] == "one" && aKeys[1] == "two" && aKeys[2] == "three" );
    CHECK( h.Values()[2] == 3 );
    CHECK( h.Remove( "two", nOut ) && nOut == 2 && ! h.Remove( "two" ) );
    std::vector<std::string> aOrder;
    for ( const THash<CString, int>::Entry &e : h )
        aOrder.push_back( (const char *)e.Key );
    CHECK( aOrder.size() == 2 && aOrder[0] == "one" && aOrder[1] == "three" );

    THash<CString, int> copy( h );
    CHECK( copy.Length() == 2 && copy.Get( "three" ) == 3 );
    copy.Put( "four", 4 );
    CHECK( ! h.HasKey( "four" ) );

        // Integer keys, against std::unordered_map, through growth.
    THash<int, int> n;
    std::unordered_map<int, int> ref;
    std::mt19937 rng( 6 );
    for ( int i = 0; i < 50000; i++ )
    {
        int nKey = (int)( rng() % 20000 ) - 10000;
        if ( rng() % 4 == 0 )
        {
            CHECK( n.Remove( nKey ) == ( ref.erase( nKey ) > 0 ) );
        }
        else
        {
            n.Put( nKey, i );
            ref[ nKey ] = i;
        }
    }
    bool bSame = n.Length() == ref.size();
    for ( std::unordered_map<int, int>::const_iterator it = ref.begin(); bSame && it != ref.end(); ++it )
        bSame = n.Get( it->first, -1 ) == it->second;
    CHECK( bSame );
    CHECK( n.BucketCount() >= n.Length() );
    size_t nBuckets = n.BucketCount();
    for ( std::unordered_map<int, int>::const_iterator it = ref.begin(); it != ref.end(); ++it )
        n.Remove( it->first );
    CHECK( n.IsEmpty() );
    n.Compact();
    CHECK( n.BucketCount() < nBuckets );

        // Keys with their own hashCode(), and pointer values deleted.
    THash<CCounted, CTracked *> owned;
    CTracked::s_nLive = 0;
    for ( int i = 0; i < 100; i++ )
        owned.Put( CCounted( i ), new CTracked( i ) );
    CHECK( owned.Get( CCounted( 42 ) )->m_nValue == 42 );
    owned.DeleteAll();
    CHECK( CTracked::s_nLive == 0 );

    THash<std::string, int> s;
    s.Put( "std", 1 );
    CHECK( s.Get( "std" ) == 1 );
}

static void testTCache( void )
{
    section( "TCache" );

    TCache<int, CString> c( 3 );
    c.Put( 1, "one" );
    c.Put( 2, "two" );
    c.Put( 3, "three" );
    CHECK( c.Length() == 3 );
    CHECK( c.Get( 1 ) != NULL && *c.Get( 1 ) == "one" );     // 1 is now the most recent
    c.Put( 4, "four" );                                       // drops 2, the least recent
    CHECK( c.Length() == 3 && c.Peek( 2 ) == NULL && c.Peek( 1 ) != NULL );
    CHECK( c.Peek( 3 ) != NULL );
    c.Put( 5, "five" );                                       // Peek() is not a use: drops 3
    CHECK( c.Peek( 3 ) == NULL );
    CHECK( c.GetHits() == 2 && c.GetHitCount( 1 ) == 2 );
    CHECK( c.Get( 99 ) == NULL && c.GetMisses() == 1 );

    std::vector<int> aOrder;
    for ( TCache<int, CString>::const_iterator it = c.begin(); it != c.end(); ++it )
        aOrder.push_back( it->Key );
    CHECK( aOrder.size() == 3 && aOrder[0] == 1 && aOrder[2] == 5 );   // least recent first

        // Expiry, on a clock the test controls.
    int64_t llNow = 1000;
    TCache<CString, int> e( 10 );
    e.SetClock( [&llNow]() { return llNow; } );
    std::vector<std::string> aEvicted;
    e.SetEvictionHandler( [&aEvicted]( const CString &key, int & ) { aEvicted.push_back( (const char *)key ); } );
    e.Put( "short", 1, 100 );
    e.Put( "long", 2, 10000 );
    e.Put( "forever", 3 );
    llNow += 500;
    CHECK( e.Get( "short" ) == NULL );                        // expired
    CHECK( e.Get( "long" ) != NULL && e.Get( "forever" ) != NULL );
    llNow += 100000;
    CHECK( e.ClearExpired() == 1 );
    CHECK( e.Length() == 1 && e.Contains( "forever" ) );
    CHECK( aEvicted.size() == 2 && aEvicted[0] == "short" && aEvicted[1] == "long" );

        // A cache of pointers deletes what it drops.
    CTracked::s_nLive = 0;
    {
        TCache<int, CTracked *> p( 5 );
        p.SetEvictionHandler( []( const int &, CTracked *&pValue ) { delete pValue; } );
        for ( int i = 0; i < 20; i++ )
            p.Put( i, new CTracked( i ) );
        CHECK( CTracked::s_nLive == 5 );
        p.SetMaxEntries( 2 );
        CHECK( CTracked::s_nLive == 2 && p.Length() == 2 );
        CTracked *pTaken = NULL;
        CHECK( p.Remove( 19, pTaken ) && pTaken->m_nValue == 19 );
        delete pTaken;
        CTracked *pLast = *p.Peek( 18 );
        p.Clear();
        delete pLast;
    }
    CHECK( CTracked::s_nLive == 0 );
}

/***********************************************************************
**  main
***********************************************************************/

void testClassicCollections( void );

int main( void )
{
    testTArray();
    testTSortedArray();
    testTSparseArray();
    testTList();
    testTStackQueue();
    testTHash();
    testTCache();
    testClassicCollections();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}

static int compareTracked( const CObject *a, const CObject *b )
{
    int na = ((const CTracked *)a)->m_nValue, nb = ((const CTracked *)b)->m_nValue;
    return ( na < nb ) ? -1 : ( na > nb ) ? 1 : 0;
}

static int compareTrackedDescending( const CObject *a, const CObject *b, void *pData )
{
    (*(int *)pData)++;
    return -compareTracked( a, b );
}

static int trackedValue( CObject *p )
{
    return ( p != NULL ) ? ((CTracked *)p)->m_nValue : -999;
}

class CTestItem : public CCacheItem
{
    public:
        CString m_strKey;
        explicit CTestItem( const char *pszKey ) : m_strKey( pszKey ) { CTracked::s_nLive++; }
        virtual ~CTestItem( void ) { CTracked::s_nLive--; }
        virtual CCacheItem *GetItem( void ) { return this; }
        virtual CString GetKey( void ) const { return m_strKey; }
        virtual int Compare( const CCacheItem *p ) const { return m_strKey.Compare( p->GetKey() ); }
#ifdef IASLIB_RTTI__
        virtual const char *GetType( void ) const { return "CTestItem"; }
#endif
};

void testClassicCollections( void )
{
    section( "CArray" );
    CTracked::s_nLive = 0;
    {
        CArray a;
        CHECK( a.IsEmpty() && a.Get( 0 ) == NULL && a[5] == NULL );
        for ( int n = 0; n < 1000; n++ )
            a.Push( new CTracked( n ) );
        CHECK( a.GetLength() == 1000 && a.Length() == 1000 && a.Count() == 1000 );
        CHECK( trackedValue( a[999] ) == 999 );
        CHECK( a.Insert( 0, new CTracked( -1 ) ) == 0 && trackedValue( a[0] ) == -1 );

            // Set() past the end appends (it used to also write past the
            // end of the buffer).
        CTracked *pExtra = new CTracked( 5000 );
        size_t nSetAt = a.Set( 99999, pExtra );
        CHECK( nSetAt == a.GetLength() - 1 && a.Find( pExtra ) == a.GetLength() - 1 );
        CTracked *pOld = (CTracked *)a.Get( 1 );
        CTracked *pNew = new CTracked( 7777 );
        CHECK( a.Set( 1, pNew ) == 1 && a.Get( 1 ) == pNew );
        delete pOld;                          // Set() leaves the old one to the caller

        CObject *pRemoved = a.Remove( 0 );
        CHECK( trackedValue( pRemoved ) == -1 );
        delete pRemoved;
        CHECK( a.Remove( 999999 ) == NULL );
        CHECK( a.Delete( 0 ) && ! a.Delete( 999999 ) );

        int nCompares = 0;
        a.Sort( compareTrackedDescending, &nCompares );
        CHECK( nCompares > 0 );
        bool bSorted = true;
        for ( size_t n = 1; n < a.GetLength(); n++ )
            if ( trackedValue( a[n - 1] ) < trackedValue( a[n] ) )
                bSorted = false;
        CHECK( bSorted );

        CArray empty;
        empty.Sort( compareTrackedDescending, &nCompares );   // used to crash
        CHECK( empty.IsEmpty() );

        CHECK( a.Swap( 0, 1 ) && ! a.Swap( 0, 0 ) && ! a.Swap( 0, 999999 ) );

        CIterator *pIt = a.Enumerate();
        size_t nSeen = 0;
        while ( pIt->HasMore() )
        {
            pIt->Next();
            nSeen++;
        }
        CHECK( nSeen == a.GetLength() );
        CHECK( pIt->Prev() == a[ a.GetLength() - 1 ] );
        delete pIt;

        CHECK( CTracked::s_nLive == (int)a.GetLength() );
    }
    CHECK( CTracked::s_nLive == 0 );                          // the destructor deleted them

    {
        CArray a;
        CTracked oKept( 1 );
        a.Push( &oKept );
        a.EmptyAll();                                         // lets go without deleting
        CHECK( a.IsEmpty() && CTracked::s_nLive == 1 );
    }

    section( "CIntArray and CStringArray" );
    {
        CIntArray i;
        for ( int n = 0; n < 100; n++ )
            i.Push( 100 - n );
        CHECK( i.GetLength() == 100 && i.Get( 0 ) == 100 && i.Get( 1000 ) == 0 );
        i[0] = 5;
        CHECK( i.Get( 0 ) == 5 );
        i[1000] = 6;                                          // a scratch value, not the array
        CHECK( i.GetLength() == 100 );
        CHECK( i.Insert( 0, 42 ) == 0 && i.Find( 42 ) == 0 );
        CHECK( i.Delete( 0 ) && ! i.Delete( 1000 ) );
        i.Sort();
        CHECK( i.Get( 0 ) == 1 && i.Get( 99 ) == 99 );
        CIntArray copy( i );                                  // copies are independent (used to free twice)
        copy.Push( 1 );
        CHECK( copy.GetLength() == 101 && i.GetLength() == 100 );
        CIterator *pIt = i.Enumerate();
        int nSum = 0;
        while ( pIt->HasMore() )
            nSum += pIt->NextInt();
        delete pIt;
        CHECK( nSum == 5050 - 100 + 5 );
    }
    {
        CStringArray s;
        s.Push( "pear" );
        s.Push( "apple" );
        s.Append( "fig" );
        s.Put( "kiwi" );
        CHECK( s.GetLength() == 4 && s.Get( 0 ) == "kiwi" );
        CHECK( s.Find( "FIG", false ) == 3 && s.Find( "plum" ) == NOT_FOUND && s.Contains( "pear" ) );
        s.Sort( true );
        CHECK( s.Join( "," ) == "apple,fig,kiwi,pear" );
        s.Sort( false );
        CHECK( s.Join( "," ) == "pear,kiwi,fig,apple" );
        s[0] = "PEAR";
        CHECK( s.Get( 0 ) == "PEAR" );
        const CStringArray &cs = s;
        CHECK( cs[99] == "" );
        CStringArray copy = s;
        copy[0] = "changed";
        CHECK( s.Get( 0 ) == "PEAR" );
        CHECK( s.Delete( 0 ) && s.GetLength() == 3 );
        CStringArray empty;
        CHECK( empty.Sort( true ) );                          // used to crash
        CIterator *pIt = s.Enumerate();
        CString strAll;
        while ( pIt->HasMore() )
            strAll += pIt->NextString();
        delete pIt;
        CHECK( strAll == "kiwifigapple" );
        s.EmptyAll();
        CHECK( s.IsEmpty() );
    }

    section( "CSortedArray and CSparseArray" );
    CTracked::s_nLive = 0;
    {
        CSortedArray s( compareTracked );
        std::mt19937 rng( 7 );
        std::multiset<int> ref;
        for ( int n = 0; n < 500; n++ )
        {
            int nValue = (int)( rng() % 100 );
            s.Push( new CTracked( nValue ) );                 // used to append unsorted, or drop
            ref.insert( nValue );
        }
        CHECK( s.GetLength() == 500 );
        bool bSame = true;
        size_t nPos = 0;
        for ( std::multiset<int>::const_iterator it = ref.begin(); it != ref.end(); ++it, nPos++ )
            if ( trackedValue( s[nPos] ) != *it )
                bSame = false;
        CHECK( bSame );
        CTracked oKey( 50 );
        size_t nFound = s.Find( &oKey );
        CHECK( nFound != NOT_FOUND && trackedValue( s[nFound] ) == 50 && ( nFound == 0 || trackedValue( s[nFound - 1] ) < 50 ) );
        CTracked oMissing( 1000 );
        CHECK( s.Find( &oMissing ) == NOT_FOUND );
        s.Insert( 0, new CTracked( 1000 ) );                  // still goes in order
        CHECK( trackedValue( s[ s.GetLength() - 1 ] ) == 1000 );
        CHECK( ! s.Swap( 0, 1 ) );
        CHECK( CTracked::s_nLive == 501 + 2 );
    }
    CHECK( CTracked::s_nLive == 0 );
    {
        CSparseArray s;
        s.Insert( 10, new CTracked( 10 ) );
        s.Insert( 1000000, new CTracked( 1000000 ) );
        s.Insert( 5, new CTracked( 5 ) );
        CHECK( s.GetLength() == 3 && s.Exists( 10 ) && ! s.Exists( 11 ) );
        CHECK( trackedValue( s.Get( 1000000 ) ) == 1000000 && s.Get( 6 ) == NULL );
        s.Insert( 10, new CTracked( 11 ) );                   // replaces (and deletes) the old one
        CHECK( trackedValue( s[10] ) == 11 && CTracked::s_nLive == 3 );
        CHECK( s.Swap( 5, 6 ) && s.Get( 5 ) == NULL && trackedValue( s.Get( 6 ) ) == 5 );
        bool bThrew = false;
        try { s.Push( NULL ); } catch ( CCollectionException & ) { bThrew = true; }
        CHECK( bThrew );
        CSparseArrayIterator *pIt = (CSparseArrayIterator *)s.Enumerate();
        std::vector<size_t> aIndexes;
        while ( pIt->HasMore() )
        {
            pIt->Next();
            aIndexes.push_back( pIt->GetSparseNumber() );
        }
        delete pIt;
        CHECK( aIndexes.size() == 3 && aIndexes[0] == 6 && aIndexes[1] == 10 && aIndexes[2] == 1000000 );
        CHECK( s.Delete( 6 ) && CTracked::s_nLive == 2 );
    }
    CHECK( CTracked::s_nLive == 0 );

    section( "CHash" );
    {
        CHash h;
        h.Push( "alpha", new CTracked( 1 ) );
        h.Push( " beta ", new CTracked( 2 ) );                // white space around keys is ignored
        h.Push( 42, new CTracked( 42 ) );
        CHECK( h.GetLength() == 3 );
        CHECK( trackedValue( h.Get( "beta" ) ) == 2 && trackedValue( h.Get( "\tbeta" ) ) == 2 );
        CHECK( trackedValue( h.Get( "42" ) ) == 42 && trackedValue( h.Get( 42 ) ) == 42 );
        CHECK( h.HasKey( "alpha" ) && ! h.HasKey( "gamma" ) && h.HasKey( 42 ) );
        h.Push( "alpha", new CTracked( 11 ) );                // replaces and deletes
        CHECK( h.GetLength() == 3 && trackedValue( h.Get( "alpha" ) ) == 11 && CTracked::s_nLive == 3 );

            // Keys that end in the same characters (the old hash ignored
            // the last one), and deleting one key of many.
        for ( int n = 0; n < 2000; n++ )
            h.Push( CString( "key" ) + n, new CTracked( n ) );
        CHECK( h.GetLength() == 2003 );
        bool bAll = true;
        for ( int n = 0; n < 2000; n++ )
            if ( trackedValue( h.Get( CString( "key" ) + n ) ) != n )
                bAll = false;
        CHECK( bAll );
        h.Delete( "key5" );                                   // used to drop the whole bucket
        CHECK( h.GetLength() == 2002 && ! h.HasKey( "key5" ) && h.HasKey( "key6" ) && h.HasKey( "key15" ) );
        CObject *pRemoved = h.Remove( "key6" );
        CHECK( trackedValue( pRemoved ) == 6 && h.GetLength() == 2001 );   // the count used to drift
        delete pRemoved;
        h.Delete( -5 );                                       // used to index outside the table
        CHECK( h.Remove( "nothing" ) == NULL );

        CStringArray aKeys = h.keySet();
        CHECK( aKeys.GetLength() == h.GetLength() && aKeys.Get( 0 ) == "alpha" && aKeys.Get( 1 ) == "beta" && aKeys.Get( 2 ) == "42" );
        CHECK( trackedValue( h.Enum( 2 ) ) == 42 && h.Enum( 999999 ) == NULL );
        CIterator *pIt = h.Enumerate();
        size_t nSeen = 0;
        while ( pIt->HasMore() )
        {
            if ( pIt->Next() != NULL )
                nSeen++;
        }
        delete pIt;
        CHECK( nSeen == h.GetLength() );
        CHECK( CTracked::s_nLive == (int)h.GetLength() );

        CHash sized( CHash::TINY ), named( "huge" );
        CHECK( sized.GetSizeName() == "TINY" && named.GetSizeName() == "HUGE" );
    }
    CHECK( CTracked::s_nLive == 0 );

    section( "CList, CStack and CQueue" );
    {
        CList l;
        CTracked *pA = new CTracked( 1 ), *pB = new CTracked( 2 ), *pC = new CTracked( 3 );
        l.AddLast( pB );
        l.AddFirst( pA );
        CHECK( l.AddAfter( pB, pC ) == 2 );
        CHECK( l.AddBefore( pA, new CTracked( 0 ) ) == 0 );
        CTracked *pRefused = new CTracked( 9 );
        CHECK( l.AddBefore( (CObject *)&l, pRefused ) == NOT_FOUND );     // not added: still the caller's
        delete pRefused;
        CHECK( l.GetLength() == 4 && trackedValue( l.GetFirst() ) == 0 && trackedValue( l.GetLast() ) == 3 );
        CHECK( l.Find( pB ) == 2 && l.Find( NULL ) == -1 );
        CHECK( trackedValue( l.GetAt( 1 ) ) == 1 && l.GetAt( 99 ) == NULL );
        CObject *pFirst = l.RemoveFirst();
        delete pFirst;
        CIterator *pIt = l.Enumerate();
        CHECK( trackedValue( pIt->Next() ) == 1 && trackedValue( pIt->Next() ) == 2 && trackedValue( pIt->Prev() ) == 2 );
        delete pIt;
        CList empty;
        CHECK( empty.RemoveFirst() == NULL && empty.GetLast() == NULL );
    }
    CHECK( CTracked::s_nLive == 0 );
    {
        CStack s;
        CHECK( s.Pop() == NULL && s.Peek() == NULL );
        for ( int n = 0; n < 10; n++ )
            s.Push( new CTracked( n ) );
        CHECK( trackedValue( s.Peek() ) == 9 );
        CObject *pTop = s.Pop();
        CHECK( trackedValue( pTop ) == 9 && s.GetLength() == 9 );
        delete pTop;

        CQueue q;
        CHECK( q.Pop() == NULL );
        for ( int n = 0; n < 1000; n++ )
            q.Push( new CTracked( n ) );
        bool bOrder = true;
        for ( int n = 0; n < 500; n++ )
        {
            CObject *p = q.Pop();
            if ( trackedValue( p ) != n )
                bOrder = false;
            delete p;
        }
        CHECK( bOrder && q.GetLength() == 500 && trackedValue( q.Peek() ) == 500 );
        CIterator *pIt = q.Enumerate();
        CHECK( trackedValue( pIt->Next() ) == 500 );          // from the front
        delete pIt;
    }
    CHECK( CTracked::s_nLive == 0 );

    section( "CCache" );
    CTracked::s_nLive = 0;
    {
        CCache c( 3, true );
        c.AddItem( new CTestItem( "a" ) );
        c.AddItem( new CTestItem( "b" ) );
        c.AddItem( new CTestItem( "c" ) );
        CTestItem oA( "a" ), oB( "b" ), oD( "d" );
        CHECK( c.Get( &oA ) != NULL && c.Get( &oA )->GetHitCount() == 2 );
        c.AddItem( new CTestItem( "d" ) );                    // drops b, the least recently used
        CHECK( c.GetLength() == 3 && c.Get( &oB ) == NULL && c.Get( "c" ) != NULL );
        c.AddItem( new CTestItem( "d" ) );                    // replaces (and deletes) the old d
        CHECK( c.GetLength() == 3 && CTracked::s_nLive == 3 + 3 );
        CHECK( c.Discard( &oA ) );                            // a is next to go
        c.AddItem( new CTestItem( "e" ) );
        CHECK( c.Get( &oA ) == NULL && c.Get( &oD ) != NULL );

        CCacheItem *pD = c.Get( "d" );
        CDate dttPast;
        dttPast.SetToCurrent();
        dttPast.Add( -1 );
        pD->SetExpiration( dttPast );
        CHECK( c.Get( "d" ) == NULL && c.GetLength() == 2 );  // expired (and deleted)

        CDate dttFuture;
        dttFuture.SetToCurrent();
        dttFuture.Add( 2 );
        CHECK( c.ClearExpired( dttFuture ) == 2 && c.IsEmpty() );   // used to read freed memory

        CCache one( 1 );
        one.AddItem( new CTestItem( "x" ) );
        one.AddItem( new CTestItem( "y" ) );                  // used to crash with one entry
        CHECK( one.GetLength() == 1 && one.Get( "y" ) != NULL );
    }
    CHECK( CTracked::s_nLive == 0 );
}
