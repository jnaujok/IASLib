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

#include <mutex>
#include "LogSink.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
        // Writes each line to standard output (or standard error), whole,
        // and flushes it.
    class CConsoleLog : public CLogSink
    {
        protected:
            std::mutex  m_mutexProtect;
            bool        m_bStandardError;

        public:
                                DEFINE_OBJECT( CConsoleLog );
                                CConsoleLog( Level level, bool bStandardError = false, bool bMakeDefault = true );
            virtual            ~CConsoleLog( void );

            virtual bool        writeLine( const char *message );
    };
} // namespace IASLib
#endif // IASLIB_CONSOLELOG_H__
