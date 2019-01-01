#pragma once

#include "../FieldQuery.h"
#include "../HandlerIter.h"

template<>
struct FieldQuery<DB::second_name_tag>
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::second_name_tag>();
        return index.equal_range(value);
    }

    static auto starts(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::second_name_tag>();
        auto end_it = index.end();
        return std::make_pair(handler_iter(index.lower_bound(value), [value, end_it](auto &it)
        {
            if (it != end_it)
            {
                auto &account = *it;
                auto &second_name = it->second_name;

                if (std::string_view(second_name->c_str(), value.size()) != value)
                {
                    it = end_it;
                }
            }
        }), end_it);
    }
};