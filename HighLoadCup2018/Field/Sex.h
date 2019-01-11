#pragma once

#include "../FieldMethodTrait.h"

template<class M>
struct t_has_method<f_sex, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_eq>, M>::value;
};

template<>
struct t_get_json_value<f_sex>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(account.is_male ? "m" : "f"));
    }
};

template<>
struct t_value<f_sex, m_eq>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return value[0] == 'm';
    }
};

template<>
struct t_select<f_sex, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::sex_tag>().equal_range(std::get<bool>(value))));
    }
};

template<>
struct t_check<f_sex, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.is_male == std::get<bool>(value);
    }
};
