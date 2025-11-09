#ifdef _WIN32

#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>
#include <psapi.h>
#include <versionhelpers.h>

std::wstring GetSystemInfoText()
{
    std::wstringstream ss;
    
    OSVERSIONINFOEXW os = { sizeof(os) };

    if (IsWindows10OrGreater()){
        ss << "OC: Windows 10 or Greater\n";
    }
    else{
        ss << "OC: Windows less than 10\n";
    }

    WCHAR computerName[256];
    DWORD size = 256;
    GetComputerNameW(computerName, &size);
    ss << L"Computer name: " << computerName << L"\n";
    WCHAR userName[256];
    size = 256;
    GetUserNameW(userName, &size);
    ss << L"User: " << userName << L"\n";
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    ss << L"Architecture: ";
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: ss << L"x64\n"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: ss << L"x86\n"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: ss << L"ARM64\n"; break;
        default: ss << L"Unknown\n"; break;
    }
    MEMORYSTATUSEX mem = { sizeof(mem) };
    GlobalMemoryStatusEx(&mem);
    PERFORMANCE_INFORMATION perf {};
    GetPerformanceInfo(&perf,sizeof(PERFORMACE_INFORMATION));
    unsigned long pageSize = perf.PageSize;
    unsigned long commitLimitMB = (perf.CommitLimit * pageSize) / (1024 * 1024);
    unsigned long commitTotalMB = (perf.CommitTotal * pageSize) / (1024 * 1024);
    unsigned long totalRAM_MB = mem.ullTotalPhys / (1024 * 1024);
    unsigned long usedRAM_MB  = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024);
    unsigned long pagefileTotalMB = commitLimitMB - totalRAM_MB;
    unsigned long pagefileUsedMB  = commitTotalMB - usedRAM_MB;
    ss << L"RAM: " << usedRAM_MB << L"MB / " << totalRAM_MB << L"MB\n";
    ss << L"Memory load: " << mem.dwMemoryLoad << L"%\n";
    ss << L"Virtual Memory: " << commitLimitMB << L"MB\n";
    ss << L"PageFile: " << pagefileUsedMB << L"MB / " << pagefileTotalMB << L"MB\n";
    ss << L"\nProcessors: " << si.dwNumberOfProcessors << L"\n";
    ss << L"Drivers: \n";
    WCHAR buffer[256];
    DWORD drives = GetLogicalDriveStringsW(256,buffer);
    
    if (drives > 0 && drives < 256){
        WCHAR* drive = buffer;
        
        
        while (*drive){
            WCHAR fsName[32] = L"";
            GetVolumeInformationW(drive, nullptr, 0, nullptr, nullptr, nullptr, fsName, 32);
            ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
            if (GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &freeBytes)) {
                unsigned long long totalGB = totalBytes.QuadPart / (1024ULL*1024*1024);
                unsigned long long freeGB  = freeBytes.QuadPart  / (1024ULL*1024*1024);

                ss << "  - " << std::wstring(drive) << "  ("
                   << std::wstring(fsName) << "): "
                   << freeGB << " GB free / "
                   << totalGB << " GB total\n";
            }
            drive += wcslen(drive) + 1;

        }
        return ss.str();
    };
}

int main()
{
    std::wstring sysInfo = GetSystemInfoText();
    std::wcout << sysInfo;
}

#elif __linux__
#include <iostream>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#include <mntent.h>
#include <sys/statfs.h>

std::string getOS()
{
    std::ifstream f("/etc/os-release");
    std::string line;
    while (std::getline(f,line)){
        if (line.rfind("PRETTY_NAME=",0) == 0){
            std::string res = line.substr(13);
            res.pop_back();
            return res;
        }
    };
}

unsigned long int getVirtualRAM()
{
    std::ifstream f("/roc/meminfo");
    unsigned long long value;
    std::string key;
    std::string unit;
    while (f >> key >> value >> unit){
        if (key == "VmallocTotal:"){
            unsigned long long mb = value / 1024;
            return mb;
        }
    }
}

std::string getLoads()
{
    std::ifstream f("/proc/loadavg");
    double l1,l2,l3;
    f >> l1 >> l2 >> l3;
    return std::to_string(l1) + ", " + std::to_string(l2) + ", " + std::to_string(l3);
}

int main()
{
    struct sysinfo sys;
    sysinfo(&sys);
    struct utsname sys2;
    uname(&sys2);
    FILE* drivers = setmntent("/proc/mounts","r"); 
    std::string username = getlogin();
    char hostname[256];
    size_t size = 256;
    gethostname(hostname,size);
    std::cout << "OC: " << getOS() << std::endl;
    std::cout << "Kernel: " << sys2.sysname << " " << sys2.release << std::endl;
    std::cout << "Architecture: " << sys2.machine << std::endl;
    std::cout << "Hostname: " << hostname << std::endl;
    std::cout << "User: " << username << std::endl;
    std::cout << "RAM: " << sys.freeram / 1024 /1024 << "MB free / " << sys.totalram / 1024  / 1024 << "MB total" << std::endl;
    std::cout << "SWAP: " << sys.totalswap/ 1024/1024 << "MB total / " << sys.freeswap /1024/1024 <<"MB free" << std::endl;
    std::cout << "Processors: " << get_nprocs() << std::endl;
    std::cout << "Load average: " << getLoads() << std::endl;
    std::cout << "Drivers: '\n'";    
    struct mntent* ent;
    while((ent = getmntent(drivers)) != nullptr) {
        struct statfs st;
        if (statfs(ent->mnt_dir, &st) != 0) continue;
        unsigned long long freeMB = st.f_bavail * st.f_frsize / 1024 / 1024;
        unsigned long long totalMB = st.f_blocks * st.f_frsize / 1024 / 1024;
        std::cout << " " << ent->mnt_dir << "\t"
                  << ent->mnt_fsname << "\t"
                  << ent->mnt_type << "\t"
                  << freeMB << "MB free / "
                  << totalMB << "MB total\n";
    }
    endmntent(drivers);
    std::cout << "Virtual memory: " << getVirtualRAM() << std::endl;
}
#endif