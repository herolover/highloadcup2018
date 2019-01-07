#pragma once

#include "../DB.h"

#include <string_view>

struct sex
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::sex_tag>();
        return index.equal_range(value[0] == L'm');
    }
};
