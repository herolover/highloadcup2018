#pragma once

#include "../FieldQuery.h"

#include "../Split.h"
#include "../UnionIter.h"
#include "../Common.h"

template<>
struct FieldQuery<DB::city_tag>
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::city_tag>();
        return index.equal_range(value);
    }

    static auto any(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::city_tag>();

        using IterType = decltype(index.begin());
        std::vector<std::pair<IterType, IterType>> range_list;
        for (auto &city : split(value))
        {
            range_list.push_back(index.equal_range(city));
        }

        return std::make_pair(union_iter<false, IterType>(range_list), union_iter<false, IterType>(range_list, true));
    }

    static auto reverse_any(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::city_tag>();

        using IterType = decltype(index.rbegin());
        std::vector<std::pair<IterType, IterType>> range_list;
        for (auto &city : split(value))
        {
            range_list.push_back(make_reverse_range(index.equal_range(city)));
        }

        return std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true));
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::city_tag>();
        if (value[0] == '0')
        {
            return std::make_pair(index.upper_bound(nullptr), index.end());
        }
        else
        {
            return index.equal_range(nullptr);
        }
    }
};