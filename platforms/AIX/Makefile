SHELL   =  /bin/sh
MOVE    =  mv
COPY    =  ln -s
RM      =  rm -f


#############################################################


IASLIB_DIR = /users/jn538057/projects/ISALib

OBJECT_DIR = $(IASLIB_DIR)/obj

INC_PATH =\
   -I${IASLIB_DIR}/inc\
   -I${IASLIB_DIR}/inc/BaseTypes\
   -I${IASLIB_DIR}/inc/Exceptions\
   -I${IASLIB_DIR}/inc/Collections

OBJS    = \
   $(OBJECT_DIR)/Object.o\
   $(OBJECT_DIR)/String_.o\
   $(OBJECT_DIR)/StringStub.o\
   $(OBJECT_DIR)/StringTokenizer.o\
   $(OBJECT_DIR)/Date.o\
   $(OBJECT_DIR)/Exception.o\
   $(OBJECT_DIR)/FileException.o\
   $(OBJECT_DIR)/StringException.o\
   $(OBJECT_DIR)/SocketException.o\
   $(OBJECT_DIR)/Array.o\
   $(OBJECT_DIR)/Hash.o\
   $(OBJECT_DIR)/HashBucket.o\
   $(OBJECT_DIR)/HashSlat.o\
   $(OBJECT_DIR)/IntArray.o\
   $(OBJECT_DIR)/List.o\
   $(OBJECT_DIR)/Stack.o\
   $(OBJECT_DIR)/StringArray.o


CC      =   xlC

DEFINES = -Daix -DAIX -DISALIB_AIX__

#CFLAGS = -qarch=rs64a -O3 -qstrict $(DEFINES)
CFLAGS = -g $(DEFINES)
   
LD_FLAGS =

libIASLib.a: $(OBJS)
	${CC} -g -o OracleTest  ${CFLAGS} ${OBJS} ${INC_PATH} ${LIBS} -w -bloadmap:loadmap 

.C.o:
	${CC} $(CFLAGS) -c $< $(INC_PATH) 

clean:
	   $(RM) *.o *.I
	   $(RM) *..dbx
	   $(RM) *.a .../lib/*.a
	   $(RM) *.i
	   $(RM) *.ixx
	   $(RM) *.dat
	   $(RM) *.bin
	   $(RM) *.str
	   $(RM) core

touch: 
	   -touch *.C


# source files.
SRC =  	file1.cpp file2.cpp file3.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../libutils.a

# include directories
INCLUDES = -I. -I../include/ -I/usr/local/include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g

# compiler
CCC = g++

# library paths
LIBS = -L../ -L/usr/local/lib -lm

# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 