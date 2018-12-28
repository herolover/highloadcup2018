#pragma once

#include "../FieldTrait.h"

template<>
struct GetTrait<AccountField::FIRST_NAME>
{
    static auto eq(DB &db, const std::wstring &value)
    {
        auto value_it = db.first_name.left.find(value);
        auto &index = db.account.get<DB::first_name_tag>();
        return index.equal_range(value_it->second);
    }

    static auto any(DB &db, const std::wstring &value)
    {
    }
};