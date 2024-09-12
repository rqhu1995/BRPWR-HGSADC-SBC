
#pragma once
#include <algorithm>
#include <utility>
#include <vector>
namespace Sort {
    // General template for sorting a vector of std::pair by a specified member.
    template <typename T1, typename T2, typename MemberType>
    void sortBy(std::vector<std::pair<T1, T2>> &vec, MemberType std::pair<T1, T2>:: *member, bool ascending = true) {
        if (ascending) {
            std::sort(vec.begin(), vec.end(),
                [member](const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) { return a.*member < b.*member; });
        } else {
            std::sort(vec.begin(), vec.end(),
                [member](const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) { return a.*member > b.*member; });
        }
    }

    template <std::size_t I, typename... Ts>
    void sortBy(std::vector<std::tuple<Ts...>> &vec, bool ascending = true) {
        if (ascending) {
            std::sort(vec.begin(), vec.end(),
                [](const std::tuple<Ts...> &a, const std::tuple<Ts...> &b) { return std::get<I>(a) < std::get<I>(b); });
        } else {
            std::sort(vec.begin(), vec.end(),
                [](const std::tuple<Ts...> &a, const std::tuple<Ts...> &b) { return std::get<I>(a) > std::get<I>(b); });
        }
    }

    template <typename T, typename MemberType>
    void sortBy(std::vector<T> &vec, MemberType T:: *field, bool ascending = true) {
        if (ascending) {
            std::sort(vec.begin(), vec.end(), [field](const T &a, const T &b) { return a.*field < b.*field; });
        } else {
            std::sort(vec.begin(), vec.end(), [field](const T &a, const T &b) { return a.*field > b.*field; });
        }
    }
} // namespace Sort