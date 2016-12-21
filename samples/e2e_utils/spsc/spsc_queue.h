#pragma once

#include <utility>
#include <new>
#include <type_traits>
#include <atomic>
#include <memory>

namespace e2e
{
    template <class ElemT, int Count>
    class constant_storage
    {
        static_assert(Count > 0, "Count must be a positive number");
        using ElemStorage = char[sizeof(ElemT)];

        ElemStorage storage[Count];
    public:
        using store_t = ElemStorage;

        store_t* data() { return storage; }

        template <class... ArgT>
        ElemT* emplace(store_t* ptr, ArgT&&... args)
        {
            auto place = reinterpret_cast<ElemT*>(ptr);
            new(place) ElemT(std::forward<ArgT>(args)...);
            return place;
        }

        auto capacity() const { return Count; }
    };

    template <class ElemT>
    class dynamic_storage
    {
        using ElemStorage = char[sizeof(ElemT)];

        int len;
        std::unique_ptr<ElemStorage[]> storage;
    public:

        dynamic_storage(int elem_count) : len(elem_count), storage(new ElemStorage[len]) {}
        using store_t = ElemStorage;

        store_t* data() { return storage.get(); }

        template <class... ArgT>
        ElemT* emplace(store_t* ptr, ArgT&&... args)
        {
            auto place = reinterpret_cast<ElemT*>(ptr);
            new(place) ElemT(std::forward<ArgT>(args)...);
            return place;
        }

        auto capacity() const { return len; }
    };

    template <class ElemT, class Storage = constant_storage<ElemT, 64>>
    class spsc_queue : private Storage
    {
        typename Storage::store_t* const data = Storage::data();

        int head = 0; // first active index
        int tail = 0; // one-past last active index
        std::atomic<int> size_;

        /*
            make sure given index contains a constructed actual object
        */
        ElemT& elem_at(int index) const
        {
            auto base_ptr = reinterpret_cast<ElemT* const>(data);
            return *(base_ptr + index);
        }

        void push()
        {
            tail = (tail + 1) % Storage::capacity();
            ++size_;
        }

    public:

        template <class... ArgT>
        spsc_queue(ArgT&&... args) : Storage(std::forward<ArgT>(args)...), size_(0) {}

        ~spsc_queue()
        {
            // pop any left over objects
            clear();
        }

        /*
            copies the given element to the queue
            undefined behaviour if (size == capacity)
        */
        void push(const ElemT& elem) // copy
        {
            Storage::emplace(data + tail, elem); // may throw
            push();
        }

        /*
            moves the given element to the queue
            undefined behaviour if (size == capacity)
        */
        void push(ElemT&& elem) // move
        {
            Storage::emplace(data + tail, std::move(elem)); // may throw
            push();
        }

        /*
            emplaces a new element in the queue with the given arguments
            undefined behaviour if (size == capacity)
        */
        template <class... ArgT>
        void emplace(ArgT&&... args)
        {
            Storage::emplace(data + tail, std::forward<ArgT>(args)...); // may throw
            push();
        }

        const ElemT& front() const
        {
            return elem_at(head);
        }
        
        ElemT& front()
        {
            return elem_at(head);
        }
        
        /*
            destructs and pops the front element
            undefined behaviour if size == 0
        */
        void pop()
        {
            if(!std::is_trivially_destructible<ElemT>::value)
            {
                elem_at(head).~ElemT();
            }
            head = (head + 1) % Storage::capacity();
            --size_;
        }

        void clear()
        {
            auto sz = size();
            while (sz --> 0)
            {
                pop();
            }
        }

        auto capacity() const
        {
            return Storage::capacity();
        }

        auto size() const
        {
            return size_.load();
        }

        auto empty() const
        {
            return size() == 0;
        }
    };
}