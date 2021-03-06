#pragma once

#include "FieldMethod.h"
#include "DB.h"
#include "Common.h"

#include <rapidjson/document.h>

#include <boost/mp11.hpp>

#include <charconv>
#include <string_view>

using namespace std::literals;

template<class F, class M>
struct t_has_method
{
    constexpr static bool value = false;
};

template<class F>
struct t_get_json_value
{
    rapidjson::Value operator()(const Account &account, rapidjson::MemoryPoolAllocator<> &allocator) const
    {
        return rapidjson::Value(rapidjson::StringRef(""));
    }
};

template<class F, class M>
struct t_value
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return value;
    }
};

template<class F>
struct t_value<F, m_null>
{
    Value operator()(DB &db, const std::string_view &value) const
    {
        return value[0] == '1';
    }
};

template<class F, class M>
struct t_select
{
    template<class Handler>
    void operator()(DB &db, const Value &value, Handler &&handler) const
    {
        throw std::runtime_error("Not implemented");
    }
};

template<class F, class M>
struct t_check
{
    bool operator()(const Account &account, const Value &value) const
    {
        throw std::runtime_error("Not implemented");
    }
};
