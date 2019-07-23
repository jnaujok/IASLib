############################################################################
#
# AIX Makefile for the Irene Adler Software Library (IBM/AIX Platforms only)
#
#	To use this makefile, you need to set the following environment
# variables:
#
#	SYBASE		Set to the Sybase installation directory.
#       ORACLE_HOME     Set to the Oracle installation directory.
#
#	Please note that this make file sets the "SYBPLATFORM" variable to
# the correct setting for an AIX server.
#
#	The calling syntax for this makefile is:
#
#    make -f makefile_aix [target]
#
#   $AUTHOR$
#   $LOG$
#
#############################################################################

SYBPLATFORM = linux

#############################################################################
#
# Directories and search paths.
#
#############################################################################

MAKE_DIR = .
APPDIR  =  ../..

LIBSDIR =   $(APPDIR)/lib/linux
TESTDIR =   $(APPDIR)/test

OBJDIR  =   $(APPDIR)/obj

BASEDIR =   $(APPDIR)/src/BaseTypes
XCPTDIR =   $(APPDIR)/src/Exceptions
COLDIR  =   $(APPDIR)/src/Collections
CONFDIR =   $(APPDIR)/src/Configuration
DBDIR   =   $(APPDIR)/src/Database
METADIR =   $(DBDIR)/Metadata
ORADIR  =   $(DBDIR)/Oracle
SYBDIR  =   $(DBDIR)/Sybase
SQLTDIR =   $(DBDIR)/Sqlite
DYNLDIR =   $(APPDIR)/src/DynamicLibraries
FILEDIR =   $(APPDIR)/src/Files
JSONDIR =   $(APPDIR)/src/JSON
NETDIR  =   $(APPDIR)/src/NetworkServices
SOCKDIR =   $(APPDIR)/src/Sockets
STATDIR =   $(APPDIR)/src/Stats
STUSDIR =   $(APPDIR)/src/Status
STRMDIR =   $(APPDIR)/src/Streams
THRDDIR =   $(APPDIR)/src/Threading
XMLDIR  =   $(APPDIR)/src/XML

BASEINC =   $(APPDIR)/inc/BaseTypes
XCPTINC =   $(APPDIR)/inc/Exceptions
COLINC  =   $(APPDIR)/inc/Collections
CONFINC =   $(APPDIR)/inc/Configuration
DBINC   =   $(APPDIR)/inc/Database
METAINC =   $(DBINC)/Metadata
ORAINC  =   $(DBINC)/Oracle
SYBINC  =   $(DBINC)/Sybase
SQLTINC =   $(DBINC)/Sqlite
DYNLINC =   $(APPDIR)/inc/DynamicLibraries
FILEINC =   $(APPDIR)/inc/Files
JSONINC =   $(APPDIR)/inc/JSON
NETINC  =   $(APPDIR)/inc/NetworkServices
SOCKINC =   $(APPDIR)/inc/Sockets
STATINC =   $(APPDIR)/inc/Stats
STUSINC =   $(APPDIR)/inc/Status
STRMINC =   $(APPDIR)/inc/Streams
THRDINC =   $(APPDIR)/inc/Threading
XMLINC  =   $(APPDIR)/inc/XML

ALLDIRS = $(BASEDIR) $(XCPTDIR) $(COLDIR)  $(CONFDIR) $(DBDIR) \
          $(METADIR) $(ORADIR)  $(SYBDIR)  $(SQLTDIR) $(DYNLDIR) \
          $(JSONDIR) $(FILEDIR) $(NETDIR)  $(SOCKDIR) $(STATDIR) \
		  $(STUSDIR) $(STRMDIR) $(THRDDIR) $(XMLDIR)

ALLINCS = -I$(BASEINC) -I$(XCPTINC) -I$(COLINC)  -I$(CONFINC) -I$(DBINC)   \
          -I$(METAINC) -I$(ORAINC)  -I$(SYBINC)  -I$(SQLTINC) -I$(DYNLINC) \
          -I$(JSONINC) -I$(FILEINC) -I$(NETINC)  -I$(SOCKINC) -I$(STATINC) \
		  -I$(STUSINC) -I$(STRMINC) -I$(THRDINC) -I$(XMLINC)

#SYBHOME   = $(SYBASE)/include           # Sybase include file directory
#ORAHOME   = $(ORACLE_HOME)/rdbms/demo   # Oracle include file directory
#ORANETDIR = $(ORACLE_HOME)/rdbms/public   # Oracle include file directory

#INCDIR    = -I$(SYBDIR) -I$(ORADIR) -I$(ORANETDIR)
#############################################################################
#
# Output Objects.
#
#############################################################################

TEST_PROGS =
ALL_LIBS   = $(LIBSDIR)/libIASLib.a
ALL_APPS   =

#
# Modify the following line to use debug/nondebug libraries
#
#SYB_LIBDIR   = $(SYBASE)/devlib     # for debug library versions
#SYB_LIBDIR    = $(SYBASE)/lib       # Sybase library directory
#ORA_LIBDIR    = $(ORACLE_HOME)/rdbms/lib       # Oracle library directory

#LIBDIR = -L$(SYB_LIBDIR) -L$(ORA_LIBDIR)

