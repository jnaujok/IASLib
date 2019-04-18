/*
 * Exception Class
 *
 *      This is the base class for all exception types that the system
 * can throw during an error condition. All other types of exceptions
 * must be derived from this class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Exception.h"
#include <iostream>

namespace IASLib
{
    IMPLEMENT_OBJECT( CException, CObject );

    CException::CException( const char *strExceptionMessage, ExceptionSeverity nPriority )
    {
        m_nPriority = nPriority;
        m_strExceptionMessage = strExceptionMessage;
        m_pCallStack = new CExceptionCallStack();
        m_pCallStack->SetMessage( strExceptionMessage );
    }

    CException::~CException( void )
    {
       delete m_pCallStack;
    }

    const char *CException::ErrnoToText( int nErrno )
    {
        static const char *strTextMeanings[] = { 
                                        "[ESUCCESS]       Successful",
                                        "[EPERM]          Not owner",
                                        "[ENOENT]         No such file or directory",
                                        "[ESRCH]          No such process",
                                        "[EINTR]          Interrupted system call",
                                        "[EIO]            I/O error",
                                        "[ENXIO]          No such device or address",
                                        "[E2BIG]          Arg list too long",
                                        "[ENOEXEC]        Exec format error",
                                        "[EBADF]          Bad file number",
                                        "[ECHILD]         No children",
                                        "[EDEADLK]        Operation would cause deadlock",
                                        "[ENOMEM]         Not enough core",
                                        "[EACCES]         Permission denied",
                                        "[EFAULT]         Bad address",
                                        "[ENOTBLK]        Block device required",
                                        "[EBUSY]          Mount device busy",
                                        "[EEXIST]         File exists",
                                        "[EXDEV]          Cross-device link",
                                        "[ENODEV]         No such device",
                                        "[ENOTDIR]        Not a directory",
                                        "[EISDIR]         Is a directory",
                                        "[EINVAL]         Invalid argument",
                                        "[ENFILE]         File table overflow",
                                        "[EMFILE]         Too many open files",
                                        "[ENOTTY]         Not a typewriter",
                                        "[ETXTBSY]        Text file busy",
                                        "[EFBIG]          File too large",
                                        "[ENOSPC]         No space left on device",
                                        "[ESPIPE]         Illegal seek",
                                        "[EROFS]          Read-only file system",
                                        "[EMLINK]         Too many links",
                                        "[EPIPE]          Broken pipe",
                                        "[EDOM]           Argument too large",
                                        "[ERANGE]         Result too large",
                                        "[EWOULDBLOCK]    Operation would block (also EAGAIN)",
                                        "[EINPROGRESS]    Operation now in progress",
                                        "[EALREADY]       Operation already in progress",
                                        "[ENOTSOCK]       Socket operation on non-socket",
                                        "[EDESTADDRREQ]   Destination address required",
                                        "[EMSGSIZE]       Message too long",
                                        "[EPROTOTYPE]     Protocol wrong type for socket",
                                        "[ENOPROTOOPT]    Protocol not available",
                                        "[EPROTONOSUPPORT]Protocol not supported",
                                        "[ESOCKTNOSUPPORT]Socket type not supported",
                                        "[EOPNOTSUPP]     Operation not supported on socket",
                                        "[EPFNOSUPPORT]   Protocol family not supported",
                                        "[EAFNOSUPPORT]   Address family not supported by protocol family",
                                        "[EADDRINUSE]     Address already in use",
                                        "[EADDRNOTAVAIL]  Can't assign requested address",
                                        "[ENETDOWN]       Network is down",
                                        "[ENETUNREACH]    Network is unreachable",
                                        "[ENETRESET]      Network dropped connection on reset",
                                        "[ECONNABORTED]   Software caused connection abort",
                                        "[ECONNRESET]     Connection reset by peer",
                                        "[ENOBUFS]        No buffer space available",
                                        "[EISCONN]        Socket is already connected",
                                        "[ENOTCONN]       Socket is not connected",
                                        "[ESHUTDOWN]      Can't send after socket shutdown",
                                        "[ETOOMANYREFS]   Too many references: can't splice",
                                        "[ETIMEDOUT]      Connection timed out",
                                        "[ECONNREFUSED]   Connection refused",
                                        "[ELOOP]          Too many levels of symbolic links",
                                        "[ENAMETOOLONG]   File name too long",
                                        "[EHOSTDOWN]      Host is down",
                                        "[EHOSTUNREACH]   No route to host",
                                        "[ENOTEMPTY]      Directory not empty",
                                        "[EPROCLIM]       Too many processes",
                                        "[EUSERS]         Too many users",
                                        "[EDQUOT]         Disc quota exceeded",
                                        "[ESTALE]         Stale NFS file handle",
                                        "[EREMOTE]        Too many levels of remote in path",
                                        "[EBADRPC]        RPC struct is bad",
                                        "[ERPCMISMATCH]   RPC version wrong",
                                        "[EPROGUNAVAIL]   RPC prog. not avail",
                                        "[EPROGMISMATCH]  Program version wrong",
                                        "[EPROCUNAVAIL]   Bad procedure for program",
                                        "[ENOLCK]         No locks available",
                                        "[ENOSYS]         Function not implemented",
                                        "[EFTYPE]         Inappropriate file type or format",
                                        "[ENOMSG]         No msg matches receive request",
                                        "[EIDRM]          Msg queue id has been removed",
                                        "[ENOSR]          Out of STREAMS resources",
                                        "[ETIME]          System call timed out",
                                        "[EBADMSG]        Next message has wrong type",
                                        "[EPROTO]         STREAMS protocol error",
                                        "[ENODATA]        No message on stream head read q",
                                        "[ENOSTR]         fd not associated with a stream",
                                        "[ECLONEME]       Tells open to clone the device",
                                        "[EDIRTY]         Mounting a dirty fs w/o force",
                                        "[EDUPPKG]        duplicate package name on install",
                                        "[EVERSION]       version number mismatch",
                                        "[ENOPKG]         unresolved package name",
                                        "[ENOSYM]         unresolved symbol name",
                                        "[ECANCELED]      operation canceled",
                                        "[EFAIL]          cannot start operation",
                                        "[UNUSED-96]      Unused Error Code!",
                                        "[EINPROG]        operation (now) in progress",
                                        "[EMTIMERS]       too many timers",
                                        "[ENOTSUP]        function not implemented",
                                        "[EAIO]           internal AIO operation complete",
                                        "[EMULTIHOP]      Reserved",
                                        "[ENOLINK]        Reserved",
                                        "[EOVERFLOW]      Value too large",
                                        "[UNUSED-104]     Unused Error Code!",
                                        "[UNUSED-105]     Unused Error Code!",
                                        "[UNUSED-106]     Unused Error Code!",
                                        "[UNUSED-107]     Unused Error Code!",
                                        "[UNUSED-108]     Unused Error Code!",
                                        "[UNUSED-109]     Unused Error Code!",
                                        "[UNUSED-110]     Unused Error Code!",
                                        "[UNUSED-111]     Unused Error Code!",
                                        "[UNUSED-112]     Unused Error Code!",
                                        "[UNUSED-113]     Unused Error Code!",
                                        "[UNUSED-114]     Unused Error Code!",
                                        "[UNUSED-115]     Unused Error Code!",
                                        "[EILSEQ]         Invalid wide character",
                                        "[UNUSED-117]     Unused Error Code!",
                                        "[UNUSED-118]     Unused Error Code!",
                                        "[UNUSED-119]     Unused Error Code!",
                                        "[UNUSED-120]     Unused Error Code!",
                                        "[UNUSED-121]     Unused Error Code!",
                                        "[UNUSED-122]     Unused Error Code!",
                                        "[ESOFT]          Correct ECC Error on media. Media is no longer stable.",
                                        "[EMEDIA]         Uncorrectable error on media. Media is damaged.",
                                        "[ERELOCATED]     Data was successfully relocated on media." 
                                        };

        static const char *strNegativeErrors[] = {
                                        "[ESUCCESS]     Successful completion.",
                                        "[ERESTART]     Restart syscall.",
                                        "[EJUSTRETURN]  Don't modify regs, just return.",
                                        "[EEMULATE]     Don't modify registers, just return with emulation.",
                                        };

        if ( nErrno < 0 )
        {
            nErrno = -nErrno;
            if ( nErrno <= 3 )
            {
                return strNegativeErrors[ nErrno ];
            }
            return "[UNUSED-NEG] Unused Error Code!";
        }

        if ( nErrno <= 125 )
        {
            return strTextMeanings[ nErrno ];
        }

        return "[UNUSED-POS] Unused Error Code!";
    }

    void CException::FileLine( const char *strFile, int nLine )
    {
      std::cout << "CException::FileLine - " << strFile << ", " << nLine << std::endl; std::cout.flush();
       m_pCallStack->SetFile( strFile );
       m_pCallStack->SetLine( nLine );
    }

    void CException::AddStack( const char *strFile, int nLine, const char *strMessage )
    {
       CExceptionCallStack *pTemp = new CExceptionCallStack( strFile, nLine, strMessage );
       pTemp->SetDownCall( m_pCallStack );
       m_pCallStack = pTemp;
    }

    CString CException::DumpStack( void )
    {
        return m_pCallStack->UnwindStack();
    }

} // namespace IASLib
