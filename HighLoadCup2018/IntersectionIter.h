#pragma once

#include <vector>
#include <utility>
#include <algorithm>

template<bool ReverseOrder, class ForwardIt>
class intersection_iter
{
public:
    intersection_iter(std::vector<std::pair<ForwardIt, ForwardIt>> &range_list, bool go_to_end = false)
        : _range_list((range_list))
    {
        if (go_to_end)
        {
            for (auto &range : _range_list)
            {
                range.first = range.second;
            }
        }
        else
        {
            next_iter();
        }
    }

    intersection_iter(intersection_iter &&) = default;

    auto operator*() const
    {
        return *_it;
    }

    auto operator->() const
    {
        return _it.operator->();
    }

    bool operator!=(const intersection_iter &another_it) const
    {
        return _range_list != another_it._range_list;
    }

    intersection_iter &operator++()
    {
        for (auto &range : _range_list)
        {
            ++range.first;
        };
        next_iter();

        return *this;
    }

private:
    void next_iter()
    {
        while (true)
        {
            auto current_range_it = std::min_element(_range_list.begin(), _range_list.end(), [](auto &&left_it, auto &&right_it)
            {
                if constexpr(ReverseOrder)
                {
                    return left_it.first != left_it.second && (right_it.first == right_it.second || *right_it.first < *left_it.first);
                }
                else
                {
                    return left_it.first != left_it.second && (right_it.first == right_it.second || *left_it.first < *right_it.first);
                }
            });
            if (current_range_it->first != current_range_it->second)
            {
                _it = current_range_it->first;

                bool every_has = true;
                for (const auto &range : _range_list)
                {
                    if (range.first == range.second || *range.first != *current_range_it->first)
                    {
                        every_has = false;
                    }
                }

                if (every_has)
                {
                    break;
                }
                else
                {
                    for (auto &range : _range_list)
                    {
                        if (range.first != range.second && *range.first == *_it)
                        {
                            ++range.first;
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    std::vector<std::pair<ForwardIt, ForwardIt>> _range_list;
    ForwardIt _it;
};
