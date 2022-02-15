// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_OBJECT_MAP_HPP
#define USRV_OBJECT_MAP_HPP

#include <vector>

#include "common.h"

NAMESPACE_OPEN

template<typename T>
class ObjectMap
{
public:
	class Iterator
	{
		public:
			Iterator(ObjectMap & map, size_t id):_map(map), _id(id) {}
			~Iterator() = default;

			const std::shared_ptr<T> & operator *() { return _map._objects[_id].obj; }
			Iterator & operator ++() { _id = _map._objects[_id].next; return *this; }
			bool operator !=(const Iterator & iter) { return _id != iter._id; }

		private:
			ObjectMap & _map;
			size_t _id;
	};

	struct Node
	{
		std::shared_ptr<T> obj = nullptr;
		size_t prev = 0;
		size_t next = 0;

		void Reset()
		{
			obj = nullptr;
			prev = 0;
			next = 0;
		}
	};

	ObjectMap(size_t alloc_num) : _alloc_num(alloc_num)
	{
		_Allocate();
	}

	~ObjectMap() = default;

	const std::shared_ptr<T> & operator[](size_t id)
	{
		return _objects[id].obj;
	}

	bool Find(size_t id)
	{
		if (id >= _objects.size() || !_objects[id].obj)
		{
			return false;
		}

		return true;
	}

	size_t Insert(std::shared_ptr<T> && obj)
	{
		auto id = _indexs[_head];
		if(!_objects[_begin].obj)
		{
			_begin = id;
		}
		_objects[id].obj = std::move(obj);

		if(_objects[_end].obj)
		{
			_objects[id].prev = _end;
		}

		_head = (_head + 1) % _objects.size();
		if (_head == _tail)
		{
			_Allocate();
		}

		_end = _indexs[_head];
		_objects[id].next = _end;

		return id;
	}

	size_t Insert(T && obj)
	{
		return Insert(std::make_shared<T>(obj));
	}

	void Erase(size_t id)
	{
		if (id >= _objects.size() || _objects[id].obj == nullptr)
		{
			return;
		}

		auto prev_id = _objects[id].prev;
		auto next_id = _objects[id].next;
		if(id == _begin)
		{
			_begin = next_id;
		}
		else
		{
			_objects[prev_id].next = next_id;
		}
		if(id == _end)
		{
			_end = id;
		}
		else
		{
			_objects[next_id].prev = prev_id;
		}
		_objects[id].Reset();

		_indexs[_tail] = id;
		_tail = (_tail + 1) % _objects.size();
	}

	void Clear()
	{
		_objects.clear();
		_head = 0;
		_tail = 0;
		_indexs.clear();
	}

	size_t Size()
	{
		return _tail >= _head ? _tail - _head : _objects.size() + _tail - _head;
	}

	Iterator begin()
	{
		return Iterator(*this, _begin);
	}

	Iterator end()
	{
		return Iterator(*this, _end);
	}

private:
	void _Allocate()
	{
		auto size = _objects.size();
		_objects.resize(size + _alloc_num);
		_head = size;
		_tail = 0;
		_indexs.resize(size + _alloc_num);
		for (size_t i = size; i < size + _alloc_num; ++i)
		{
			_indexs[i] = i;
		}
	}

private:
	size_t _alloc_num;
	std::vector<Node> _objects;
	size_t _head = 0;
	size_t _tail = 0;
	std::vector<size_t> _indexs;
	size_t _begin = 0;
	size_t _end = 0;
};

NAMESPACE_CLOSE

#endif // USRV_OBJECT_MAP_HPP
