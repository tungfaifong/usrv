// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef OBJECT_LIST_HPP
#define OBJECT_LIST_HPP

#include <vector>

#include "common.h"

NAMESPACE_OPEN

template<typename T>
class ObjectList
{
public:
	ObjectList(size_t alloc_num) : _alloc_num(alloc_num)
	{
		_Allocate();
	}

	const std::shared_ptr<T> operator[](size_t id)
	{
		if (id >= _objects.size())
		{
			return nullptr;
		}
		return _objects[id];
	}

	size_t Insert(std::shared_ptr<T> && obj)
	{
		if (_size == _objects.size())
		{
			_Allocate();
		}

		auto id = _indexs[_size++];
		_objects[id] = std::move(obj);

		return id;
	}

	void Erase(size_t id)
	{
		if (id >= _objects.size() || _objects[id] == nullptr)
		{
			return;
		}

		_objects[id] = nullptr;
		_indexs[--_size] = id;
	}

	void Clear()
	{
		_size = 0;
		_objects.clear();
		_indexs.clear();
	}

private:
	void _Allocate()
	{
		_objects.resize(_objects.size() + _alloc_num);
		_indexs.resize(_indexs.size() + _alloc_num);
		for (size_t i = _size; i < _alloc_num; ++i)
		{
			_indexs[i] = i;
		}
	}

private:
	size_t _alloc_num;
	size_t _size = 0;
	std::vector<std::shared_ptr<T>> _objects;
	std::vector<size_t> _indexs;
};

NAMESPACE_CLOSE

#endif // OBJECT_LIST_HPP
