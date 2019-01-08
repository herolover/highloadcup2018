#pragma once

#include "../RequestHandler.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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

    static void handle(DB &db, const FilterAccounts &request, HttpServer::HttpResponse &response)
    {
        if (!request.filter.empty())
        {
            auto index_by_field_it = std::min_element(request.filter.begin(), request.filter.end(), [](auto &&a, auto &&b)
            {
                auto a_priority = std::visit([](auto &&field)
                {
                    return field.priority;
                }, a.field);

                auto b_priority = std::visit([](auto &&field)
                {
                    return field.priority;
                }, b.field);

                return a_priority < b_priority;
            });

            for_field_method(index_by_field_it->field, index_by_field_it->method, [&db, &request, &response, &index_by_field_it](auto &&field, auto &&method)
            {
                t_select<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(db, index_by_field_it->value, [&db, &request, &response, &index_by_field_it](auto &&range)
                {
                    std::vector<uint32_t> id_list;
                    // TODO if range is already sorted, count limit here
                    for (auto &it = range.first; it != range.second; ++it)
                    {
                        id_list.push_back(it->get_id());
                    }

                    // TODO replace by partial sort
                    std::sort(id_list.begin(), id_list.end(), std::greater<int>());

                    JSONResult result(request);
                    std::size_t counter = 0;
                    for (std::size_t i = 0; i < id_list.size() && counter < request.limit; ++i)
                    {
                        auto &id = id_list[i];
                        auto &id_index = db.account.get<DB::id_tag>();
                        auto &account = id_index[id - id_index.front().id];

                        bool is_suitable = true;
                        for (auto filter_it = request.filter.begin(); filter_it != request.filter.end(); ++filter_it)
                        {
                            if (filter_it != index_by_field_it)
                            {
                                bool check_result = for_field_method(filter_it->field, filter_it->method, [&account, &filter_it](auto &&field, auto &&method)
                                {
                                    return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(account, filter_it->value);
                                });

                                if (!check_result)
                                {
                                    is_suitable = false;
                                    break;
                                }
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
                });
            });
        }
        else
        {
            JSONResult result(request);
            auto &index = db.account.get<DB::id_tag>();
            std::size_t counter = 0;
            for (auto it = index.rbegin(); it != index.rend() && counter < request.limit; ++it, ++counter)
            {
                result.add_account(*it);
            }

            response.result(boost::beast::http::status::ok);
            response.body() = result.get_json();
            response.prepare_payload();
        }
    }
};
