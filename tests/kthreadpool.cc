#include "doctest.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#include "kthreadpool/kthreadpool.hh"

TEST_SUITE("KThreadPool")
{
    TEST_CASE("KThreadPool - execute two tasks on the same key")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(1);
            ktpool.add_task(1, [&i]() { ++i; return true; });
            ktpool.add_task(1, [&i]() { i = i * 2; return true; });
        }
        CHECK(i == 2);
    }

    /*
    TEST_CASE("KThreadPool - execute tasks with different keys")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(8);
            ktpool.add_task(1, [&i]() { printf("A\n"); ++i; std::this_thread::sleep_for(10000ms); printf("Task A completed\n"); return true; });
            ktpool.add_task(1, [&i]() { printf("B\n"); i = i * 2; printf("Task B completed\n"); return true; });
            while (i == 0)
                ;
            ktpool.add_task(2, [&i]() { printf("C\n"); i += 2; printf("Task C completed\n"); return true; });
        }
        CHECK(i == 6);
    }
     */

}