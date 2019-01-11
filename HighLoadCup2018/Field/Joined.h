#pragma once

#include "../FieldMethodTrait.h"

template<>
struct t_value<f_joined, m_year>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        uint16_t year = 0;
        std::from_chars(value.data(), value.data() + value.size(), year);

        return year;
    }
};

template<>
struct t_select<f_joined, m_year>
{
    template<class Handler>
    void operator()(DB &db, const Value &year, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::joined_year_tag>().equal_range(std::get<uint16_t>(year))));
    }
};
