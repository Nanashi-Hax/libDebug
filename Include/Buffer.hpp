#pragma once

#include <cstdint>
#include <atomic>

namespace Library::Debug
{
    class SpinMutex
    {
    public:
        void lock()
        {
            while (mFlag.test_and_set(std::memory_order_acquire)){}
        }

        void unlock()
        {
            mFlag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
    };

    template<typename K, typename V, uint32_t Max>
    class Map
    {
    public:
        bool insert(const K& key, const V& value)
        {
            LockGuard guard(mMutex);
            for (uint32_t i = 0; i < count; i++)
            {
                if (data[i].key == key)
                {
                    data[i].value = value;
                    return true;
                }
            }
            if (count >= Max) return false;
            data[count++] = { key, value };
            return true;
        }

        bool exist(const K& key) const
        {
            LockGuard guard(mMutex);
            for (uint32_t i = 0; i < count; i++) if (data[i].key == key) return true;
            return false;
        }

        bool try_get(const K& key, V& out) const
        {
            LockGuard guard(mMutex);
            for (uint32_t i = 0; i < count; i++)
            {
                if (data[i].key == key)
                {
                    out = data[i].value;
                    return true;
                }
            }
            return false;
        }

        bool erase(const K& key)
        {
            LockGuard guard(mMutex);
            for (uint32_t i = 0; i < count; i++)
            {
                if (data[i].key == key)
                {
                    data[i] = data[count - 1]; // swap remove
                    count--;
                    return true;
                }
            }
            return false;
        }

        uint32_t size() const
        {
            LockGuard guard(mMutex);
            return count;
        }

    private:
        class LockGuard
        {
        public:
            explicit LockGuard(SpinMutex& mutex) : mMutex(mutex)
            {
                mMutex.lock();
            }

            ~LockGuard()
            {
                mMutex.unlock();
            }

        private:
            SpinMutex& mMutex;
        };

        struct Entry
        {
            K key;
            V value;
        };

        Entry data[Max];
        mutable SpinMutex mMutex{};
        uint32_t count = 0;
    };

    template<typename T, uint32_t Size>
    class RingBuffer
    {
        static_assert((Size & (Size - 1)) == 0, "Size must be power of two");
        static constexpr uint32_t kMask = Size - 1;
    
    public:
        RingBuffer()
        {
            for (uint32_t i = 0; i < Size; i++)
            {
                mSlots[i].sequence.store(i, std::memory_order_relaxed);
            }
        }

        bool push(const T& value)
        {
            uint32_t tail = mTail.load(std::memory_order_relaxed);
            while (true)
            {
                Slot& slot = mSlots[tail & kMask];
                uint32_t sequence = slot.sequence.load(std::memory_order_acquire);
                int32_t diff = static_cast<int32_t>(sequence) - static_cast<int32_t>(tail);

                if (diff == 0)
                {
                    if (mTail.compare_exchange_weak(tail, tail + 1, std::memory_order_relaxed))
                    {
                        slot.value = value;
                        slot.sequence.store(tail + 1, std::memory_order_release);
                        return true;
                    }
                }
                else if (diff < 0)
                {
                    return false; // full
                }
                else
                {
                    tail = mTail.load(std::memory_order_relaxed);
                }
            }
        }
    
        bool pop(T& out)
        {
            uint32_t head = mHead.load(std::memory_order_relaxed);
            while (true)
            {
                Slot& slot = mSlots[head & kMask];
                uint32_t sequence = slot.sequence.load(std::memory_order_acquire);
                int32_t diff = static_cast<int32_t>(sequence) - static_cast<int32_t>(head + 1);

                if (diff == 0)
                {
                    if (mHead.compare_exchange_weak(head, head + 1, std::memory_order_relaxed))
                    {
                        out = slot.value;
                        slot.sequence.store(head + Size, std::memory_order_release);
                        return true;
                    }
                }
                else if (diff < 0)
                {
                    return false; // empty
                }
                else
                {
                    head = mHead.load(std::memory_order_relaxed);
                }
            }
        }
    
        void clear()
        {
            while (pop_discard())
            {
            }
        }
    
    private:
        struct Slot
        {
            std::atomic<uint32_t> sequence{0};
            T value{};
        };

        bool pop_discard()
        {
            uint32_t head = mHead.load(std::memory_order_relaxed);
            while (true)
            {
                Slot& slot = mSlots[head & kMask];
                uint32_t sequence = slot.sequence.load(std::memory_order_acquire);
                int32_t diff = static_cast<int32_t>(sequence) - static_cast<int32_t>(head + 1);

                if (diff == 0)
                {
                    if (mHead.compare_exchange_weak(head, head + 1, std::memory_order_relaxed))
                    {
                        slot.sequence.store(head + Size, std::memory_order_release);
                        return true;
                    }
                }
                else if (diff < 0)
                {
                    return false; // empty
                }
                else
                {
                    head = mHead.load(std::memory_order_relaxed);
                }
            }
        }

        alignas(64) std::atomic<uint32_t> mHead{0};
        alignas(64) std::atomic<uint32_t> mTail{0};
        Slot mSlots[Size]{};
    };
}
