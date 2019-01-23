#pragma once

#include "../RequestHandler.h"
#include "../Convert.h"

#include <boost/mp11.hpp>

#include <list>
#include <algorithm>

template<>
struct RequestHandler<GroupAccounts>
{
    using GroupKey = std::variant<Account::Sex, Account::Status, std::string_view>;

    struct JSONResult
    {
        rapidjson::Document document;
        rapidjson::Value group_array;
        rapidjson::StringBuffer buffer;

        JSONResult()
            : group_array(rapidjson::kArrayType)
        {
            document.SetObject();
        }

        void add_group(const std::string_view &key_name, const GroupKey &value, uint32_t counter)
        {
            rapidjson::Value group(rapidjson::kObjectType);
            std::visit([this, &key_name, &group](auto &&value)
            {
                using key_value_type = std::decay_t<decltype(value)>;
                if constexpr(std::is_same_v<Account::Sex, key_value_type>)
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), rapidjson::StringRef(convert_sex(value)), document.GetAllocator());
                }
                else if constexpr(std::is_same_v<Account::Status, key_value_type>)
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), rapidjson::StringRef(convert_account_status(value)), document.GetAllocator());
                }
                else if constexpr(std::is_same_v<std::string_view, key_value_type>)
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), rapidjson::StringRef(value.data(), value.size()), document.GetAllocator());
                }
            }, value);
            group.AddMember("count", counter, document.GetAllocator());

            group_array.PushBack(std::move(group), document.GetAllocator());
        }

        const char *get_json()
        {
            document.AddMember("groups", std::move(group_array), document.GetAllocator());

            rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::ASCII<>> writer(buffer);
            document.Accept(writer);

            return buffer.GetString();
        }
    };

    struct GroupCounter
    {
        GroupKey key;
        uint32_t counter;

        bool operator<(const GroupKey &another) const
        {
            return key < another;
        }

        bool operator>(const GroupKey &another) const
        {
            return key > another;
        }

        bool operator<(const GroupCounter &another) const
        {
            return key < another.key;
        }

        bool operator>(const GroupCounter &another) const
        {
            return key > another.key;
        }
    };

    template<class BeginIt, class EndIt>
    static void filter(const GroupAccounts &request, HttpServer::HttpResponse &response, BeginIt &&begin, EndIt &&end)
    {
        std::vector<GroupCounter> single_key;
        auto count_key = [&single_key](const GroupKey &key)
        {
            auto it = std::lower_bound(single_key.begin(), single_key.end(), key);
            if (it != single_key.end() && it->key == key)
            {
                ++it->counter;
            }
            else
            {
                single_key.insert(it, GroupCounter{key, 1});
            }
        };

        for (; begin != end; ++begin)
        {
            auto &account = begin->account();

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
                if (request.key.size() == 1)
                {
                    std::visit([&account, &count_key](auto &&field)
                    {
                        using field_type = std::decay_t<decltype(field)>;
                        if constexpr(std::is_same_v<f_sex, field_type>)
                        {
                            count_key(account.sex);
                        }
                        else if constexpr(std::is_same_v<f_status, field_type>)
                        {
                            count_key(account.status);
                        }
                        else if constexpr(std::is_same_v<f_interests, field_type>)
                        {
                            for (auto &interest : account.interest_list)
                            {
                                count_key(interest);
                            }
                        }
                        else if constexpr(std::is_same_v<f_country, field_type>)
                        {
                            if (account.country)
                            {
                                count_key((std::string_view)*account.country);
                            }
                        }
                        else if constexpr(std::is_same_v<f_city, field_type>)
                        {
                            if (account.city)
                            {
                                count_key((std::string_view)*account.city);
                            }
                        }
                    }, request.key[0]);
                }
            }
        }

        auto limit = single_key.size();
        if (limit > request.limit)
        {
            limit = request.limit;
        }

        if (request.is_large_first)
        {
            std::partial_sort(single_key.begin(), single_key.begin() + limit, single_key.end(), [](const auto &a, const auto &b)
            {
                return a.counter > b.counter;
            });
        }
        else
        {
            std::partial_sort(single_key.begin(), single_key.begin() + limit, single_key.end(), [](const auto &a, const auto &b)
            {
                return a.counter < b.counter;
            });
        }

        JSONResult result;
        std::size_t counter = 0;
        for (auto it = single_key.begin(); it != single_key.end() && counter < request.limit; ++it, ++counter)
        {
            std::visit([&result, &it](auto &&field)
            {
                using field_type = std::decay_t<decltype(field)>;
                result.add_group(field_type::name, it->key, it->counter);
            }, request.key[0]);
        }

        response.result(boost::beast::http::status::ok);
        response.body() = result.get_json();
        response.prepare_payload();
    }

    static void handle(DB &db, GroupAccounts &request, HttpServer::HttpResponse &response)
    {
        response.result(boost::beast::http::status::not_implemented);
        response.prepare_payload();
        if (request.filter.empty())
        {
        }
        else if (request.filter.size() == 1 && std::holds_alternative<f_likes>(request.filter.front().field))
        {
            auto &likes_filter = request.filter.front();
            std::visit([&db, &request, &response, &likes_filter](auto &&field, auto &&method)
            {
                t_select<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(db, likes_filter.value, [&db, &request, &response](auto &&range)
                {
                    filter(request, response, range.first, range.second);
                });
            }, likes_filter.field, likes_filter.method);
        }
        else
        {
        }
    }
};
