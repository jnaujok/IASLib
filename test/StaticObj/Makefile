############################################################################
#
# Linux Makefile for the Irene Adler Software Library (Linux Platforms only)
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
#    make -f makefile_linux [target]
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
APPDIR  =  .

LIBDIR =   ../../lib/linux
TESTDIR =   $(APPDIR)/bin

OBJDIR  =   $(APPDIR)/obj

ALLINCS = -I../../inc

#############################################################################
#
# Output Objects.
#
#############################################################################

TEST_PROGS = 
ALL_LIBS   = 
ALL_APPS   = StaticObj

# Specify the include and library search paths for the compiler
INCLUDE   = -I/usr/local/include -I/usr/include  $(ALLINCS)

LIBPATH   = -L$(LIBDIR) -lIASLib -lm -ldl -lpthread

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
DEFS    = -Dlinux -DIASLIB_LINUX__ -DIASLIB_NETWORKING__ -DIASLIB_MULTI_THREADED__ \
          -DIASLIB_PTHREAD__  -DIASLIB_STATS__

#
# Specify the compile-time options.
#
#OPTIONS = -g0 -mt -pg -D__THREADED__
#OPTIONS_NO_THREAD = -g0 -pg
# non-debug version
OPTIONS =  -O3


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
HEADERS = $(APPDIR)/stdafx.h

############################################
#  Objects
#
OBJECTS = $(OBJDIR)/stdafx.o 

ALL_OBJS = $(OBJECTS) 
# 
# 'make all' builds all of the components
#
all: $(ALL_OBJS) $(ALL_LIBS) $(ALL_APPS);

StaticObj: $(ALL_OBJS) $(APPDIR)/StaticObj.cpp $(LIBDIR)/libIASLib.a;
	$(COMPILE) $(ALL_OBJS) $(LIBPATH) $(APPDIR)/StaticObj.cpp -o $(APPDIR)/StaticObj

#
## GENERAL LIBRARY
#

$(OBJDIR)/stdafx.o:  $(HEADERS) $(APPDIR)/stdafx.cpp;
	$(COMPILE) -c $(APPDIR)/stdafx.cpp -o $(OBJDIR)/stdafx.o


#
## TEST PROGRAMS
#


#
## Clean Commands
#

clean: 
	rm $(ALL_LIBS) $(ALL_APPS) $(ALL_OBJS)


