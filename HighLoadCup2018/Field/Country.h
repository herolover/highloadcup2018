#pragma once

#include "../FieldMethodTrait.h"

template<>
struct t_get_json_value<f_country>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(*account.country));
    }
};

template<>
struct t_select<f_country, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::country_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_country, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::country_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range(nullptr) : std::make_pair(index.upper_bound(nullptr), index.end())));
    }
};

template<>
struct t_check<f_country, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.country && account.country == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_country, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == !account.country;
    }
};