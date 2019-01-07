#pragma once

#include "FieldMethod.h"
#include "DB.h"
#include "Common.h"

#include <rapidjson/document.h>

#include <charconv>
#include <string_view>

using namespace std::literals;

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
    template<class ForwardIt>
    bool operator()(const ForwardIt &it, const Value &value) const
    {
        throw std::runtime_error("Not implemented");
    }
};

template<class F, class M>
struct t_value
{
    Value operator()(const std::string_view &value) const
    {
        return value;
    }
};

template<class F>
struct t_value<F, m_null>
{
    Value operator()(const std::string_view &value) const
    {
        return value[0] == '1';
    }
};

template<class F>
struct t_get_json_value
{
    rapidjson::Value operator()(const Account &account) const
    {
        return rapidjson::Value(rapidjson::StringRef(""));
    }
};
