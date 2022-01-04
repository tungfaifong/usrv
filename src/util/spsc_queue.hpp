// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>
// A single producer single consumer lock-free queue based on ring buffer.

#ifndef USRV_SPSC_QUEUE_HPP
#define USRV_SPSC_QUEUE_HPP

#include <atomic>
#include <cstring>
#include <new>

namespace usrv
{

#define MIN(x, y) x < y ? x : y

template <size_t BLOCK_NUM>
class SpscQueue
{
public:
#ifdef __cpp_lib_hardware_interference_size
	static constexpr size_t CACHELINE_SIZE = std::hardware_destructive_interference_size;
	static constexpr size_t BLOCK_SIZE = CACHELINE_SIZE;
#else
	static constexpr size_t CACHELINE_SIZE = 64;
	static constexpr size_t BLOCK_SIZE = CACHELINE_SIZE;
#endif

	static constexpr size_t BYTES = BLOCK_NUM * BLOCK_SIZE;
	static_assert(BLOCK_NUM && !(BLOCK_NUM & (BLOCK_NUM - 1)), "BLOCK_NUM must be a power of 2");

	struct Header
	{
		uint16_t size;
		uint16_t data16;
		uint32_t data32;
	};
	static constexpr size_t HEADER_SIZE = sizeof(Header);

	// block push
	void Push(const char * data, Header header)
	{
		const auto write_idx = _write_idx.load(std::memory_order_relaxed);
		const auto need_block = (header.size + HEADER_SIZE - 1) / BLOCK_SIZE + 1;
		while (_empty_block < need_block)
		{
			_empty_block = BLOCK_NUM - write_idx + _read_idx.load(std::memory_order_acquire);
		}

		const auto offset = (write_idx & (BLOCK_NUM - 1)) * BLOCK_SIZE;
		memcpy(_buff + offset, &header, HEADER_SIZE);
		const auto len = MIN(header.size, BYTES - offset - HEADER_SIZE);
		memcpy(_buff + offset + HEADER_SIZE, data, len);
		memcpy(_buff, data + len, header.size - len);

		_empty_block -= need_block;

		_write_idx.store(write_idx + need_block, std::memory_order_release);
	}

	bool TryPush(const char * data, Header header)
	{
		const auto write_idx = _write_idx.load(std::memory_order_relaxed);
		const auto need_block = (header.size + HEADER_SIZE - 1) / BLOCK_SIZE + 1;
		if (_empty_block < need_block)
		{
			_empty_block = BLOCK_NUM - write_idx + _read_idx.load(std::memory_order_acquire);
			if (_empty_block < need_block)
			{
				// full return
				return false;
			}
		}

		const auto offset = (write_idx & (BLOCK_NUM - 1)) * BLOCK_SIZE;
		memcpy(_buff + offset, &header, HEADER_SIZE);
		const auto len = MIN(header.size, BYTES - offset - HEADER_SIZE);
		memcpy(_buff + offset + HEADER_SIZE, data, len);
		memcpy(_buff, data + len, header.size - len);

		_empty_block -= need_block;

		_write_idx.store(write_idx + need_block, std::memory_order_release);
		return true;
	}

	bool TryPop(char * data, Header * header)
	{
		const auto read_idx = _read_idx.load(std::memory_order_relaxed);
		const auto write_idx = _write_idx.load(std::memory_order_acquire);
		if (read_idx == write_idx)
		{
			// empty return
			return false;
		}

		const auto offset = (read_idx & (BLOCK_NUM - 1)) * BLOCK_SIZE;
		memcpy(header, _buff + offset, HEADER_SIZE);
		const auto len = MIN(header->size, BYTES - offset - HEADER_SIZE);
		memcpy(data, _buff + offset + HEADER_SIZE, len);
		memcpy(data + len, _buff, header->size - len);

		_read_idx.store(read_idx + (header->size + HEADER_SIZE - 1) / BLOCK_SIZE + 1, std::memory_order_release);
		return true;
	}

	bool Empty()
	{
		return _write_idx.load(std::memory_order_acquire) == _read_idx.load(std::memory_order_acquire);
	}

private:
	alignas(CACHELINE_SIZE) char _buff[BYTES] = {};
	alignas(CACHELINE_SIZE) std::atomic<size_t> _write_idx = {0};
	alignas(CACHELINE_SIZE) std::atomic<size_t> _read_idx = {0};
	alignas(CACHELINE_SIZE) size_t _empty_block = 0;
};

}

#endif // USRV_SPSC_QUEUE_HPP