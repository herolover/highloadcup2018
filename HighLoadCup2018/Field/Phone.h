#pragma once

#include "../DB.h"

#include <string_view>

struct phone
{
    static auto code(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::phone_code_tag>();
        return index.equal_range(value);
    }

    static auto null(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::phone_tag>();
        if (value[0] == '0')
        {
            return std::make_pair(index.upper_bound(""), index.end());
        }
        else
        {
            return index.equal_range("");
        }
    }
};
