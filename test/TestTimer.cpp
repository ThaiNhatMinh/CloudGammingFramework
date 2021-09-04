#include <chrono>
#include <thread>
#include "ipc/WaitableTimer.hh"

int main()
{
    WaitableTimer timer;
    if (!timer.Create("Local/t1")) return -1;
    timer.SetTime(1000);
    auto start = std::chrono::high_resolution_clock::now();
    if (WaitForSingleObject(timer.GetHandle(), INFINITE) != WAIT_OBJECT_0)
        printf("WaitForSingleObject failed (%d)\n", GetLastError());
    else printf("Timer was signaled.\n");

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    std::cout << "Waited " << elapsed.count() << " ms\n";
    return 0;
}