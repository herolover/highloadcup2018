#pragma once

#include "../FieldMethodTrait.h"
#include "../Split.h"
#include "../IntersectionIter.h"

#include <charconv>
#include <algorithm>

template<>
struct t_value<f_likes, m_contains>
{
    Value operator()(const std::string_view &value) const
    {
        std::vector<uint32_t> id_list;
        for (auto &string_id : split(value))
        {
            uint32_t id = 0;
            std::from_chars(string_id.data(), string_id.data() + string_id.size(), id);

            id_list.push_back(id);
        }

        std::sort(id_list.begin(), id_list.end());

        return std::move(id_list);
    }
};

template<>
struct t_select<f_likes, m_contains>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &id : std::get<std::vector<uint32_t>>(value))
        {
            auto &id_list = db.liked_by[id];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        handler(std::make_pair(intersection_iter<true, IterType>(range_list), intersection_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_check<f_likes, m_contains>
{
    bool operator()(const Account &account, const Value &value) const
    {
        for (auto &id : std::get<std::vector<uint32_t>>(value))
        {
            if (!std::binary_search(account.like.begin(), account.like.end(), id))
            {
                return false;
            }
        }

        return true;
    }
};
