#pragma once

#include "../RequestHandler.h"

#include <boost/mp11.hpp>

#include <list>
#include <algorithm>

template<>
struct RequestHandler<GroupAccounts>
{
    using GroupKey = std::variant<bool, Account::Status, Account::interest_t, Account::country_t, Account::city_t>;

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

        void add_group(const std::string_view &key_name, const GroupKey &key_value, uint32_t counter)
        {
            rapidjson::Value group(rapidjson::kObjectType);
            std::visit([this, &key_name, &group](auto &&key_value)
            {
                using key_value_type = std::decay_t<decltype(key_value)>;
                if constexpr(std::is_same_v<bool, key_value_type>)
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), key_value, document.GetAllocator());
                }
                else if constexpr(std::is_same_v<Account::Status, key_value_type>)
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), (int)key_value, document.GetAllocator());
                }
                else
                {
                    group.AddMember(rapidjson::StringRef(key_name.data()), rapidjson::StringRef(*key_value), document.GetAllocator());
                }
            }, key_value);
            group.AddMember("count", counter, document.GetAllocator());

            group_array.PushBack(std::move(group), document.GetAllocator());
        }

        const char *get_json()
        {
            document.AddMember("groups", std::move(group_array), document.GetAllocator());

            rapidjson::Writer writer(buffer);
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
        std::list<GroupCounter> single_key;
        auto count_key = [&single_key](const GroupKey &key)
        {
            auto it = std::lower_bound(single_key.begin(), single_key.end(), key);
            if (it->key == key)
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
                if (request.key.size() == 1)
                {
                    std::visit([&account, &single_key, &count_key](auto &&field)
                    {
                        using field_type = std::decay_t<decltype(field)>;
                        if constexpr(std::is_same_v<f_sex, field_type>)
                        {
                            count_key(account.is_male);
                        }
                        else if constexpr(std::is_same_v<f_status, field_type>)
                        {
                            count_key(account.status);
                        }
                        else if constexpr(std::is_same_v<f_interests, field_type>)
                        {
                            for (auto &interest : account.interest)
                            {
                                count_key(interest);
                            }
                        }
                        else if constexpr(std::is_same_v<f_country, field_type>)
                        {
                            count_key(account.country);
                        }
                        else if constexpr(std::is_same_v<f_city, field_type>)
                        {
                            count_key(account.city);
                        }
                    }, request.key[0]);
                }
            }
        }

        single_key.sort([](const auto &a, const auto &b)
        {
            return a.counter > b.counter;
        });

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

    static void handle(DB &db, const GroupAccounts &request, HttpServer::HttpResponse &response)
    {
        using priority_list = boost::mp11::mp_list<f_likes, f_city, f_interests, f_country, f_birth, f_joined, f_status, f_sex>;

        auto select_by_filter_it = std::min_element(request.filter.begin(), request.filter.end(), [](auto &&a, auto &&b)
        {
            using a_type = std::decay_t<decltype(a.field)>;
            using b_type = std::decay_t<decltype(b.field)>;
            return boost::mp11::mp_find<priority_list, a_type>::value < boost::mp11::mp_find<priority_list, b_type>::value;
        });

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
