#include "doctest.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#include "kthreadpool/kthreadpool.hh"

TEST_SUITE("KThreadPool")
{
    TEST_CASE("KThreadPool - execute ")
    {
        KThreadPool ktpool(8);
        size_t i = 0;
        ktpool.add_task(1, [&i]() { ++i; return true; });
        ktpool.add_task(1, [&i]() { i *= 2; return true; });
        std::this_thread::sleep_for(50ms);
        CHECK(i == 4);
    }

    TEST_CASE("KThreadPool - execute ")
    {
        KThreadPool ktpool(8);
        size_t i = 0;
        ktpool.add_task(1, [&i]() { ++i; std::this_thread::sleep_for(50ms); return true; });
        ktpool.add_task(1, [&i]() { i *= 2; return true; });
        std::this_thread::sleep_for(20ms);
        ktpool.add_task(2, [&i]() { i += 2; return true; });
        std::this_thread::sleep_for(50ms);
        CHECK(i == 6);
    }

}