#pragma once

#include "../DB.h"

#include <string_view>
#include <ctime>

struct premium
{
    static auto now(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::premium_tag>();
        return std::make_pair(index.lower_bound(std::time(nullptr)), index.end());
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::premium_tag>();
        if (value[0] == '0')
        {
            return std::make_pair(index.upper_bound(0), index.end());
        }
        else
        {
            return index.equal_range(0);
        }
    }
};
