#pragma once

#include <vector>
#include <utility>

template<bool ReverseOrder, class ForwardIt, class Compare = std::less<>>
class union_iter
{
public:
    union_iter(std::vector<std::pair<ForwardIt, ForwardIt>> &range_list, bool go_to_end = false, Compare comp = Compare())
        : _range_list((range_list))
        , _comp(comp)
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
    union_iter(const union_iter &) = default;
    union_iter(union_iter &&) = default;

    auto operator*() const
    {
        return *_current_range_it->first;
    }

    auto operator->() const
    {
        return _current_range_it->first.operator->();
    }

    bool operator!=(const union_iter &another_it) const
    {
        return _range_list != another_it._range_list;
    }

    union_iter &operator++()
    {
        ++_current_range_it->first;
        next_iter();

        return *this;
    }

private:
    void next_iter()
    {
        _current_range_it = std::min_element(_range_list.begin(), _range_list.end(), [this](auto &&left_it, auto &&right_it)
        {
            if constexpr(ReverseOrder)
            {
                return left_it.first != left_it.second && (right_it.first == right_it.second || _comp(*right_it.first, *left_it.first));
            }
            else
            {
                return left_it.first != left_it.second && (right_it.first == right_it.second || _comp(*left_it.first, *right_it.first));
            }
        });
    }

    std::vector<std::pair<ForwardIt, ForwardIt>> _range_list;
    Compare _comp;
    typename std::vector<std::pair<ForwardIt, ForwardIt>>::iterator _current_range_it;
};
