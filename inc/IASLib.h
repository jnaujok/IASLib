/*
 * IAS Library
 *
 *	This header file includes all of the various parts of the
 * Irene Adler Software Library based on various defined constants
 * that switch parts of the software "on" and "off" based on the
 * end user's intentions.
 *  Please note that at a bare minimum you must define the software
 * platform that is being built.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/2005
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB__
#define IASLIB__

#pragma pack(push, 1)

#ifndef IASLIB_WIN32__
#ifndef IASLIB_SUN__
#ifndef IASLIB_DEC__
#ifndef IASLIB_AIX__
#ifndef IASLIB_MAC__
#ifndef IASLIB_LINUX__
#pragma message( "ERROR! Platform type not defined for IAS Library!" )
#endif
#endif
#endif
#endif
#endif
#endif

#include "IASLibConfig.h"

    // Older compilers do not define the boolean type. If you get lots of errors
    // about "bool is undefined," then define the value "IASLIB_DEFINE_BOOL__" to
    // fix them.
#ifdef IASLIB_DEFINE_BOOL__
#include "BaseTypes/Bool.h"
#endif

//**************
//  BASE TYPES
//**************

    // Boolean support 
    // Define IASLIB_DEFINE_BOOL__ if you need to define Boolean values on your platform
#include "BaseTypes/Bool.h"

    // Base object for all library classes
#include "BaseTypes/Object.h"

    // String classes
#ifdef IASLIB_USE_DEEP_COPY_STRING__
#include "BaseTypes/String_Copy.h"
#else
#include "BaseTypes/String_.h"
#endif
#include "BaseTypes/StringTokenizer.h"

    // Date and Time
#include "BaseTypes/Date.h"

    // Data Blocks
#include "BaseTypes/DataBlock.h"

    // UUID Support
#include "BaseTypes/Uuid.h"

    // Variant Support
#include "BaseTypes/Variant.h"

//***************
//  BLOCKCHAIN
//***************

#include "Blockchain/Block.h"
#include "Blockchain/Blockchain.h"

//***************
//  COLLECTIONS
//***************

    // Arrays
#include "Collections/Array.h"
#include "Collections/IntArray.h"
#include "Collections/StringArray.h"
#include "Collections/SortedArray.h"

    // Hash
#include "Collections/Hash.h"

    // Linked List
#include "Collections/List.h"

    // Stack
#include "Collections/Stack.h"

    // Queue
#include "Collections/Queue.h"

    // LRU Cache
#include "Collections/Cache.h"
#include "Collections/CacheItem.h"

//***************
//  COMPRESSION
//***************

//*****************
//  CONFIGURATION
//*****************

#include "Configuration/Profile.h"
#include "Configuration/Settings.h"

#ifdef IASLIB_DATABASE__
//************
//  DATABASE
//************
#include "Database/BaseResult.h"
#include "Database/BulkCopyConnection.h"
#include "Database/Connection.h"
#include "Database/ConnectionArray.h"
#include "Database/Cursor.h"
#include "Database/Database.h"
#include "Database/OutParam.h"
#include "Database/ResultSet.h"
#include "Database/StoredProc.h"
    //-------------
    //  Meta-Data
    //-------------
#include "Database/Metadata/DataField.h"
#include "Database/Metadata/DataIndex.h"
#include "Database/Metadata/DataTable.h"

#ifdef IASLIB_DB_ORACLE__
    //--------------
    //  Oracle 7.3
    //--------------
#include "Database/Oracle/Ora_Connection.h"
#include "Database/Oracle/Ora_Cursor.h"
#include "Database/Oracle/Ora_Database.h"
#include "Database/Oracle/Ora_ResultSet.h"
#include "Database/Oracle/Ora_StoredProc.h"
#endif

#ifdef IASLIB_DB_SQLITE__
    //--------------
    //  SQLite 3.0
    //--------------
#include "Database/Sqlite/Sqlt_Connection.h"
#include "Database/Sqlite/Sqlt_Cursor.h"
#include "Database/Sqlite/Sqlt_Database.h"
#endif

#ifdef IASLIB_DB_SYBASE__
    //----------
    //  Sybase
    //----------
#include "Database/Sybase/Syb_BulkCopyConnection.h"
#include "Database/Sybase/Syb_Connection.h"
#include "Database/Sybase/Syb_Cursor.h"
#include "Database/Sybase/Syb_Database.h"
#include "Database/Sybase/Syb_ResultSet.h"
#include "Database/Sybase/Syb_StoredProc.h"
#endif

//*****************
//  DATABASE ENDS
//*****************
#endif // IASLIB_DATABASE__

//*********************
//  DYNAMIC LIBRARIES
//*********************

#include "DynamicLibraries/DynamicLibrary.h"

//**************
//  ENCRYPTION
//**************

#include "Encryption/Sha256.h"

//**************
//  EXCEPTIONS
//**************

#include "Exceptions/Exception.h"
#include "Exceptions/CollectionException.h"
#include "Exceptions/FileException.h"
#include "Exceptions/SocketException.h"
#include "Exceptions/StringException.h"
#include "Exceptions/XMLException.h"

#ifdef IASLIB_WIN32__
#include "Exceptions/WindowsSystemException.h"
#endif

//*********
//  FILES
//*********

#include "Files/Directory.h"
#include "Files/DirectoryEntry.h"
#include "Files/File.h"
#include "Files/FileStat.h"
#include "Files/RWFile.h"
#include "Files/MemMappedFile.h"

//***********
//  LOGGING
//***********

#include "Logging/LogSink.h"
#include "Logging/LogFile.h"
#include "Logging/RotatingLogFile.h"
#include "Logging/ConsoleLog.h"
#include "Logging/LogContext.h"
#include "Logging/ContextValue.h"

//******************
//  MEMORY MANAGER
//******************

#include "MemoryManager/MemoryManager.h"
#include "MemoryManager/MemoryFixedHeap.h"
#include "MemoryManager/MemoryBlock.h"

//********************
//  NETWORK SERVICES
//********************

#include "NetworkServices/Entity.h"
#include "NetworkServices/GenericClient.h"
#include "NetworkServices/GenericHandler.h"
#include "NetworkServices/GenericListener.h"
#include "NetworkServices/GenericRequest.h"
#include "NetworkServices/GenericResponse.h"
#include "NetworkServices/GenericServer.h"
#include "NetworkServices/Header.h"
#include "NetworkServices/HeaderList.h"
#include "NetworkServices/Mail.h"

#include "NetworkServices/Entities/NullEntity.h"
#include "NetworkServices/Entities/SdpEntity.h"
#include "NetworkServices/Entities/TextPlainEntity.h"

#include "NetworkServices/HTTP/HttpClient.h"
#include "NetworkServices/HTTP/HttpHeader.h"
#include "NetworkServices/HTTP/HttpHeaderList.h"
#include "NetworkServices/HTTP/HttpListener.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"
#include "NetworkServices/HTTP/HttpServer.h"

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpHandlerFactory.h"
#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"
#include "NetworkServices/HTTP/Handlers/HttpGetHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpHeadHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpPostHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpOptionsHandler.h"

#include "NetworkServices/SIP/SipClient.h"
#include "NetworkServices/SIP/SipHeader.h"
#include "NetworkServices/SIP/SipHeaderList.h"
#include "NetworkServices/SIP/SipListener.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"
#include "NetworkServices/SIP/SipServer.h"

#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "NetworkServices/SIP/Handlers/SipHandlerFactory.h"
#include "NetworkServices/SIP/Handlers/BaseSipHandlerFactory.h"
#include "NetworkServices/SIP/Handlers/SipAckHandler.h"
#include "NetworkServices/SIP/Handlers/SipByeHandler.h"
#include "NetworkServices/SIP/Handlers/SipCancelHandler.h"
#include "NetworkServices/SIP/Handlers/SipInviteHandler.h"
#include "NetworkServices/SIP/Handlers/SipOptionsHandler.h"
#include "NetworkServices/SIP/Handlers/SipRegisterHandler.h"

//*************
//  RESOURCES
//*************

#include "Resources/Resource.h"
#include "Resources/ResourceDirectory.h"
#include "Resources/ResourceFile.h"
#include "Resources/ResourceFileNode.h"
#include "Resources/ResourceIndex.h"

//***********
//  SOCKETS
//***********
#include "Sockets/ClientSocket.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/SecureSocket.h"
#include "Sockets/SecureClientSocket.h"
#include "Sockets/Socket.h"
#include "Sockets/InternetAddress.h"
#include "Sockets/UDPSocket.h"

//*********
//  STATS
//*********

#ifdef IASLIB_STATS__
#include "Stats/Histogram.h"
#endif

//**********
//  STATUS
//**********

#include "Status/CPUUsage.h"

//***********
//  STREAMS
//***********

#include "Streams/Stream.h"
#include "Streams/FileStream.h"
#include "Streams/NullStream.h"
#include "Streams/SocketStream.h"
#include "Streams/StringStream.h"
#include "Streams/BufferedStream.h"

//*************
//  THREADING
//*************

#include "Threading/Mutex.h"
#include "Threading/Semaphore.h"
#include "Threading/ServerThread.h"
#include "Threading/Thread.h"
#include "Threading/ThreadMonitor.h"
#include "Threading/PooledThread.h"
#include "Threading/ThreadPool.h"
#include "Threading/ThreadTask.h"

//*******
//  XML
//*******

#include "XML/XMLDocument.h"
#include "XML/XMLElement.h"
#include "XML/XMLIndex.h"
#include "XML/XMLProperty.h"

#pragma pack(pop)

#endif // IASLIB__