# Specify the include and library search paths for the compiler
INCLUDE   = -I/usr/local/include -I/usr/include  $(ALLINCS)

#OCISHAREDLIBS = -ldbtools8 -lknlopt

LIBPATH   = $(LIBDIR) -lm -ldl

##############################################################################
#
# Macros for compile/link lines
#
##############################################################################

#
# Specify the compile-time defines.
#
#DEFS 	= -DDEBUG -D__DEBUG__ -D$(SYBPLATFORM)=1 -DGENLIB_SUN_
# non-debug version
DEFS    = -Dlinux \
		  -DIASLIB_LINUX__ \
		  -DIASLIB_NETWORKING__ \
		  -DIASLIB_MULTI_THREADED__ -DIASLIB_PTHREAD__ \
		  -DIASLIB_RTTI__ \
		  -DIASLIB_DATABASE__ -DIASLIB_DB_SQLITE__ \
		  -DIASLIB_STD_STREAM_SUPPORT__

#
# Specify the compile-time options.
#
OPTIONS = -ggdb -fvar-tracking -pg -D__THREADED__ -Wall -Wextra -D_REENTRANT
#OPTIONS_NO_THREAD = -g0 -pg
# non-debug version
#OPTIONS = -O3 -m32 -D_REENTRANT

#
# Define the compiler command and compile flags.
#
# `sybopts.sh_sun compile` echoes a platform-specific command to invoke the C
# compiler. You can replace this invocation with the comand to invoke your
# preferred compiler if you like.
#
COMPILE = g++ $(OPTIONS) $(INCLUDE) $(DEFS)

##############################################################################
#
# The targets and how to build them.
#
##############################################################################

##########################################
#  Headers
#
BASEHEADERS = $(BASEINC)/Object.h $(BASEINC)/String_.h $(BASEINC)/StringTokenizer.h \
              $(BASEINC)/Bool.h $(BASEINC)/StringStub.h $(BASEINC)/Date.h \
	      $(BASEINC)/DataBlock.h $(BASEINC)/String_Copy.h

COLHEADERS  = $(COLINC)/Collection.h $(COLINC)/Array.h $(COLINC)/Cache.h \
	      $(COLINC)/CacheItem.h $(COLINC)/Hash.h $(COLINC)/HashBucket.h \
              $(COLINC)/HashSlat.h $(COLINC)/IntArray.h $(COLINC)/Iterator.h \
              $(COLINC)/List.h $(COLINC)/Queue.h $(COLINC)/SortedArray.h \
              $(COLINC)/SparseArray.h $(COLINC)/Stack.h $(COLINC)/StringArray.h

XCPTHEADERS = $(XCPTINC)/Exception.h $(XCPTINC)/CollectionException.h \
              $(XCPTINC)/FileException.h $(XCPTINC)/SocketException.h \
              $(XCPTINC)/StringException.h $(XCPTINC)/XMLException.h \
			  $(XCPTINC)/StreamException.h $(XCPTINC)/ThreadException.h

CONFHEADERS = $(CONFINC)/Profile.h $(CONFINC)/Settings.h

DBHEADERS   = $(DBINC)/BaseResult.h $(DBINC)/BulkCopyConnection.h $(DBINC)/ConnectionArray.h \
              $(DBINC)/Connection.h $(DBINC)/Cursor.h $(DBINC)/Database.h \
              $(DBINC)/OutParam.h $(DBINC)/ResultSet.h $(DBINC)/StoredProc.h

METAHEADERS = $(METAINC)/DataField.h $(METAINC)/DataIndex.h \
              $(METAINC)/DataTable.h

ORAHEADERS  = $(ORAINC)/Ora_Connection.h $(ORAINC)/Ora_Cursor.h \
              $(ORAINC)/Ora_Database.h $(ORAINC)/Ora_ResultSet.h \
              $(ORAINC)/Ora_StoredProc.h

SYBHEADERS  = $(SYBINC)/Syb_BulkCopyConnection.h $(SYBINC)/Syb_Connection.h \
              $(SYBINC)/Syb_Cursor.h $(SYBINC)/Syb_Database.h \
              $(SYBINC)/Syb_ResultSet.h $(SYBINC)/Syb_StoredProc.h

SQLTHEADERS = $(SQLTINC)/Sqlt_Connection.h $(SQLTINC)/Sqlt_Cursor.h \
              $(SQLTINC)/Sqlt_Database.h

DYNLHEADERS = $(DYNLINC)/DynamicLibrary.h

JSONHEADERS = $(JSONINC)/JsonNode.h $(JSONINC)/ObjectNode.h $(JSONINC)/ArrayNode.h \
              $(JSONINC)/ValueNode.h $(JSONINC)/MissingNode.h

FILEHEADERS = $(FILEINC)/File.h $(FILEINC)/FileStat.h $(FILEINC)/Directory.h \
              $(FILEINC)/DirectoryEntry.h $(FILEINC)/RWFile.h

NETHEADERS  = $(NETINC)/HTTPClient.h $(NETINC)/HTTPHeader.h \
              $(NETINC)/HTTPHeaderList.h $(NETINC)/HTTPResponse.h \
              $(NETINC)/HTTPServer.h $(NETINC)/Mail.h \
              $(NETINC)/Server.h

