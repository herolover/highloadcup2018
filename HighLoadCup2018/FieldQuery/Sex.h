#pragma once

#include "../FieldQuery.h"

template<>
struct FieldQuery<DB::sex_tag>
{
    static auto eq(DB &db, const std::string &value)
    {
        auto &index = db.account.get<DB::sex_tag>();
        return index.equal_range(value[0] == 'm');
    }
};