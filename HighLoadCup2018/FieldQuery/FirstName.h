#pragma once

#include "../FieldQuery.h"

#include "../HandlerIter.h"
#include "../MakeAnyIter.h"
#include "../Common.h"
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

        return std::make_pair(make_any_iter<false>(index.begin(), index.end(), std::move(range_list)), index.end());
    }

    static auto reverse_any(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();

        using iter_type = decltype(index.rbegin());
        std::vector<std::pair<iter_type, iter_type>> range_list;
        for (auto &first_name : split(value))
        {
            range_list.emplace_back(make_reverse_range(index.equal_range(first_name)));
        }

        return std::make_pair(make_any_iter<true>(index.rbegin(), index.rend(), std::move(range_list)), index.rend());
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
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