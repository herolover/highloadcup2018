#pragma once

#include "DB.h"
#include "Split.h"
#include "FieldMethod.h"
#include "FieldMethodTrait.h"

#include "Field/Birth.h"
#include "Field/City.h"
#include "Field/Country.h"
#include "Field/Email.h"
#include "Field/FirstName.h"
#include "Field/Interests.h"
#include "Field/Joined.h"
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

struct Filter
{
    Field field;
    Method method;
    Value value;
};

struct FilterAccounts
{
    std::vector<Filter> filter;
    uint8_t limit;
};

struct GroupAccounts
{
    std::vector<Filter> filter;
    std::vector<Field> key;
    bool is_large_first;
    uint8_t limit;
};

struct SearchForAccount
{
    uint32_t account_id;
    Filter filter;
    uint8_t limit;
};

struct RecommendForAccount
{
    SearchForAccount search;
};

struct SuggestForAccount
{
    SearchForAccount search;
};

struct AddAccount
{
    const char *body;
    std::size_t size;
};

struct UpdateAccount
{
    uint32_t account_id;
    const char *body;
    std::size_t size;
};

struct AddLikes
{
};

struct BadRequest
{
    boost::beast::http::status status = boost::beast::http::status::not_found;
};

using ParsedRequest = std::variant<FilterAccounts, GroupAccounts, RecommendForAccount, SuggestForAccount, AddAccount, UpdateAccount, AddLikes, BadRequest>;

