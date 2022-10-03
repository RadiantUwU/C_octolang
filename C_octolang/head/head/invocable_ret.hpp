#pragma once

#include <concepts>
#include <type_traits>

template<class F, class R, class... Args >
concept r_invocable = std::regular_invocable<F, Args...> && std::same_as<
	typename std::invoke_result<F, Args...>::type,
	R
>;