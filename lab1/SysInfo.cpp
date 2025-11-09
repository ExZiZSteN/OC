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
    return sys.freeram / 1024 / 1024;
}

u_int64_t SysInfo::GetTotalMemory() const{
    struct sysinfo sys;
    sysinfo(&sys);
    return sys.totalram / 1024 / 1024;
}

unsigned SysInfo::GetProcessorCount() const{
    return get_nprocs();
}
#elif _WIN32


#endif