SOCKHEADERS = $(SOCKINC)/Socket.h $(SOCKINC)/ClientSocket.h $(SOCKINC)/ServerSocket.h \
              $(SOCKINC)/RemoteAddress.h  $(SOCKINC)/ClientSocketSource.h $(SOCKINC)/FixedConnectionPool.h \
			  $(SOCKINC)/SecureClientSocket.h $(SOCKINC)/SecureSocket.h $(SOCKINC)/SocketConfig.h \
			  $(SOCKINC)/UDPSocket.h $(SOCKINC)/SafeAddressInfo.h

STATHEADERS = $(STATINC)/Histogram.h

STUSHEADERS = $(STUSINC)/CPUUsage.h

STRMHEADERS = $(STRMINC)/Stream.h $(STRMINC)/FileStream.h $(STRMINC)/NullStream.h \
              $(STRMINC)/SocketStream.h $(STRMINC)/StringStream.h $(STRMINC)/BufferedStream.h

THRDHEADERS = $(THRDINC)/Mutex.h $(THRDINC)/Semaphore.h $(THRDINC)/Thread.h \
              $(THRDINC)/ThreadMonitor.h $(THRDINC)/ServerThread.h \
			  $(THRDINC)/ThreadPool.h $(THRDINC)/PooledThread.h \
			  $(THRDINC)/ThreadTask.h

XMLHEADERS  = $(XMLINC)/XMLDocument.h $(XMLINC)/XMLElement.h \
              $(XMLINC)/XMLIndex.h $(XMLINC)/XMLParser.h \
              $(XMLINC)/XMLProperty.h

############################################
#  Objects
#
BASEOBJECTS = $(OBJDIR)/String_.o $(OBJDIR)/StringTokenizer.o $(OBJDIR)/Object.o \
              $(OBJDIR)/StringStub.o $(OBJDIR)/Date.o $(OBJDIR)/DataBlock.o \
              $(OBJDIR)/String_Copy.o

COLOBJECTS  = $(OBJDIR)/Collection.o $(OBJDIR)/Array.o $(OBJDIR)/Cache.o \
	          $(OBJDIR)/CacheItem.o $(OBJDIR)/Hash.o $(OBJDIR)/HashBucket.o \
              $(OBJDIR)/HashSlat.o $(OBJDIR)/IntArray.o \
              $(OBJDIR)/List.o $(OBJDIR)/Queue.o $(OBJDIR)/SortedArray.o \
              $(OBJDIR)/SparseArray.o $(OBJDIR)/Stack.o $(OBJDIR)/StringArray.o

CONFOBJECTS = $(OBJDIR)/Profile.o $(OBJDIR)/Settings.o

DBOBJECTS   = $(OBJDIR)/BaseResult.o $(OBJDIR)/ConnectionArray.o \
              $(OBJDIR)/Database.o

METAOBJECTS = $(OBJDIR)/DataField.o $(OBJDIR)/DataIndex.o \
              $(OBJDIR)/DataTable.o

ORAOBJECTS  = $(OBJDIR)/Ora_Connection.o $(OBJDIR)/Ora_Cursor.o \
              $(OBJDIR)/Ora_Database.o $(OBJDIR)/Ora_ResultSet.o \
              $(OBJDIR)/Ora_StoredProc.o

SYBOBJECTS  = $(OBJDIR)/Syb_BulkCopyConnection.o $(OBJDIR)/Syb_Connection.o \
              $(OBJDIR)/Syb_Cursor.o $(OBJDIR)/Syb_Database.o \
              $(OBJDIR)/Syb_ResultSet.o $(OBJDIR)/Syb_StoredProc.o

SQLTOBJECTS = $(OBJDIR)/Sqlt_Connection.o $(OBJDIR)/Sqlt_Cursor.o \
              $(OBJDIR)/Sqlt_Database.o

DYNLOBJECTS = $(OBJDIR)/DynamicLibrary.o

XCPTOBJECTS = $(OBJDIR)/Exception.o $(OBJDIR)/CollectionException.o \
              $(OBJDIR)/FileException.o $(OBJDIR)/SocketException.o \
              $(OBJDIR)/StringException.o $(OBJDIR)/XMLException.o \
			  $(OBJDIR)/StreamException.o $(OBJDIR)/ThreadException.o

FILEOBJECTS = $(OBJDIR)/Directory.o $(OBJDIR)/DirectoryEntry.o $(OBJDIR)/File.o \
	          $(OBJDIR)/FileStat.o $(OBJDIR)/RWFile.o

JSONOBJECTS = $(OBJDIR)/JsonNode.o $(OBJDIR)/ObjectNode.o $(OBJDIR)/ArrayNode.o \
              $(OBJDIR)/ValueNode.o $(OBJDIR)/MissingNode.o

NETOBJECTS  = $(OBJDIR)/HTTPClient.o $(OBJDIR)/HTTPHeader.o \
              $(OBJDIR)/HTTPHeaderList.o $(OBJDIR)/Mail.o

SOCKOBJECTS = $(OBJDIR)/ClientSocket.o $(OBJDIR)/ServerSocket.o $(OBJDIR)/Socket.o \
              $(OBJDIR)/RemoteAddress.o $(OBJDIR)/SocketConfig.o \
			  $(OBJDIR)/UDPSocket.o $(OBJDIR)/SafeAddressInfo.o

