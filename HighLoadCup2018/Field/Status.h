#pragma once

#include "../FieldMethodTrait.h"
#include "../UnionIter.h"
#include "../Convert.h"

template<class M>
struct t_has_method<f_status, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_eq, m_neq>, M>::value;
};

template<>
struct t_get_json_value<f_status>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(convert_account_status(account.status)));
    }
};

template<>
struct t_value<f_status, m_eq>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return convert_account_status(value);
    }
};

template<>
struct t_value<f_status, m_neq>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return convert_account_status(value);
    }
};

template<>
struct t_select<f_status, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::status_tag>().equal_range(std::get<Account::Status>(value))));
    }
};

template<>
struct t_select<f_status, m_neq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto status = std::get<Account::Status>(value);
        auto &index = db.account.get<DB::status_tag>();

        using IterType = decltype(index.rbegin());
        std::vector<std::pair<IterType, IterType>> range_list;
        if (status != Account::Status::BUSY)
        {
            range_list.emplace_back(make_reverse_range(index.equal_range(Account::Status::BUSY)));
        }
        if (status != Account::Status::COMPLICATED)
        {
            range_list.emplace_back(make_reverse_range(index.equal_range(Account::Status::COMPLICATED)));
        }
        if (status != Account::Status::FREE)
        {
            range_list.emplace_back(make_reverse_range(index.equal_range(Account::Status::FREE)));
        }

        handler(std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_check<f_status, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.status == std::get<Account::Status>(value);
    }
};

template<>
struct t_check<f_status, m_neq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.status != std::get<Account::Status>(value);
    }
};
