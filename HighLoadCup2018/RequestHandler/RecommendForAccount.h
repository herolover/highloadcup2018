#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<RecommendForAccount>
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
            json_account.AddMember("birth", t_get_json_value<f_birth>()(account, _document.GetAllocator()), _document.GetAllocator());
            if (account.premium_status != Account::PremiumStatus::NO)
            {
                json_account.AddMember("premium", t_get_json_value<f_premium>()(account, _document.GetAllocator()), _document.GetAllocator());
            }
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

    template<class BeginIt, class EndIt>
    static void filter(const RecommendForAccount &request, HttpServer::HttpResponse &response, const Account &account, BeginIt &&begin, EndIt &&end)
    {
        JSONResult result(request.search.limit);

        std::vector<const Account *> account_list;

        auto is_premium_status_or_status_changed = [&account_list](auto &&current_account)
        {
            return std::tie(account_list.back()->premium_status, account_list.back()->status) != std::tie(current_account.premium_status, current_account.status);
        };
        for (; begin != end && !(account_list.size() >= request.search.limit && is_premium_status_or_status_changed(begin->account())); ++begin)
        {
            auto &with_account = begin->account();

            if (account_list.size() > 0 && account_list.back()->id == with_account.id)
            {
                continue;
            }

            auto &filter = request.search.filter;
            if (filter.field.index() != 0)
            {
                bool check_result = for_field_method(filter.field, filter.method, [&with_account, &filter](auto &&field, auto &&method)
                {
                    return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(with_account, filter.value);
                });
                if (!check_result)
                {
                    continue;
                }
            }

            account_list.push_back(&with_account);
        }

        std::size_t limit = request.search.limit;
        if (limit > account_list.size())
        {
            limit = account_list.size();
        }

        std::partial_sort(account_list.begin(), account_list.begin() + limit, account_list.end(),
                          [&account](auto &&a, auto &&b)
        {
            auto a_common_interests = (a->interest_mask & account.interest_mask).count();
            auto b_common_interests = (b->interest_mask & account.interest_mask).count();

            auto a_age_difference = std::abs(a->birth - account.birth);
            auto b_age_difference = std::abs(b->birth - account.birth);

            return std::tie(a->premium_status, a->status, b_common_interests, a_age_difference, a->id) <
                   std::tie(b->premium_status, b->status, a_common_interests, b_age_difference, b->id);
        });

        for (auto &a : account_list)
        {
            result.add_account(*a);

            if (result.is_full())
            {
                break;
            }
        }

        response.result(boost::beast::http::status::ok);
        response.body() = result.get_json();
        response.prepare_payload();
    }

    static void handle(DB &db, RecommendForAccount &request, HttpServer::HttpResponse &response)
    {
        auto &account = db.account.get<DB::id_tag>().find(request.search.account_id)->account();

        using IterType = typename DB::InterestIndex::index<DB::recommend_tag>::type::iterator;
        std::vector<std::pair<IterType, IterType>> range_list;
        for (auto &interest : account.interest_list)
        {
            range_list.push_back(db.interest_account_list[interest].get<DB::recommend_tag>().equal_range(account.sex == Account::Sex::MALE ? Account::Sex::FEMALE : Account::Sex::MALE));
        }

        auto less = [](auto &&a, auto &&b)
        {
            return std::tie(a.account().premium_status, a.account().status, a.account().id) < std::tie(b.account().premium_status, b.account().status, b.account().id);
        };

        filter(request, response, account, union_iter<false, IterType, decltype(less)>(range_list, false, less), union_iter<false, IterType, decltype(less)>(range_list, true, less));
    }
};
