#pragma once

#include "../FieldQuery.h"

#include "../Split.h"
#include "../AnyIter.h"

template<>
struct FieldQuery<DB::first_name_tag>
{
    static auto eq(DB &db, const std::wstring &value)
    {
        auto value_it = db.first_name.left.find(value);
        auto &index = db.account.get<DB::first_name_tag>();
        return index.equal_range(value_it->second);
    }

    static auto any(DB &db, const std::wstring &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
        using iter_type = decltype(index.begin());
        std::vector<std::pair<iter_type, iter_type>> range_list;
        for (auto &first_name : split<wchar_t>(value))
        {
            range_list.emplace_back(index.equal_range(db.get_first_name_id(first_name)));
        }

        return std::make_pair(any_iter(std::move(range_list)), range_list.back().second);
    }

    static auto null(DB &db, const std::wstring &value)
    {
        auto &index = db.account.get<DB::first_name_tag>();
        if (value[0] == L'0')
        {
            return std::make_pair(index.upper_bound(-1), index.end());
        }
        else
        {
            return index.equal_range(-1);
        }
    }
};