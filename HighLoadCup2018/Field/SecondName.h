#pragma once

#include "../DB.h"
#include "../HandlerIter.h"

#include <string_view>

struct second_name
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

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::second_name_tag>();
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
