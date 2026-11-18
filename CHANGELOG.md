# Changelog

All notable changes to IASLib are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/) from 2.5.0 on.

## [Unreleased] — 3.0 in progress

The base types and collections, reviewed one by one and cleaned up, and template versions of every collection.

### ⚠️ Breaking changes

- **`CCString` (`String_Copy.h`) is removed.** It was unused, and `IASLIB_USE_DEEP_COPY_STRING__` could not have worked (it named a different class). `CString` copies are now safe across threads.
- **`IASLibChar__` is `char` on every platform.** It was `TCHAR` on Windows, which the string code never handled as wide characters.
- **`CString` details:**
  - Doubles and floats become the shortest text that reads back as the same value (`"0.1"`, `"2"`), always with a `.` decimal point; they were `"%f"` (`"2.000000"`) in the C locale's format.
  - `Replace()` and `Remove()` with strings make one pass, so text put in is not searched again.
  - `Escape()` also escapes `+`, so `Unescape()` gives back the original.
  - The `const` `operator []` returns a character, not a reference.
  - `getBytes()` returns `const unsigned char *`.
  - `hashcode()` is `hashCode()` (the old name still works) and overrides `CObject`'s, so equal strings hash alike everywhere.
  - Stream input (`IASLIB_STD_STREAM_SUPPORT__`) reads a word, as `std::string` does.
  - `dumpStubMemory()` is gone.
- **`CObject`:**
  - `GetType()` and `IsType()` are `const`.
  - `SetAssertHandler()` takes the handler to use.
  - A copied object gets a new ID.
  - `Object.h` no longer includes the threading headers.
- **Collections:**
  - Collections that own `CObject` pointers (`CArray`, `CHash`, `CList`, `CQueue`, `CSparseArray`, `CCache`) can't be copied. Every copy used to free the same objects twice.
  - `CSparseArray` is no longer a `CSortedArray`.
  - `CHash` keeps entries in the order they were added, and number keys equal their digits as strings.
  - `CQueue` iterates from the front.
  - `CCollection::m_nElements` is gone; `GetLength()` is virtual.
  - `CSortedArray::Find()` returns `NOT_FOUND` instead of an encoded insertion point.
  - `CCacheItem`'s list links are gone.
  - `HashBucket.h`, `HashSlat.h` and the unfinished template drafts (`TCollection.h`, `TEntry.h`, `TIterator.h`, `TMap.h`, `TDoubleList.h`, the old `TCache.h`) are removed.
- **`CUUID`:**
  - 16 bytes, written in the standard 36-character form.
  - `getBytes()` and `toString()` are `const`.
  - It no longer uses `TRandom`.
- **`CStringTokenizer`:** with every delimiter counting, `"a,"` gives `"a"` and `""` (n delimiters, n + 1 tokens).
- **`CMutex` can't be copied.** Copying one copied the operating system's mutex.

### Added

- **Template collections** (`Templates/TCollections.h`):
  - `TArray`, `TSortedArray`, `TSparseArray`, `TList`, `TStack` and `TQueue` (a ring buffer).
  - `THash`, which iterates in the order entries were added.
  - `TCache`, least recently used with optional expiry and an eviction handler.
  - They hold values, move what they can, and work with range-based `for`.
  - They grow by half their size, but never by more than a megabyte at once, and have `Reserve()` and `Compact()`.
- **`CString`:**
  - `c_str()`, `IsEmpty()`, `StartsWith()` and `EndsWith()`.
  - `Reserve()` and `Compact()`.
  - Move construction and assignment.
  - `+`, `+=` and `=` for every integer type and `double`.
  - `printf` format checking on `Format()` and `FormatString()`.
- **`CStringArray`:** `Find()`, `Contains()` and `Join()`.
- **`CIntArray`:** `Find()` and `Sort()`.
- **`CArray`:** `Find()`, `Reserve()` and `Compact()`.
- **`CQueue`:** `Get()`.
- **`CCache`:** `Get( key )`.
- **`CCacheItem`:** accessors for its dates and hits.
- **`CUUID`:** `getVersion()`, `isNil()`, `Nil()`, comparisons and `hashCode()`. It parses braced and undashed forms.
- **`TRandom`:** `SeedFromSystem()`, and it works with the standard library's random distributions.
- **`CVariant`:** `isEmpty()` and `isNumber()`. Its number getters convert, and read numbers from strings.
- **`CMutexLock`:** locks a mutex for a scope.
- **Tests:** `TestBaseTypes` and `TestCollections`.

