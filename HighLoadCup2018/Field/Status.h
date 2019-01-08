#pragma once

#include "../FieldMethodTrait.h"
#include "../HandlerIter.h"
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
    Value operator()(const std::string_view &value) const
    {
        return convert_account_status(value);
    }
};

template<>
struct t_value<f_status, m_neq>
{
    Value operator()(const std::string_view &value) const
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
        auto &index = db.account.get<DB::status_tag>();
        auto ignore_range = index.equal_range(std::get<Account::Status>(value));

        handler(std::make_pair(handler_iter(index.begin(), [ignore_range = std::move(ignore_range)](auto &it)
        {
            if (it == ignore_range.first)
            {
                it = ignore_range.second;
            }
        }), index.end()));
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
