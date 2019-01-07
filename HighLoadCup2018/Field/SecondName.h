#pragma once

#include "../FieldMethodTrait.h"
#include "../HandlerIter.h"

template<>
struct t_get_json_value<f_second_name>
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(*account.second_name));
    }
};

template<>
struct t_select<f_second_name, m_eq>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        handler(make_reverse_range(db.account.get<DB::second_name_tag>().equal_range(std::get<std::string_view>(value))));
    }
};

template<>
struct t_select<f_second_name, m_starts>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &second_name_begin = std::get<std::string_view>(value);

        auto &index = db.account.get<DB::second_name_tag>();
        auto end_it = index.end();
        handler(std::make_pair(handler_iter(index.lower_bound(second_name_begin), [second_name_begin, end_it](auto &it)
        {
            if (it != end_it)
            {
                auto &account = *it;
                auto &second_name = it->second_name;

                if (std::string_view(second_name->c_str(), second_name_begin.size()) != second_name_begin)
                {
                    it = end_it;
                }
            }
        }), end_it));
    }
};

template<>
struct t_select<f_second_name, m_null>
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        auto &index = db.account.get<DB::second_name_tag>();
        handler(make_reverse_range(std::get<bool>(value) ? index.equal_range(nullptr) : std::make_pair(index.upper_bound(nullptr), index.end())));
    }
};

template<>
struct t_check<f_second_name, m_eq>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return account.second_name && account.second_name == std::get<std::string_view>(value);
    }
};

template<>
struct t_check<f_second_name, m_starts>
{
    bool operator()(const Account &account, const Value &value) const
    {
        auto &second_name_begin = std::get<std::string_view>(value);
        return account.second_name && std::string_view(account.second_name->c_str(), second_name_begin.size()) == second_name_begin;
    }
};

template<>
struct t_check<f_second_name, m_null>
{
    bool operator()(const Account &account, const Value &value) const
    {
        return std::get<bool>(value) == !account.second_name;
    }
};
