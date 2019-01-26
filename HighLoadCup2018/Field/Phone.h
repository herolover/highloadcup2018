#pragma once

#include "../FieldMethodTrait.h"

template<class M>
struct t_has_method<f_phone, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_code, m_null>, M>::value;
};

template<>
struct t_get_json_value<f_phone>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(account.phone));
    }
};

template<>
struct t_select<f_phone, m_code>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::phone_code_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_phone, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::phone_code_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range(""sv) : std::make_pair(index.upper_bound(""sv), index.end())));
    }
};

template<>
struct t_check<f_phone, m_code>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.phone_code == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_phone, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == account.phone.empty();
    }
};
