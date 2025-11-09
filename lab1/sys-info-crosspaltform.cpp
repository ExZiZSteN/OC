#include "SysInfo.h"
#include <iostream>

int main()
{
    SysInfo system {};
    std::cout << "OS name: " << system.GetOSName() << std::endl;
    std::cout << "OS version: " <<  system.GetOSVersion() << std::endl;
    std::cout << "Free memory: " << system.GetFreeMemory() / 1024 /1024 <<"MB" << std::endl;
    std::cout << "Total memory: " << system.GetTotalMemory() / 1024 / 1024 << "MB" << std::endl;
    std::cout << "Processors: " << system.GetProcessorCount() << std::endl;
}
