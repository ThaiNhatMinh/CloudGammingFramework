#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <list>
#include <thread>
#include "doctest.h"
#include "ipc/Event.hh"
#include "ipc/PollHandle.hh"


void ThreadTest(PollHandle<2>* poll)
{
    auto start = std::chrono::high_resolution_clock::now();
    poll->Poll(1000);
    std::chrono::duration<float, std::milli> delta = std::chrono::high_resolution_clock::now() - start;
    CHECK(delta.count() < 1000);
}

TEST_CASE("Test Event class")
{
    const uint32_t NUM_EVENT = 20;
    Event main;
    std::list<Event> child;
    std::list<PollHandle<2>> poll;
    std::list<std::thread> threads;
    REQUIRE(main.Create("Local\\TestEvent"));
    for(uint32_t i = 0; i < NUM_EVENT; i++)
    {
        Event t;
        REQUIRE(t.Open("Local\\TestEvent"));
        child.push_back(std::move(t));
        poll.push_back({});
        REQUIRE(poll.back().AddEvent(child.back(), [](const Event* e){return PollAction::STOP_POLL;}));
        threads.emplace_back(ThreadTest, &poll.back());
    }
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(10));
    for(uint32_t i = 0; i < NUM_EVENT; i++)
    {
        main.Signal();
    }
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1500));
    
    for(auto iter = threads.begin(); iter != threads.end(); iter++)
    {
        iter->join();
    }
}
