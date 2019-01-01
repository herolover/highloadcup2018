#pragma once

#include "../FieldQuery.h"

#include <charconv>

template<>
struct FieldQuery<DB::birth_tag>
{
    static auto lt(DB &db, const std::string_view &value)
    {
        uint32_t date = 0;
        std::from_chars(value.data(), value.data() + value.size(), date);

        auto &index = db.account.get<DB::birth_tag>();
        return std::make_pair(index.begin(), index.lower_bound(date));
    }

    static auto gt(DB &db, const std::string_view &value)
    {
        uint32_t date = 0;
        std::from_chars(value.data(), value.data() + value.size(), date);

        auto &index = db.account.get<DB::birth_tag>();
        return std::make_pair(index.upper_bound(date), index.end());
    }

    static auto year(DB &db, const std::string_view &value)
    {
        uint32_t year = 0;
        std::from_chars(value.data(), value.data() + value.size(), year);

        auto &index = db.account.get<DB::birth_year_tag>();
        return index.equal_range(year);
    }
};