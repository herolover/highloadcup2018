#pragma once

#include "../DB.h"

#include <string_view>

struct country
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::country_tag>();
        return index.equal_range(value);
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::country_tag>();
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
