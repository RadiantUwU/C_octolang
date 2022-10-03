#pragma once

#include <concepts>
template <class T, typename R>
concept iterator_concept = requires (T t) {
	{*t} -> std::same_as<R>;
	{++t} -> std::same_as<T&>;
};
template <class T, typename R>
concept iterable = iterator_concept<typename T::iterator,R>;