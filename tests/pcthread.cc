#include "doctest.h"

#include <chrono>
using namespace std::chrono_literals;

#include "util/pcthread.hh"

class MyPCThread : public ProducerConsumerThread<int> {
public:
    MyPCThread() : ProducerConsumerThread("MyPCThread") {}

    int value() const { return value_.load(); }
protected:
    void action(int&& t) override { value_ += t; }
private:
    std::atomic<int> value_ { 0 };
};

TEST_SUITE("Producer Consumer Thread")
{
    TEST_CASE("Start/stop")
    {
        MyPCThread t;
        t.start();
        t.stop();
    }

    TEST_CASE("Produce/consume")
    {
        MyPCThread t;
        t.start();
        t.push(1);
        t.push(2);
        t.stop();
        CHECK(t.value() == 3);
    }
}