// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_HEAP_HPP
#define USRV_HEAP_HPP

#include <vector>

#include "common.h"
#include "object_list.hpp"

NAMESPACE_OPEN

template<typename T, typename CMP>
class Heap
{
public:
#define PARENT(idx) (idx - 1) / 2
#define CHILD(idx) 2 * idx + 1
#define SIBLINGR(idx) idx + 1

	struct HeapNode
	{
		size_t key;
		T value;
	};

	Heap(CMP cmp, size_t alloc_num, size_t kp_alloc_num): _cmp(cmp), _alloc_num(alloc_num), _key2pos(kp_alloc_num)
	{
		_Allocate();
	}

	~Heap() = default;

	size_t Emplace(T && obj)
	{
		auto pos = _size;
		auto key = _key2pos.Insert(std::move(pos));
		_heap[pos].key = key;
		_heap[pos].value = std::move(obj);
		++_size;
		_ShiftUp(pos);
		if(_size == _heap.size())
		{
			_Allocate();
		}
		return key;
	}

	T Pop(size_t pos = 0)
	{
		std::swap(_heap[pos], _heap[_size - 1]);
		auto node = _heap[_size - 1];
		auto obj = std::move(node.value);
		--_size;
		_ShiftDown(pos);
		_key2pos.Erase(node.key);
		return obj;
	}

	T PopByKey(size_t key)
	{
		auto pos = _key2pos[key];
		return Pop(*pos);
	}

	bool FindByKey(size_t key)
	{
		return _key2pos.Find(key);
	}

	bool Empty()
	{
		return _heap.empty();
	}

private:
	void _Allocate()
	{
		_heap.resize(_heap.size() + _alloc_num);
	}

	bool _ShiftUp(size_t pos)
	{
		size_t current = pos;
		size_t parent = PARENT(current);
		auto tmp = _heap[pos];
		while (current > 0 && _cmp(tmp.value, _heap[parent].value))
		{
			_heap[current] = std::move(_heap[parent]);
			*_key2pos[_heap[parent].key] = current;
			current = parent;
			parent = PARENT(parent);
		}
		_heap[current] = std::move(tmp);
		*_key2pos[tmp.key] = current;
		return true;
	}

	bool _ShiftDown(const size_t pos)
	{
		size_t current = pos;
		size_t child = CHILD(current);
		auto tmp = _heap[pos];
		while(child < _size)
		{
			if(SIBLINGR(child) < _size && _cmp(_heap[SIBLINGR(child)].value, _heap[child].value))
			{
				child = SIBLINGR(child);
			}

			if(_cmp(tmp.value, _heap[child].value))
			{
				break;
			}

			_heap[current] = std::move(_heap[child]);
			*_key2pos[_heap[child].key] = current;
			current = child;
			child = CHILD(child);
		}
		_heap[current] = std::move(tmp);
		*_key2pos[tmp.key] = current;
		return true;
	}

private:
	CMP _cmp;
	size_t _alloc_num;
	std::vector<HeapNode> _heap;
	size_t _size = 0;
	ObjectList<size_t> _key2pos;
};

NAMESPACE_CLOSE

#endif // USRV_HEAP_HPP