#include "ipc/FileMapping.hh"
#include <iostream>
#include <thread>
#include <string>

int main()
{
    std::thread t1([]()
                   {
                       FileMapping map1;
                       if (!map1.Create("Local\\test1", 50))
                       {
                           std::cout << "Failed to open\n";
                           return 0;
                       }
                       std::string msg("Hello there");
                       std::cout << map1.Write(msg.c_str(), msg.length()) << std::endl;
                       std::cout << map1.Read(msg.length()) << std::endl;
                       std::cout<< "Done\n";
                       Sleep(40000);
                   });
    std::thread t2([]()
                   {
                       Sleep(2000);
                       FileMapping map1;
                       if (!map1.Open("Local\\test1", 50))
                       {
                           std::cout << "Failed to open\n";
                           return;
                       }
                       std::string msg = map1.Read(11);
                       std::cout << "Other: " << msg << std::endl;
                   });
    t1.join();
    t2.join();
}