#$(OBJDIR)/ClientSocketSource.o $(OBJDIR)/FixedConnectionPool.o
#			  $(OBJDIR)/SecureClientSocket.o $(OBJDIR)/SecureSocket.o

STATOBJECTS = $(OBJDIR)/Histogram.o

STUSOBJECTS = $(OBJDIR)/CPUUsage.o

STRMOBJECTS = $(OBJDIR)/Stream.o $(OBJDIR)/FileStream.o $(OBJDIR)/NullStream.o \
              $(OBJDIR)/SocketStream.o $(OBJDIR)/StringStream.o $(OBJDIR)/BufferedStream.o

THRDOBJECTS = $(OBJDIR)/Mutex.o $(OBJDIR)/Semaphore.o $(OBJDIR)/Thread.o \
              $(OBJDIR)/ThreadMonitor.o $(OBJDIR)/ServerThread.o $(OBJDIR)/ThreadPool.o \
			  $(OBJDIR)/PooledThread.o

XMLOBJECTS  = $(OBJDIR)/XMLDocument.o $(OBJDIR)/XMLChunk.o $(OBJDIR)/XMLData.o \
				  $(OBJDIR)/XMLElement.o $(OBJDIR)/XMLIndex.o $(OBJDIR)/XMLProperty.o

ALL_OBJS    = $(BASEOBJECTS) $(COLOBJECTS)  $(CONFOBJECTS) $(XCPTOBJECTS) \
              $(DBOBJECTS)   $(ORAOBJECTS)  $(SYBOBJECTS)  $(SQLTOBJECTS) \
              $(DYNLOBJECTS) $(JSONOBJECTS) $(FILEOBJECTS) $(NETOBJECTS)  \
			  $(SOCKOBJECTS) $(STATOBJECTS) $(STUSOBJECTS) $(STRMOBJECTS) \
			  $(THRDOBJECTS) $(XMLOBJECTS)

#
# 'make all' builds all of the components
#
all: $(ALL_OBJS) $(ALL_LIBS) $(ALL_APPS);

$(LIBSDIR)/libIASLib.a: $(ALL_OBJS);
	ld -r -o $(LIBSDIR)/libIASLib.a $(ALL_OBJS)

#
## GENERAL LIBRARY
#

$(OBJDIR)/Object.o:  $(BASEHEADERS) $(BASEDIR)/Object.cpp;
	$(COMPILE) -c $(BASEDIR)/Object.cpp -o $(OBJDIR)/Object.o

$(OBJDIR)/String_.o:  $(BASEHEADERS) $(BASEDIR)/String_.cpp;
	$(COMPILE) -c $(BASEDIR)/String_.cpp -o $(OBJDIR)/String_.o

$(OBJDIR)/StringStub.o:  $(BASEHEADERS) $(BASEDIR)/StringStub.cpp;
	$(COMPILE) -c $(BASEDIR)/StringStub.cpp -o $(OBJDIR)/StringStub.o

$(OBJDIR)/StringTokenizer.o:  $(BASEHEADERS) $(BASEDIR)/StringTokenizer.cpp;
	$(COMPILE) -c $(BASEDIR)/StringTokenizer.cpp -o $(OBJDIR)/StringTokenizer.o

$(OBJDIR)/Date.o:  $(BASEHEADERS) $(BASEDIR)/Date.cpp;
	$(COMPILE) -c $(BASEDIR)/Date.cpp -o $(OBJDIR)/Date.o

$(OBJDIR)/DataBlock.o:  $(BASEHEADERS) $(BASEDIR)/DataBlock.cpp;
	$(COMPILE) -c $(BASEDIR)/DataBlock.cpp -o $(OBJDIR)/DataBlock.o

$(OBJDIR)/String_Copy.o:  $(BASEHEADERS) $(BASEDIR)/String_Copy.cpp;
	$(COMPILE) -c $(BASEDIR)/String_Copy.cpp -o $(OBJDIR)/String_Copy.o

#
##  COLLECTION LIBRARY
#

$(OBJDIR)/Collection.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Collection.cpp;
	$(COMPILE) -c $(COLDIR)/Collection.cpp -o $(OBJDIR)/Collection.o

$(OBJDIR)/Array.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Array.cpp;
	$(COMPILE) -c $(COLDIR)/Array.cpp -o $(OBJDIR)/Array.o

$(OBJDIR)/Cache.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Cache.cpp;
	$(COMPILE) -c $(COLDIR)/Cache.cpp -o $(OBJDIR)/Cache.o

$(OBJDIR)/CacheItem.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/CacheItem.cpp;
	$(COMPILE) -c $(COLDIR)/CacheItem.cpp -o $(OBJDIR)/CacheItem.o

$(OBJDIR)/Hash.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Hash.cpp;
	$(COMPILE) -c $(COLDIR)/Hash.cpp -o $(OBJDIR)/Hash.o

$(OBJDIR)/HashBucket.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/HashBucket.cpp;
	$(COMPILE) -c $(COLDIR)/HashBucket.cpp -o $(OBJDIR)/HashBucket.o

