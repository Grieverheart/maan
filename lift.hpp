#ifndef __LIFT_HPP
#define __LIFT_HPP

#include <tuple>

template<int ...> struct sequence{};
template<int N, int ...S> struct generator: generator<N - 1, N - 1, S...>{};
template<int ...S> struct generator<0, S...>{typedef sequence<S...> type;};

template<typename Sequence>
struct lift_impl;

template<template<int...> class Sequence, int... Indices>
struct lift_impl<Sequence<Indices...>>{
    template<class F, typename... ArgsT>
    static auto lift(F&& func, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(ArgsT...)>::type {
        return func(std::forward<ArgsT>(std::get<Indices>(args))...);
    }
    template<class F, class U, typename... ArgsT>
    static auto lift(F&& func, U&& arg, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(U, ArgsT...)>::type {
        return func(std::forward<U>(arg), std::forward<ArgsT>(std::get<Indices>(args))...);
    }
};

template<
    class F, typename... ArgsT,
    typename Sequence = typename generator<sizeof...(ArgsT)>::type
>
auto lift(F&& func, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(ArgsT...)>::type {
    return lift_impl<Sequence>::lift(std::forward<F>(func), std::forward<std::tuple<ArgsT...>>(args));
}

template<
    class F, class U, typename... ArgsT,
    typename Sequence = typename generator<sizeof...(ArgsT)>::type
>
auto lift(F&& func, U&& arg, std::tuple<ArgsT...>&& args) -> typename std::result_of<F(U, ArgsT...)>::type {
    return lift_impl<Sequence>::lift(std::forward<F>(func), std::forward<U>(arg), std::forward<std::tuple<ArgsT...>>(args));
}

#endif
