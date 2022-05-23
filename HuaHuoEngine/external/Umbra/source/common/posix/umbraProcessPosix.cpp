/*!
 *
 * Umbra PVS Booster Base
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
 * \brief   Threading library.
 *
 */

#include "umbraPrivateDefs.hpp"

#if UMBRA_IS_POSIX && !UMBRA_IS_TARGET

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <uuid.h>

#include "umbraProcess.hpp"
#include "umbraString.hpp"
#include "umbraUUID.hpp"

using namespace Umbra;

static String processError;

static void setProcessError(const char* err, ...)
{
    char *buffer;
    buffer = UMBRA_NEW_ARRAY(char, 512);
    if (!buffer)
    {
        perror ("error");
        perror (strerror(errno));
        return;
    }

    va_list args;
    va_start (args, err);
    vsnprintf (buffer, 512, err, args);
    perror (buffer);
    va_end (args);

    processError = buffer;
    UMBRA_DELETE_ARRAY(buffer);
}

const String& Umbra::getProcessError(void)
{
    // todo
    return processError;
}

namespace Umbra
{
    class ProcessSharedMemoryImpl
    {
    public:
        ProcessSharedMemoryImpl() :
          size(0) {}

        unsigned int size;
    };

    class ProcessAllocIdentifier
    {
        key_t key;
    };
} // namespace Umbra

void* Umbra::processAlloc(String identifier, unsigned int size, bool&, ProcessSharedMemoryImpl** impl)
{
    processError = "";

    void* data = NULL;

    (*impl) = UMBRA_NEW(ProcessSharedMemoryImpl);
    (*impl)->size = size;

    String filename = String(P_tmpdir "/") + identifier;

    // check if file exists
    int fd = open(filename.toCharPtr(), O_RDONLY);
    int res = 0;

    if(fd == -1 && errno == ENOENT)
    {
        // File did not exist: create it (first process does this)
        // (assumes no race condition)

        fd = open(filename.toCharPtr(), O_CREAT|O_RDWR, 0666);

        if(fd != -1)
        {
            res = lseek(fd, size-1, SEEK_SET);
            if(res == -1)
            {
                close(fd);
                UMBRA_DELETE(*impl);
                *impl = 0;
                setProcessError( "couldn't create memory mapped file (lseek): %s", strerror( errno ) );
                return NULL;
            }

            char writedata = 0;
            res = write(fd, &writedata, 1);
            if(res != 1)
            {
                close(fd);
                UMBRA_DELETE(*impl);
                *impl = 0;
                setProcessError( "couldn't create memory mapped file (write): %s", strerror( errno ) );
                return NULL;
            }
        }
    } else
    if(fd == -1)
    {
        UMBRA_DELETE(*impl);
        *impl = 0;
        setProcessError( "couldn't access memory mapped file (open): %s", strerror( errno ) );
        return NULL;
    } else
    {
        close(fd);
        fd = open(filename.toCharPtr(), O_RDWR);
        if(fd == -1)
        {
            UMBRA_DELETE(*impl);
            *impl = 0;
            setProcessError( "couldn't access memory mapped file (open): %s", strerror( errno ) );
            return NULL;
        }
    }

    data = mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
    if(data == MAP_FAILED)
    {
        close(fd);
        UMBRA_DELETE(*impl);
        *impl = 0;
        setProcessError( "couldn't access memory mapped file (mmap): %s", strerror( errno ) );
        return NULL;
    }

    close(fd);
    return data;

    //if(size < SHMMIN) size = SHMMIN;
    //if(size < 4096) size = 4096;

    /*key_t key = atoi(identifier.toCharPtr());

    (*impl)->shmid = shmget(key, size, IPC_CREAT | 0777);

    if((*impl)->shmid < 0)
    {
       delete *impl;
       *impl = 0;
       setProcessError("shmget failed: %d", (*impl)->shmid);
       return NULL;
    }

    data = shmat((*impl)->shmid, NULL, 0);

    if(data == (char*)-1)
    {
        delete *impl;
        *impl = 0;
        setProcessError("shmat failed");
        return NULL;
    }

    return data; */
}

void Umbra::processFree(void* data, ProcessSharedMemoryImpl* impl)
{
    munmap(data, impl->size);
    UMBRA_DELETE(impl);
}

namespace Umbra
{
    class ImplProcess
    {
    public:
        ImplProcess() :
           started(false),
           pid(0)
        {
        }

        bool                started;
        String              executable;
        Array<String>       commandline;
        pid_t               pid;
    };

} // namespace Umbra

Process::Process()
{
    m_impl = UMBRA_NEW(ImplProcess);
}

Process::Process(const String& executable)
{
    m_impl = UMBRA_NEW(ImplProcess);

    m_impl->executable = executable;
}

Process::Process(const String& executable, Array<String>& commandline)
{
    m_impl = UMBRA_NEW(ImplProcess);

    m_impl->executable = executable;
    m_impl->commandline = commandline;
}

void Process::setExecutable(const String& executable)
{
    if(m_impl->started)
        return;

    m_impl->executable = executable;
}

void Process::setCommandLine(Array<String>& commandline)
{
    if(m_impl->started)
        return;

    m_impl->commandline = commandline;
}

Process::~Process(void)
{
    waitToFinish();
    UMBRA_DELETE(m_impl);
}