$(OBJDIR)/HashSlat.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/HashSlat.cpp;
	$(COMPILE) -c $(COLDIR)/HashSlat.cpp -o $(OBJDIR)/HashSlat.o

$(OBJDIR)/IntArray.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/IntArray.cpp;
	$(COMPILE) -c $(COLDIR)/IntArray.cpp -o $(OBJDIR)/IntArray.o

$(OBJDIR)/List.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/List.cpp;
	$(COMPILE) -c $(COLDIR)/List.cpp -o $(OBJDIR)/List.o

$(OBJDIR)/Queue.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Queue.cpp;
	$(COMPILE) -c $(COLDIR)/Queue.cpp -o $(OBJDIR)/Queue.o

$(OBJDIR)/SortedArray.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/SortedArray.cpp;
	$(COMPILE) -c $(COLDIR)/SortedArray.cpp -o $(OBJDIR)/SortedArray.o

$(OBJDIR)/SparseArray.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/SparseArray.cpp;
	$(COMPILE) -c $(COLDIR)/SparseArray.cpp -o $(OBJDIR)/SparseArray.o

$(OBJDIR)/Stack.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/Stack.cpp;
	$(COMPILE) -c $(COLDIR)/Stack.cpp -o $(OBJDIR)/Stack.o

$(OBJDIR)/StringArray.o:  $(BASEHEADERS) $(COLHEADERS) $(COLDIR)/StringArray.cpp;
	$(COMPILE) -c $(COLDIR)/StringArray.cpp -o $(OBJDIR)/StringArray.o

#
##  EXCEPTION LIBRARY
#

$(OBJDIR)/Exception.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/Exception.cpp;
	$(COMPILE) -c $(XCPTDIR)/Exception.cpp -o $(OBJDIR)/Exception.o

$(OBJDIR)/CollectionException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/CollectionException.cpp;
	$(COMPILE) -c $(XCPTDIR)/CollectionException.cpp -o $(OBJDIR)/CollectionException.o

$(OBJDIR)/FileException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/FileException.cpp;
	$(COMPILE) -c $(XCPTDIR)/FileException.cpp -o $(OBJDIR)/FileException.o

$(OBJDIR)/SocketException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/SocketException.cpp;
	$(COMPILE) -c $(XCPTDIR)/SocketException.cpp -o $(OBJDIR)/SocketException.o

$(OBJDIR)/StringException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/StringException.cpp;
	$(COMPILE) -c $(XCPTDIR)/StringException.cpp -o $(OBJDIR)/StringException.o

$(OBJDIR)/XMLException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/XMLException.cpp;
	$(COMPILE) -c $(XCPTDIR)/XMLException.cpp -o $(OBJDIR)/XMLException.o

$(OBJDIR)/StreamException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/StreamException.cpp;
	$(COMPILE) -c $(XCPTDIR)/StreamException.cpp -o $(OBJDIR)/StreamException.o

$(OBJDIR)/ThreadException.o: $(BASEHEADERS) $(XCPTHEADERS) $(XCPTDIR)/ThreadException.cpp;
	$(COMPILE) -c $(XCPTDIR)/ThreadException.cpp -o $(OBJDIR)/ThreadException.o

#
## CONFIGURATION OBJECTS
#

$(OBJDIR)/Profile.o: $(BASEHEADERS) $(CONFHEADERS) $(CONFDIR)/Profile.cpp;
	$(COMPILE) -c $(CONFDIR)/Profile.cpp -o $(OBJDIR)/Profile.o

$(OBJDIR)/Settings.o: $(BASEHEADERS) $(CONFHEADERS) $(CONFDIR)/Settings.cpp;
	$(COMPILE) -c $(CONFDIR)/Settings.cpp -o $(OBJDIR)/Settings.o

#
## DATABASE OBJECTS
#

$(OBJDIR)/BaseResult.o: $(BASEHEADERS) $(DBHEADERS) $(DBDIR)/BaseResult.cpp;
	$(COMPILE) -c $(DBDIR)/BaseResult.cpp -o $(OBJDIR)/BaseResult.o

$(OBJDIR)/ConnectionArray.o: $(BASEHEADERS) $(DBHEADERS) $(DBDIR)/ConnectionArray.cpp;
	$(COMPILE) -c $(DBDIR)/ConnectionArray.cpp -o $(OBJDIR)/ConnectionArray.o

$(OBJDIR)/Database.o: $(BASEHEADERS) $(DBHEADERS) $(DBDIR)/Database.cpp;
	$(COMPILE) -c $(DBDIR)/Database.cpp -o $(OBJDIR)/Database.o

#
## (DATABASE) METADATA OBJECTS
#

$(OBJDIR)/DataField.o: $(BASEHEADERS) $(DBHEADERS) $(METAHEADERS) $(METADIR)/DataField.cpp;
	$(COMPILE) -c $(METADIR)/DataField.cpp -o $(OBJDIR)/DataField.o

$(OBJDIR)/DataIndex.o: $(BASEHEADERS) $(DBHEADERS) $(METAHEADERS) $(METADIR)/DataIndex.cpp;
	$(COMPILE) -c $(METADIR)/DataIndex.cpp -o $(OBJDIR)/DataIndex.o

