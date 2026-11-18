# IASLib
The Irene Adler Software Library (C++)

[![Build and test](https://github.com/jnaujok/IASLib/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/jnaujok/IASLib/actions/workflows/build.yml)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](LICENSE)
[![C++11](https://img.shields.io/badge/C%2B%2B-11-00599C.svg?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/11)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-064F8C.svg?logo=cmake&logoColor=white)](https://cmake.org/)
[![Version](https://img.shields.io/badge/version-2.5-brightgreen.svg)](CMakeLists.txt)
[![Since 1994](https://img.shields.io/badge/since-1994-8A2BE2.svg)](#iaslib)

[![Linux](https://img.shields.io/badge/Linux-GCC%20%7C%20Clang-FCC624.svg?logo=linux&logoColor=black)](#platforms)
[![macOS](https://img.shields.io/badge/macOS-Apple%20Clang-000000.svg?logo=apple&logoColor=white)](#platforms)
[![Windows](https://img.shields.io/badge/Windows-MSVC-0078D6.svg?logo=windows&logoColor=white)](#platforms)
[![TLS](https://img.shields.io/badge/TLS-1.2%20%7C%201.3%20(OpenSSL)-721412.svg?logo=openssl&logoColor=white)](#secure-sockets-https-and-sip-over-tls)
[![Sanitizers](https://img.shields.io/badge/tested%20with-ASan%20%7C%20UBSan%20%7C%20TSan%20%7C%20Valgrind-success.svg)](#testing)

[![SIP](https://img.shields.io/badge/SIP-RFC%203261-orange.svg)](https://www.rfc-editor.org/rfc/rfc3261)
[![HTTP/1.1](https://img.shields.io/badge/HTTP%2F1.1-RFC%209110%20%7C%209112-orange.svg)](https://www.rfc-editor.org/rfc/rfc9112)
[![JSON](https://img.shields.io/badge/JSON-RFC%208259-orange.svg)](https://www.rfc-editor.org/rfc/rfc8259)
[![Merkle](https://img.shields.io/badge/Merkle%20trees-RFC%206962-orange.svg)](https://www.rfc-editor.org/rfc/rfc6962)
[![Time zones](https://img.shields.io/badge/time%20zones-IANA%20tzdb-orange.svg)](https://www.iana.org/time-zones)

[![Last commit](https://img.shields.io/github/last-commit/jnaujok/IASLib.svg)](https://github.com/jnaujok/IASLib/commits/master)
[![Code size](https://img.shields.io/github/languages/code-size/jnaujok/IASLib.svg)](https://github.com/jnaujok/IASLib)
[![Top language](https://img.shields.io/github/languages/top/jnaujok/IASLib.svg)](https://github.com/jnaujok/IASLib)
[![Issues](https://img.shields.io/github/issues/jnaujok/IASLib.svg)](https://github.com/jnaujok/IASLib/issues)
[![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](#contributing)

This library started as a project, way back in 1994, to write a cross-platform library in C++ that could run on Windows, Mac, or Linux systems equally well in order to write a platform-independent Ray Tracing program that ran on a non-homogenous Parallel Virtual Machine (PVM) network as part of a college Independent study.

This was so successful, that the library kept on growing. Over the years it's been used for lots of different projects, and has kept growing to provide more and more functionality. 

It became the basis of a system I developed for MCI (before the WorldCom days) that provided a WebServer for hundreds of concurrent users back in the late 1990's -- even outperforming the Netscape Enterprise Server that ran alongside it.

It was used to develop a caching solution for a major shipping carrier that performed over 2 billion operations per day. And parts of it were used to replace an Oracle layer that was so effective that it took one operation from 45 minutes to under 10 seconds to run. 

The library includes things like memory debugging and leak detection, including the detection of buffer overwrites or underwrites, including the file and line where the memory was allocated, and where it was freed. It detects memory leaks at program shutdown. It even provides Run-Time Type Information that can allow you to determine the type of any class derived from a CObject base type.

Overall, this library has been majorly helpful, right up to the present day where I was able to implement a working SIP server (SBC) in just a few weeks using the code in the library (and extending some of it, of course) to produce what I think is a very helpful framework for delivering cross-platform solutions. 

All without caring what platform you're running on.

## Why "Irene Adler"?

Irene Adler appears in the Sherlock Holmes story "A Scandal in Bohemia" published in 1891. In the story, we find that there is one time, and one time only, where Sherlock Holmes has been outwitted, and that was by Miss Adler, who forever afterwards was referred to by Holmes merely as, "The Woman".

I always considered this cross-platform library as a clever bit of coding, so I went looking for a clever person to use as a name without it being too obvious. I found Miss Adler, and decided that my "Clever Little Library" should be named for her.

It was years later that the movie "Sherlock Holmes", and the TV series of "Sherlock", and "Elementary" all decided to make her a central character, front and center to the movies and the shows. So, now my choice of name is far less obscure than it used to be. 

---

## Contents

- [Features](#features)
- [Platforms](#platforms)
- [Getting started](#getting-started)
- [Build options](#build-options)
- [Examples](#examples)
- [Testing](#testing)
- [Project layout](#project-layout)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [Security](#security)
- [License](#license)

## Features

**Foundations**
- `CObject` base class with run-time type information and per-class allocation tracking.
- Memory debugging: buffer overrun and underrun detection, file and line of every allocation and free, and a leak report at shutdown.
- `CString`: shared text with thread-safe copy-on-write, `printf`-checked formatting, and locale-independent numbers.
- Template collections — `TArray`, `TSortedArray`, `TSparseArray`, `TList`, `TStack`, `TQueue`, `THash` and `TCache` (LRU with expiry) — with range-based `for`, and the classic `CObject` collections built on them.
- RFC 9562 UUIDs, the ISAAC random number generator, variants, streams, files and logging.
- Threads, mutexes, semaphores and thread pools, using Win32 threads on Windows and POSIX threads everywhere else.

**Dates and time zones**
- `CDate` parses a very wide range of date and time formats, and works across the whole historical calendar: Julian before 15 October 1582, Gregorian after, with no year 0.
- ISO 8601 formatting and parsing, Julian Day numbers for astronomy, and an optional Herschel 4000-year rule.
- Each date can carry its own UTC offset, and arithmetic and comparisons work on the actual instant.
- IANA time zones such as `America/New_York`, read directly from the zone database. On Windows they come from Windows' own ICU library. Daylight-saving gaps and overlaps are handled.

**JSON** (RFC 8259)
- A strict parser that reports errors by line and column. Options allow comments, trailing commas, and rejecting duplicate keys, and there is a nesting limit.
- A Jackson-style node tree: `path()` chains, JSON Pointer, find helpers, deep equality and copying.
- Compact, pretty-printed and ASCII-only output, with doubles written in their shortest exact form.

**Networking**
- Portable sockets: TCP, UDP, timed reads and accepts, and IPv4/IPv6 addresses.
- **SIP server** (RFC 3261): server transactions with their timers, CANCEL, `received`/`rport`, keep-alives, and UDP, TCP and TLS transports.
- **HTTP/1.1 server** (RFC 9110/9112): keep-alive and pipelining, chunked bodies, `100-continue`, request-smuggling defences and virtual hosts.

**Secure sockets** (OpenSSL)
- `CTlsContext` and `CSecureSocket`: TLS 1.2+, certificate and host-name verification, and Server Name Indication with a certificate per host.
- HTTPS, and SIP over TLS. On HTTPS, a request whose `Host` doesn't match the TLS host name is answered with 421.
- Uses the Windows Trusted Root store, which OpenSSL doesn't read by itself.

**Blockchain**
- Proof-of-work blocks with a fixed, unambiguous 100-byte header.
- Full chain validation that pinpoints the block that was tampered with.
- Mining that can be limited, cancelled or run on another thread.
- Saving and loading as JSON or binary.
- RFC 6962 Merkle trees with inclusion proofs.

**Also included**
- XML 1.0 documents (entities, CDATA, DOCTYPE, errors with line and column, safe against external entities), SQLite access (with Sybase, MySQL and Oracle connectors as options), SHA-256, UUIDs and configuration files.

## Platforms

The same source compiles on every platform. A single platform flag (`IASLIB_LINUX__`, `IASLIB_MAC__` or `IASLIB_WIN32__`, which CMake sets for you) selects the system-specific code.

| Platform | Compiler | Checked by CI |
|---|---|:---:|
| Linux | GCC | ✅ |
| Linux | Clang | ✅ |
| macOS | Apple Clang | ✅ |
| Windows | MSVC | ✅ |

## Getting started

### Requirements

- A C++11 compiler and CMake 3.10 or later.
- **Optional:** OpenSSL for TLS, and SQLite 3 for the database layer.
  - Linux: `sudo apt-get install libssl-dev libsqlite3-dev tzdata`
  - macOS: `brew install openssl@3` (SQLite comes with macOS)
  - Windows: `vcpkg install sqlite3:x64-windows`, plus an OpenSSL installation

### Build

```sh
git clone https://github.com/jnaujok/IASLib.git
cd IASLib
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

On macOS, point CMake at Homebrew's OpenSSL with `-DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)"`. On Windows, pass vcpkg's toolchain file (`-DCMAKE_TOOLCHAIN_FILE=...`) and, if needed, `-DOPENSSL_ROOT_DIR`.

### Use

Link against `IASLib` and include either `IASLib.h` or the individual headers:

```cmake
add_subdirectory(IASLib)
target_link_libraries(my_app PRIVATE IASLib)
```

## Build options

| Option | Default | What it does |
|---|:---:|---|
| `USE_THREADS` | `ON` | Multi-threading support. |
| `USE_NETWORK` | `ON` | Sockets, SIP and HTTP. TLS is built in when OpenSSL is found (`IASLIB_TLS__`). |
| `USE_JSON` | `ON` | The JSON library (needed for saving blockchains as JSON). |
| `USE_DATABASE` | `ON` | The database layer. |
| `USE_SQLITE` | `ON` | SQLite connector. |
| `USE_MYSQL` / `USE_ORACLE` / `USE_SYBASE` | `OFF` | Other database connectors. |
| `USE_RTTI` | `ON` | `CObject` run-time type information. |
| `USE_MEMORY_DEBUGGING` | `ON` | CObjects come from `CMemoryDebugHeap`: fenceposts, bad-free detection, and leak reports when tracking is switched on. |
| `USE_MEMORY_MANAGER` | `OFF` | CObjects come from `CMemoryManager::Get()` (malloc unless changed with `Set()`), without the debugging heap. |
| `USE_DEBUG` | `ON` | Debugging features. |
| `USE_HERSCHEL_CORRECTION` | `OFF` | `CDate` treats years divisible by 4000 as common years. |
| `USE_TIMEZONE_ICU` | `OFF` | Use ICU for named time zones off Windows (Windows always uses ICU). |

## Examples

### Dates and time zones

```cpp
CDate when = CDate::Parse( "Sat, 02 Jun 2001 12:14:11 GMT" );
printf( "%s\n", (const char *)when.FormatISO8601() );          // 2001-06-02T12:14:11Z

CDate::SetDefaultTimezone( "America/New_York" );                 // follows daylight saving
CDate tokyo = when.ToTimeZone( "Asia/Tokyo" );
double jd = when.GetJulianDate();                                 // for astronomy
```

### JSON

```cpp
CJsonNode *root = CJsonParser::parse( "{\"users\":[{\"name\":\"Ann\",\"age\":31}]}" );
CString name = root->path( "users" ).path( 0 ).path( "name" ).asText();   // "Ann"
int age = root->at( "/users/0/age" ).asInt();                             // JSON Pointer
delete root;

CObjectNode obj;
obj.put( "name", "Ann" )->put( "admin", true );
obj.putArray( "tags" )->add( "a" )->add( 2.5 );
printf( "%s\n", (const char *)obj.toPrettyString() );
```

### An HTTPS server with virtual hosts

```cpp
std::shared_ptr<CTlsContext> tls = std::make_shared<CTlsContext>( CTlsContext::SERVER );
tls->setCertificateFile( "www.crt" );
tls->setPrivateKeyFile( "www.key" );
tls->addHostContext( "api.example.com", apiTls );     // a second certificate (SNI)

CBasicHttpServer server( 443 );
server.AddVirtualHost( "www.example.com", wwwHandlers );
server.AddVirtualHost( "api.example.com", apiHandlers );
server.setTlsContext( tls );
server.Start();
if ( ! server.waitUntilReady( 5000 ) )
    printf( "%s\n", (const char *)server.getStartupError() );
// ...
server.Stop();
server.Wait();
```

### A SIP server on UDP, TCP and TLS

```cpp
CBasicSipServer sip( 5060 );                          // UDP and TCP
sip.AddHandlerFactory( new CBaseSipHandlerFactory() );
sip.setTlsContext( tls, 5061 );                        // and SIP over TLS
sip.Start();
```

### Template collections

```cpp
#include "IASLib.h"
using namespace IASLib;

THash<CString, int> counts;                 // iterates in the order keys were added
for ( const char *word : { "to", "be", "or", "not", "to", "be" } )
    counts[ word ]++;
for ( const auto &entry : counts )
    printf( "%s: %d\n", entry.Key.c_str(), entry.Value );

TSortedArray<int> sorted;                   // binary search, equal values keep their order
sorted.Push( 42 );
sorted.Push( 7 );
size_t at = sorted.Find( 42 );              // 1

TCache<CString, CString> cache( 1000 );     // least recently used goes first
cache.Put( "session", "data", 30000 );      // expires in 30 seconds
CString *value = cache.Get( "session" );
```

The collections grow by half their size but never by more than a megabyte at once, so small ones are cheap to append to and large ones carry bounded slack; `Reserve()` and `Compact()` take control where it matters.

### A blockchain

```cpp
CBlockchain chain( 16 );                               // 16 leading zero bits of work
chain.addBlock( "first record" );
chain.addBlock( entries );                             // several binary entries, one Merkle root

std::string reason;
if ( ! chain.validate( 16, &reason ) )                 // the difficulty you require
    printf( "tampered: %s\n", reason.c_str() );

std::string saved = chain.toJsonText();
std::unique_ptr<CBlockchain> loaded = CBlockchain::FromJsonText( saved, &reason );
```

## Testing

`ctest` runs eleven suites:

| Suite | Covers |
|---|---|
| `TestDate` | The calendar across 1000 BC to AD 9999, parsing, ISO 8601, Julian Days, offsets, named zones (cross-checked against the system), and ICU |
| `TestJson` | RFC 8259 conformance, error positions, options, numbers, Unicode, building trees, and fuzzing |
| `TestHttp` | Framing, the protocol core, virtual hosts, 421 handling, and a live server |
| `TestSip` | Parsing, transactions, CANCEL, Via handling, hostile input, and a live server over UDP and TCP |
| `TestTls` | TLS contexts, HTTPS, SNI, `sips:`, SIP over TLS, and trust stores |
| `TestBlockchain` | SHA-256 and RFC 6962 test vectors, inclusion proofs, tamper detection, saving, and threads |
| `TestBaseTypes` | `CObject`, `CString` (sharing, fixed buffers, numbers, searching, escaping, threads), the tokenizer, `CDataBlock`, UUIDs, ISAAC against Jenkins' test vector, and `CVariant` |
| `TestCollections` | Every template collection against the standard library on random operations, with counted elements to catch leaks, and the classic collections |
| `TestStreams` | String, file, buffered, socket and null streams: text that reads back, big-endian binary, lines, the end of a stream, and flushing |
| `TestSocketClasses` | Addresses, the config builder, TCP clients and servers over loopback (both ends' addresses, data, errors, timeouts, non-blocking), UDP, and IPv6 where available |
| `TestStats` | CHistogram CSV/XML output, CCoverageMap allocation and release, CCPUUsage times |
| `TestLogging` | The default sink, levels and macros, long messages, `${name}` context expansion, CLogFile across threads, CRotatingLogFile periods, names, rotation and pruning |
| `TestMemory` | Fixed-size and pooled heaps, the debugging heap (fenceposts, bad frees, leak tracking), and CObjects returning to the allocator they came from |
| `TestResources` | The resource manager: put/get/replace/remove, SHA-256 checks, reopening, folders and names, Compact(), crash and damage handling, read-only use, the LRU cache, threads |
| `TestResourceManager` | The resource manager: paths, sources by priority (archive and loose files), typed loaders, shared handles, the memory budget, reloading, background loading with main-thread callbacks |
| `TestDatabase` | The SQLite driver on an in-memory database: statements and bound parameters, cursors (NULLs, blobs, empty results, columns by name), the updatable cursor, transactions, errors, the list of connections |
| `TestHttpClient` | CHttpClient against CBasicHttpServer and scripted servers on localhost (keep-alive, chunked, read-to-close, HEAD, binary bodies, timeouts), response framing, header and URI injection refusal |
| `TestMail` | CMail against a fake SMTP server thread (EHLO/HELO fallback, multi-line replies, refusals, timeouts), message headers, CRLF and dot-stuffing, address and subject injection refusal |
| `TestUtilities` | Configuration (CSettings, CProfile: missing files, comments kept, changes by others, the cache), CDynamicLibrary (loading, missing libraries and functions), CSHA256 (test vectors, reuse), exceptions (rethrowing keeps the class, errno text, call stacks) |
| `TestFiles` | The Files folder: CFile modes, lines, Rename/Exists and 64-bit methods past 4 GB (sparse files); CFileStat; CDirectory; CMemMappedFile (growth, separate pointers, direct access, past 4 GB); CRWFile writer and reader; an archive with data past 4 GB |
| `TestThreading` | Starting and joining threads from any thread, return codes, `ExitThread()`, `Kill()`, exceptions, the monitor, semaphores, the thread pool and the server thread |
| `TestXMLDocument` | XML reading and writing, entities and CDATA, well-formedness errors with positions, XXE and entity-expansion protection, the element API, a real file, and fuzzing |

The suites are run under AddressSanitizer/UndefinedBehaviorSanitizer, ThreadSanitizer and Valgrind during development:

```sh
cmake -S . -B asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g"
cmake --build asan && ctest --test-dir asan
```

[GitHub Actions](https://github.com/jnaujok/IASLib/actions/workflows/build.yml) builds and tests every push on all four platform and compiler combinations.

## Project layout

```
inc/            public headers, by area (BaseTypes, Collections, JSON, NetworkServices, Sockets, ...)
src/            the implementation, mirroring inc/
test/           the test suites, one directory each
.github/        CI workflow and issue templates
CMakeLists.txt  the build
```

## Changelog

What changed, including breaking changes, is in [CHANGELOG.md](CHANGELOG.md).

## Contributing

Contributions are welcome.

1. Fork the repository and create a branch.
2. Keep the code portable: platform-specific code goes behind the platform flags, and system headers come in through `BaseTypes/Platform.h`.
3. Add or extend a test suite for what you change, and run `ctest`, ideally under a sanitizer as well.
4. Open a pull request. CI must pass on Linux, macOS and Windows.

Please follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Security

Please report vulnerabilities privately, as described in [SECURITY.md](SECURITY.md), rather than in a public issue.

## License

IASLib is licensed under the **GNU Lesser General Public License v3.0**; see [LICENSE](LICENSE). You can link it into your own applications, including commercial ones; changes to the library itself must be shared under the same licence.

Copyright © 1994–2026 The Irene Adler Software Group, a division of BlackStar Enterprises, LLC.
