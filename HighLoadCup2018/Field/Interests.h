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
struct t_value<f_interests, m_eq>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return std::make_pair(value, db.get_interest_mask(value));
    }
};

template<>
struct t_value<f_interests, m_contains>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        auto interest_list = split(value);
        Account::interest_mask_t mask;
        for (auto &interest : interest_list)
        {
            mask |= db.get_interest_mask(interest);
        }

        return std::make_pair(std::move(interest_list), mask);
    }
};

template<>
struct t_value<f_interests, m_any>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        auto interest_list = split(value);
        std::sort(interest_list.begin(), interest_list.end());

        Account::interest_mask_t mask;
        for (auto &interest : interest_list)
        {
            mask |= db.get_interest_mask(interest);
        }

        return std::make_pair(std::move(interest_list), mask);
    }
};

template<>
struct t_select<f_interests, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &account_list = db.interest[std::get<std::pair<std::string_view, Account::interest_mask_t>>(value).first];
        handler(std::make_pair(account_list.rbegin(), account_list.rend()));
    }
};

template<>
struct t_select<f_interests, m_contains>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &interest_list = std::get<std::pair<std::vector<std::string_view>, Account::interest_mask_t>>(value).first;
        auto &account_list = db.interest[interest_list.front()];
        handler(std::make_pair(account_list.rbegin(), account_list.rend()));
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

        for (auto &interest : std::get<std::pair<std::vector<std::string_view>, Account::interest_mask_t>>(value).first)
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.rbegin(), id_list.rend()));
        }

        handler(std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_check<f_interests, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &test_mask = std::get<std::pair<std::string_view, Account::interest_mask_t>>(value).second;
        return (account.interest_mask & test_mask) == test_mask;
    }
};

template<>
struct t_check<f_interests, m_contains>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &test_mask = std::get<std::pair<std::vector<std::string_view>, Account::interest_mask_t>>(value).second;
        return (account.interest_mask & test_mask) == test_mask;
    }
};

template<>
struct t_check<f_interests, m_any>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &test_mask = std::get<std::pair<std::vector<std::string_view>, Account::interest_mask_t>>(value).second;
        return (account.interest_mask & test_mask).any();
    }
};
