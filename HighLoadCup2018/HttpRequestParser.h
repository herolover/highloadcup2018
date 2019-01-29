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

// *INDENT-OFF*
using GroupKey = std::variant<
     invalid,
     f_sex,
     f_status,
     f_country,
     f_city,
     f_interests,
     std::pair<f_city, f_sex>,
     std::pair<f_city, f_status>,
     std::pair<f_country, f_sex>,
     std::pair<f_country, f_status>>;
// *INDENT-ON*

template<class T>
struct GroupKeyTrait
{
    using type = std::string_view;

    static std::string_view key_name()
    {
        return T::name;
    }
};

template<class T1, class T2>
struct GroupKeyTrait<std::pair<T1, T2>>
{
    using type = std::pair<std::string_view, std::string_view>;

    static std::pair<std::string_view, std::string_view> key_name()
    {
        return std::make_pair(T1::name, T2::name);
    }
};

GroupKey make_group_key(std::string_view value)
{
    if (value == "sex")
    {
        return f_sex();
    }
    else if (value == "status")
    {
        return f_status();
    }
    else if (value == "country")
    {
        return f_country();
    }
    else if (value == "city")
    {
        return f_city();
    }
    else if (value == "interests")
    {
        return f_interests();
    }
    else if (value == "city,sex")
    {
        return std::make_pair(f_city(), f_sex());
    }
    else if (value == "city,status")
    {
        return std::make_pair(f_city(), f_status());
    }
    else if (value == "country,sex")
    {
        return std::make_pair(f_country(), f_sex());
    }
    else if (value == "country,status")
    {
        return std::make_pair(f_country(), f_status());
    }

    return invalid();
}

struct GroupAccounts
{
    std::vector<Filter> filter;
    Filter likes_filter;
    Filter birth_year_filter;
    Filter joined_year_filter;
    Filter interests_filter;
    GroupKey group_key;
    bool is_large_first;
    uint8_t limit;
};

struct SearchForAccount
{
    DB::AccountIt account_it;
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
    DB::AccountIt account_it;
    const char *body;
    std::size_t size;
};

struct AddLikes
{
    const char *body;
    std::size_t size;
};

struct BadRequest
{
    boost::beast::http::status status = boost::beast::http::status::not_found;
};

using ParsedRequest = std::variant<FilterAccounts, GroupAccounts, RecommendForAccount, SuggestForAccount, AddAccount, UpdateAccount, AddLikes, BadRequest>;

inline ParsedRequest parse_http_request(DB &db, const boost::beast::http::request<boost::beast::http::string_body> &request, const std::string_view &decoded_target)
{
    ParsedRequest result = BadRequest();

    auto pos = decoded_target.find('?');
    if (pos == std::string_view::npos)
    {
        return result;
    }

    auto target = std::string_view(decoded_target.data() + 1, pos - 2);
    auto params = std::string_view(decoded_target.data() + pos + 1, decoded_target.size() - pos - 1);

    auto target_parts = split(target, '/');
    if (target_parts[0] == "accounts"sv)
    {
        if (target_parts.size() == 2)
        {
            if (target_parts[1] == "filter"sv)
            {
                bool is_valid = true;
                bool has_limit = false;
                FilterAccounts filter_accounts;
                for (auto &param : split(params, '&'))
                {
                    auto key_value = split(param, '=');
                    if (key_value.size() != 2 || key_value[1] == ""sv)
                    {
                        is_valid = false;
                        break;
                    }

                    if (key_value[0] == "limit"sv)
                    {
                        auto[p, ec] = std::from_chars(key_value[1].data(), key_value[1].data() + key_value[1].size(), filter_accounts.limit);
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
            else if (target_parts[1] == "group"sv)
            {
                bool is_valid = true;
                bool has_keys = false;
                bool has_order = false;
                bool has_limit = false;
                GroupAccounts group_accounts;
                for (auto &param : split(params, '&'))
                {
                    auto key_value = split(param, '=');
                    if (key_value.size() != 2 || key_value[1] == ""sv)
                    {
                        is_valid = false;
                        break;
                    }

                    if (key_value[0] == "keys"sv)
                    {
                        has_keys = true;
                        group_accounts.group_key = make_group_key(key_value[1]);
                        is_valid = group_accounts.group_key.index() != 0;
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
                            if (std::holds_alternative<f_likes>(field))
                            {
                                group_accounts.likes_filter = group_accounts.filter.back();
                            }
                            else if (std::holds_alternative<f_birth>(field))
                            {
                                group_accounts.birth_year_filter = group_accounts.filter.back();
                            }
                            else if (std::holds_alternative<f_joined>(field))
                            {
                                group_accounts.joined_year_filter = group_accounts.filter.back();
                            }
                            else if (std::holds_alternative<f_interests>(field))
                            {
                                group_accounts.interests_filter = group_accounts.filter.back();
                            }
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
            else if (target_parts[1] == "new"sv)
            {
                AddAccount add_account{request.body().data(), request.body().size()};
                result = std::move(add_account);
            }
            else if (target_parts[1] == "likes"sv)
            {
                AddLikes add_likes{request.body().data(), request.body().size()};
                result = std::move(add_likes);
            }
            else
            {
                UpdateAccount update_account;
                uint32_t account_id = 0;
                auto[p, ec] = std::from_chars(target_parts[1].data(), target_parts[1].data() + target_parts[1].size(), account_id);

                bool has_account = false;
                if (ec == std::errc())
                {
                    std::tie(has_account, update_account.account_it) = db.has_account_with_lock(account_id);
                    if (has_account)
                    {
                        update_account.body = request.body().data();
                        update_account.size = request.body().size();
                        result = std::move(update_account);
                    }
                }
                else
                {
                    std::get<BadRequest>(result).status = boost::beast::http::status::not_found;
                }
            }
        }
        else if (target_parts.size() == 3 && (target_parts[2] == "recommend"sv || target_parts[2] == "suggest"sv))
        {
            SearchForAccount search;
            uint32_t account_id = 0;
            auto[p, ec] = std::from_chars(target_parts[1].data(), target_parts[1].data() + target_parts[1].size(), account_id);

            if (ec == std::errc())
            {
                bool has_account = false;
                std::tie(has_account, search.account_it) = db.has_account(account_id);
                if (has_account)
                {
                    bool is_valid = true;
                    bool has_limit = false;
                    for (auto &param : split(params, '&'))
                    {
                        auto key_value = split(param, '=');
                        if (key_value.size() != 2 || key_value[1] == ""sv)
                        {
                            is_valid = false;
                            break;
                        }
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
        }
    }

    return result;
}
