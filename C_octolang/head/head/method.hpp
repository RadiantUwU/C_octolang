#pragma once

#include <functional>

class method final {
	void* f;
	void* o;
public:
	method(void* f, void* o) : f(f), o(o) {}
	template <typename R,typename... Args>
	R operator()(Args... args) {
		return std::invoke_r(f, o, args...);
	}
};