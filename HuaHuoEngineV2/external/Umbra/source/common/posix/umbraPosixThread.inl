#ifndef UMBRAPOSIXTHREAD_HPP
#define UMBRAPOSIXTHREAD_HPP
/*!
 *
 * Umbra PVS
 * -----------------------------------------
 *
 * (C) 2007-2010 Umbra Software Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software Ltd., and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software Ltd. and legal action against the party in breach.
 *
 * \file
 * \brief   Threading implementation for POSIX.
 *
 */

#include "umbraThread.hpp"
#include "umbraMemory.hpp"
#include "umbraString.hpp"
#include "umbraChecksum.hpp"

#include <pthread.h>
#include <unistd.h>
#include <time.h>

#if UMBRA_OS == UMBRA_OSX
// [jasin] in OSX, we have to use mach semaphores, as posix semaphores
// aren't fully supported.
#   include <dispatch/dispatch.h>
#   include <semaphore.h>
#   include <mach/mach.h>
#   include <libkern/OSAtomic.h>
#   include <sys/errno.h>
#elif UMBRA_OS == UMBRA_IOS
#   include <mach/semaphore.h>
#   include <mach/mach.h>
#   include <libkern/OSAtomic.h>
#   include <semaphore.h>
#   include <sys/errno.h>
#else
#   include <semaphore.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <errno.h>
#endif

/*****************************************************************************
 *
 * Class:           ImpThread
 *
 * Description:     Implementation for the Thread class in POSIX.
 *
 * Notes:
 *
 *****************************************************************************/



//------------------------------------------------------------------------
// Some required waiting times.
//------------------------------------------------------------------------

const int                       DTOR_THREAD_EXIT_WAIT_MS = 100;
const int                       DTOR_THREAD_EXIT_TRIES = 200;
const int                       SMALL_WAIT_MS = 10;
const int                       YIELD_WAIT_MS = 0;

namespace Umbra
{
#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
    mach_port_t getPrivateBootstrapPort();
#endif

    class ImpThread
    {
    public:
        enum State
        {
            DESTRUCT    = -3,   // Destruction of the Thread has started.
            TERMINATED  = -2,   // After this the thread is unusable.
            NOT_RUN     = -1,   // State before first run
            STARTING    = 0,    // Thread is being started.
            RUNNING     = 1,    // Thread is running
            FINISHED    = 2,    // Thread has finished its run and return value is available.
            OUT_OF_LOOP = 3     // Thread has exited the inner loop.
        };
        inline                      ImpThread       (Allocator* a = NULL);
        inline                      ~ImpThread      (void);

        inline static void          yield           (void);
        inline static void          sleep           (int millis);
        inline void                 setFunction     (Runnable * runMe);
        inline bool                 run             (void * param);
        inline bool                 isFinished      (void) const;
        inline uint32               getExitCode     (void) const;
        inline void                 threadLoop      (void * param);
        inline bool                 waitToFinish    (unsigned int timeoutMs);
        inline void                 setPriority     (int priority);

    private:
        ImpThread       (const ImpThread&);     // not allowed!
        ImpThread&                  operator=       (const ImpThread&);     // not allowed!
        inline State                getState        (void) const    { m_waitCSect.enter(); State s = m_state; m_waitCSect.leave(); return s; }
        inline void                 setState        (State s)       { m_waitCSect.enter(); m_state = s; m_waitCSect.leave(); }

        //--------------------------------------------------------------------
        // member variables
        //--------------------------------------------------------------------

        pthread_t                   m_thread;
        unsigned long               m_returnValue;                          // Return value of the users function.
        Runnable *                  m_client;                               // Runnable that wishes itself to be run
        void *                      m_realParam;                            // Parameters given by the user for the function.
        volatile State              m_state;                                // state of the Thread. See enumeration.
        mutable CriticalSection     m_waitCSect;                            // Mutex for waiting the thread to finish. !! SHOULD THIS BE VOLATILE
    };

    static void* threadFunc(void* pThis);

