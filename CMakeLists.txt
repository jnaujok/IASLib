cmake_minimum_required(VERSION 3.10)

#set project name
project(IASLib VERSION 2.0)

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

#Bring the headers, such as Student.h into the project
include_directories(
        inc
        inc/BaseTypes
        inc/Blockchain
        inc/Collections
        inc/Configuration
        inc/Database
        inc/Database/Metadata
        inc/Database/MySQL
        inc/Database/Oracle
        inc/Database/Sqlite
        inc/Database/Sybase
        inc/DynamicLibraries
        inc/Encryption
        inc/Exceptions
        inc/Files
        inc/JSON
        inc/Logging
        inc/MemoryManager
        inc/NetworkServices
        inc/NetworkServices/Entities
        inc/NetworkServices/HTTP
        inc/NetworkServices/HTTP/Handlers
        inc/NetworkServices/SIP
        inc/NetworkServices/SIP/Handlers
        inc/Resources
        inc/Sockets
        inc/Stats
        inc/Status
        inc/Streams
        inc/Templates
        inc/Threading
        inc/XML
)

file( GLOB BASE_TYPES "src/BaseTypes/*.cpp")
file( GLOB BLOCKCHAIN "src/Blockchain/*.cpp")
file( GLOB COLLECTIONS "src/Collections/*.cpp")
file( GLOB CONFIGURATION "src/Configuration/*.cpp")
file( GLOB DATABASE
        "src/Database/*.cpp"
        "src/Database/Metadata/*.cpp"
        "src/Database/MySQL/*.cpp"
        "src/Database/Oracle/*.cpp"
        "src/Database/Sqlite/*.cpp"
        "src/Database/Sybase/*.cpp")
file( GLOB DYNAMIC_LIBRARIES "src/DynamicLibraries/*.cpp" )
file( GLOB ENCRYPTION "src/Encryption/*.cpp")
file( GLOB EXCEPTIONS "src/Exceptions/*.cpp")
file( GLOB FILES "src/Files/*.cpp")
file( GLOB JSON "src/JSON/*.cpp")
file( GLOB LOGGING "src/Logging/*.cpp")
file( GLOB NETWORK_SERVICES
        "src/NetworkServices/*.cpp"
        "src/NetworkServices/Entities/*.cpp"
        "src/NetworkServices/HTTP/*.cpp"
        "src/NetworkServices/HTTP/Handlers/*.cpp"
        "src/NetworkServices/SIP/*.cpp"
        "src/NetworkServices/SIP/Handlers/*.cpp")
file( GLOB SOCKETS "src/Sockets/*.cpp")
file( GLOB STATS "src/Stats/*.cpp")
file( GLOB STATUS "src/Status/*.cpp")
file( GLOB STREAMS "src/Streams/*.cpp")
file( GLOB THREADING "src/Threading/*.cpp")
file( GLOB XML "src/XML/*.cpp")
set(
        SOURCES
        ${BASE_TYPES}
        ${BLOCKCHAIN}
        ${COLLECTIONS}
        ${CONFIGURATION}
        ${DATABASE}
        ${DYNAMIC_LIBRARIES}
        ${ENCRYPTION}
        ${EXCEPTIONS}
        ${FILES}
        ${JSON}
        ${LOGGING}
        ${NETWORK_SERVICES}
        ${SOCKETS}
        ${STATS}
        ${STATUS}
        ${STREAMS}
        ${THREADING}
        ${XML}
)

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


configure_file(${PROJECT_SOURCE_DIR}/inc/IASLibConfig.h.in ${PROJECT_SOURCE_DIR}/inc/IASLibConfig.h)

#add library file
add_library(IASLib ${SOURCES})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
