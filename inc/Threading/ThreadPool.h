/*
 * Thread Pool Class
 *
 *  This class provides a pool of threads that can be allocated, run a
 * task, and then be returned to the pool to wait in standby until needed
 * again.
 *  This allows us to limit the overhead involved in the creation and the
 * subsequent deletion of threads and their contexts. If a task is assigned
 * to a thread pool, when threads are not available to process it, then the
 * task will be queued until such time as a thread becomes available to
 * process that task. If the queue is exhausted, then an exception will
 * be thrown.
 * Tasks can be added with a maximum timeout value. If a timeout is set,
 * a task which takes longer than that amount of time will be interrupted
 * and the thread returned to the pool. Note that this may require a large
 * amount of overhead and could cause performace impacts. Timeouts should
 * not be used as a standard means of terminating tasks.
 *
 * Using a task identifier allows retrieval of task status and task
 * results from the pool, but care must be taken as every task with an
 * identifier will require storage for the result after completion and
 * will not be cleared until retrieved. In other words, if you use an
 * identifier you -MUST- retrieve the results.
 *
 * Note: If the task identifier is passed as NULL, the results of the task
 * will not be stored, but status and results cannot be retrieved.
 *
 * ======================================================================
 * Since this module is based on threads, it is extremely dependent on
 * the underlying threads provided by the system. Although all possible
 * care has been taken to assure that operations are equivalent on all
 * different systems, this dependence may cause differences on different
 * systems.
 * ======================================================================
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/30/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADPOOL_H__
#define IASLIB_THREADPOOL_H__

#include "../Collections/Array.h"
#include "../Collections/Queue.h"
#include "../Collections/Hash.h"
#include "ThreadTask.h"
#include "Thread.h"
#include "Mutex.h"

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
class CThreadPool : public CObject
{
    protected:
        CArray m_aAvailableThreads;
        CArray m_aBusyThreads;
        size_t m_nTotalThreads;
        size_t m_nCurrentThreads;
        size_t m_nPeakThreads;
        CMutex m_mutexArray;
        CQueue m_qTaskQueue;
        CHash m_hashResults;
        CHash m_hashTaskIds;

        static bool m_bInitialized;

    public:
        CThreadPool(int maximumThreads = 16);
        virtual ~CThreadPool(void);

        DEFINE_OBJECT(CThreadPool);

        bool AddTask( CThreadTask *task );
        CThreadTask::TASK_STATUS TaskStatus(const char *strIdentifier);

        CObject *GetResults(const char *strIdentifier);

        size_t GetActiveThreads(void) { return m_nCurrentThreads; }
        size_t GetPeakThreads(void) { return m_nPeakThreads; }
        size_t GetIdleThreads(void) { return m_nTotalThreads - m_nCurrentThreads; }
        size_t GetQueueSize(void) { return m_qTaskQueue.Count(); }
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADPOOL_H__
