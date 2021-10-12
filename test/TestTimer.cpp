#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <chrono>
#include <thread>
#include "ipc/WaitableTimer.hh"
#include "doctest.h"

TEST_CASE("Test Event class")
{
    WaitableTimer timer;
    REQUIRE(timer.Create("Local/t1"));

    auto t = std::thread([&timer]()
    {
        Sleep(1000);
        timer.SetTime(1000);
    });
    auto start = std::chrono::high_resolution_clock::now();
    if (WaitForSingleObject(timer.GetHandle(), INFINITE) != WAIT_OBJECT_0)
    {
        FAIL_CHECK("WaitForSingleObject failed", GetLastError());
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    std::cout << "Waited " << elapsed.count() << " ms\n";
    CHECK(elapsed.count() - 2000.0f <= 1.0f);
    t.join();
}