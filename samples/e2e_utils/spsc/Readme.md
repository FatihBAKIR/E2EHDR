# Lock-free Single Producer Single Consumer Queue

This is a very bare bones lock free spsc queue implementation in c++14 (easily portable to c++11). As the name suggests, it is very useful for applications where you need to implement simplex communication between two threads.

Also, memory for elements is either automatic (capacity determined at compile time) or singly allocated (capacity determined at run time), therefore the container doesn't expand when more space is needed.

Objects of any type can be stored in the queue, be it non-copyable, non-movable or both. The type doesn't need to be default constructible.

## Usage

Basic Usage
---

```cpp

e2e::spsc_queue<int> ints; // default capacity is 64

ints.push(5);

std::cout << ints.front() << '\n';

ints.pop();

std::cout << ints.size() << '\n'

```

Compile time determined capacity
---

```cpp
e2e::spsc_queue<int, e2e::constant_storage<int, 1024>> ints;

assert(ints.capacity() == 1024);
```

Run time determined capacity
---

```cpp
e2e::spsc_queue<int, e2e::dynamic_storage<int>> ints(2048);

assert(ints.capacity() == 2048);
```

## Reference

+ `push`

`spsc_queue::push` has two overloads: one that makes a copy and one moves.

+ `emplace`

`spsc_queue::emplace` emplaces a new object directly on the internal storage.

This may be more desirable for expensive to copy/move or non-move/copy types.

```cpp
spsc_queue<copy_only_type> muts;
muts.push(copy_only_type(arguments...)); // expensive = construction + copy construction + destruction
muts.emplace(arguments...); // cheap = construction
```

+ `front`

`spsc_queue::front` returns a reference to the front object stored in the queue.

Remember that `pop` doesn't return anything, so you have to acquire the object using this method before popping from the queue.

+ `pop`

`spsc_queue::pop` destroys the front object.

+ `size`

Returns the number of objects in the queue.

+ `capacity`

Returns the total capacity of the queue.

+ `empty`

Returns whether the queue is empty or not.