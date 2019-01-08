#pragma once

#include "../RequestHandler.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <boost/mp11.hpp>

#include <algorithm>

namespace rj = rapidjson;

template<>
struct RequestHandler<FilterAccounts>
{
    struct JSONResult
    {
        const FilterAccounts &request;
        rj::Document document;
        rj::Value account_array;
        rj::StringBuffer buffer;

        JSONResult(const FilterAccounts &request)
            : request(request)
            , account_array(rj::kArrayType)
        {
            document.SetObject();
        }

        void add_account(const Account &account)
        {
            rj::Value json_account(rj::kObjectType);
            json_account.AddMember("id", account.id, document.GetAllocator());
            json_account.AddMember("email", rj::Value(rj::StringRef(account.email)), document.GetAllocator());

            for (auto &filter : request.filter)
            {
                for_field_method(filter.field, filter.method, [this, &account, &json_account, &filter](auto &&field, auto &&method)
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
                            json_account.AddMember(rj::StringRef(field.name.data()), t_get_json_value<field_type>()(account, document.GetAllocator()), document.GetAllocator());
                        }
                    }
                });
            }

            account_array.PushBack(std::move(json_account), document.GetAllocator());
        }

        const char *get_json()
        {
            document.AddMember("accounts", std::move(account_array), document.GetAllocator());

            rj::Writer writer(buffer);
            document.Accept(writer);

            return buffer.GetString();
        }
    };

    template<class BeginIt, class EndIt>
    static void filter(const FilterAccounts &request, HttpServer::HttpResponse &response, BeginIt &&begin, EndIt &&end)
    {
        JSONResult result(request);
        std::size_t counter = 0;
        for (; begin != end && counter < request.limit; ++begin)
        {
            auto &account = begin->account();

            bool is_suitable = true;
            for (auto &filter : request.filter)
            {
                bool check_result = for_field_method(filter.field, filter.method, [&account, &filter](auto &&field, auto &&method)
                {
                    return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(account, filter.value);
                });

                if (!check_result)
                {
                    is_suitable = false;
                    break;
                }
            }
            if (is_suitable)
            {
                ++counter;
                result.add_account(account);
            }
        }

        response.result(boost::beast::http::status::ok);
        response.body() = result.get_json();
        response.prepare_payload();
    }

    static void handle(DB &db, const FilterAccounts &request, HttpServer::HttpResponse &response)
    {
        using dont_select_by_method = boost::mp11::mp_list<m_lt, m_gt, m_starts, m_now, m_neq>;

        auto select_by_filter_it = request.filter.begin();
        for (; select_by_filter_it != request.filter.end(); ++select_by_filter_it)
        {
            bool is_suitable = std::visit([](auto &&method)
            {
                return !boost::mp11::mp_contains<dont_select_by_method, std::decay_t<decltype(method)>>::value;
            }, select_by_filter_it->method);

            if (is_suitable)
            {
                break;
            }
        }

        if (select_by_filter_it != request.filter.end())
        {
            for_field_method(select_by_filter_it->field, select_by_filter_it->method, [&db, &request, &response, &select_by_filter_it](auto &&field, auto &&method)
            {
                t_select<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(db, select_by_filter_it->value, [&db, &request, &response](auto &&range)
                {
                    filter(request, response, range.first, range.second);
                });
            });
        }
        else
        {
            auto &index = db.account.get<DB::id_tag>();
            filter(request, response, index.rbegin(), index.rend());
        }
    }
};