$(OBJDIR)/DataTable.o: $(BASEHEADERS) $(DBHEADERS) $(METAHEADERS) $(METADIR)/DataTable.cpp;
	$(COMPILE) -c $(METADIR)/DataTable.cpp -o $(OBJDIR)/DataTable.o

#
## (DATABASE) ORACLE OBJECTS
#

$(OBJDIR)/Ora_Connection.o: $(BASEHEADERS) $(DBHEADERS) $(ORAHEADERS) $(ORADIR)/Ora_Connection.cpp;
	$(COMPILE) -c $(ORADIR)/Ora_Connection.cpp -o $(OBJDIR)/Ora_Connection.o

$(OBJDIR)/Ora_Cursor.o: $(BASEHEADERS) $(DBHEADERS) $(ORAHEADERS) $(ORADIR)/Ora_Cursor.cpp;
	$(COMPILE) -c $(ORADIR)/Ora_Cursor.cpp -o $(OBJDIR)/Ora_Cursor.o

$(OBJDIR)/Ora_Database.o: $(BASEHEADERS) $(DBHEADERS) $(ORAHEADERS) $(ORADIR)/Ora_Database.cpp;
	$(COMPILE) -c $(ORADIR)/Ora_Database.cpp -o $(OBJDIR)/Ora_Database.o

$(OBJDIR)/Ora_ResultSet.o: $(BASEHEADERS) $(DBHEADERS) $(ORAHEADERS) $(ORADIR)/Ora_ResultSet.cpp;
	$(COMPILE) -c $(ORADIR)/Ora_ResultSet.cpp -o $(OBJDIR)/Ora_ResultSet.o

$(OBJDIR)/Ora_StoredProc.o: $(BASEHEADERS) $(DBHEADERS) $(ORAHEADERS) $(ORADIR)/Ora_StoredProc.cpp;
	$(COMPILE) -c $(ORADIR)/Ora_StoredProc.cpp -o $(OBJDIR)/Ora_StoredProc.o

$(OBJDIR)/Syb_BulkCopyConnection.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_BulkCopyConnection.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_BulkCopyConnection.cpp -o $(OBJDIR)/Syb_BulkCopyConnection.o

#
## (DATABASE) SYBASE OBJECTS
#

$(OBJDIR)/Syb_Connection.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_Connection.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_Connection.cpp -o $(OBJDIR)/Syb_Connection.o

$(OBJDIR)/Syb_Cursor.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_Cursor.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_Cursor.cpp -o $(OBJDIR)/Syb_Cursor.o

$(OBJDIR)/Syb_Database.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_Database.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_Database.cpp -o $(OBJDIR)/Syb_Database.o

$(OBJDIR)/Syb_ResultSet.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_ResultSet.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_ResultSet.cpp -o $(OBJDIR)/Syb_ResultSet.o

$(OBJDIR)/Syb_StoredProc.o: $(BASEHEADERS) $(DBHEADERS) $(SYBHEADERS) $(SYBDIR)/Syb_StoredProc.cpp;
	$(COMPILE) -c $(SYBDIR)/Syb_StoredProc.cpp -o $(OBJDIR)/Syb_StoredProc.o

#
## (DATABASE) SQLITE OBJECTS
#

$(OBJDIR)/Sqlt_Connection.o: $(BASEHEADERS) $(DBHEADERS) $(SQLTHEADERS) $(SQLTDIR)/Sqlt_Connection.cpp;
	$(COMPILE) -c $(SQLTDIR)/Sqlt_Connection.cpp -o $(OBJDIR)/Sqlt_Connection.o

$(OBJDIR)/Sqlt_Cursor.o: $(BASEHEADERS) $(DBHEADERS) $(SQLTHEADERS) $(SQLTDIR)/Sqlt_Cursor.cpp;
	$(COMPILE) -c $(SQLTDIR)/Sqlt_Cursor.cpp -o $(OBJDIR)/Sqlt_Cursor.o

$(OBJDIR)/Sqlt_Database.o: $(BASEHEADERS) $(DBHEADERS) $(SQLTHEADERS) $(SQLTDIR)/Sqlt_Database.cpp;
	$(COMPILE) -c $(SQLTDIR)/Sqlt_Database.cpp -o $(OBJDIR)/Sqlt_Database.o

#
## DYNAMIC LIBRARY OBJECTS
#

$(OBJDIR)/DynamicLibrary.o: $(BASEHEADERS) $(DYNLHEADERS) $(DYNLDIR)/DynamicLibrary.cpp;
	$(COMPILE) -c $(DYNLDIR)/DynamicLibrary.cpp -o $(OBJDIR)/DynamicLibrary.o

#
## JSON OBJECTS
#

$(OBJDIR)/JsonNode.o: $(BASEHEADERS) $(JSONHEADERS) $(JSONDIR)/JsonNode.cpp;
	$(COMPILE) -c $(JSONDIR)/JsonNode.cpp -o $(OBJDIR)/JsonNode.o

$(OBJDIR)/ObjectNode.o: $(BASEHEADERS) $(JSONHEADERS) $(JSONDIR)/ObjectNode.cpp;
	$(COMPILE) -c $(JSONDIR)/ObjectNode.cpp -o $(OBJDIR)/ObjectNode.o

