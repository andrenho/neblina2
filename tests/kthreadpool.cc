#include "doctest.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#include "kthreadpool/kthreadpool.hh"

TEST_SUITE("KThreadPool")
{
    TEST_CASE("execute two tasks on the same key")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(1);
            ktpool.add_task(1, [&i]() { ++i; return true; });
            ktpool.add_task(1, [&i]() { i = i * 2; return true; });
        }
        CHECK(i == 2);
    }

    TEST_CASE("execute tasks with different keys")
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(8);
            ktpool.add_task(1, [&i]() { ++i; std::this_thread::sleep_for(300ms); return true; });
            ktpool.add_task(1, [&i]() { i = i * 2; return true; });
            while (i == 0)
                ;
            ktpool.add_task(2, [&i]() { i += 2; return true; });
        }
        CHECK(i == 6);
    }

#ifndef VALGRIND
    TEST_CASE("Load test" * doctest::skip(getenv("VALGRIND") != nullptr))
    {
        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(16);
            for (size_t j = 0; j < 16; ++j)
                for (size_t k = 0; k < 1000; ++k)
                    ktpool.add_task(j, [&i]() { ++i; return true; });
        }
        CHECK(i == 16000);
    }
#endif

}