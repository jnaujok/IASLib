cmake_minimum_required(VERSION 3.10)

#set project name
project(IASLib VERSION 3.0.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
option(USE_JSON "Enable JSON support." ON)
option(USE_HERSCHEL_CORRECTION "CDate treats years divisible by 4000 as common years by default." OFF)
# Windows always uses ICU (icu.dll) for named time zones; this turns it on
# elsewhere, where zone files are normally used, mainly for testing.
option(USE_TIMEZONE_ICU "Named time zones may come from the ICU library (non-Windows)." OFF)

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
file( GLOB MEMORY_MANAGER "src/MemoryManager/*.cpp")
file( GLOB NETWORK_SERVICES
        "src/NetworkServices/*.cpp"
        "src/NetworkServices/Entities/*.cpp"
        "src/NetworkServices/HTTP/*.cpp"
        "src/NetworkServices/HTTP/Handlers/*.cpp"
        "src/NetworkServices/SIP/*.cpp"
        "src/NetworkServices/SIP/Handlers/*.cpp")
file( GLOB RESOURCES "src/Resources/*.cpp")
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
        ${LOGGING}
        ${MEMORY_MANAGER}
        ${NETWORK_SERVICES}
        ${SOCKETS}
        ${STATS}
        ${STATUS}
        ${STREAMS}
        ${THREADING}
        ${XML}
        ${RESOURCES})

if(USE_JSON)
    list(APPEND SOURCES ${JSON})
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Set up our internal environment
if(LINUX)
    add_definitions(-DIASLIB_LINUX__)
endif()

if(NOT WIN32)
    # 64-bit file offsets for CFile's Big methods on 32-bit systems too.
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

if(WIN32)
    add_definitions(-DIASLIB_WIN32__)
endif()

if(APPLE)
    add_definitions(-DIASLIB_MAC__)
endif()

if(USE_THREADS)
    add_definitions(-DIASLIB_MULTI_THREADED__)
    if(NOT WIN32)
        # Thread.cpp only creates threads through pthreads when this is set.
        add_definitions(-DIASLIB_PTHREAD__)
    endif()
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
if (USE_JSON)
    add_definitions(-DIASLIB_JSON_SUPPORT__)
endif()
if (USE_HERSCHEL_CORRECTION)
    add_definitions(-DIASLIB_HERSCHEL_CORRECTION__)
endif()
if (USE_TIMEZONE_ICU AND NOT WIN32)
    add_definitions(-DIASLIB_TIMEZONE_ICU__)
endif()


configure_file(${PROJECT_SOURCE_DIR}/inc/IASLibConfig.h.in ${PROJECT_SOURCE_DIR}/inc/IASLibConfig.h)

#add library file
add_library(IASLib ${SOURCES})

