#include "SysInfo.h"

#ifdef __linux__

std::string SysInfo::GetOSName() const{
    std::ifstream f("/etc/os-release");
    std::string line;
    while (std::getline(f,line)){
        if (line.rfind("PRETTY_NAME=",0) == 0){
            std::string res = line.substr(13);
            res.pop_back();
            return res;
        }
    };
};

std::string SysInfo::GetOSVersion() const{
    struct utsname sys;
    uname(&sys);
    return sys.release;
};

u_int64_t SysInfo::GetFreeMemory() const{
    struct sysinfo sys;
    sysinfo(&sys);
    return sys.freeram;
}

u_int64_t SysInfo::GetTotalMemory() const{
    struct sysinfo sys;
    sysinfo(&sys);
    return sys.totalram;
}
unsigned SysInfo::GetProcessorCount() const{
    return get_nprocs();
}
#elif _WIN32

std::string SysInfo::GetOSName() const{
    std::stringstream ss;
    

    if (IsWindows10OrGreater()){
        ss << "Windows 10 or Greater";
    }
    else{
        ss << "Windows less than 10";
    }
    return ss.str();
};

std::string SysInfo::GetOSVersion() const{
    std::stringstream ss;
    OSVERSIONINFOEXW os = { sizeof(os) };
    GetVersionExW((OSVERSIONINFOW*)&os);
    ss << os.dwBuildNumber;
    return ss.str();
};

uint64_t SysInfo::GetFreeMemory() const{
    MEMORYSTATUSEX memory = {sizeof(memory)};
    GlobalMemoryStatusEx(&memory);
    return (memory.ullTotalPhys - memory.ullAvailPhys);
}

uint64_t SysInfo::GetTotalMemory() const{
    MEMORYSTATUSEX memory = {sizeof(memory)};
    GlobalMemoryStatusEx(&memory);
    return memory.ullTotalPhys  ;
}

unsigned SysInfo::GetProcessorCount() const{
    
    SYSTEM_INFO system;
    GetSystemInfo(&system);
    return system.dwNumberOfProcessors;
}

#endif