/***********************************************************************
**  Console Log Class
**
**  Description:
**      This class defines a log file that writes to the console (screen)
** rather than a permanent store.
**
**  Author: Jeffrey R. Naujok
**  Created: September 12, 2019
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

#ifndef IASLIB_CONSOLELOG_H__
#define IASLIB_CONSOLELOG_H__

#include "LogSink.h"
#include "../Threading/Mutex.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CConsoleLog : public CLogSink
    {
        protected:
#ifdef IASLIB_MULTI_THREADED__
            CMutex      m_mutexProtect;
#endif

        public:
                                DEFINE_OBJECT( CConsoleLog );
                                CConsoleLog( Level level );
            virtual            ~CConsoleLog( void );

            virtual bool        writeLine( const char *message );

    };
} // namespace IASLib
#endif // IASLIB_CONSOLELOG_H__
