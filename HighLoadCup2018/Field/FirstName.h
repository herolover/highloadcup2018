#pragma once

#include "../FieldMethodTrait.h"
#include "../Split.h"
#include "../UnionIter.h"

#include <algorithm>

template<class M>
struct t_has_method<f_first_name, M>
{
    constexpr static bool value = boost::mp11::mp_contains<boost::mp11::mp_list<m_eq, m_any, m_null>, M>::value;
};

template<>
struct t_get_json_value<f_first_name>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(*account.first_name));
    }
};

template<>
struct t_value<f_first_name, m_any>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        auto first_name_list = split(value);
        std::sort(first_name_list.begin(), first_name_list.end());

        return std::move(first_name_list);
    }
};

template<>
struct t_select<f_first_name, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::first_name_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_first_name, m_any>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::first_name_tag>();

        using IterType = decltype(index.rbegin());
        std::vector<std::pair<IterType, IterType>> range_list;
        for (auto &first_name : std::get<std::vector<std::string_view>>(value))
        {
            range_list.emplace_back(make_reverse_range(index.equal_range(first_name)));
        }

        handler(std::make_pair(union_iter<true, IterType>(range_list), union_iter<true, IterType>(range_list, true)));
    }
};

template<>
struct t_select<f_first_name, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::first_name_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range(nullptr) : std::make_pair(index.upper_bound(nullptr), index.end())));
    }
};

template<>
struct t_check<f_first_name, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.first_name && account.first_name == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_first_name, m_any>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &first_name_list = std::get<std::vector<std::string_view>>(value);
        return account.first_name && std::binary_search(first_name_list.begin(), first_name_list.end(), *account.first_name);
    }
};

template<>
struct t_check<f_first_name, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == !account.first_name;
    }
};
