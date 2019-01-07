#pragma once

#include "../FieldMethodTrait.h"

template<>
struct t_get_json_value<f_birth>
{
    rapidjson::Value operator()(const Account &account) const
    {
        return rapidjson::Value(account.birth);
    }
};

template<>
struct t_value<f_birth, m_lt>
{
    Value operator()(const std::string_view &value)
    {
        int32_t date = 0;
        std::from_chars(value.data(), value.data() + value.size(), date);

        return date;
    }
};

template<>
struct t_value<f_birth, m_gt>
{
    Value operator()(const std::string_view &value)
    {
        int32_t date = 0;
        std::from_chars(value.data(), value.data() + value.size(), date);

        return date;
    }
};

template<>
struct t_value<f_birth, m_year>
{
    Value operator()(const std::string_view &value)
    {
        uint16_t year = 0;
        std::from_chars(value.data(), value.data() + value.size(), year);

        return year;
    }
};

template<>
struct t_select<f_birth, m_lt>
{
    template<class Handler>
    void operator()(DB &db, const Value &date, Handler &&handler) const
    {
        auto &index = db.account.get<DB::birth_tag>();
        handler(make_reverse_range(std::make_pair(index.begin(), index.lower_bound(std::get<int32_t>(date)))));
    }
};

template<>
struct t_select<f_birth, m_gt>
{
    template<class Handler>
    void operator()(DB &db, const Value &date, Handler &&handler) const
    {
        auto &index = db.account.get<DB::birth_tag>();
        handler(make_reverse_range(std::make_pair(index.upper_bound(std::get<int32_t>(date)), index.end())));
    }
};

template<>
struct t_select<f_birth, m_year>
{
    template<class Handler>
    void operator()(DB &db, const Value &year, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::birth_year_tag>().equal_range(std::get<uint16_t>(year))));
    }
};

template<>
struct t_check<f_birth, m_lt>
{
    template<class ForwardIt>
    bool operator()(const ForwardIt &it, const Value &date) const
    {
        return it->birth < std::get<int32_t>(date);
    }
};

template<>
struct t_check<f_birth, m_gt>
{
    template<class ForwardIt>
    bool operator()(const ForwardIt &it, const Value &date) const
    {
        return it->birth > std::get<int32_t>(date);
    }
};

template<>
struct t_check<f_birth, m_year>
{
    template<class ForwardIt>
    bool operator()(const ForwardIt &it, const Value &year) const
    {
        return it->birth_year == std::get<uint16_t>(year);
    }
};
