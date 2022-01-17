// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_HEAP_HPP
#define USRV_HEAP_HPP

#include <vector>

#include "common.h"
#include "object_list.hpp"

NAMESPACE_OPEN

template<typename T>
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

	Heap(std::function<bool(T &, T &)> cmp, size_t kp_alloc_num): _cmp(cmp), _key2pos(kp_alloc_num) {}
	~Heap() = default;

	size_t Emplace(T && obj)
	{
		auto pos = _heap.size();
		auto key = _key2pos.Insert(std::move(pos));
		HeapNode node;
		node.key = key;
		node.value = std::move(obj);
		_heap.emplace_back(node);
		_ShiftUp(pos);
		return key;
	}

	T Pop(size_t pos = 0)
	{
		std::swap(_heap[pos], _heap[_heap.size() - 1]);
		auto obj = std::move(_heap.back().value);
		_heap.pop_back();
		_ShiftDown(pos);
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
	bool _ShiftUp(size_t pos)
	{
		size_t current = pos;
		size_t parent = PARENT(current);
		auto tmp = _heap[pos];
		while (current > 0 && _cmp(tmp.value, _heap[parent].value))
		{
			_heap[current] = _heap[parent];
			auto p_key = _key2pos[_heap[parent].key];
			*p_key = current;
			current = parent;
			parent = PARENT(parent);
		}
		_heap[current] = tmp;
		auto c_key = _key2pos[tmp.key];
		*c_key = current;
		return true;
	}

	bool _ShiftDown(const size_t pos)
	{
		size_t current = pos;
		size_t child = CHILD(current);
		auto tmp = _heap[pos];
		while(child < _heap.size())
		{
			if(SIBLINGR(child) < _heap.size() && _cmp(_heap[SIBLINGR(child)].value, _heap[child].value))
			{
				child = SIBLINGR(child);
			}

			if(_cmp(tmp.value, _heap[child].value))
			{
				break;
			}

			_heap[current] = _heap[child];
			auto c_key = _key2pos[_heap[child].key];
			*c_key = current;
			current = child;
			child = CHILD(child);
		}
		_heap[current] = tmp;
		auto c_key = _key2pos[tmp.key];
		*c_key = current;
		return true;
	}

private:
	std::function<bool(T &, T &)> _cmp;
	std::vector<HeapNode> _heap;
	ObjectList<size_t> _key2pos;
};

NAMESPACE_CLOSE

#endif // USRV_HEAP_HPP