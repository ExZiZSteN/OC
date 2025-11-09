#pragma once
#include <string>

#ifdef __linux__
#include <fstream>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

class SysInfo
{
    public:
    std::string GetOSName() const;
    std::string GetOSVersion() const;
    u_int64_t GetFreeMemory() const;
    u_int64_t GetTotalMemory() const;
    unsigned GetProcessorCount() const;
};

#elif _WIN32
#include <windows.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <VersionHelpers.h>
class SysInfo
{
    public:
    std::string GetOSName() const;
    std::string GetOSVersion() const;
    uint64_t GetFreeMemory() const;
    uint64_t GetTotalMemory() const;
    unsigned GetProcessorCount() const;
};

#endif