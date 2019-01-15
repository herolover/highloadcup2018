#pragma once

#include "../RequestHandler.h"

#include <rapidjson/rapidjson.h>

#include <algorithm>

template<>
struct RequestHandler<SuggestForAccount>
{
    class JSONResult
    {
    public:
        JSONResult(std::size_t limit)
            : _account_array(rapidjson::kArrayType)
            , _limit(limit)
        {
            _document.SetObject();
        }

        bool is_full() const
        {
            return _account_array.Size() == _limit;
        }

        void add_account(const Account &account)
        {
            rapidjson::Value json_account(rapidjson::kObjectType);
            json_account.AddMember("id", account.id, _document.GetAllocator());
            json_account.AddMember("email", t_get_json_value<f_email>()(account, _document.GetAllocator()), _document.GetAllocator());
            json_account.AddMember("status", t_get_json_value<f_status>()(account, _document.GetAllocator()), _document.GetAllocator());
            if (account.first_name)
            {
                json_account.AddMember("fname", t_get_json_value<f_first_name>()(account, _document.GetAllocator()), _document.GetAllocator());
            }
            if (account.second_name)
            {
                json_account.AddMember("sname", t_get_json_value<f_second_name>()(account, _document.GetAllocator()), _document.GetAllocator());
            }
            _account_array.PushBack(std::move(json_account), _document.GetAllocator());
        }

        const char *get_json()
        {
            _document.AddMember("accounts", std::move(_account_array), _document.GetAllocator());

            rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::ASCII<>> writer(_buffer);
            _document.Accept(writer);

            return _buffer.GetString();
        }

    private:
        rapidjson::Document _document;
        rapidjson::Value _account_array;
        rapidjson::StringBuffer _buffer;
        std::size_t _limit;
    };

    struct Similarity
    {
        DB::AccountReference with_account;
        int32_t value;

        Similarity(DB::AccountReference with_account, int32_t value)
            : with_account(with_account)
            , value(value)
        {
        }

        bool operator<(const Similarity &c) const
        {
            return value < c.value;
        }
    };

    static void handle(DB &db, SuggestForAccount &request, HttpServer::HttpResponse &response)
    {
        std::vector<Similarity> similarity_list;

        auto &index = db.account.get<DB::id_tag>();
        auto &account = index.find(request.search.account_id)->account();
        for (auto &like : account.like_list)
        {
            for (auto &with_account : db.liked_by[like.id])
            {
                if (with_account.account().id == account.id)
                {
                    continue;
                }

                bool is_suitable = true;
                auto &filter = request.search.filter;
                if (filter.field.index() != 0)
                {
                    is_suitable = for_field_method(filter.field, filter.method, [&with_account, &filter](auto &&field, auto &&method)
                    {
                        return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(with_account.account(), filter.value);
                    });
                }
                if (!is_suitable)
                {
                    continue;
                }

                int32_t ts_counter = 0;
                int32_t sum_ts = 0;
                auto &like_list = with_account.account().like_list;
                auto like_it = std::lower_bound(like_list.begin(), like_list.end(), like.id, Like::is_less());
                for (; like_it != like_list.end() && like_it->id == like.id; ++like_it)
                {
                    sum_ts += like_it->ts;
                    ++ts_counter;
                }

                similarity_list.emplace_back(with_account, std::abs(like.ts - sum_ts / ts_counter));
            }
        }

        std::sort(similarity_list.begin(), similarity_list.end());

        JSONResult result(request.search.limit);

        auto next_account = [](auto &it, auto last)
        {
            uint32_t id = it->id;
            do
            {
                ++it;
            }
            while (it != last && it->id == id);
        };
        for (auto it = similarity_list.begin(); it != similarity_list.end() && !result.is_full(); ++it)
        {
            auto &with_account = it->with_account.account();

            auto first1 = with_account.like_list.rbegin();
            auto last1 = with_account.like_list.rend();
            auto first2 = account.like_list.rbegin();
            auto last2 = account.like_list.rend();
            while (first1 != last1 && !result.is_full())
            {
                if (first2 == last2)
                {
                    while (first1 != last1 && !result.is_full())
                    {
                        result.add_account(index.find(first1->id)->account());
                        next_account(first1, last1);
                    }
                    break;
                }

                if (first1->id > first2->id)
                {
                    result.add_account(index.find(first1->id)->account());
                    next_account(first1, last1);
                }
                else
                {
                    if (first1->id == first2->id)
                    {
                        next_account(first1, last1);
                    }
                    next_account(first2, last2);
                }
            }
        }

        response.result(boost::beast::http::status::ok);
        response.body() = result.get_json();
        response.prepare_payload();
    }
};
