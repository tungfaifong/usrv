// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "noncopyable.hpp"
#include "nonmovable.hpp"

NAMESPACE_OPEN

template<class T>
class Singleton : public noncopyable, public nonmovable
{
public:
	static T * Instance()
	{
		static T instance;
		return &instance; 
	}

protected:
	Singleton() = default;
	~Singleton() = default;
}; 

NAMESPACE_CLOSE

#endif // SINGLETON_HPP