$(OBJDIR)/ArrayNode.o: $(BASEHEADERS) $(JSONHEADERS) $(JSONDIR)/JsonNode.cpp;
	$(COMPILE) -c $(JSONDIR)/ArrayNode.cpp -o $(OBJDIR)/ArrayNode.o

$(OBJDIR)/ValueNode.o: $(BASEHEADERS) $(JSONHEADERS) $(JSONDIR)/ValueNode.cpp;
	$(COMPILE) -c $(JSONDIR)/ValueNode.cpp -o $(OBJDIR)/ValueNode.o

$(OBJDIR)/MissingNode.o: $(BASEHEADERS) $(JSONHEADERS) $(JSONDIR)/MissingNode.cpp;
	$(COMPILE) -c $(JSONDIR)/MissingNode.cpp -o $(OBJDIR)/MissingNode.o


#
## FILE OBJECTS
#

$(OBJDIR)/Directory.o: $(BASEHEADERS) $(FILEHEADERS) $(FILEDIR)/Directory.cpp;
	$(COMPILE) -c $(FILEDIR)/Directory.cpp -o $(OBJDIR)/Directory.o

$(OBJDIR)/DirectoryEntry.o: $(BASEHEADERS) $(FILEHEADERS) $(FILEDIR)/DirectoryEntry.cpp;
	$(COMPILE) -c $(FILEDIR)/DirectoryEntry.cpp -o $(OBJDIR)/DirectoryEntry.o

$(OBJDIR)/File.o: $(BASEHEADERS) $(FILEHEADERS) $(FILEDIR)/File.cpp;
	$(COMPILE) -c $(FILEDIR)/File.cpp -o $(OBJDIR)/File.o

$(OBJDIR)/FileStat.o: $(BASEHEADERS) $(FILEHEADERS) $(FILEDIR)/FileStat.cpp;
	$(COMPILE) -c $(FILEDIR)/FileStat.cpp -o $(OBJDIR)/FileStat.o

$(OBJDIR)/RWFile.o: $(BASEHEADERS) $(FILEHEADERS) $(FILEDIR)/RWFile.cpp;
	$(COMPILE) -c $(FILEDIR)/RWFile.cpp -o $(OBJDIR)/RWFile.o

#
## NETWORK SERVICES OBJECTS
#

$(OBJDIR)/HTTPClient.o: $(BASEHEADERS) $(NETHEADERS) $(NETDIR)/HTTPClient.cpp;
	$(COMPILE) -c $(NETDIR)/HTTPClient.cpp -o $(OBJDIR)/HTTPClient.o

$(OBJDIR)/HTTPHeader.o: $(BASEHEADERS) $(NETHEADERS) $(NETDIR)/HTTPHeader.cpp;
	$(COMPILE) -c $(NETDIR)/HTTPHeader.cpp -o $(OBJDIR)/HTTPHeader.o

$(OBJDIR)/HTTPHeaderList.o: $(BASEHEADERS) $(NETHEADERS) $(NETDIR)/HTTPHeaderList.cpp;
	$(COMPILE) -c $(NETDIR)/HTTPHeaderList.cpp -o $(OBJDIR)/HTTPHeaderList.o

$(OBJDIR)/Mail.o: $(BASEHEADERS) $(NETHEADERS) $(NETDIR)/Mail.cpp;
	$(COMPILE) -c $(NETDIR)/Mail.cpp -o $(OBJDIR)/Mail.o

#
## SOCKET OBJECTS
#

$(OBJDIR)/Socket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/Socket.cpp;
	$(COMPILE) -c $(SOCKDIR)/Socket.cpp -o $(OBJDIR)/Socket.o

$(OBJDIR)/ClientSocket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/ClientSocket.cpp;
	$(COMPILE) -c $(SOCKDIR)/ClientSocket.cpp -o $(OBJDIR)/ClientSocket.o

$(OBJDIR)/ServerSocket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/ServerSocket.cpp;
	$(COMPILE) -c $(SOCKDIR)/ServerSocket.cpp -o $(OBJDIR)/ServerSocket.o

#$(OBJDIR)/ClientSocketSource.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/ClientSocketSource.cpp;
#	$(COMPILE) -c $(SOCKDIR)/ClientSocketSource.cpp -o $(OBJDIR)/ClientSocketSource.o

#$(OBJDIR)/FixedConnectionPool.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/FixedConnectionPool.cpp;
#	$(COMPILE) -c $(SOCKDIR)/FixedConnectionPool.cpp -o $(OBJDIR)/FixedConnectionPool.o

$(OBJDIR)/RemoteAddress.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/RemoteAddress.cpp;
	$(COMPILE) -c $(SOCKDIR)/RemoteAddress.cpp -o $(OBJDIR)/RemoteAddress.o

#$(OBJDIR)/SecureClientSocket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/SecureClientSocket.cpp;
#	$(COMPILE) -c $(SOCKDIR)/SecureClientSocket.cpp -o $(OBJDIR)/SecureClientSocket.o

#$(OBJDIR)/SecureSocket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/SecureSocket.cpp;
#	$(COMPILE) -c $(SOCKDIR)/SecureSocket.cpp -o $(OBJDIR)/SecureSocket.o

