// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <vector>
#include <memory>

#include "common.h"

NAMESPACE_OPEN

template<typename T>
class ObjectPool
{
public:
	ObjectPool(size_t alloc_num): _alloc_num(alloc_num)
	{
		Allocate();
	}

	~ObjectPool()
	{
		_objects.clear();
	}

	void Allocate()
	{
		for (size_t i = 0; i < _alloc_num; ++i)
		{
			_objects.emplace_back(std::make_shared<T>());
		}
	}

public:
	std::shared_ptr<T> Get()
	{
		if(_objects.empty())
		{
			Allocate();
		}

		auto obj = std::move(_objects.back());
		_objects.pop_back();
		return obj;
	}

	void Put(std::shared_ptr<T> && obj)
	{
		_objects.emplace_back(std::move(obj));
	}

public:
	size_t _alloc_num;
	std::vector<std::shared_ptr<T>> _objects;
};

NAMESPACE_CLOSE

#endif // OBJECT_POOL_HPP
