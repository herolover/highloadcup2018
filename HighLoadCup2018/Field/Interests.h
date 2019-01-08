#pragma once

#include "../FieldMethodTrait.h"
#include "../UnionIter.h"
#include "../IntersectionIter.h"
#include "../Split.h"

#include <algorithm>

template<class M>
struct t_has_method<f_interests, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_contains, m_any>, M>::value;
};

template<>
struct t_value<f_interests, m_contains>
{
    Value operator()(const std::string_view &value) const
    {
        auto interest_list = split(value);
        std::sort(interest_list.begin(), interest_list.end());

        return std::move(interest_list);
    }
};

template<>
struct t_value<f_interests, m_any>
{
    Value operator()(const std::string_view &value) const
    {
        auto interest_list = split(value);
        std::sort(interest_list.begin(), interest_list.end());

        return std::move(interest_list);
    }
};

template<>
struct t_select<f_interests, m_contains>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : std::get<std::vector<std::string_view>>(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        handler(std::make_pair(intersection_iter<true, IterType>(range_list), intersection_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_select<f_interests, m_any>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        using IterType = std::vector<DB::AccountReference>::reverse_iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : std::get<std::vector<std::string_view>>(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        handler(std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_check<f_interests, m_contains>
{
    bool operator()(const Account &account, const Value &value) const
    {
        for (auto &interest : std::get<std::vector<std::string_view>>(value))
        {
            if (!std::binary_search(account.interest.begin(), account.interest.end(), interest, string_view_compare()))
            {
                return false;
            }
        }

        return true;
    }
};

template<>
struct t_check<f_interests, m_any>
{
    bool operator()(const Account &account, const Value &value) const
    {
        for (auto &interest : std::get<std::vector<std::string_view>>(value))
        {
            if (std::binary_search(account.interest.begin(), account.interest.end(), interest, string_view_compare()))
            {
                return true;
            }
        }

        return false;
    }
};
