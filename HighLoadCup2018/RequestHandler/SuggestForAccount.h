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

            rapidjson::Writer writer(_buffer);
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
        double value;

        Similarity(DB::AccountReference with_account, double value)
            : with_account(with_account)
            , value(value)
        {
        }

        bool operator<(const Similarity &c) const
        {
            return value > c.value;
        }

        struct is_less
        {
            using is_transparent = void;

            bool operator()(const Similarity &similarity, uint32_t id)
            {
                return similarity.with_account.id() < id;
            }

            bool operator()(uint32_t id, const Similarity &similarity)
            {
                return id < similarity.with_account.id();
            }
        };
    };

    template<class It>
    static int32_t compute_average_ts(It &begin, It end)
    {
        uint32_t id = begin->id;
        int64_t sum_ts = 0;
        uint8_t count = 0;
        while (begin != end && begin->id == id)
        {
            sum_ts += begin->ts;
            ++begin;
            ++count;
        }

        return static_cast<int32_t>(sum_ts / count);
    }

    static void handle(DB &db, SuggestForAccount &request, HttpServer::HttpResponse &response)
    {
        std::vector<Similarity> similarity_list;

        auto &account = *request.search.account_it;
        for (auto like_it = account.like_list.begin(); like_it != account.like_list.end();)
        {
            auto like_id = like_it->id;
            auto average_ts = compute_average_ts(like_it, account.like_list.end());
            for (auto &with_account : db.liked_by[like_id])
            {
                if (with_account.account().id == account.id)
                {
                    continue;
                }

                bool is_suitable = true;
                auto &filter = request.search.filter;
                if (filter.field.index() != 0)
                {
                    is_suitable = std::visit([&with_account, &filter](auto &&field, auto &&method)
                    {
                        return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(with_account.account(), filter.value);
                    }, filter.field, filter.method);
                }
                if (!is_suitable)
                {
                    continue;
                }

                auto &like_list = with_account.account().like_list;
                auto with_account_like_it = std::lower_bound(like_list.begin(), like_list.end(), like_id, Like::is_less());
                auto with_account_average_ts = compute_average_ts(with_account_like_it, like_list.end());

                auto diff_ts = std::abs(average_ts - with_account_average_ts);
                double similarity_value = 1.0;
                if (diff_ts != 0)
                {
                    similarity_value /= diff_ts;
                }
                auto similarity_it = std::lower_bound(similarity_list.begin(), similarity_list.end(), with_account.id(), Similarity::is_less());
                if (similarity_it != similarity_list.end() && similarity_it->with_account.id() == with_account.id())
                {
                    similarity_it->value += similarity_value;
                }
                else
                {
                    similarity_list.emplace(similarity_it, with_account, similarity_value);
                }
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
                        result.add_account(*db.find_account(first1->id));
                        next_account(first1, last1);
                    }
                    break;
                }

                if (first1->id > first2->id)
                {
                    result.add_account(*db.find_account(first1->id));
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
