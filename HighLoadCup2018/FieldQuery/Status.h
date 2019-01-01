#pragma once

#include "../FieldQuery.h"

#include "../HandlerIter.h"
#include "../DBLoader.h"

template<>
struct FieldQuery<DB::status_tag>
{
    static auto eq(DB &db, const std::string_view &value)
    {
        auto status = convert_account_status(value);
        auto &index = db.account.get<DB::status_tag>();
        return index.equal_range(status);
    }

    static auto neq(DB &db, const std::string_view &value)
    {
        auto status = convert_account_status(value);
        auto &index = db.account.get<DB::status_tag>();
        auto ignore_range = index.equal_range(status);

        return std::make_pair(handler_iter(index.begin(), [ignore_range = std::move(ignore_range)](auto &it)
        {
            if (it == ignore_range.first)
            {
                it = ignore_range.second;
            }
        }), index.end());
    }
};
