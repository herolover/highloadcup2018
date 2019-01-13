#pragma once

#include "../FieldMethodTrait.h"
#include "../Split.h"
#include "../UnionIter.h"

#include <algorithm>

template<class M>
struct t_has_method<f_city, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_eq, m_any, m_null>, M>::value;
};

template<>
struct t_get_json_value<f_city>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(*account.city));
    }
};

template<>
struct t_value<f_city, m_any>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        auto city_list = split(value);
        std::sort(city_list.begin(), city_list.end());

        return std::move(city_list);
    }
};

template<>
struct t_select<f_city, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &city, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::city_tag>().equal_range(std::get<std::string_view>(city))));
    }
};

template<>
struct t_select<f_city, m_any>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::city_tag>();

        using IterType = decltype(index.rbegin());
        std::vector<std::pair<IterType, IterType>> range_list;
        for (auto &city : std::get<std::vector<std::string_view>>(value))
        {
            range_list.push_back(make_reverse_range(index.equal_range(city)));
        }

        handler(std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_select<f_city, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &is_null, Handler &&handler) const
    {
        auto &index = db.account.get<DB::city_tag>();
        handler(make_reverse_range(std::get<bool>(is_null) ? index.equal_range(nullptr) : std::make_pair(index.upper_bound(nullptr), index.end())));
    }
};

template<>
struct t_check<f_city, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.city && account.city == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_city, m_any>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &city_list = std::get<std::vector<std::string_view>>(value);
        return (bool)account.city && std::binary_search(city_list.begin(), city_list.end(), *account.city);
    }
};

template<>
struct t_check<f_city, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == !account.city;
    }
};
