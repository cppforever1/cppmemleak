#pragma once

#ifndef NEWDELETE
#define NEWDELETE

#include <map>
#include <string>
#include <time.h>
#include <ctime>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <errno.h>
#include <chrono>
#include <sstream>

const size_t MAX_PATH = PATH_MAX;
extern char *program_invocation_short_name;

typedef struct _INFO
{
    bool bISArray;
    time_t DateTime;
    size_t ThreadId;
    pid_t ProcessId;
    void *pMemory;
    size_t MemorySize;
    size_t Linenumber;
    char Filename[MAX_PATH];
    char Function[MAX_PATH];
    char ProcessName[MAX_PATH];
} INFO, *PINFO;

// Function to convert time_t to a string with milliseconds
std::string timeToStringWithMs(time_t timeValue)
{
    // Convert time_t to struct tm
    std::tm tm = *std::localtime(&timeValue);

    // Get current time with milliseconds
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Format time
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms.count();

    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const INFO &info)
{
    os << "ptr = " << info.pMemory << std::endl
       << "size = " << info.MemorySize << std::endl
       << "line = " << info.Linenumber << std::endl
       << "ProcessID = " << info.ProcessId << std::endl
       << "ThreadID = " << info.ThreadId << std::endl
       << "ProcessName = " << info.ProcessName << std::endl
       << "Function = " << info.Function << std::endl
       << "File = " << info.Filename << std::endl
       << "Is Array[] = " << (info.bISArray ? "True" : "False") << std::endl
       << "Time = " << timeToStringWithMs(info.DateTime);
    return os;
}

std::map<size_t, std::map<void *, PINFO>> MemoryMap;

void *AddMemory(size_t memsize, char const *filename, char const *function, size_t linenumber, bool bISArray)
{
    const size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

    PINFO pmem = (PINFO)malloc(sizeof(INFO));

    if (nullptr != pmem)
    {
        pmem->bISArray = bISArray;
        pmem->DateTime = time(0);
        pmem->pMemory = malloc(memsize);
        pmem->Linenumber = linenumber;
        pmem->MemorySize = memsize;
        pmem->ProcessId = getpid();
        pmem->ThreadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
        const std::size_t FilenameCopySize = std::min(MAX_PATH - 1, strlen(filename));
        strncpy(pmem->Filename, filename, FilenameCopySize);
        pmem->Filename[MAX_PATH - 1] = '\0'; // Ensure null termination
        const std::size_t FunctionCopySize = std::min(MAX_PATH - 1, strlen(function));
        strncpy(pmem->Function, function, FunctionCopySize);
        pmem->Function[MAX_PATH - 1] = '\0'; // Ensure null termination
        const std::size_t ProcessNameCopySize = std::min(MAX_PATH - 1, strlen(program_invocation_short_name));
        strncpy(pmem->ProcessName, program_invocation_short_name, ProcessNameCopySize);
        pmem->ProcessName[MAX_PATH - 1] = '\0'; // Ensure null termination

        // save leak per thread in map
        MemoryMap[tid][pmem->pMemory] = pmem;
    }

    return pmem->pMemory;
}

void RemoveMemory(void *mem)
{
    const size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

    // if thread not found return
    if (MemoryMap.find(tid) == MemoryMap.end())
        return;

    // if memory found?
    if (MemoryMap[tid].find(mem) != MemoryMap[tid].end())
    {
        PINFO pmem = (PINFO)MemoryMap[tid][mem];

        if (nullptr != pmem)
        {
            MemoryMap[tid].erase(mem);
            free(pmem->pMemory);
            free(pmem);
        }
    }
}

// delete all memory
void ClearAllMemoryLeaks()
{
    for (auto &[tid, mem] : MemoryMap)
    {
        for (auto &[tmem, tinfo] : mem)
        {
            if (nullptr != tmem)
            {
                RemoveMemory(tmem);
            }
        }

        MemoryMap.erase(tid);
    }
}

// print all undelete memory
void StreamMemoryLeak(std::ostream &os)
{
    int memoryLeakCounter = 0;

    for (auto &[tid, mem] : MemoryMap)
    {
        os << "Thread ID: " << tid << std::endl;

        for (auto &[tmem, tinfo] : mem)
        {
            if (nullptr != tmem)
            {
                memoryLeakCounter++;
                os << *tinfo << std::endl;
                os << std::string(80, '-') << std::endl;
            }
        }
    }

    if (0 == memoryLeakCounter)
    {
        os << std::endl << std::endl <<  "********** Well Done! NO memory link detected **********" << std::endl << std::endl;
        sleep(10000);
    }
}

void SaveMemoryLeak(const std::string filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open MemoryLeak.txt for writing." << std::endl;
        return;
    }

    StreamMemoryLeak(file);
    file.close();
}

void *operator new(size_t size, const char *filename, int line, const char *function)
{
    return AddMemory(size, filename, function, line, false);
}
void *operator new[](size_t size, const char *filename, int line, const char *function)
{
    return AddMemory(size, filename, function, line, true);
}
void operator delete(void *mem)
{
    RemoveMemory(mem);
}
void operator delete[](void *mem)
{
    RemoveMemory(mem);
}

#define new new (__FILE__, __LINE__, __FUNCTION__)

#endif // NEWDELETE