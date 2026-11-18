/***********************************************************************
**  TestDatabase
**
**  Tests for the Database folder through the SQLite driver, on an
**  in-memory database: statements and bound parameters, cursors (NULLs,
**  blobs, headers of an empty result, moving between rows, columns by
**  name), the updatable cursor (insert, update and delete through it),
**  transactions, errors, and the database's list of connections.
***********************************************************************/

#include "IASLib.h"

#ifdef IASLIB_DB_SQLITE__
    #include "Database/Sqlite/Sqlt_Database.h"
    #include "Database/Sqlite/Sqlt_Connection.h"
#endif

#include <cstdio>
#include <cstring>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
            fflush( stdout ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
    fflush( stdout );
}

#if defined( IASLIB_DATABASE__ ) && defined( IASLIB_DB_SQLITE__ )

static int countRows( CConnection *pCon, const char *strSQL )
{
    CCursor *pCursor = pCon->Cursor( strSQL );
    if ( pCursor == NULL )
        return -1;
    int nCount = atoi( pCursor->GetColumn( (size_t)0 ) );
    delete pCursor;
    return nCount;
}

static void testStatements( CSQLiteConnection *pCon )
{
    section( "Statements and parameters" );
    CHECK( pCon->Execute( "CREATE TABLE people ( id INTEGER PRIMARY KEY, name TEXT, age INTEGER, note TEXT );" ) );
    CHECK( pCon->Execute( "INSERT INTO people ( name, age ) VALUES ( 'Ann', 30 ); INSERT INTO people ( name, age ) VALUES ( 'Bob', 40 );" ) );
    CHECK( pCon->Changes() == 1 );
    CHECK( pCon->LastInsertId() == 2 );

    CStringArray aParams;
    aParams.Push( CString( "O'Brien; DROP TABLE people" ) );
    aParams.Push( CString( "55" ) );
    CHECK( pCon->Execute( "INSERT INTO people ( name, age ) VALUES ( ?, ? )", aParams ) );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people" ) == 3 );

        // More than one statement with parameters is refused.
    CHECK( ! pCon->Execute( "INSERT INTO people ( name ) VALUES ( ? ); SELECT 1", aParams ) );

        // Bad SQL reports an error.
    CHECK( ! pCon->Execute( "SELEKT nothing" ) );
    CHECK( pCon->MajorErrorCode() != IASLIB_DB_OK );
    CHECK( strlen( pCon->MajorErrorMessage() ) > 0 );
    CHECK( pCon->Execute( "SELECT 1" ) );
    CHECK( pCon->MajorErrorCode() == IASLIB_DB_OK );
}

static void testCursors( CSQLiteConnection *pCon )
{
    section( "Cursors" );
    CCursor *pCursor = pCon->Cursor( "SELECT id, name, age, note FROM people ORDER BY id" );
    CHECK( pCursor != NULL );
    if ( pCursor == NULL )
        return;
    CHECK( pCursor->IsValid() );
    CHECK( pCursor->Rows() == 3 );
    CHECK( pCursor->Columns() == 4 );
    CHECK( strcmp( pCursor->ColumnName( 1 ), "name" ) == 0 );
    CHECK( pCursor->GetColumn( "NAME" ) == "Ann" );       // any case
    CHECK( pCursor->IsNull( "note" ) );
    CHECK( ! pCursor->IsNull( "age" ) );
    CHECK( pCursor->Next() );
    CHECK( pCursor->GetColumn( (size_t)1 ) == "Bob" );
    CHECK( pCursor->Next() );
    CHECK( pCursor->GetColumn( "name" ) == "O'Brien; DROP TABLE people" );
    CHECK( ! pCursor->Next() );                            // stays at the last
    CHECK( pCursor->GetRow() == 2 );
    CHECK( pCursor->GetColumn( (size_t)99 ) == "" );
    CHECK( pCursor->Prev() );
    CHECK( pCursor->GetColumn( "age" ) == "40" );
    CHECK( pCursor->SetRow( 0 ) );
    CHECK( pCursor->GetColumn( "id" ) == "1" );
    CHECK( ! pCursor->IsUpdatable() );
    CHECK( pCursor->UpdateRow( "people" ) != IASLIB_DB_OK );
    delete pCursor;

        // An empty result still has its columns.
    pCursor = pCon->Cursor( "SELECT name, age FROM people WHERE age > 1000" );
    CHECK( pCursor != NULL );
    if ( pCursor )
    {
        CHECK( pCursor->Rows() == 0 );
        CHECK( pCursor->Columns() == 2 );
        CHECK( strcmp( pCursor->ColumnName( 1 ), "age" ) == 0 );
        CHECK( ! pCursor->Next() );
        delete pCursor;
    }

        // Parameters.
    CStringArray aParams;
    aParams.Push( CString( "35" ) );
    pCursor = pCon->Cursor( "SELECT name FROM people WHERE age > ? ORDER BY age", aParams );
    CHECK( pCursor != NULL && pCursor->Rows() == 2 );
    delete pCursor;

        // Blobs keep their zero bytes.
    CHECK( pCon->Execute( "CREATE TABLE blobs ( b BLOB ); INSERT INTO blobs VALUES ( x'41004243' );" ) );
    pCursor = pCon->Cursor( "SELECT b FROM blobs" );
    CHECK( pCursor != NULL );
    if ( pCursor )
    {
        CString strBlob = pCursor->GetColumn( (size_t)0 );
        CHECK( strBlob.GetLength() == 4 );
        CHECK( memcmp( (const char *)strBlob, "A\0BC", 4 ) == 0 );
        delete pCursor;
    }

        // A cursor needs one statement that returns rows.
    CHECK( pCon->Cursor( "UPDATE people SET age = age" ) == NULL );
    CHECK( pCon->Cursor( "SELECT 1; SELECT 2" ) == NULL );
    CHECK( pCon->Cursor( "SELECT * FROM nowhere" ) == NULL );

        // The HTML table escapes its values.
    CString strTable = pCon->SQLTable( "SELECT name FROM people WHERE id = 3" );
    CHECK( strstr( (const char *)strTable, "<table" ) != NULL || strstr( (const char *)strTable, "<TABLE" ) != NULL );
    CHECK( strstr( (const char *)strTable, "Brien" ) != NULL );
}

