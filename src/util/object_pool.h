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
	ObjectPool(size_t init_num)
	{
		for (auto i = 0; i < init_num; ++i)
		{
			_objects.emplace_back(std::move(std::make_shared<T>()));
		}
	}

	~ObjectPool()
	{
		_objects.clear();
	}

public:
	std::shared_ptr<T> Get()
	{
		if (!_objects.empty())
		{
			auto obj = std::move(_objects.back());
			_objects.pop_back();
			return std::move(obj);
		}
		else
		{
			return std::move(std::make_shared<T>());
		}
	}

	void Return(std::shared_ptr<T> && obj)
	{
		_objects.emplace_back(std::move(obj));
	}

private:
	std::vector<std::shared_ptr<T>> _objects;
};

NAMESPACE_CLOSE

#endif // OBJECT_POOL_HPP
