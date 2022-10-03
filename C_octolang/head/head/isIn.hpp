#pragma once

#include <algorithm>

#include "iterable.hpp"

template<class T, typename R> 
bool isIn(T i, R r) { //.begin, .end
	auto end = i.end();
	return std::find(i.begin(), end, r) != end;
}