#include "Planet/FpsLocker.hh"

int main()
{
    FpsLocker locker;
    locker.SetFps(60);
    int i = 0;
    while (true)
    {
        i++;
        locker.FrameEnd();
    }
}

// #include <iostream>
// #include <cstdio>
// #include <chrono>
// #include <thread>

// std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
// std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

// int main()
// {
//     while (true)
//     {
//         // Maintain designated frequency of 5 Hz (200 ms per frame)
//         a = std::chrono::system_clock::now();
//         std::chrono::duration<double, std::milli> work_time = a - b;

//         if (work_time.count() < 200.0)
//         {
//             std::chrono::duration<double, std::milli> delta_ms(200.0 - work_time.count());
//             auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
//             std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
//         }

//         b = std::chrono::system_clock::now();
//         std::chrono::duration<double, std::milli> sleep_time = b - a;

//         // Your code here

//         printf("Time: %f \n", (work_time + sleep_time).count());
//     }
// }