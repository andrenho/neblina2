#include "doctest.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#include "kthreadpool/kthreadpool.hh"

TEST_SUITE("KThreadPool")
{
    TEST_CASE("KThreadPool - execute ")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(1);
            ktpool.add_task(1, [&i]() { ++i; return true; });
            ktpool.add_task(1, [&i]() { i = i * 2; return true; });
        }
        CHECK(i == 2);
    }

    TEST_CASE("KThreadPool - execute ")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(8);
            ktpool.add_task(1, [&i]() { ++i; std::this_thread::sleep_for(200ms); return true; });
            ktpool.add_task(1, [&i]() { i = i * 2; return true; });
            std::this_thread::sleep_for(120ms);
            ktpool.add_task(2, [&i]() { i += 2; return true; });
        }
        CHECK(i == 6);
    }

}