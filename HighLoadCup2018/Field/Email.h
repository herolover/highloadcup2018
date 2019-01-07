#pragma once

#include "../DB.h"

#include <string_view>

struct email
{
    static auto domain(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::email_domain_tag>();
        return index.equal_range(value);
    }

    static auto lt(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::email_tag>();
        return std::make_pair(index.begin(), index.lower_bound(value));
    }

    static auto gt(DB &db, const std::string_view &value)
    {
        auto &index = db.account.get<DB::email_tag>();
        return std::make_pair(index.upper_bound(value), index.end());
    }
};