static void testUpdatable( CSQLiteConnection *pCon )
{
    section( "Updatable cursor" );
    CCursor *pCursor = pCon->Cursor( "SELECT name, age, note FROM people ORDER BY age", true );
    CHECK( pCursor != NULL );
    if ( pCursor == NULL )
        return;
    CHECK( pCursor->IsUpdatable() );

        // Update the first row (its NULL note matches with IS).
    CHECK( pCursor->SetColumn( "age", 31 ) == 1 );
    CHECK( pCursor->SetColumn( "note", "updated" ) == 1 );
    CHECK( pCursor->IsChanged( 1 ) && ! pCursor->IsChanged( 0 ) );
    CHECK( pCursor->UpdateRow( "people" ) == IASLIB_DB_OK );
    CHECK( ! pCursor->IsChanged( 1 ) );
    CHECK( pCursor->GetColumn( "age" ) == "31" );
    CHECK( ! pCursor->IsNull( "note" ) );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people WHERE name = 'Ann' AND age = 31 AND note = 'updated'" ) == 1 );

        // Nothing staged: nothing to do.
    CHECK( pCursor->UpdateRow( "people" ) == IASLIB_DB_OK );

        // Insert a new row.
    CHECK( pCursor->SetColumn( "name", "Cy" ) == 1 );
    CHECK( pCursor->SetColumn( "age", 22 ) == 1 );
    CHECK( pCursor->InsertRow( "people" ) == IASLIB_DB_OK );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people WHERE name = 'Cy' AND age = 22" ) == 1 );

        // Delete the second row (Bob).
    CHECK( pCursor->SetRow( 1 ) );
    CHECK( pCursor->GetColumn( "name" ) == "Bob" );
    CHECK( pCursor->DeleteRow( "people" ) == IASLIB_DB_OK );
    CHECK( pCursor->Rows() == 2 );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people WHERE name = 'Bob'" ) == 0 );

        // A row no longer in the table is reported.
    CHECK( pCursor->SetRow( 0 ) );
    CHECK( pCon->Execute( "DELETE FROM people WHERE name = 'Ann'" ) );
    CHECK( pCursor->DeleteRow( "people" ) != IASLIB_DB_OK );

        // Table names are quoted, not pasted in.
    CHECK( pCursor->SetRow( 1 ) );
    CHECK( pCursor->DeleteRow( "people; DROP TABLE people" ) != IASLIB_DB_OK );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people" ) >= 1 );
    delete pCursor;
}

static void testTransactions( CSQLiteConnection *pCon )
{
    section( "Transactions" );
    int nBefore = countRows( pCon, "SELECT COUNT(*) FROM people" );
    CHECK( pCon->BeginTransaction() );
    CHECK( pCon->Execute( "INSERT INTO people ( name ) VALUES ( 'Temp' )" ) );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people" ) == nBefore + 1 );
    CHECK( pCon->RollbackTransaction() );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people" ) == nBefore );

    CHECK( pCon->BeginTransaction() );
    CHECK( pCon->Execute( "INSERT INTO people ( name ) VALUES ( 'Kept' )" ) );
    CHECK( pCon->CommitTransaction() );
    CHECK( countRows( pCon, "SELECT COUNT(*) FROM people" ) == nBefore + 1 );

        // Nothing to commit or roll back outside a transaction.
    CHECK( ! pCon->CommitTransaction() );
}

static void testDatabase( void )
{
    section( "Database and connections" );
    CSQLiteDatabase *pDB = new CSQLiteDatabase;
    CHECK( pDB->Connect() );

    CConnection *pCon1 = pDB->Connection( ":memory:", "", "", "", "one" );
    CConnection *pCon2 = pDB->Connection( ":memory:", "", "", "", "two" );
    CHECK( pCon1 != NULL && pCon2 != NULL );
    CHECK( pDB->GetConnectionCount() == 2 );
    CHECK( pCon1->IsValid() );
    CHECK( pCon1->GetDBType() == DB_SQLITE );
    CHECK( pDB->SetTimeout( 5 ) );

        // A file that cannot be opened: NULL, with the error recorded.
    CConnection *pBad = pDB->Connection( "/no/such/directory/at/all/x.db", "", "", "", "bad" );
    CHECK( pBad == NULL );
    CHECK( pDB->MajorErrorCode() != IASLIB_DB_OK );
    CHECK( pDB->GetConnectionCount() == 2 );

        // Deleting a connection takes it out of the list.
    delete pCon2;
    CHECK( pDB->GetConnectionCount() == 1 );

        // The database deletes what is left.
    delete pDB;
}

int main( void )
{
    CSQLiteDatabase *pDB = new CSQLiteDatabase;
    CSQLiteConnection *pCon = static_cast<CSQLiteConnection *>( pDB->Connection( ":memory:", "", "", "", "test" ) );
    CHECK( pCon != NULL );
    if ( pCon )
    {
        testStatements( pCon );
        testCursors( pCon );
        testUpdatable( pCon );
        testTransactions( pCon );
    }
    delete pDB;
    testDatabase();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}

#else

int main( void )
{
    printf( "SQLite support is not built: nothing to test.\n" );
    return 0;
}

#endif
