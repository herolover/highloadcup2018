#pragma once

#include "../FieldMethodTrait.h"

#include <ctime>

template<class M>
struct t_has_method<f_premium, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_now, m_null>, M>::value;
};

template<>
struct t_get_json_value<f_premium>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        rapidjson::Value premium(rapidjson::kObjectType);
        premium.AddMember("start", account.premium_start, allocator);
        premium.AddMember("finish", account.premium_finish, allocator);

        return std::move(premium);
    }
};

template<>
struct t_select<f_premium, m_now>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::premium_tag>().equal_range(Account::PremiumStatus::ACTIVE)));
    }
};

template<>
struct t_select<f_premium, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::premium_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range(Account::PremiumStatus::NO) : std::make_pair(index.upper_bound(Account::PremiumStatus::NO), index.end())));
    }
};

template<>
struct t_check<f_premium, m_now>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.premium_status == Account::PremiumStatus::ACTIVE;
    }
};

template<>
struct t_check<f_premium, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == (account.premium_status == Account::PremiumStatus::NO);
    }
};