Process::Error Process::run(void)
{
    processError = "";

    if(m_impl->started)
        return E_OTHER;

    FILE* f = fopen(m_impl->executable.toCharPtr(), "rb");
    if(f == NULL)
    {
        setProcessError("executable not found: \"%s\"", m_impl->executable.toCharPtr());
        return ERROR_EXECUTABLE_NOT_FOUND;
    }

    fclose(f);

    pid_t child_pid = fork();

    char** argv = UMBRA_NEW_ARRAY(char*, m_impl->commandline.getSize()+2);

    argv[0] = UMBRA_NEW_ARRAY(char, m_impl->executable.length()+1);
    memcpy(argv[0], m_impl->executable.toCharPtr(), m_impl->executable.length()+1);

    for(int i = 0; i < m_impl->commandline.getSize(); i++)
    {
        argv[i+1] = UMBRA_NEW_ARRAY(char, m_impl->commandline[i].length()+1);
        memcpy(argv[i+1], m_impl->commandline[i].toCharPtr(), m_impl->commandline[i].length()+1);
    }
    argv[m_impl->commandline.getSize()+1] = NULL;

    if(child_pid < 0)
    {
        setProcessError("fork failed: %s", strerror( errno ) );
        return E_OTHER;
    } else
    if(child_pid == 0)
    {
        execv(m_impl->executable.toCharPtr(), argv);

        setProcessError("execv \"%s\" failed: %s\n", m_impl->executable.toCharPtr(), strerror( errno ) );
        exit(0);
    }

    for(int i = 0; i < m_impl->commandline.getSize()+1; i++)
    {
        UMBRA_DELETE_ARRAY(argv[i]);
    }

    UMBRA_DELETE_ARRAY(argv);

    m_impl->pid = child_pid;

    return ERROR_OK;

}

bool Process::isFinished(void) const
{
    if(m_impl->pid == 0)
        return true;

    int status = 0;
    pid_t ret = waitpid(m_impl->pid, &status, WCONTINUED | WNOHANG | WUNTRACED);

    if(ret == 0)
        return false;

    if(!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status))
        return false;

    return true;
}

uint32 Process::getExitCode(void) const
{
    if(m_impl->pid == 0)
        return 0;

    int status;
    pid_t ret = waitpid(m_impl->pid, &status, WCONTINUED | WNOHANG | WUNTRACED);

    if(ret == 0)
        return 0;

    if(WIFEXITED(status))
        return WEXITSTATUS(status);

    return 0;
}

void Process::waitToFinish(void)
{
    if(m_impl->pid == 0)
        return;

    int status = 0;
    pid_t ret = 0;

    do
    {
        ret = waitpid(m_impl->pid, &status, WCONTINUED | WUNTRACED);
    } while(ret == 0 || (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status)));
}

HandleProcess::HandleProcess(ParentProcess)
    : m_isParent(true)
{
    m_handle = getppid();
}

HandleProcess::HandleProcess(HandleProcess::OSProcessHandle handle)
    : m_handle(handle),
      m_isParent(false)
{
#if defined (UMBRA_DEBUG)
    int status = 0;
    // works only for child processes under posix
    UMBRA_ASSERT(waitpid(m_handle, &status, WCONTINUED | WNOHANG | WUNTRACED) == m_handle);
#endif
}

bool HandleProcess::isFinished(void) const
{
    if (m_isParent)
    {
        // Under posix, orphaned processes are adopted by another process, e.g. init under Linux.
        // We'll know that the parent has died if parent's pid changes.
        // This might fail in the unlikely event that parent dies immediately after
        // launching child.
        return getppid() != m_handle;
    }

    if(m_handle == 0)
        return false;

    int status = 0;
    pid_t ret = waitpid(m_handle, &status, WCONTINUED | WNOHANG | WUNTRACED);

    if(ret == -1)
    {
        setProcessError( "waitpid error: %s", strerror( errno ) );
        return false;
    }

    if(ret == 0)
        return false;

    if(!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status))
        return false;

    return true;
}

uint32 HandleProcess::getExitCode(void) const
{
    if (m_isParent)
    {
        // Fine. There's no way.
        UMBRA_ASSERT(false);
        return 0;
    }

    if(m_handle == 0)
        return 0;

    int status;
    pid_t ret = waitpid(m_handle, &status, WCONTINUED | WNOHANG | WUNTRACED);

    if(ret == -1)
    {
        setProcessError( "waitpid error: %s", strerror( errno ) );
        return 0;
    }

    if(ret == 0)
        return 0;

    if(WIFEXITED(status))
        return WEXITSTATUS(status);

    return 0;
}

void HandleProcess::waitToFinish(void)
{
    if (m_isParent)
    {
        // Fine. There's no way.
        UMBRA_ASSERT(false);
        return;
    }

    if(m_handle == 0)
        return;

    int status;
    pid_t ret = 0;

    do
    {
        ret = waitpid(m_handle, &status, WCONTINUED | WUNTRACED);

        if(ret == -1)
        {
            setProcessError( "waitpid error: %s", strerror( errno ) );
            return;
        }
    } while(ret == 0 || (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status)));
}

String Umbra::generateProcessUID(void)
{
    UUID uuid = UUID::generate();
    char str[UUID::charLength] = "";
    uuid.string(str);
    return String(str);
}

bool Process::is64BitProcess()
{
#if __LP64__ || __x86_64__
    return true;
#else
    return false;
#endif
}

bool Process::is64BitCapable()
{
#if __LP64__ || __x86_64__
    return true;
#else
    return false;
#endif
}

// Expect UUIDs to be 128 bits
UMBRA_CT_ASSERT(sizeof(uuid_t) == 16);

UUID UUID::generate(void)
{
    UUID   result;
    uuid_t system;
    uuid_generate_time(system);

    memcpy(&result, &system, 16);

#if UMBRA_BYTE_ORDER == UMBRA_LITTLE_ENDIAN
    UINT8* data = (UINT8*)&result;
    for (int i = 0; i < 16; i+=4)
    {
        swap(data[i],   data[i+3]);
        swap(data[i+1], data[i+2]);
    }
#endif

    return result;
}


#endif
