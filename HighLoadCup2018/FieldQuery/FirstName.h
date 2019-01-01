#pragma once

#include "../FieldQuery.h"

#include "../HandlerIter.h"
#include "../Split.h"

template<>
struct FieldQuery<DB::first_name_tag>
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
        return index.equal_range(value);
    }

    static auto any(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
        using iter_type = decltype(index.begin());
        std::vector<std::pair<iter_type, iter_type>> range_list;
        for (auto &first_name : split(value))
        {
            range_list.emplace_back(index.equal_range(first_name));
        }

        auto begin_it = range_list.front().first;
        auto end_it = range_list.back().second;

        std::size_t current_range = 0;
        return std::make_pair(handler_iter(begin_it, [range_list = std::move(range_list), current_range](auto &it) mutable
        {
            while (it == range_list[current_range].second)
            {
                ++current_range;
                if (current_range < range_list.size())
                {
                    it = range_list[current_range].first;
                }
                else
                {
                    break;
                }
            }
        }), end_it);
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
        if (value[0] == L'0')
        {
            return std::make_pair(index.upper_bound(nullptr), index.end());
        }
        else
        {
            return index.equal_range(nullptr);
        }
    }
};