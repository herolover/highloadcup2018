#pragma once

#include <vector>
#include <utility>

template<class iter_type>
struct any_iter
{
    using range_type = std::pair<iter_type, iter_type>;

    iter_type iter;
    std::size_t current_range = 0;
    std::vector<range_type> range_list;

    any_iter(std::vector<range_type> range_list)
        : range_list(range_list)
        , iter(range_list.front().first)
    {
    }

    auto operator*() const
    {
        return *iter;
    }

    auto operator->() const
    {
        return iter.operator->();
    }

    bool operator!=(const iter_type &another_it) const
    {
        return iter != another_it;
    }

    any_iter &operator++()
    {
        ++iter;
        if (iter == range_list[current_range].second)
        {
            ++current_range;
            if (current_range < range_list.size())
            {
                iter = range_list[current_range].first;
            }
        }

        return *this;
    }
};
