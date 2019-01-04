#pragma once

#include "HandlerIter.h"

#include <vector>
#include <utility>

template<bool ReverseOrder, class ForwardIt>
auto make_any_iter(ForwardIt begin_it, ForwardIt end_it, std::vector<std::pair<ForwardIt, ForwardIt>> &&range_list)
{
    decltype(range_list.begin()) current_range_it;
    return handler_iter(begin_it, [current_range_it, range_list = std::move(range_list), end_it](auto &it) mutable
    {
        if (it != end_it)
        {
            auto current_range_it = std::min_element(range_list.begin(), range_list.end(), [](auto &left_it, auto &right_it)
            {
                if constexpr(ReverseOrder)
                {
                    return left_it.first != left_it.second && (right_it.first == right_it.second || !(*left_it.first < *right_it.first));
                }
                else
                {
                    return left_it.first != left_it.second && (right_it.first == right_it.second || *left_it.first < *right_it.first);
                }
            });
            if (current_range_it->first != current_range_it->second)
            {
                it = current_range_it->first;
                ++(current_range_it->first);
            }
            else
            {
                it = end_it;
            }
        }
    });
}