$(OBJDIR)/SocketConfig.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/SocketConfig.cpp;
	$(COMPILE) -c $(SOCKDIR)/SocketConfig.cpp -o $(OBJDIR)/SocketConfig.o

$(OBJDIR)/UDPSocket.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/UDPSocket.cpp;
	$(COMPILE) -c $(SOCKDIR)/UDPSocket.cpp -o $(OBJDIR)/UDPSocket.o

$(OBJDIR)/SafeAddressInfo.o: $(BASEHEADERS) $(SOCKHEADERS) $(SOCKDIR)/SafeAddressInfo.cpp;
	$(COMPILE) -c $(SOCKDIR)/SafeAddressInfo.cpp -o $(OBJDIR)/SafeAddressInfo.o

#
## STATISTICAL OBJECTS
#

$(OBJDIR)/Histogram.o: $(BASEHEADERS) $(STATHEADERS) $(STATDIR)/Histogram.cpp;
	$(COMPILE) -c $(STATDIR)/Histogram.cpp -o $(OBJDIR)/Histogram.o

#
## STATUS OBJECTS
#

$(OBJDIR)/CPUUsage.o: $(BASEHEADERS) $(STUSHEADERS) $(STUSDIR)/CPUUsage.cpp;
	$(COMPILE) -c $(STUSDIR)/CPUUsage.cpp -o $(OBJDIR)/CPUUsage.o

#
## STREAM OBJECTS
#

$(OBJDIR)/Stream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/Stream.cpp;
	$(COMPILE) -c $(STRMDIR)/Stream.cpp -o $(OBJDIR)/Stream.o

$(OBJDIR)/FileStream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/FileStream.cpp;
	$(COMPILE) -c $(STRMDIR)/FileStream.cpp -o $(OBJDIR)/FileStream.o

$(OBJDIR)/NullStream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/NullStream.cpp;
	$(COMPILE) -c $(STRMDIR)/NullStream.cpp -o $(OBJDIR)/NullStream.o

$(OBJDIR)/SocketStream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/SocketStream.cpp;
	$(COMPILE) -c $(STRMDIR)/SocketStream.cpp -o $(OBJDIR)/SocketStream.o

$(OBJDIR)/StringStream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/StringStream.cpp;
	$(COMPILE) -c $(STRMDIR)/StringStream.cpp -o $(OBJDIR)/StringStream.o

$(OBJDIR)/BufferedStream.o: $(BASEHEADERS) $(STRMHEADERS) $(STRMDIR)/BufferedStream.cpp;
	$(COMPILE) -c $(STRMDIR)/BufferedStream.cpp -o $(OBJDIR)/BufferedStream.o

#
## THREADING OBJECTS
#

$(OBJDIR)/Mutex.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/Mutex.cpp;
	$(COMPILE) -c $(THRDDIR)/Mutex.cpp -o $(OBJDIR)/Mutex.o

$(OBJDIR)/Semaphore.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/Semaphore.cpp;
	$(COMPILE) -c $(THRDDIR)/Semaphore.cpp -o $(OBJDIR)/Semaphore.o

$(OBJDIR)/Thread.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/Thread.cpp;
	$(COMPILE) -c $(THRDDIR)/Thread.cpp -o $(OBJDIR)/Thread.o

$(OBJDIR)/ThreadMonitor.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/ThreadMonitor.cpp;
	$(COMPILE) -c $(THRDDIR)/ThreadMonitor.cpp -o $(OBJDIR)/ThreadMonitor.o

$(OBJDIR)/ServerThread.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/ServerThread.cpp;
	$(COMPILE) -c $(THRDDIR)/ServerThread.cpp -o $(OBJDIR)/ServerThread.o

$(OBJDIR)/ThreadPool.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/ThreadPool.cpp;
	$(COMPILE) -c $(THRDDIR)/ThreadPool.cpp -o $(OBJDIR)/ThreadPool.o

$(OBJDIR)/PooledThread.o: $(BASEHEADERS) $(THRDHEADERS) $(THRDDIR)/PooledThread.cpp;
	$(COMPILE) -c $(THRDDIR)/PooledThread.cpp -o $(OBJDIR)/PooledThread.o

#
## XML OBJECTS
#

$(OBJDIR)/XMLDocument.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLDocument.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLDocument.cpp -o $(OBJDIR)/XMLDocument.o

$(OBJDIR)/XMLChunk.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLChunk.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLChunk.cpp -o $(OBJDIR)/XMLChunk.o

$(OBJDIR)/XMLData.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLChunk.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLData.cpp -o $(OBJDIR)/XMLData.o

$(OBJDIR)/XMLElement.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLElement.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLElement.cpp -o $(OBJDIR)/XMLElement.o

$(OBJDIR)/XMLIndex.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLIndex.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLIndex.cpp -o $(OBJDIR)/XMLIndex.o

$(OBJDIR)/XMLProperty.o: $(BASEHEADERS) $(XMLHEADERS) $(XMLDIR)/XMLProperty.cpp;
	$(COMPILE) -c $(XMLDIR)/XMLProperty.cpp -o $(OBJDIR)/XMLProperty.o

#
## TEST PROGRAMS
#


#
## Clean Commands
#

clean:
	rm -f $(ALL_LIBS) $(ALL_APPS) $(ALL_OBJS)


