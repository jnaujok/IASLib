cmake_minimum_required(VERSION 3.10)

#set project name
project(TestXMLDocument VERSION 1.0)

option(USE_THREADS "Use multi-threaded code support." ON)
option(USE_DATABASE "Database support - database must be specified." ON)
option(USE_SYBASE "Use the Sybase database." OFF)
option(USE_SQLITE "Use the Sqlite database." ON)
option(USE_MYSQL "Use the MySQL database." OFF)
option(USE_ORACLE "Use the Oracle database." OFF)
option(USE_NETWORK "Use Network Services." ON)
option(USE_RTTI "Use Run-Time Type Information." ON )
option(USE_DEBUG "Use debugging features of the library." ON)
option(USE_MEMORY_MANAGER "Use extended memory manager." OFF)
option(USE_MEMORY_DEBUGGING "Use memory debugging features." ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Set up our internal environment
if(LINUX)
    add_definitions(-DIASLIB_LINUX__)
endif()

if(WIN32)
    add_definitions(-DIASLIB_WIN32__)
endif()

if(APPLE)
    add_definitions(-DIASLIB_MAC__)
endif()

if(USE_THREADS)
    add_definitions(-DIASLIB_MULTI_THREADED__)
endif()
if(USE_DATABASE)
    add_definitions(-DIASLIB_DATABASE__)
endif()
if(USE_SYBASE)
    add_definitions(-DIASLIB_DB_SYBASE__)
endif()
if(USE_SQLITE)
    add_definitions(-DIASLIB_DB_SQLITE__)
endif()
if(USE_MYSQL)
    add_definitions(-DIASLIB_DB_MYSQL__)
endif()
if(USE_ORACLE)
    add_definitions(-DIASLIB_DB_ORACLE__)
endif()
if(USE_NETWORK)
    add_definitions(-DIASLIB_NETWORKING__)
endif()
if(USE_RTTI)
    add_definitions(-DIASLIB_RTTI__)
endif()
if (USE_DEBUG)
    add_definitions(-DIASLIB_DEBUG__)
endif()
if (USE_MEMORY_MANAGER)
    add_definitions(-DIASLIB_MEMORY_MANAGER__)
endif()
if (USE_MEMORY_DEBUGGING)
    add_definitions(-DIASLIB_MEMORY_DEBUGGING__)
endif()

include_directories(
        ../../inc
        ../../inc/BaseTypes
        ../../inc/Blockchain
        ../../inc/Collections
        ../../inc/Configuration
        ../../inc/Database
        ../../inc/Database/Metadata
        ../../inc/Database/MySQL
        ../../inc/Database/Oracle
        ../../inc/Database/Sqlite
        ../../inc/Database/Sybase
        ../../inc/DynamicLibraries
        ../../inc/Encryption
        ../../inc/Exceptions
        ../../inc/Files
        ../../inc/JSON
        ../../inc/Logging
        ../../inc/MemoryManager
        ../../inc/NetworkServices
        ../../inc/NetworkServices/Entities
        ../../inc/NetworkServices/HTTP
        ../../inc/NetworkServices/HTTP/Handlers
        ../../inc/NetworkServices/SIP
        ../../inc/NetworkServices/SIP/Handlers
        ../../inc/Resources
        ../../inc/Sockets
        ../../inc/Stats
        ../../inc/Status
        ../../inc/Streams
        ../../inc/Templates
        ../../inc/Threading
        ../../inc/XML
)

target_link_libraries(../../cmake-build-debug/libIASLib.a)

#add library file
add_executable(TestXMLDocument TestXMLDocument.cpp )

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)