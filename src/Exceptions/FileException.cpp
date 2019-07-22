/*
 * File Exception Class
 *
 *      This class is used to wrap file exception objects used by the
 * file class when it encounters an unexpected error condition. This
 * class is derived from the base exception class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "FileException.h"
#include <errno.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CFileException, CException );

    CFileException::CFileException( int nErrorNumber ) : CException( "Unknown File Exception", CException::FATAL )
    {

        m_nErrorNumber = nErrorNumber;

        switch ( m_nErrorNumber )
        {
            case EACCES:
                m_strExceptionMessage = "File access has been denied.";
                m_nPriority = CException::FATAL;
                break;

            case EINTR:
                m_strExceptionMessage = "A signal interrupted the file operation.";
                m_nPriority = CException::WARNING;
                break;

            case EISDIR:
                m_strExceptionMessage = "The named file is a directory and cannot be opened with write access.";
                m_nPriority = CException::NORMAL;
                break;

            case EMFILE:
                m_strExceptionMessage = "There are too many file descriptors currently open in the calling process.";
                m_nPriority = CException::FATAL;
                break;


            case ENAMETOOLONG:
                m_strExceptionMessage = "The length of the filename exceeds PATH_MAX or a pathname component is longer than NAME_MAX.";
                m_nPriority = CException::FATAL;
                break;

            case ENFILE:
                m_strExceptionMessage = "The maximum allowable number of files is currently open in the system.";
                m_nPriority = CException::FATAL;
                break;

            case ENOENT:
                m_strExceptionMessage = "A component of filename does not name an existing file or filename is an empty string.";
                m_nPriority = CException::FATAL;
                break;

            case ENOSPC:
                m_strExceptionMessage = "The directory or file system that would contain the new file cannot be expanded, the file does not exist, and it was to be created.";
                m_nPriority = CException::FATAL;
                break;

            case ENOTDIR:
                m_strExceptionMessage = "A component of the path prefix is not a directory.";
                m_nPriority = CException::FATAL;
                break;


            case ENXIO:
                m_strExceptionMessage = "The named file is a character special or block special file, and the device associated with this special file does not exist.";
                m_nPriority = CException::FATAL;
                break;

            case EROFS:
                m_strExceptionMessage = "The named file resides on a read-only file system and mode requires write access.";
                m_nPriority = CException::FATAL;
                break;

            case EINVAL:
                m_strExceptionMessage = "A passed parameter was invalid.";
                m_nPriority = CException::FATAL;
                break;

            case EAGAIN:
                m_strExceptionMessage = "The file is writing to the output device and calling this function would cause this process to block.";
                m_nPriority = CException::WARNING;
                break;

            case EBADF:
                m_strExceptionMessage = "The file descriptor underlying stream is not valid.";
                m_nPriority = CException::FATAL;
                break;

            case EFBIG:
                m_strExceptionMessage = "An attempt was made to write a file that exceeds the maximum file size or the process's file size limit; or the file is a regular file and an attempt was made to write at or beyond the offset maximum associated with the corresponding stream.";
                m_nPriority = CException::FATAL;
                break;

            case EIO:
                m_strExceptionMessage = "The process is a member of a background process group attempting to write to its controlling terminal, TOSTOP is set, the process is neither ignoring nor blocking SIGTTOU and the process group of the process is orphaned.";
                m_nPriority = CException::FATAL;
                break;

            case EPIPE:
                m_strExceptionMessage = "Attempt to read/write to a closed pipe. SIGPIPE will be generated.";
                break;

    #ifndef WIN32
            case ETXTBSY:
                m_strExceptionMessage = "The file is a pure procedure (shared text) file that is being executed and mode requires write access.";
                m_nPriority = CException::FATAL;
                break;

            case ELOOP:
                m_strExceptionMessage = "Too many symbolic links were encountered in resolving path.";
                m_nPriority = CException::HIGH;
                break;
    #endif

    #ifdef IASLIB_SUN_
            case EOVERFLOW:
                m_strExceptionMessage = "The current value of the file position cannot be represented correctly in an object of type fpos_t.";
                m_nPriority = CException::FATAL;
                break;
    #endif

            default:
                break;
        }

        m_pCallStack->SetMessage( m_strExceptionMessage );
    }


    CFileException::~CFileException( void )
    {
        // Nothing to do for now.
    }

} // namespace IASLib
