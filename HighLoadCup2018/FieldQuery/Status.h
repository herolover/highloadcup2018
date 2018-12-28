#pragma once

#include "../FieldQuery.h"

#include "../DBLoader.h"

template<>
struct FieldQuery<DB::status_tag>
{
    static auto eq(DB &db, const std::wstring &value)
    {
        auto status = convert_account_status(value.c_str());
        auto &index = db.account.get<DB::status_tag>();
        return index.equal_range(status);
    }

    template<class iter_type, class ignore_range_type>
    struct neq_iter
    {
        iter_type iter;
        ignore_range_type ignore_range;

        neq_iter(iter_type iter, ignore_range_type ignore_range)
            : iter(iter)
            , ignore_range(ignore_range)
        {
        }

        auto operator*() const
        {
            return *iter;
        }

        auto operator->() const
        {
            return iter.operator->();
        }

        bool operator!=(const iter_type &another_it) const
        {
            return iter != another_it;
        }

        neq_iter &operator++()
        {
            ++iter;
            if (iter == ignore_range.first)
            {
                iter = ignore_range.second;
            }

            return *this;
        }
    };

    static auto neq(DB &db, const std::wstring &value)
    {
        auto status = convert_account_status(value.c_str());
        auto &index = db.account.get<DB::status_tag>();
        return std::make_pair(neq_iter(index.begin(), index.equal_range(status)), index.end());
    }
};