### Fixed

- **`CString`:**
  - **Memory errors:**
    - Shortening a string to nothing wrote to freed memory.
    - `ToHex()` wrote past its buffer.
    - `hashcode()` crashed on an empty string.
    - Fixed-buffer strings could overrun their buffer, and appended text past the length they recorded.
  - **Wrong results:**
    - `"abc" == str` compared pointers.
    - `"" < "a"` was false.
    - `operator []` changed every copy of a string.
    - `Remove( char )` kept the old length.
    - `Replace()` looped forever when the new text contained the old, or the old was empty.
    - `LastIndexOf()` never found a match at 0.
    - `"abc"` didn't match the wildcard `"abc*"`.
    - `Copy()` did nothing on an empty string.
    - `Unescape()` read past the end, and turned bad escapes into NULs.
  - **Limits:**
    - `Format()` cut text off at 64 KB, through a shared buffer.
    - Every string had its own mutex.
    - Appending a character at a time copied the whole string each time.
  - **Missing:** `hasDelimitedTokens()` was declared but never defined.
- **`CObject`:**
  - Object IDs were counted without a lock, unless the build was multi-threaded.
  - The memory-debugging allocator crashed on a failed allocation or a NULL delete.
  - `ASSERT` broke `if`/`else`.
- **Templates and memory debugging:** classes built with memory debugging hid placement `new`.
- **`CDataBlock`** never recorded its size, so the first guardpost check read far outside the buffer. It also wrote the back guardpost inside the data, freed `malloc` memory with `delete`, and could throw from its destructor.
- **`CUUID`:**
  - It held 32 bytes and printed a form of varying length.
  - It couldn't parse a UUID (`*uuid + 1`).
  - It was seeded from the clock, so two programs started in the same second made the same UUIDs.
- **`TRandom` (ISAAC):**
  - The default integer type was 64 bits on Linux and macOS.
  - Seeding without a seed filled only 8 of its 256 words.
  - ISAAC-64 mixed with the 32-bit form.
  - Copies freed the same tables twice.
  - `UINT32` clashed with Windows' own type.
- **`CVariant`:**
  - The default constructor left every member unset.
  - `v = "text"` stored the boolean `true`.
  - NULL arrays and objects crashed `getString()`.
  - `getInt()` gave 0 for `long` values.
- **`CStringTokenizer`:** a one-character string gave no tokens in every-delimiter mode, and trailing empty tokens were dropped.
- **`CArray`:**
  - `Set()` past the end wrote past the buffer.
  - Copying or assigning an array freed its objects twice.
  - `Sort()` of an empty array read out of bounds.
  - It grew four slots at a time and shrank on almost every `Remove()`.
- **`CSortedArray`** never sorted. `Push()` appended at the end, or dropped (and leaked) a value equal to one already there.
- **`CSparseArray`** inherited that.
- **`CHash`:**
  - `Delete()` removed every key in the same bucket.
  - `Remove()` lost count.
  - Negative number keys indexed outside the table.
  - The string hash ignored the last character.
  - A key with surrounding spaces could be dropped and leaked by `Push()`.
  - Copies and assignment freed buckets twice.
  - Iterating rescanned the table for every element.
- **`CCache`:**
  - `ClearExpired()` read the item it had just freed.
  - Adding a key twice left a deleted item on the LRU list.
  - A one-entry cache crashed when it evicted.
- **Other collections:**
  - `CQueue::Pop()` moved every element.
  - `CStringArray::EmptyAll()` leaked every string.
  - `CIntArray` and `CStringArray` copies freed twice, and their sorts crashed when empty.
  - `CConnectionArray` freed memory with the wrong `delete`.
- **Copies that freed twice:** `CLogContext::addValue( CArray & )` and the thread pool's shutdown each copied an owning array.
- **`CStream << unsigned long`** wrote `"123ld"`. `printf` checking also found `size_t`s printed with `%d`.
- **Exceptions:**
  - The `IASLIB_THROW_*` macros threw a shared `static` exception, which two threads could change at once.
  - `CCollectionException` named itself `"CCollectionExcetption"`.

## [2.5.0] — 2026-11-17

A large update: the library now builds and passes its tests on Linux (GCC and Clang), macOS and Windows (MSVC) from the same source, and gains named time zones, a complete JSON library, TLS, and a rebuilt blockchain.

### ⚠️ Breaking changes

