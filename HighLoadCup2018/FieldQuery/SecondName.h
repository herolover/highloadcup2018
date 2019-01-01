#pragma once

#include "../FieldQuery.h"

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

    }
};