inline ParsedRequest parse_http_request(DB &db, const boost::beast::http::request<boost::beast::http::string_body> &request, const std::string_view &decoded_target)
{
    ParsedRequest result = BadRequest();

    auto target_parts = split(std::string_view(decoded_target.data() + 1, decoded_target.size() - 1), '/');
    if (target_parts[0] == "accounts"sv)
    {
        if (target_parts[1] == "filter"sv && target_parts.size() == 3)
        {
            bool is_valid = true;
            bool has_limit = false;
            FilterAccounts filter_accounts;
            for (auto &param : split(std::string_view(target_parts[2].data() + 1, target_parts[2].size() - 1), '&'))
            {
                auto key_value = split(param, '=');

                if (key_value[0] == "limit"sv)
                {
                    auto [p, ec] = std::from_chars(key_value[1].data(), key_value[1].data() + key_value[1].size(), filter_accounts.limit);
                    if (ec == std::errc())
                    {
                        has_limit = true;
                    }
                    else
                    {
                        is_valid = false;
                        break;
                    }
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
                            auto value = std::visit([&db, &key_value, &is_valid](auto &&field, auto &&method)
                            {
                                using field_type = std::decay_t<decltype(field)>;
                                using method_type = std::decay_t<decltype(method)>;

                                is_valid = t_has_method<field_type, method_type>::value;

                                return t_value<field_type, method_type>()(db, key_value[1]);
                            }, field, method);

                            if (!is_valid)
                            {
                                break;
                            }

                            filter_accounts.filter.push_back(Filter{std::move(field), std::move(method), std::move(value)});
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
                result = std::move(filter_accounts);
            }
            else
            {
                std::get<BadRequest>(result).status = boost::beast::http::status::bad_request;
            }
        }
        else if (target_parts[1] == "group"sv && target_parts.size() == 3)
        {
            bool is_valid = true;
            bool has_keys = false;
            bool has_order = false;
            bool has_limit = false;
            GroupAccounts group_accounts;
            for (auto &param : split(std::string_view(target_parts[2].data() + 1, target_parts[2].size() - 1), '&'))
            {
                auto key_value = split(param, '=');
                if (key_value[0] == "keys"sv)
                {
                    has_keys = true;
                    for (auto &key : split(key_value[1]))
                    {
                        auto field = make_field(key);
                        is_valid = std::visit([](auto &&field)
                        {
                            return boost::mp11::mp_contains<boost::mp11::mp_list<f_sex, f_status, f_interests, f_country, f_city>, std::decay_t<decltype(field)>>::value;
                        }, field);
                        if (!is_valid)
                        {
                            break;
                        }
                        group_accounts.key.push_back(std::move(field));
                    }

                    if (!is_valid)
                    {
                        break;
                    }
                }
                else if (key_value[0] == "order"sv)
                {
                    if (key_value[1] == "1"sv)
                    {
                        has_order = true;
                        group_accounts.is_large_first = false;
                    }
                    else if (key_value[1] == "-1"sv)
                    {
                        has_order = true;
                        group_accounts.is_large_first = true;
                    }
                    else
                    {
                        is_valid = false;
                        break;
                    }
                }
                else if (key_value[0] == "limit"sv)
                {
                    auto[p, ec] = std::from_chars(key_value[1].data(), key_value[1].data() + key_value[1].size(), group_accounts.limit);
                    if (ec == std::errc())
                    {
                        has_limit = true;
                    }
                    else
                    {
                        is_valid = false;
                        break;
                    }
                }
                else if (key_value[0] != "query_id"sv)
                {
                    auto field = make_field(key_value[0]);
                    if (field.index() != 0)
                    {
                        Method method;
                        Value value;
                        std::visit([&db, &method, &value, &key_value](auto &&field)
                        {
                            using field_type = std::decay_t<decltype(field)>;
                            if constexpr(std::is_same_v<f_birth, field_type> || std::is_same_v<f_joined, field_type>)
                            {
                                method = m_year();
                                value = t_value<field_type, m_year>()(db, key_value[1]);
                            }
                            else
                            {
                                method = m_eq();
                                value = t_value<field_type, m_eq>()(db, key_value[1]);
                            }
                        }, field);

                        group_accounts.filter.push_back(Filter{std::move(field), std::move(method), std::move(value)});
                    }
                    else
                    {
                        is_valid = false;
                        break;
                    }
                }
            }

            if (is_valid && has_keys && has_order && has_limit)
            {
                result = std::move(group_accounts);
            }
            else
            {
                std::get<BadRequest>(result).status = boost::beast::http::status::bad_request;
            }
        }
        else if ((target_parts[2] == "recommend"sv || target_parts[2] == "suggest"sv) && target_parts.size() == 4)
        {
            SearchForAccount search;
            auto[p, ec] = std::from_chars(target_parts[1].data(), target_parts[1].data() + target_parts[1].size(), search.account_id);

            auto &index = db.account.get<DB::id_tag>();
            if (ec == std::errc() && search.account_id > 0 && index.find(search.account_id) != index.end())
            {
                bool is_valid = true;
                bool has_limit = false;
                for (auto &param : split(std::string_view(target_parts[3].data() + 1, target_parts[3].size() - 1), '&'))
                {
                    auto key_value = split(param, '=');
                    if (key_value[0] == "limit"sv)
                    {
                        auto[p, ec] = std::from_chars(key_value[1].data(), key_value[1].data() + key_value[1].size(), search.limit);
                        if (ec == std::errc() && search.limit != 0)
                        {
                            has_limit = true;
                        }
                        else
                        {
                            is_valid = false;
                            break;
                        }
                    }
                    else if (key_value[0] == "country"sv)
                    {
                        search.filter.field = f_country();
                        search.filter.method = m_eq();
                        search.filter.value = t_value<f_country, m_eq>()(db, key_value[1]);
                    }
                    else if (key_value[0] == "city"sv)
                    {
                        search.filter.field = f_city();
                        search.filter.method = m_eq();
                        search.filter.value = t_value<f_city, m_eq>()(db, key_value[1]);
                    }
                }

                if (is_valid && has_limit)
                {
                    if (target_parts[2] == "recommend"sv)
                    {
                        RecommendForAccount recommend;
                        recommend.search = std::move(search);
                        result = std::move(recommend);
                    }
                    else
                    {
                        SuggestForAccount suggest;
                        suggest.search = std::move(search);
                        result = std::move(suggest);
                    }
                }
                else
                {
                    std::get<BadRequest>(result).status = boost::beast::http::status::bad_request;
                }
            }
        }
        else if (target_parts[1] == "new"sv)
        {
            AddAccount add_account{request.body().data(), request.body().size()};
            result = std::move(add_account);
        }
        else if (target_parts[1] == "likes"sv)
        {
        }
        else if (target_parts.size() == 3)
        {
            UpdateAccount update_account;
            auto[p, ec] = std::from_chars(target_parts[1].data(), target_parts[1].data() + target_parts[1].size(), update_account.account_id);

            if (ec == std::errc() && db.has_account(update_account.account_id))
            {
                update_account.body = request.body().data();
                update_account.size = request.body().size();
                result = std::move(update_account);
            }
            else
            {
                std::get<BadRequest>(result).status = boost::beast::http::status::bad_request;
            }
        }
    }

    return result;
}
