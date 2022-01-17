// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_HEAP_HPP
#define USRV_HEAP_HPP

#include <vector>

#include "common.h"

NAMESPACE_OPEN

template<typename T, typename CMP = std::less<T>>
class Heap
{
public:
#define PARENT(idx) (idx - 1) / 2
#define CHILD(idx) 2 * idx + 1
#define SIBLINGR(idx) idx + 1

	Heap() = default;
	~Heap() = default;

	size_t Emplace(T && obj)
	{
		_heap.emplace_back(obj);
		_ShiftUp(_heap.size() - 1);
		return 0;
	}

	T Pop(size_t pos = 0)
	{
		std::swap(_heap[pos], _heap[_heap.size() - 1]);
		auto obj = std::move(_heap.back());
		_heap.pop_back();
		_ShiftDown(pos);
		return obj;
	}

private:
	bool _ShiftUp(size_t index)
	{
		size_t current = index;
		size_t parent = PARENT(current);
		auto tmp = _heap[index];
		while (current > 0 && CMP()(tmp, _heap[parent]))
		{
			_heap[current] = _heap[parent];
			current = parent;
			parent = PARENT(parent);
		}
		_heap[current] = tmp;
		return true;
	}

	bool _ShiftDown(const size_t index)
	{
		size_t current = index;
		size_t child = CHILD(current);
		auto tmp = _heap[index];
		while(child < _heap.size())
		{
			if(SIBLINGR(child) < _heap.size() && CMP()(_heap[SIBLINGR(child)], _heap[child]))
			{
				child = SIBLINGR(child);
			}

			if(CMP()(tmp, _heap[child]))
			{
				break;
			}

			_heap[current] = _heap[child];
			current = child;
			child = CHILD(child);
		}
		_heap[current] = tmp;
		return true;
	}

private:
	std::vector<T> _heap;
};

NAMESPACE_CLOSE

#endif // USRV_HEAP_HPP