#include <iostream>
#include "spsc_queue.h"

#include <string>
#include <thread>
#include <cassert>
#include <memory>

bool basic_test()
{
    e2e::spsc_queue<int> ints;
    assert(ints.capacity() == 64);
    assert(ints.size() == 0);

    ints.push(3);

    assert(ints.front() == 3);
    assert(ints.size() == 1);

    ints.push(5);

    assert(ints.front() == 3);
    assert(ints.size() == 2);

    ints.pop();

    assert(ints.front() == 5);
    assert(ints.size() == 1);

    ints.pop();

    assert(ints.size() == 0);

    return true;
}

bool dynamic_test()
{
    e2e::spsc_queue<int, e2e::dynamic_storage<int>> ints {128};
    assert(ints.capacity() == 128);
    assert(ints.size() == 0);

    const auto cap = ints.capacity();

    for (int i = 0; i < cap; ++i)
    {
        ints.push(i);
    }

    assert(ints.size() == ints.capacity());

    for (int i = 0; i < cap; ++i)
    {
        assert(ints.front() == i);
        ints.pop();
    }

    return true;
}

bool thread_test_1()
{
    e2e::spsc_queue<int> some_ints;

    volatile bool run = true;

    std::thread producer([&]{
        int i = 0;

        while (i < 1000 * 100)
        {
            some_ints.push(i++ * 2);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        run = false;
    });

    std::thread consumer([&]{
        while(some_ints.empty());

        int i = 0;
        while (run || !some_ints.empty())
        {
            if (some_ints.empty()) continue;
            auto front = some_ints.front();
            some_ints.pop();
            assert(front / 2 == i++);
        }
    });

    producer.join();
    consumer.join();

    return true;
}

bool move_only_test()
{
    e2e::spsc_queue<std::unique_ptr<int>> ptrs;

    ptrs.push(std::make_unique<int>(3));

    assert(*ptrs.front() == 3);

    ptrs.pop();

    return true;
}

int main()
{
    std::cout << "Basic Test";
    assert(basic_test());
    std::cout << " Passed!\n";

    std::cout << "Dynamic Memory Test";
    assert(dynamic_test());
    std::cout << " Passed!\n";

    std::cout << "Concurrency Test" << std::flush;
    assert(thread_test_1());
    std::cout << " Passed!\n";

    std::cout << "Move Only Type Test";
    assert(move_only_test());
    std::cout << " Passed!\n";

    std::cout << "All Tests Passed!\n";
}