// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <memory>

#include "noncopyable.hpp"
#include "nonmovable.hpp"

NAMESPACE_OPEN

template<class T>
class Singleton : public noncopyable, public nonmovable
{
public:
	static std::shared_ptr<T> Instance()
	{
		return _instance; 
	}

protected:
	Singleton() = default;
	~Singleton() = default;

	static std::shared_ptr<T> _instance;
};

template<class T>
std::shared_ptr<T> Singleton<T>::_instance = std::make_shared<T>();

NAMESPACE_CLOSE

#endif // SINGLETON_HPP
