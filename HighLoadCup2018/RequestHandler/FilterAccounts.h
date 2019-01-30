#pragma once

#include "../RequestHandler.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <boost/mp11.hpp>

#include <algorithm>

template<>
struct RequestHandler<FilterAccounts>
{
    struct JSONResult
    {
        const FilterAccounts &request;
        rapidjson::Document document;
        rapidjson::Value account_array;
        rapidjson::StringBuffer buffer;

        JSONResult(const FilterAccounts &request)
            : request(request)
            , account_array(rapidjson::kArrayType)
        {
            document.SetObject();
        }

        void add_account(const Account &account)
        {
            rapidjson::Value json_account(rapidjson::kObjectType);
            json_account.AddMember("id", account.id, document.GetAllocator());
            json_account.AddMember("email", rapidjson::Value(rapidjson::StringRef(account.email)), document.GetAllocator());

            for (auto &filter : request.filter)
            {
                std::visit([this, &account, &json_account, &filter](auto &&field, auto &&method)
                {
                    using field_type = std::decay_t<decltype(field)>;
                    using method_type = std::decay_t<decltype(method)>;
                    if constexpr(!std::is_same_v<f_email, field_type> &&
                                 !std::is_same_v<f_interests, field_type> &&
                                 !std::is_same_v<f_likes, field_type>)
                    {
                        constexpr bool is_method_null = std::is_same_v<m_null, method_type>;
                        if (!(is_method_null && std::get<bool>(filter.value)))
                        {
                            json_account.AddMember(rapidjson::StringRef(field.name.data()), t_get_json_value<field_type>()(account, document.GetAllocator()), document.GetAllocator());
                        }
                    }
                }, filter.field, filter.method);
            }

            account_array.PushBack(std::move(json_account), document.GetAllocator());
        }

        const char *get_json()
        {
            document.AddMember("accounts", std::move(account_array), document.GetAllocator());

            rapidjson::Writer writer(buffer);
            document.Accept(writer);

            return buffer.GetString();
        }
    };

    template<class BeginIt, class EndIt>
    static void filter(const FilterAccounts &request, HttpServer::HttpResponse &response, BeginIt &&begin, EndIt &&end)
    {
        JSONResult result(request);
        std::size_t counter = 0;
        uint32_t last_added_account_id = 0;
        for (; begin != end && counter < request.limit; ++begin)
        {
            auto &account = begin->account();
            if (account.id == last_added_account_id)
            {
                continue;
            }

            bool is_suitable = true;
            for (auto &filter : request.filter)
            {
                bool check_result = std::visit([&account, &filter](auto &&field, auto &&method)
                {
                    return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(account, filter.value);
                }, filter.field, filter.method);

                if (!check_result)
                {
                    is_suitable = false;
                    break;
                }
            }
            if (is_suitable)
            {
                result.add_account(account);
                last_added_account_id = account.id;
                ++counter;
            }
        }

        response.result(boost::beast::http::status::ok);
        response.body() = result.get_json();
        response.prepare_payload();
    }

    static void optimize_filter_list(DB &db, FilterAccounts &request)
    {
        std::vector<Filter>::iterator first_name_it = request.filter.end();
        std::vector<Filter>::iterator sex_it = request.filter.end();
        for (auto it = request.filter.begin(); it != request.filter.end(); ++it)
        {
            std::visit([&first_name_it, &sex_it, &it](auto &&field)
            {
                if constexpr(std::is_same_v<f_first_name, std::decay_t<decltype(field)>>)
                {
                    first_name_it = it;
                }
                if constexpr(std::is_same_v<f_sex, std::decay_t<decltype(field)>>)
                {
                    sex_it = it;
                }
            }, it->field);
        }

        if (first_name_it != request.filter.end() && sex_it != request.filter.end())
        {
            bool is_male = std::get<Account::Sex>(sex_it->value) == Account::Sex::MALE;
            auto &name_list = is_male ? db.female_first_name : db.male_first_name;
            std::visit([&name_list](auto &&value)
            {
                if constexpr(std::is_same_v<std::vector<std::string_view>, std::decay_t<decltype(value)>>)
                {
                    for (auto &name : value)
                    {
                        if (std::binary_search(name_list.begin(), name_list.end(), name, common_less()))
                        {
                            name = "";
                        }
                    }
                }
                else if constexpr(std::is_same_v<std::string_view, std::decay_t<decltype(value)>>)
                {
                    if (std::binary_search(name_list.begin(), name_list.end(), value, common_less()))
                    {
                        value = "";
                    }
                }
            }, first_name_it->value);
        }
    }

    static void handle(DB &db, FilterAccounts &request, HttpServer::HttpResponse &response)
    {
        optimize_filter_list(db, request);

        using field_priority_list = boost::mp11::mp_list<f_likes, f_second_name, f_phone, f_city, f_first_name, f_interests, f_country, f_birth, f_email, f_premium, f_status, f_sex>;
        using method_ignore_list = boost::mp11::mp_list<m_lt, m_gt, m_starts, m_neq, m_null>;

        auto select_by_filter_it = std::min_element(request.filter.begin(), request.filter.end(), [](auto &&a, auto &&b)
        {
            auto a_field_value = std::visit([](auto &&field, auto &&method)
            {
                using method_type = std::decay_t<decltype(method)>;
                if constexpr(boost::mp11::mp_contains<method_ignore_list, method_type>::value)
                {
                    return boost::mp11::mp_size<field_priority_list>::value;
                }

                return boost::mp11::mp_find<field_priority_list, std::decay_t<decltype(field)>>::value;
            }, a.field, a.method);

            auto b_field_value = std::visit([](auto &&field, auto &&method)
            {
                using method_type = std::decay_t<decltype(method)>;
                if constexpr(boost::mp11::mp_contains<method_ignore_list, method_type>::value)
                {
                    return boost::mp11::mp_size<field_priority_list>::value;
                }

                return boost::mp11::mp_find<field_priority_list, std::decay_t<decltype(field)>>::value;
            }, b.field, b.method);

            return a_field_value < b_field_value;
        });

        bool is_selectable = false;
        if (select_by_filter_it != request.filter.end())
        {
            is_selectable = std::visit([](auto &&method)
            {
                return !boost::mp11::mp_contains<method_ignore_list, std::decay_t<decltype(method)>>::value;
            }, select_by_filter_it->method);
        }

        if (is_selectable)
        {
            std::visit([&db, &request, &response, &select_by_filter_it](auto &&field, auto &&method)
            {
                t_select<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(db, select_by_filter_it->value, [&db, &request, &response](auto &&range)
                {
                    filter(request, response, range.first, range.second);
                });
            }, select_by_filter_it->field, select_by_filter_it->method);
        }
        else
        {
            filter(request, response, db.account.rbegin(), db.account.rend());
        }
    }
};
