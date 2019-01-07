#pragma once

#include "Split.h"
#include "FieldMethod.h"
#include "FieldMethodTrait.h"

#include "Field/Birth.h"
#include "Field/City.h"
#include "Field/Country.h"
#include "Field/Email.h"
#include "Field/FirstName.h"
#include "Field/Interests.h"
#include "Field/Likes.h"
#include "Field/Phone.h"
#include "Field/Premium.h"
#include "Field/SecondName.h"
#include "Field/Sex.h"
#include "Field/Status.h"

#include <string_view>
#include <variant>
#include <vector>
#include <algorithm>

#include <boost/beast.hpp>

using namespace std::literals;

struct FilterAccounts
{
    struct Filter
    {
        Field field;
        Method method;
        Value value;
    };
    std::vector<Filter> filter;
    uint8_t limit;
};

struct GroupAccounts
{
};

struct RecommendForAccount
{
    uint32_t account_id;
};

struct SuggestForAccount
{
    uint32_t account_id;
};

struct AddAccount
{
};

struct UpdateAccount
{
};

struct AddLikes
{
};

struct BadRequest
{
};

using ParsedRequest = std::variant<FilterAccounts, GroupAccounts, RecommendForAccount, SuggestForAccount, AddAccount, UpdateAccount, AddLikes, BadRequest>;

inline ParsedRequest parse_http_request(const boost::beast::http::request<boost::beast::http::string_body> &request, const std::string_view &decoded_target)
{
    ParsedRequest result = BadRequest();

    auto target_parts = split(std::string_view(decoded_target.data() + 1, decoded_target.size() - 1), '/');
    if (target_parts[0] == "accounts"sv)
    {
        if (target_parts[1] == "filter" && target_parts.size() == 3 && target_parts[2][0] == '?')
        {
            bool is_valid = true;
            bool has_limit = false;
            FilterAccounts filter_accounts;
            for (auto &param : split(std::string_view(target_parts[2].data() + 1, target_parts[2].size() - 1), '&'))
            {
                auto key_value = split(param, '=');

                if (key_value[0] == "limit"sv)
                {
                    has_limit = true;
                    std::from_chars(key_value[1].data(), key_value[1].data() + key_value[1].size(), filter_accounts.limit);
                }
                else if (key_value[0] != "query_id"sv)
                {
                    auto field_method = split(key_value[0], '_');

                    if (field_method.size() == 2)
                    {
                        auto field = make_field(field_method[0]);
                        auto method = make_method(field_method[1]);
                        if (field.index() != 0 && method.index() != 0)
                        {
                            auto value = for_field_method(field, method, [&key_value](auto &&field, auto &&method)
                            {
                                return t_value<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(key_value[1]);
                            });

                            filter_accounts.filter.push_back(FilterAccounts::Filter{std::move(field), std::move(method), std::move(value)});
                        }
                        else
                        {
                            is_valid = false;
                            break;
                        }
                    }
                    else
                    {
                        is_valid = false;
                        break;
                    }
                }
            }

            if (is_valid && has_limit)
            {
                result = filter_accounts;
            }
        }
    }

    return result;
}