# A project that adds IASLib with add_subdirectory() or FetchContent gets
# its headers and feature macros by linking to the IASLib target. (They were
# set for this directory only, so such a project had to repeat them.)
get_directory_property(IASLIB_INCLUDE_DIRS INCLUDE_DIRECTORIES)
get_directory_property(IASLIB_DEFINITIONS COMPILE_DEFINITIONS)
# Installed, the headers keep their layout under include/IASLib.
include(GNUInstallDirs)
set(IASLIB_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/IASLib")
foreach(dir ${IASLIB_INCLUDE_DIRS})
    if(IS_DIRECTORY "${dir}")
        file(RELATIVE_PATH rel "${PROJECT_SOURCE_DIR}/inc" "${dir}")
        if(NOT rel MATCHES "^\\.\\.")
            target_include_directories(IASLib PUBLIC "$<BUILD_INTERFACE:${dir}>"
                                                     "$<INSTALL_INTERFACE:${IASLIB_INSTALL_INCLUDEDIR}/${rel}>")
        else()
            target_include_directories(IASLib PUBLIC "$<BUILD_INTERFACE:${dir}>")
        endif()
    endif()
endforeach()
# The same name in a build tree as from the installed package.
add_library(IASLib::IASLib ALIAS IASLib)
target_compile_definitions(IASLib PUBLIC ${IASLIB_DEFINITIONS})

if(USE_THREADS AND NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(IASLib PUBLIC Threads::Threads)
endif()
if(USE_SQLITE)
    find_package(SQLite3)
    if(SQLite3_FOUND)
        target_link_libraries(IASLib PUBLIC SQLite::SQLite3)
    else()
        target_link_libraries(IASLib PUBLIC sqlite3)
    endif()
endif()
if(USE_NETWORK)
    # Secure sockets use OpenSSL (on Windows, set OPENSSL_ROOT_DIR if CMake
    # does not find it).
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        target_link_libraries(IASLib PUBLIC OpenSSL::SSL OpenSSL::Crypto)
        # Secure sockets (CTlsContext, CSecureSocket, HTTPS and SIP over TLS).
        target_compile_definitions(IASLib PUBLIC IASLIB_TLS__)
    endif()
endif()
target_link_libraries(IASLib PUBLIC ${CMAKE_DL_LIBS})
if(WIN32)
    # Winsock (MSVC also gets it from a #pragma in Platform.h).
    target_link_libraries(IASLib PUBLIC ws2_32 crypt32)
endif()

# ---- Installing and packaging --------------------------------------------
#
#   cmake --install build --prefix /usr/local     the library, headers and CMake package
#   cpack --config build/CPackConfig.cmake -G DEB  (or -G RPM) a package for Linux

set(IASLIB_PKG_THREADS OFF)
if(USE_THREADS AND NOT WIN32)
    set(IASLIB_PKG_THREADS ON)
endif()
set(IASLIB_PKG_SQLITE_TARGET OFF)
if(USE_SQLITE AND SQLite3_FOUND)
    set(IASLIB_PKG_SQLITE_TARGET ON)
endif()
set(IASLIB_PKG_TLS OFF)
if(USE_NETWORK AND OPENSSL_FOUND)
    set(IASLIB_PKG_TLS ON)
endif()

include(CMakePackageConfigHelpers)
set(IASLIB_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/IASLib")
install(TARGETS IASLib EXPORT IASLibTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY inc/ DESTINATION ${IASLIB_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(EXPORT IASLibTargets NAMESPACE IASLib:: DESTINATION ${IASLIB_CMAKE_DIR})
configure_package_config_file(cmake/IASLibConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/IASLibConfig.cmake"
                              INSTALL_DESTINATION ${IASLIB_CMAKE_DIR})
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/IASLibConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/IASLibConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/IASLibConfigVersion.cmake"
        DESTINATION ${IASLIB_CMAKE_DIR})
install(FILES README.md CHANGELOG.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CPACK_PACKAGE_NAME "iaslib")
    set(CPACK_PACKAGE_VENDOR "The Irene Adler Software Group")
    set(CPACK_PACKAGE_CONTACT "Jeffrey R. Naujok")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IASLib: a C++ library of base types, collections, threads, sockets, HTTP/SIP servers, JSON, XML, databases and more")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/jnaujok/IASLib")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

        # Debian/Ubuntu: a -dev package (a static library and headers).
    set(CPACK_DEBIAN_PACKAGE_NAME "libiaslib-dev")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsqlite3-dev, libssl-dev")

        # Fedora/RHEL/SUSE: a -devel package.
    set(CPACK_RPM_PACKAGE_NAME "iaslib-devel")
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_LICENSE "LGPL-3.0-only")
    set(CPACK_RPM_PACKAGE_REQUIRES "sqlite-devel, openssl-devel")
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/lib/cmake;/usr/lib64/cmake;/usr/include;/usr/share/doc")
    include(CPack)
endif()

# The tests are built when IASLib is the top-level project, and can be
# turned off (or on) with IASLIB_BUILD_TESTS.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(IASLIB_TESTS_DEFAULT ON)
else()
    set(IASLIB_TESTS_DEFAULT OFF)
endif()
option(IASLIB_BUILD_TESTS "Build the IASLib test programs." ${IASLIB_TESTS_DEFAULT})
if(IASLIB_BUILD_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY( test )
endif()