- **Legacy HTTP classes removed.** `CHTTPClient`, `CHTTPHeader`, `CHTTPHeaderList`, `CHTTPResponse`, `CHTTPServer` and `CServer` (the `NetworkServices/HTTP*.h` files) were replaced by the classes in `NetworkServices/HTTP/`. On case-insensitive file systems (macOS, Windows) their headers hid the new ones. They remain in git history.
- **`CHash::HUGE` is now `CHash::ENORMOUS`.** Apple's `math.h` defines `HUGE` as a macro. Stored hashes still use the text `"HUGE"`.
- **Blockchain difficulty is in leading zero bits**, no longer in hexadecimal zeros: the old `6` is `24` now. The default is 16.
- **Blockchain hashes and formats changed entirely.** Blocks hash a fixed binary header and hold binary entries. Chains made with the old code can't be verified by the new code.
- **`CBlock::getHash()` returns a binary digest.** Use `getHashString()` for hexadecimal text.
- **`MerkleNode` removed**, replaced by `CMerkleTree`. It never compiled, so nothing could have depended on it.
- **JSON API.** `longValue()` and `asLong()` return `long long`, since `long` is 32 bits on Windows. `binaryValue()` fills a byte vector instead of returning `char *`. The find methods return `std::vector`. `CPojoNode` and `CJsonDocument` are removed.
- **Windows builds target Windows 10** (`_WIN32_WINNT 0x0A00`) unless a program sets its own target. The previous target, Windows 2000, was too old for the socket code. `windows.h` is now included lean, without the `min`/`max` macros or GDI.

### Added

- **Named time zones** (`CTimeZone`):
  - IANA zones such as `America/New_York`, read from the system's zone database, with the full history and the rules for future years.
  - On Windows, zones come from Windows' own ICU library (`icu.dll`).
  - The system zone is read from `$TZ` or `/etc/localtime` with the same code, so it behaves identically on every platform.
- **`CDate`:**
  - ISO 8601 formatting and parsing.
  - Julian Day conversions.
  - A UTC offset stored with each date, with conversions between zones (`ToTimeZone`, `ToUTC`, `ToLocalTime`, `AssumeTimeZone`).
  - `SetDefaultTimezone()` for dates without a zone. They follow the system's zone by default.
  - An optional Herschel 4000-year rule (`USE_HERSCHEL_CORRECTION`).
- **JSON library** (RFC 8259):
  - A strict parser with line and column errors, options for comments, trailing commas and duplicate keys, and a nesting limit.
  - A Jackson-style node tree with `path()` chains, JSON Pointer, find methods, deep equality and copying.
  - Compact, pretty and ASCII-only output, with shortest-exact doubles regardless of locale.
  - `JSON` is now built by default.
- **SIP server** (RFC 3261):
  - A rebuilt protocol core with server transactions and their timers, CANCEL, the RFC 6026 accepted state, `received`/`rport`, CRLF keep-alives and compact headers.
  - `CBasicSipServer`, a working server over UDP and TCP.
  - The base class's `Run()` stays a stub for derived servers.
- **HTTP/1.1 server** (RFC 9110/9112):
  - A protocol core with keep-alive and pipelining, chunked bodies, `100-continue`, and request-smuggling defences.
  - `CBasicHttpServer`.
- **Secure sockets** (OpenSSL):
  - `CTlsContext`: certificates and keys from files or PEM text, TLS 1.2+, and verification of certificates and host names.
  - A working `CSecureSocket` for both clients and servers, safe for one thread to read while another writes, and free of SIGPIPE.
  - HTTPS (`CBasicHttpServer::setTlsContext`), and SIP over TLS on its own port (`CBasicSipServer::setTlsContext`).
  - Server Name Indication: a certificate per host name (`addHostContext`).
  - `sips:` requests must arrive over TLS (416 otherwise).
  - On Windows, the system's Trusted Root store is used for verification.
- **Virtual hosts** (`CHttpServer::AddVirtualHost`): separate handler factories per host name, including `*.example.com` patterns.
  - HTTPS requests whose host isn't the TLS host name get 421 Misdirected Request.
  - Requests carry how they arrived: `isSecureConnection()`, `getTlsServerName()` and `getHostName()`.
- **Blockchain:**
  - Blocks with binary entries committed by a Merkle root, and a fixed 100-byte header.
  - Full chain validation that reports which block fails and why, with the required difficulty set by the reader.
  - Mining that can be limited, cancelled or done on another thread.
  - Thread-safe chains.
  - Saving to and loading from JSON and binary.
  - `CMerkleTree`: RFC 6962 hashing with inclusion proofs and RFC 9162 verification.
