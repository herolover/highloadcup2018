#pragma once

#include "../FieldMethodTrait.h"

template<class M>
struct t_has_method<f_email, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_domain, m_lt, m_gt>, M>::value;
};

template<>
struct t_get_json_value<f_email>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(account.email));
    }
};

template<>
struct t_select<f_email, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::email_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_email, m_domain>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::email_domain_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_email, m_lt>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::email_tag>();
        handler(make_reverse_range(std::make_pair(index.begin(), index.lower_bound(std::get<std::string_view>(value)))));
    }
};

template<>
struct t_select<f_email, m_gt>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::email_tag>();
        handler(make_reverse_range(std::make_pair(index.upper_bound(std::get<std::string_view>(value)), index.end())));
    }
};

template<>
struct t_check<f_email, m_domain>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.email_domain == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_email, m_lt>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.email< std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_email, m_gt>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<std::string_view>(value) < account.email;
    }
};
