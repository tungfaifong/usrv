// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#include "common.h"

NAMESPACE_OPEN

class noncopyable
{
public:
	noncopyable(const noncopyable&) = delete;
	const noncopyable& operator=(const noncopyable&) = delete;

protected:
	noncopyable() = default;
	~noncopyable() = default;
}; 

NAMESPACE_CLOSE

#endif // NONCOPYABLE_HPP