- **SHA-256:** binary `Digest()`, `ToHex()`/`FromHex()`, and `updateData()` for inputs of any length. `StringDigest()` now takes a `const CString &`, so it also accepts temporaries.
- **Sockets:**
  - `ReadWithTimeout()`, `Accept( timeout )` and `Shutdown()`.
  - Waiting that works on every platform (`poll`, or `WSAPoll` on Windows).
  - `ReleaseHandle()`.
  - `IsSecure()` and `GetTlsServerName()`.
- **`BaseTypes/Platform.h`:** the operating system headers in the right order for each platform. **`Threading/ThreadPlatform.h`:** pthreads chosen from the platform flag alone.
- **Tests:** `TestSip`, `TestHttp`, `TestDate`, `TestJson`, `TestTls` and `TestBlockchain`, all run by `ctest`.
- **Docs:** this changelog, and a README with build, usage and testing sections.

### Fixed

- **`CDate`:**
  - **Calendar arithmetic:** across the Julian/Gregorian change and for BC years.
  - **Parsing:** formats that failed or read stale values.
  - **Time zone and timestamp conversions.**
  - **Julian Day on Windows:** a 32-bit overflow made it wrong for every modern date there.
  - **Before AD 1:** the last second of each day was placed in the next day.
  - **macOS:** the C library gave wrong offsets before 1900.
  - **Repeated hours:** the hour that happens twice when the clocks go back was resolved wrongly in zones east of UTC.
  - **Windows parsing:** every date pattern had a copy using `sscanf_s` without its buffer sizes. All 124 copies are gone.
- **Threading:**
  - Data races in `CThread` and `CMutex`.
  - macOS had no threading at all, because pthreads were only chosen for Linux.
  - `CSemaphore` used unnamed POSIX semaphores, which macOS doesn't support, and printed on every construction.
- **Memory:**
  - `CException` copies freed the same memory twice.
  - Copying an empty `CStringArray` leaked.
  - `CString::Pad` and `CIntArray` freed arrays with the wrong form of `delete`.
  - Copying a `CBlockchain` freed every block twice.
- **Sockets:**
  - Timed reads and accepts never timed out on Windows, so servers couldn't stop.
  - `SO_REUSEADDR` let a second server take over a port in use on Windows.
  - `CUDPSocket` didn't link on Windows.
  - Address handling bugs.
- **Structure packing:** `IASLib.h` changed it for every class, so applications and the library laid objects out differently.
- **Blockchain:**
  - Ambiguous hash input let data be changed while keeping the hash.
  - Data after a NUL byte wasn't hashed.
  - The genesis block was never hashed.
  - The difficulty wasn't part of the hash.
- **SHA-256:** messages over 512 MB hashed wrongly (32-bit length counter).
- **Windows C runtime crashes:** `strncpy_s` ended the program when text filled the buffer, and the time functions did the same for dates outside 1970–3000.
- **Unbounded formatting** in exceptions, and string stream truncation.
- **`TestXMLDocument`** crashed in release builds: two test functions never returned a value.

### Removed

- The legacy HTTP classes and `CServer` (see **Breaking changes**).
- `MerkleNode`, `CPojoNode` and `CJsonDocument`.
- The committed `build/` directory, a CMake cache from 2022 that stopped CMake configuring anywhere else. `build/` is now ignored.

### Build and CI

- **CMake:**
  - Threads are enabled properly.
  - SQLite is found with `find_package` (vcpkg on Windows).
  - OpenSSL is looked for on every platform; TLS is built when it's found (`IASLIB_TLS__`).
  - Winsock and `crypt32` are linked on Windows.
  - New `USE_TIMEZONE_ICU` option.
- **GitHub Actions:**
  - One workflow builds and tests on Linux (GCC and Clang), macOS (Apple Clang) and Windows (MSVC), for every push to `master` and `claude/` branches and every pull request.
  - Failed builds list their errors grouped by file, and hung tests time out.
  - It replaces two broken workflows: one used a retired GitHub action, and the other was copied from another project.
  - Pushing a `v*` tag creates a GitHub release, with that version's changelog section as its notes.
- **Sanitizers:** the suites run clean under AddressSanitizer/UndefinedBehaviorSanitizer, ThreadSanitizer and Valgrind.
