/*
 * Thread Task Class
 *
 *  This class provides a base, abstract class to handle the allocation of
 * threadable tasks within the IAS library. It is recommended to subclass
 * this class with a concrete class for implementing tasks that are meant to
 * run threaded.
 *
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/30/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADTASK_H__
#define IASLIB_THREADTASK_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "Thread.h"
#include "Mutex.h"

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    class CThreadTask : public CObject
    {
        public:
            enum TASK_STATUS
            {
                PREQUEUE,
                QUEUED,
                RUNNING,
                SLEEPING,
                BLOCKED,
                COMPLETE,
                UNKNOWN
            };
        protected:
            CString                 m_strIdentifier;
            TASK_STATUS             m_currentStatus;

        public:
                                    CThreadTask( const char *strIdentifier )
                                    {
                                        if ( strIdentifier )
                                        {
                                            m_strIdentifier = strIdentifier;
                                        }
                                        else
                                        {
                                            m_strIdentifier = "";
                                        }

                                        m_currentStatus = PREQUEUE;
                                    };
            virtual                ~CThreadTask( void ) {}

                                    DECLARE_OBJECT(CThreadTask, CObject );

            virtual CObject        *Run( void ) = 0;
            virtual TASK_STATUS     GetStatus( void ) { return m_currentStatus; }

        private:
            friend class CThreadPool;

            void                    addToQueue(void) { m_currentStatus = QUEUED; }
            void                    setRunning( void ) { m_currentStatus = RUNNING; }
            void                    setSleeping( void ) { m_currentStatus = SLEEPING; }
            void                    setBlocked( void ) { m_currentStatus = BLOCKED; }
            void                    setComplete( void ) { m_currentStatus = COMPLETE; }
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADTASK_H__