    /*****************************************************************************
     *
     * Function:        ImpThread::ImpThread()
     *
     * Description:     ImpThread constructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpThread::ImpThread(Allocator* a):
        m_waitCSect(a)
    {
        setState(NOT_RUN);
        m_client    = NULL;



        //m_hThread = (HANDLE) _beginthreadex(NULL,0,threadFunc, reinterpret_cast<void *>(this), CREATE_SUSPENDED, &m_threadId);

        //UMBRA_ASSERT((m_thread != NULL) && "Thread construction failed!");
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::~ImpThread()
     *
     * Description:     ImpThread destructor
     *
     * Returns:
     *
     * Notes:           Exits the thread finally.
     *
     *****************************************************************************/

    //#include <cstdio>
    inline ImpThread::~ImpThread(void)
    {

        UMBRA_ASSERT(getState() != STARTING && "Trying to destruct a starting thread.");
        UMBRA_ASSERT(getState() != RUNNING  && "Trying to destruct a running thread. Probably suspended and not resumed to the end.");

        //--------------------------------------------------------------------
        // This exits the thread. i.e. Drops it out from the loop and lets
        // thread main function exit with return code.
        //--------------------------------------------------------------------

        /* if (getState() != NOT_RUN)
        {
            setState(DESTRUCT);

            //--------------------------------------------------------------------
            // Below is this->resume()!
            //--------------------------------------------------------------------

            //resume();

            //--------------------------------------------------------------------
            // Check that the loop has exited or the thread is terminated or
            // it is clear that the thread has no intention of running to the end.
            // NOTE [WILI 050702]: CHANGED TO YIELD!!! THE SLEEPING NOW HAPPENED
            // ALWAYS AND THUS CAUSED THREAD EXIT TO TAKE AT LEAST 100MS!!!!
            //--------------------------------------------------------------------

            int i = 0;

            while (getState() != OUT_OF_LOOP && getState() != TERMINATED &&
                   i < DTOR_THREAD_EXIT_TRIES)
            {
                ImpThread::yield();
                i++;
            }

            //--------------------------------------------------------------------
            // If thread does not exit, we terminate it.
            //--------------------------------------------------------------------
            if (i == DTOR_THREAD_EXIT_TRIES)
            {
                //          printf ("terminating\n");
                terminate();
            }
        } */

        //-------------------------------------------------------------------
        // Here the thread should not be running and we can safely deallocate
        // ourselves.
        //--------------------------------------------------------------------

        //pthread_destroy(&m_thread);

        //  printf ("closeHandle\n");
        //CloseHandle(m_hThread);
        //  printf ("done\n");
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::setFunction()
     *
     * Description:     Sets the task to be accomplished by this thread on its next
     *                  run() invocation.
     *
     * Parameters:      runMe = Runnable class that implements the desired functionality.
     *
     * Notes:           Use run() to actually run the thread and give the possible
     *                  parameters.
     *                  This is NOT thread safe with the run method. Calling these
     *                  two from two different threads may result in undefined
     *                  behaviour.
     *
     *****************************************************************************/

    inline void ImpThread::setFunction(Runnable * runMe)
    {
        UMBRA_ASSERT(runMe != NULL && "NULL Runnable is invalid");
        m_client = runMe;
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::threadFunc()
     *
     * Description:     Internal function used as the inner loop of the thread.
     *
     * Parameters:      param = Empty. Required by the specification.
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpThread::threadLoop(void *)
    {
        while (getState() != DESTRUCT)
        {
            UMBRA_ASSERT((getState() == STARTING) && "State wrong for running the thread");
            //UMBRA_ASSERT((m_thread != NULL) && "Thread not properly initialized");
            UMBRA_ASSERT((m_client != NULL) && "Runnable class not provided");
            setState(RUNNING);
            m_returnValue = m_client->run(m_realParam);
            setState(FINISHED);
            break;
        }

        //setState(OUT_OF_LOOP);
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::run()
     *
     * Description:     Starts the thread
     *
     * Parameters:      param = parameter to be passed to the runnable function.
     *
     * Returns:         True is succeeds, false on failure.
     *
     * Notes:           This yields the calling thread to give time to the Thread
     *                  begin its execution.
     *                  This will wait for max 1 second in 10 ms intervals for
     *                  the Thread to prepare itself for the next run, in case it is
     *                  in some kind of middle state.
     *
     * \todo [wili 050702] Changed the comparison to while (res > 1 ..) .. The old
     *                     implementation made all threads to wait for a second!!!!
     *                     (kinda slow creation process IMHO :)).
     *
     *****************************************************************************/

    inline bool ImpThread::run (void* param)
    {
        UMBRA_ASSERT((m_client != NULL) && "Runnable for the Thread not set!");
        UMBRA_ASSERT((getState() != STARTING) && "Still starting the previous run.");
        UMBRA_ASSERT((getState() != RUNNING) && "Thread is still running and tried to run it.");
        UMBRA_ASSERT((getState() != TERMINATED) && "Thread terminated and tried to still run it.");

        //  printf ("initing resume\n");
        m_realParam = param;
        setState(STARTING);

        //--------------------------------------------------------------------
        // Creates thread with default security, default stack size, this as argument
        // argument and suspended.
        //--------------------------------------------------------------------

        pthread_create(&m_thread, NULL, threadFunc, this);

        //  printf ("wait until on its way\n");

        while (getState() == STARTING)              // wait until thread is propery on its way..
        {
            Thread::yield();
        }

        return true;
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::isFinished() const
     *
     * Description:     Checks if the thread has finished it latest run.
     *
     * Returns:         True if thread has finished latest run, false otherwise
     *
     * Notes:           Thread that has never started returns also false. Thread
     *                  that has been terminated returns false.
     *
     *****************************************************************************/

    inline bool ImpThread::isFinished(void) const
    {
        return getState() == FINISHED;
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::getExitCode() const
     *
     * Description:     Gets the exit code of the function.
     *
     * Returns:         Exit code.
     *
     * Notes:           Check that run is finished before querying this. Otherwise
     *                  you will get an old value.
     *
     *****************************************************************************/

    inline uint32 ImpThread::getExitCode(void) const
    {
        return m_returnValue;
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::suspend()
     *
     * Description:     Puts the calling thread to sleep for millis milliseconds
     *
     * Parameters:      millis = millisecons to sleep.
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpThread::sleep(int millis)
    {
#if UMBRA_OS == UMBRA_PS3
        sleep(millis / 1000);
#else
        usleep(1000*millis);
#endif

    }

    /*****************************************************************************
     *
     * Function:        ImpThread::yield()
     *
     * Description:     Yields the current time slice of the calling thread.
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpThread::yield(void)
    {
        sleep(YIELD_WAIT_MS);
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::waitToFinish()
     *
     * Description:
     *
     * Notes:
     *
     *****************************************************************************/
#if UMBRA_OS == UMBRA_LINUX
int pthread_join_timeout (pthread_t thread_id, unsigned int timeoutMs)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_nsec += timeoutMs * 1000000;

    ts.tv_sec  += ts.tv_nsec / 1000000000;
    ts.tv_nsec  = ts.tv_nsec % 1000000000;

    return pthread_timedjoin_np(thread_id, 0, &ts) == ETIMEDOUT;
}
#else
// http://therealdavebarry.blogspot.fi/2006/03/waiting-for-thread-pthreadjoin-with.html
#include <sys/time.h>

#define PTHREAD_JOIN_POLL_INTERVAL 10

struct waitData
{
    pthread_t waitID;
    pthread_t helpID;
    int done;
};

void sleep_msecs(int msecs)
{
    struct timeval tv;

    tv.tv_sec = msecs/1000;
    tv.tv_usec = (msecs % 1000) * 1000;
    select (0,NULL,NULL,NULL,&tv);
}

unsigned int get_ticks()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_usec/1000 + tv.tv_sec * 1000);
}

void* join_timeout_helper(void *arg)
{
    waitData *data = (waitData*)arg;

    pthread_join(data->waitID, NULL);
    data->done = true;
    return (void *)0;
}

int pthread_join_timeout(pthread_t wid, unsigned int msecs)
{
    pthread_t id;
    waitData data;
    unsigned int start = get_ticks();
    int timedOut = false;

    data.waitID = wid;
    data.done = false;

    if (pthread_create(&id, NULL, join_timeout_helper, &data) != 0)
        return -1;

    do {
    if (data.done)
        break;
        /* you could also yield to your message loop here... */
        sleep_msecs(PTHREAD_JOIN_POLL_INTERVAL);
    } while ((get_ticks() - start) < msecs);

    if (!data.done)
    {
#if (UMBRA_OS == UMBRA_ANDROID)
        pthread_kill(id, 0);
#else
        pthread_cancel(id);
#endif
        timedOut = true;
    }
    /* free helper thread resources */
    pthread_join(id, NULL);
    return (timedOut);
}
#endif


    inline bool ImpThread::waitToFinish(unsigned int timeoutMs)
    {
        //--------------------------------------------------------------------
        // Following is paired with the excution of the client code.
        // CriticalSection is entered just before excution and left after.
        // So following will hold until the thread has finished running.
        //--------------------------------------------------------------------

        UMBRA_ASSERT(getState() != TERMINATED && "Thread terminated and tried to wait for it to run.");
        UMBRA_ASSERT(m_thread != 0);

        if (timeoutMs == 0xffffffff)    // how about just always call _join_timeout()
            pthread_join(m_thread, 0);
        else
            if (pthread_join_timeout(m_thread, timeoutMs))
                return false;

        while (getState() != FINISHED)
        {
            //      Thread::yield();//sleep(SMALL_WAIT_MS);
            sleep(SMALL_WAIT_MS);
        }

        return true;
    }

    /*****************************************************************************
     *
     * Function:        ImpThread::setPriority(int)
     *
     * Description:
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpThread::setPriority(int)
    {
        // \todo [Hannu] m_thread is created in run so this function should merely store the priority
        //UMBRA_ASSERT(m_thread != NULL);
        //SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL + priority);
    }

    /*****************************************************************************
     *
     * Class:           ImpCriticalSection
     *
     * Description:
     *
     * Notes:
     *
     *****************************************************************************/

    class ImpCriticalSection
    {
    public:
        inline                  ImpCriticalSection      (void);
        inline                  ~ImpCriticalSection     (void);

        inline void             enter                   (void);
        inline void             leave                   (void);

    private:
        ImpCriticalSection      (const ImpCriticalSection&);        // not allowed!
        ImpCriticalSection&     operator=               (const ImpCriticalSection&);        // not allowed!

        //--------------------------------------------------------------------
        // member variables
        //--------------------------------------------------------------------


#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
        OSSpinLock              m_spinlock;
#elif UMBRA_OS == UMBRA_ANDROID
        pthread_mutex_t         m_spinlock;
#else
        pthread_spinlock_t      m_spinlock;
#endif

    };


    /*****************************************************************************
     *
     * Function:        ImpCriticalSection::ImpCriticalSection()
     *
     * Description:     ImpCriticalSection constructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpCriticalSection::ImpCriticalSection(void)
    {
#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
        m_spinlock = OS_SPINLOCK_INIT;
#elif UMBRA_OS == UMBRA_ANDROID
        // TODO
#else
        pthread_spin_init(&m_spinlock, 0);
#endif
    }

    /*****************************************************************************
     *
     * Function:        ImpCriticalSection::~ImpCriticalSection()
     *
     * Description:     ImpCriticalSection destructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpCriticalSection::~ImpCriticalSection(void)
    {
#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
        // NADA
#elif UMBRA_OS == UMBRA_ANDROID
        // TODO
#else
        pthread_spin_destroy(&m_spinlock);
#endif
    }

    /*****************************************************************************
     *
     * Function:        ImpCriticalSection::enter()
     *
     * Description:     Enters the critical section i.e. blocks other threads from
     *                  entering THIS critical section at the same time.
     *
     * Notes:           Blocks until critical section may be entered.
     *                  Once the block is entered, remember to leave it.
     *
     *****************************************************************************/

    inline void ImpCriticalSection::enter(void)
    {
#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
        OSSpinLockLock(&m_spinlock);
#elif UMBRA_OS == UMBRA_ANDROID
        // TODO
#else
        pthread_spin_lock(&m_spinlock);
#endif
    }

    /*****************************************************************************
     *
     * Function:        ImpCriticalSection::leave()
     *
     * Description:     Leaves the critical section.
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpCriticalSection::leave(void)
    {
#if (UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_IOS)
        OSSpinLockUnlock(&m_spinlock);
#elif UMBRA_OS == UMBRA_ANDROID
        // TODO
#else
        pthread_spin_unlock(&m_spinlock);
#endif
    }


    /*****************************************************************************
     *
     * Class:           ImpMutex
     *
     * Description:     Implementation of the Mutual Exclusion entity.
     *
     * Notes:           If performance is an issue, prefer CriticalSection.
     *
     *****************************************************************************/

    class ImpMutex
    {
    public:
        inline          ImpMutex        (void);
        inline          ~ImpMutex       (void);

        inline void     lock            (void);
        inline void     release         (void);
        inline bool     tryLock         (int millis);
    private:
        ImpMutex        (const ImpMutex&);      // not allowed!
        ImpMutex&       operator=       (const ImpMutex&);      // not allowed!

        //--------------------------------------------------------------------
        // member variables
        //--------------------------------------------------------------------

        pthread_mutex_t m_mutex;
    };



    /*****************************************************************************
     *
     * Function:        ImpMutex::ImpMutex()
     *
     * Description:     ImpMutex constructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpMutex::ImpMutex(void)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &attr);
//      m_hMutex = CreateMutex(NULL, FALSE, NULL);
//      UMBRA_ASSERT(m_hMutex != NULL && "Mutex initialization failed.");
    }


    /*****************************************************************************
     *
     * Function:        ImpMutex::~ImpMutex()
     *
     * Description:     ImpMutex destructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpMutex::~ImpMutex(void)
    {
        pthread_mutex_destroy(&m_mutex);
        //UMBRA_ASSERT(m_hMutex != NULL);
        //CloseHandle(m_hMutex);
    }

    /*****************************************************************************
     *
     * Function:        ImpMutex::lock()
     *
     * Description:     Blocks until the Mutex is acquired.
     *
     * Notes:           Will assert itself in the debug build for any anomalies.
     *                  Remember to release the Mutex!
     *
     *****************************************************************************/

    inline void ImpMutex::lock(void)
    {

        pthread_mutex_lock(&m_mutex);

//      UMBRA_ASSERT(m_hMutex != NULL);
//#ifdef UMBRA_DEBUG
//      DWORD result = WaitForSingleObject(m_hMutex, INFINITE);
//#else
//      WaitForSingleObject(m_hMutex, INFINITE);
//#endif
//      UMBRA_ASSERT(result != WAIT_ABANDONED_0 && "Other thread exited or terminated without releasing this mutex!");
//      UMBRA_ASSERT(result != WAIT_FAILED && "Indefinite waiting failed for some reason.");
//      UMBRA_ASSERT(result == WAIT_OBJECT_0 && "Mutex not acquired.");

    }


    /*****************************************************************************
     *
     * Function:        ImpMutex::release()
     *
     * Description:     Releases the mutex.
     *
     * Notes:           Asserts on failure in debug build.
     *
     *****************************************************************************/

    inline void ImpMutex::release(void)
    {
        pthread_mutex_unlock(&m_mutex);
//      UMBRA_ASSERT(m_hMutex != NULL);
//#ifdef UMBRA_DEBUG
//      BOOL result = ReleaseMutex(m_hMutex);
//#else
//      ReleaseMutex(m_hMutex);
//#endif
//      UMBRA_ASSERT(result && "Release failed. Maybe mutex was not acquired in the first place");
    }


    /*****************************************************************************
     *
     * Function:        ImpMutex::tryLock()
     *
     * Description:     Trys to lock the Mutex
     *
     * Parameters:      millis = how many milliseconds to try for the lock. Result
     *                  on using 0 is unknown.
     *
     * Returns:         True if locked, false otherwise.
     *
     * Notes:
     *                  Remember to release the Mutex!
     *
     *****************************************************************************/

    inline bool ImpMutex::tryLock(int)
    {
        return pthread_mutex_trylock(&m_mutex) == 0;

        //UMBRA_ASSERT(millis >= 0);
        //UMBRA_ASSERT(millis != INFINITE);
        //UMBRA_ASSERT(m_hMutex != NULL);
        //DWORD result = WaitForSingleObject(m_hMutex, millis);
        //UMBRA_ASSERT(result != WAIT_ABANDONED_0 && "Other thread exited or terminated without releasing this mutex!");
        //return result == WAIT_OBJECT_0;
    }

    /*****************************************************************************
     *
     * Class:           ImpSemaphore
     *
     * Description:     Semaphore limits the amount of threads accessing e.g.
     *                  limited resource at any given moment.
     *
     * Notes:
     *
     *****************************************************************************/

    class ImpSemaphore
    {
    public:
        inline              ImpSemaphore        (int initialCount, int maxCount);
        inline              ImpSemaphore        (const String& s, int initialCount, int maxCount);
        inline              ~ImpSemaphore       (void);

        inline void         up                  (void);
        inline void         down                (void);
        inline bool         tryDown             (int millis);
        inline bool         checkDown           (void);

    private:
        ImpSemaphore        (const ImpSemaphore&);      // not allowed!
        ImpSemaphore&       operator=           (const ImpSemaphore&);      // not allowed!

        //--------------------------------------------------------------------
        // member variables
        //--------------------------------------------------------------------
#if UMBRA_OS == UMBRA_OSX
        bool       m_isAnon;
        dispatch_semaphore_t m_anonSem;
#endif
        sem_t*     m_sem;
        int        m_maxCount;
    };

    /*****************************************************************************
     *
     * Function:        ImpSemaphore::ImpSemaphore()
     *
     * Description:     ImpSemaphore constructor
     *
     * Parameters:      initialCount    = What is the initial counter value. [0, maxCount].
     *                  maxCount        = Maximum counter value. This many downs can be made
     *                                  without blocking.
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpSemaphore::ImpSemaphore(int initialCount, int maxCount)
    {
        UMBRA_ASSERT(initialCount >= 0);
        UMBRA_ASSERT(maxCount > 0);
        UMBRA_ASSERT(initialCount <= maxCount && "Invalid setup");
        int ret;

#if (UMBRA_OS != UMBRA_OSX)
        m_sem = new sem_t;
        ret = sem_init(m_sem, 0, initialCount);
#else
        m_isAnon = true;
        m_anonSem = dispatch_semaphore_create(initialCount);
        UMBRA_UNREF(maxCount);
        ret = !m_anonSem;
#endif

        UMBRA_UNREF(ret);
        UMBRA_ASSERT(ret == 0);
        m_maxCount = maxCount;
    }

    inline ImpSemaphore::ImpSemaphore(const String& name, int initialCount, int maxCount)
    {
        UMBRA_ASSERT(initialCount >= 0);
        UMBRA_ASSERT(maxCount > 0);
        UMBRA_ASSERT(initialCount <= maxCount && "Invalid setup");

#if UMBRA_OS == UMBRA_PS3
        UMBRA_ASSERT(!"implement!");
#else
        String prefixed("/");
        prefixed += name;
#if UMBRA_OS == UMBRA_OSX
        m_isAnon = false;

        // 31 char max length for semaphore names on OSX
        if (prefixed.length() > 31)
        {
            UINT64 h = fnv64Hash((unsigned char*)prefixed.toCharPtr(), prefixed.length());
            char str[32];
            sprintf(str, "/%llx", h);
            prefixed = str;
        }
#endif
        m_sem = sem_open(prefixed.toCharPtr(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, initialCount);
        if (m_sem == SEM_FAILED)
        {
            printf("semaphore \"%s\" creation failed: %s\n", name.toCharPtr(), strerror(errno));
            fflush(stdout);
        }
        UMBRA_ASSERT(m_sem);
#endif

        m_maxCount = maxCount;
    }

    /*****************************************************************************
     *
     * Function:        ImpSemaphore::~ImpSemaphore()
     *
     * Description:     ImpSemaphore destructor
     *
     * Notes:
     *
     *****************************************************************************/

    inline ImpSemaphore::~ImpSemaphore(void)
    {
#if (UMBRA_OS != UMBRA_OSX)
        sem_destroy(m_sem);
#else
        if (m_isAnon)
            dispatch_release(m_anonSem);
        else
            sem_close(m_sem);
#endif
    }

    /*****************************************************************************
     *
     * Function:        ImpSemaphore::down()
     *
     * Description:     Downs the semaphore i.e. decreases the counter if counter
     *                  is over zero, otherwise blocks until the counter becomes
     *                  greater than zero.
     *
     * Notes:           Remember to up the semaphore after the use!
     *
     *****************************************************************************/

    inline void ImpSemaphore::down(void)
    {
#if UMBRA_OS == UMBRA_OSX
        if (m_isAnon)
            dispatch_semaphore_wait(m_anonSem, DISPATCH_TIME_FOREVER);
        else
#endif
        sem_wait(m_sem);
    }

    /*****************************************************************************
     *
     * Function:        ImpSemaphore::tryDown()
     *
     * Description:     Downs the semaphore i.e. decreases the counter if counter
     *                  is over zero, otherwise blocks until the counter becomes
     *                  greater than zero or the specified time passes.
     *
     * Parameters:      millis = How many milliseconds to wait for availability.
     *
     * Returns:         True if downing was successful, false otherwise
     *
     * Notes:           Remember to up the semaphore after the use!
     *
     *****************************************************************************/

    inline bool ImpSemaphore::tryDown(int millis)
    {
#if (UMBRA_OS == UMBRA_OSX)
        int ret = 0;
        if (m_isAnon)
        {
            dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, (int64_t)millis*1000000);
            ret = dispatch_semaphore_wait(m_anonSem, time);
        }
        else
        {
            // Darwin doesn't support sem_timedwait, so simulating
            do
            {
                ret = sem_trywait(m_sem);
                if (!ret || (errno != EAGAIN && errno != EINTR && errno != ETIMEDOUT))
                    break;
                usleep(1000);
            } while (millis-- > 0);

            if (!(!ret || errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT))
            {
                printf("ret: %d, errno: %d\n", ret, errno);
                puts(strerror(errno));
            }
            UMBRA_ASSERT(!ret || errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT);
        }
        return ret == 0;
#elif (UMBRA_OS == UMBRA_IOS || UMBRA_OS == UMBRA_PS3)
        UMBRA_ASSERT(!"not implemented");
        return false;
#else
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        ts.tv_nsec += millis * 1000000;

        ts.tv_sec  += ts.tv_nsec / 1000000000;
        ts.tv_nsec  = ts.tv_nsec % 1000000000;

        int ret = 0;
        do
        {
            ret = sem_timedwait(m_sem, &ts);
        } while(ret == -1 && errno == EINTR);

        if (ret == -1 && errno == ETIMEDOUT)
            return false;

        UMBRA_ASSERT(ret == 0);

        return true;
#endif
    }

    /*****************************************************************************
     *
     * Function:        ImpSemaphore::up()
     *
     * Description:     Ups semaphore i.e. increases the counter.
     *
     * Notes:
     *
     *****************************************************************************/

    inline void ImpSemaphore::up(void)
    {
#if UMBRA_OS == UMBRA_OSX
        if (m_isAnon)
            dispatch_semaphore_signal(m_anonSem);
        else
#endif
        sem_post(m_sem);
    }

    static void* threadFunc(void* pThis)
    {
        UMBRA_ASSERT(pThis != NULL && "Invalid pointer, cannot start thread loop");
        reinterpret_cast<ImpThread *>(pThis)->threadLoop(NULL);
        return NULL;
    }

    inline bool ImpSemaphore::checkDown(void)
    {
        return tryDown(0);
    }

int Thread::allocTls (void)
{
    pthread_key_t key;
    if (pthread_key_create(&key, NULL) != 0)
        return -1;
    return (int)key;
}

void Thread::freeTls (int idx)
{
    if (idx != -1)
        pthread_key_delete((pthread_key_t)idx);
}

void Thread::setTlsValue(int idx, UINTPTR value)
{
    pthread_setspecific((pthread_key_t)idx, (void*)value);
}

UINTPTR Thread::getTlsValue(int idx)
{
    return (UINTPTR)pthread_getspecific((pthread_key_t)idx);
}

#if UMBRA_OS == UMBRA_OSX || UMBRA_OS == UMBRA_LINUX
namespace Atomic
{
#if UMBRA_OS == UMBRA_OSX
Umbra::INT32 add(volatile Umbra::INT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = OSAtomicAdd32(a, value);
    return result;
}
Umbra::INT64 add(volatile Umbra::INT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = OSAtomicAdd64(a, value);
    return result;
}
Umbra::UINT32 add(volatile Umbra::UINT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = OSAtomicAdd32(a, (Umbra::INT32*)value);
    return (Umbra::UINT32&)result;
}
Umbra::UINT64 add(volatile Umbra::UINT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = OSAtomicAdd64(a, (Umbra::INT64*)value);
    return (Umbra::UINT64&)result;
}

Umbra::INT32 sub(volatile Umbra::INT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = OSAtomicAdd32(-a, value);
    return result;
}
Umbra::INT64 sub(volatile Umbra::INT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = OSAtomicAdd64(-a, value);
    return result;
}
Umbra::UINT32 sub(volatile Umbra::UINT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = OSAtomicAdd32(-a, (Umbra::INT32*)value);
    return (Umbra::UINT32&)result;
}
Umbra::UINT64 sub(volatile Umbra::UINT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = OSAtomicAdd64(-a, (Umbra::INT64*)value);
    return (Umbra::UINT64&)result;
}

#else

Umbra::INT32 add(volatile Umbra::INT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = __sync_fetch_and_add(value, a) + a;
    return result;
}
Umbra::INT64 add(volatile Umbra::INT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = __sync_fetch_and_add(value, a) + a;
    return result;
}
Umbra::UINT32 add(volatile Umbra::UINT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::UINT32 result = __sync_fetch_and_add(value, a) + a;
    return result;
}
Umbra::UINT64 add(volatile Umbra::UINT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::UINT64 result = __sync_fetch_and_add(value, a) + a;
    return result;
}

Umbra::INT32 sub(volatile Umbra::INT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT32 result = __sync_fetch_and_sub(value, a) - a;
    return result;
}
Umbra::INT64 sub(volatile Umbra::INT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::INT64 result = __sync_fetch_and_sub(value, a) - a;
    return result;
}
Umbra::UINT32 sub(volatile Umbra::UINT32* value, int a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::UINT32 result = __sync_fetch_and_sub(value, a) - a;
    return result;
}
Umbra::UINT64 sub(volatile Umbra::UINT64* value, INT64 a)
{
    UMBRA_ASSERT(!(((UINTPTR)value) & 3));
    Umbra::UINT64 result = __sync_fetch_and_sub(value, a) - a;
    return result;
}

#endif

size_t add(volatile size_t* value, size_t a)
{
#if defined(__LP64__)
    return (size_t)add((volatile Umbra::UINT64*)value, (Umbra::INT64&)a);
#else
    return (size_t)add((volatile Umbra::UINT32*)value, (int&)a);
#endif
}
size_t sub(volatile size_t* value, size_t a)
{
#if defined(__LP64__)
    return (size_t)sub((volatile Umbra::UINT64*)value, (Umbra::INT64&)a);
#else
    return (size_t)sub((volatile Umbra::UINT32*)value, (int&)a);
#endif
}
}
#endif

} // namespace Umbra

#endif //UMBRAPOSIXTHREAD_HPP
