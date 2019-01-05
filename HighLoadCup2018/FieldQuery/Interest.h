#pragma once

#include "../FieldQuery.h"

#include "../UnionIter.h"
#include "../IntersectionIter.h"
#include "../Common.h"
#include "../Split.h"

template<>
struct FieldQuery<DB::interest_tag>
{
    static auto contains(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.begin(), id_list.end()));
        }

        return std::make_pair(intersection_iter<false, IterType>(range_list), intersection_iter<false, IterType>(range_list, true));
    }

    static auto reverse_contains(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        return std::make_pair(intersection_iter<true, IterType>(range_list), intersection_iter<true, IterType>(range_list, true));
    }

    static auto any(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.begin(), id_list.end()));
        }

        return std::make_pair(union_iter<false, IterType>(range_list), union_iter<false, IterType>(range_list, true));
    }

    static auto reverse_any(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        return std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true));
    }
};
