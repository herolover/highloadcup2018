#pragma once

#include "../DB.h"

#include "../Split.h"
#include "../IntersectionIter.h"

#include <charconv>
#include <algorithm>
#include <string_view>
#include <utility>

struct likes
{
    static auto contains(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &id_string : split(value))
        {
            uint32_t id = 0;
            std::from_chars(id_string.data(), id_string.data() + id_string.size(), id);

            auto &id_list = db.liked_by[id];
            range_list.push_back(std::make_pair(id_list.begin(), id_list.end()));
        }

        return std::make_pair(intersection_iter<false, IterType>(range_list), intersection_iter<false, IterType>(range_list, true));
    }

    static auto reverse_contains(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &id_string : split(value))
        {
            uint32_t id = 0;
            std::from_chars(id_string.data(), id_string.data() + id_string.size(), id);

            auto &id_list = db.liked_by[id];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        return std::make_pair(intersection_iter<true, IterType>(range_list), intersection_iter<true, IterType>(range_list, true));
    }
};
