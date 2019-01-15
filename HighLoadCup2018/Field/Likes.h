#pragma once

#include "../FieldMethodTrait.h"
#include "../Split.h"
#include "../IntersectionIter.h"

#include <charconv>
#include <algorithm>

template<class M>
struct t_has_method<f_likes, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_contains>, M>::value;
};

template<>
struct t_value<f_likes, m_eq>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        uint32_t id = 0;
        std::from_chars(value.data(), value.data() + value.size(), id);

        return id;
    }
};

template<>
struct t_value<f_likes, m_contains>
{
    Value operator()(DB &db, const std::string_view &value) const
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
struct t_select<f_likes, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &account_list = db.liked_by[std::get<uint32_t>(value)];
        handler(std::make_pair(account_list.rbegin(), account_list.rend()));
    }
};

template<>
struct t_select<f_likes, m_contains>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &id_list = std::get<std::vector<uint32_t>>(value);
        auto &account_list = db.liked_by[id_list.front()];
        handler(std::make_pair(account_list.rbegin(), account_list.rend()));
    }
};

template<>
struct t_check<f_likes, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::binary_search(account.like_list.begin(), account.like_list.end(), std::get<uint32_t>(value), Like::is_less());
    }
};

template<>
struct t_check<f_likes, m_contains>
{
    bool operator()(const Account &account, const Value &value) const
    {
        for (auto &id : std::get<std::vector<uint32_t>>(value))
        {
            if (!std::binary_search(account.like_list.begin(), account.like_list.end(), id, Like::is_less()))
            {
                return false;
            }
        }

        return true;
    }
};
