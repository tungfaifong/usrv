// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>
// A single producer single consumer lock-free queue based on ring buffer.

#ifndef USRV_SPSC_QUEUE_HPP
#define USRV_SPSC_QUEUE_HPP

#include <atomic>
#include <cassert>
#include <cstring>
#include <new>

#include "common.h"

NAMESPACE_OPEN

template<class T>
class SpscQueue
{
public:
#ifdef __cpp_lib_hardware_interference_size
	static constexpr size_t CACHELINE_SIZE = std::hardware_destructive_interference_size;
#else
	static constexpr size_t CACHELINE_SIZE = 64;
#endif

	SpscQueue(size_t size):_size(size), _slots(static_cast<T*>(malloc(sizeof(T) * size)))
	{
		assert(_size >= 2);
		if (!_slots) {
			throw std::bad_alloc();
		}
	}

	~SpscQueue()
	{
		if (!std::is_trivially_destructible<T>::value) {
			size_t read_idx = _read_idx;
			size_t write_idx = _write_idx;
			while (read_idx != write_idx) {
				_slots[read_idx].~T();
				if (++read_idx == _size) {
					read_idx = 0;
				}
			}
		}

		free(_slots);
	}

	template <class... Args>
	bool Push(Args&&... args)
	{
		const auto write_idx = _write_idx.load(std::memory_order_relaxed);
		auto next_idx = write_idx + 1;
		if (next_idx == _size) {
			next_idx = 0;
		}

		if (next_idx == _read_idx.load(std::memory_order_acquire))
		{
			return false;
		}

		new (&_slots[write_idx]) T(std::forward<Args>(args)...);
		_write_idx.store(next_idx, std::memory_order_release);

		return true;
	}

	bool Pop(T& t)
	{
		const auto read_idx = _read_idx.load(std::memory_order_relaxed);
		if (read_idx == _write_idx.load(std::memory_order_acquire))
		{
			return false;
		}

		auto next_idx = read_idx + 1;
		if (next_idx == _size) {
			next_idx = 0;
		}

		t = std::move(_slots[read_idx]);
		_slots[read_idx].~T();

		_read_idx.store(next_idx, std::memory_order_release);

		return true;
	}

	bool Empty()
	{
		return _write_idx.load(std::memory_order_acquire) == _read_idx.load(std::memory_order_acquire);
	}

	bool Full()
	{
		auto next_idx = _write_idx.load(std::memory_order_acquire) + 1;
		if (next_idx == _size) {
			next_idx = 0;
		}
		return next_idx == _read_idx.load(std::memory_order_acquire);
	}

private:
	alignas(CACHELINE_SIZE) size_t _size;
	T * _slots;

	alignas(CACHELINE_SIZE) std::atomic<size_t> _read_idx = {0};
	alignas(CACHELINE_SIZE) std::atomic<size_t> _write_idx = {0};

	char _padding[CACHELINE_SIZE - sizeof(std::atomic<size_t>)];
};

NAMESPACE_CLOSE

#endif // USRV_SPSC_QUEUE_HPP