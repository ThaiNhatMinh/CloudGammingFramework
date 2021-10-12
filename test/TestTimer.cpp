#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <chrono>
#include <thread>
#include "ipc/WaitableTimer.hh"

TEST_CASE("Test WaitableTimer class")
{
    WaitableTimer timer;
    REQUIRE(timer.Create("Local/t1"));

    auto t = std::thread([&timer]()
    {
        Sleep(1000);
        timer.SetTime(1000);
    });
    auto start = std::chrono::high_resolution_clock::now();

    auto rc = WaitForSingleObject(timer.GetHandle(), 2500);
    CHECK(rc != WAIT_FAILED);
    CHECK(rc != WAIT_TIMEOUT);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    CHECK(elapsed.count() - 2000.0f <= 1.0f);
    t.join();
}