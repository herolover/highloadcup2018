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
    static void handle(DB &db, const FilterAccounts &request, HttpServer::HttpResponse &response)
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
            uint8_t counter = 0;
            t_select<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(db, index_by_field_it->value, [&counter, &db, &request, &response, &index_by_field_it](auto &&range)
            {
                std::vector<uint32_t> id_list;

                for (auto &it = range.first; it != range.second && counter < request.limit; ++it)
                {
                    bool is_suitable = true;
                    for (auto filter_it = request.filter.begin(); filter_it != request.filter.end(); ++filter_it)
                    {
                        if (filter_it != index_by_field_it)
                        {
                            bool check_result = for_field_method(filter_it->field, filter_it->method, [&it, &filter_it](auto &&field, auto &&method)
                            {
                                return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(it, filter_it->value);
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
                        id_list.emplace_back(it->id);
                        ++counter;
                    }
                }

                std::sort(id_list.begin(), id_list.end(), std::greater<int>());

                rj::Document document;
                rj::Value account_array(rj::kArrayType);
                for (auto &id : id_list)
                {
                    auto &account = db.account.get<DB::id_tag>()[id - 1];

                    rj::Value json_account(rj::kObjectType);
                    json_account.AddMember("id", id, document.GetAllocator());
                    json_account.AddMember("email", rj::Value(rj::StringRef(account.email)), document.GetAllocator());

                    for (auto &filter : request.filter)
                    {
                        std::visit([&document, &account, &json_account](auto &&field)
                        {
                            using field_type = std::decay_t<decltype(field)>;
                            if (field.name != "id"sv && field.name != "email"sv)
                            {
                                json_account.AddMember(rj::StringRef(field.name.data()), t_get_json_value<field_type>()(account), document.GetAllocator());
                            }
                        }, filter.field);
                    }

                    account_array.PushBack(json_account, document.GetAllocator());
                }

                document.SetObject();
                document.AddMember("accounts", account_array, document.GetAllocator());

                rj::StringBuffer buffer;
                rj::Writer writer(buffer);
                document.Accept(writer);

                response.result(boost::beast::http::status::ok);
                response.body() = buffer.GetString();
                response.prepare_payload();
            });
        });
    }
};
