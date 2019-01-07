#pragma once

#include "../FieldMethodTrait.h"

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
        auto &index = db.account.get<DB::phone_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range("") : std::make_pair(index.upper_bound(""), index.end())));
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
