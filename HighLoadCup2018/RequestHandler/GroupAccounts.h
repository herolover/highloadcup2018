#pragma once

#include "../RequestHandler.h"
#include "../Convert.h"

#include <boost/mp11.hpp>

#include <list>
#include <algorithm>

template<class T>
struct GroupCounter
{
    T key;
    uint32_t count;

    bool operator<(const T &another_key) const
    {
        return key < another_key;
    }

    bool operator<(const GroupCounter &group_counter) const
    {
        return std::make_tuple(count, key) < std::make_tuple(group_counter.count, group_counter.key);
    }

    bool operator>(const GroupCounter &group_counter) const
    {
        return std::make_tuple(count, key) > std::make_tuple(group_counter.count, group_counter.key);
    }
};

template<class T>
struct GroupCounterList
{
    std::vector<GroupCounter<T>> group_counter;

    void count_key(const T &key)
    {
        auto it = std::lower_bound(group_counter.begin(), group_counter.end(), key);
        if (it != group_counter.end() && it->key == key)
        {
            ++it->count;
        }
        else
        {
            group_counter.insert(it, GroupCounter<T> {key, 1});
        }
    }

    void count_key(const T &key, std::size_t count)
    {
        auto it = std::lower_bound(group_counter.begin(), group_counter.end(), key);
        if (it != group_counter.end() && it->key == key)
        {
            it->count += count;
        }
        else
        {
            group_counter.insert(it, GroupCounter<T> {key, count});
        }
    }

    void sort(std::size_t limit, bool is_large_first)
    {
        if (limit > group_counter.size())
        {
            limit = group_counter.size();
        }

        if (is_large_first)
        {
            std::partial_sort(group_counter.begin(), group_counter.begin() + limit, group_counter.end(), std::greater<GroupCounter<T>>());
        }
        else
        {
            std::partial_sort(group_counter.begin(), group_counter.begin() + limit, group_counter.end());
        }
    }
};

template<class I, class F, class T>
struct count_by_field
{
    void operator()(const I &item, GroupCounterList<T> &group_counter, std::size_t count) const
    {
    }
};

template<class I>
struct count_by_field<I, f_sex, std::string_view>
{
    void operator()(const I &item, GroupCounterList<std::string_view> &group_counter, std::size_t count) const
    {
        group_counter.count_key(convert_sex(item.sex), count);
    }
};

template<class I>
struct count_by_field<I, f_status, std::string_view>
{
    void operator()(const I &item, GroupCounterList<std::string_view> &group_counter, std::size_t count)
    {
        group_counter.count_key(convert_account_status(item.status), count);
    }
};

template<class I>
struct count_by_field<I, f_city, std::string_view>
{
    void operator()(const I &item, GroupCounterList<std::string_view> &group_counter, std::size_t count)
    {
        if (item.city)
        {
            group_counter.count_key((std::string_view)*item.city, count);
        }
        else
        {
            group_counter.count_key(""sv, count);
        }
    }
};

template<class I>
struct count_by_field<I, f_country, std::string_view>
{
    void operator()(const I &item, GroupCounterList<std::string_view> &group_counter, std::size_t count)
    {
        if (item.country)
        {
            group_counter.count_key((std::string_view)*item.country, count);
        }
        else
        {
            group_counter.count_key(""sv, count);
        }
    }
};

template<>
struct count_by_field<Account, f_interests, std::string_view>
{
    void operator()(const Account &account, GroupCounterList<std::string_view> &group_counter, std::size_t count)
    {
        for (auto &interest : account.interest_list)
        {
            group_counter.count_key(interest, count);
        }
    }
};

template<class I>
struct count_by_field<I, std::pair<f_city, f_sex>, std::pair<std::string_view, std::string_view>>
{
    void operator()(const I &item,
                    GroupCounterList<std::pair<std::string_view, std::string_view>> &group_counter, std::size_t count)
    {
        if (item.city)
        {
            group_counter.count_key(std::make_pair((std::string_view)*item.city, convert_sex(item.sex)), count);
        }
        else
        {
            group_counter.count_key(std::make_pair(""sv, convert_sex(item.sex)), count);
        }
    }
};

template<class I>
struct count_by_field<I, std::pair<f_city, f_status>, std::pair<std::string_view, std::string_view>>
{
    void operator()(const I &item,
                    GroupCounterList<std::pair<std::string_view, std::string_view>> &group_counter, std::size_t count)
    {
        if (item.city)
        {
            group_counter.count_key(std::make_pair((std::string_view)*item.city, convert_account_status(item.status)), count);
        }
        else
        {
            group_counter.count_key(std::make_pair(""sv, convert_account_status(item.status)), count);
        }
    }
};

template<class I>
struct count_by_field<I, std::pair<f_country, f_sex>, std::pair<std::string_view, std::string_view>>
{
    void operator()(const I &item,
                    GroupCounterList<std::pair<std::string_view, std::string_view>> &group_counter, std::size_t count)
    {
        if (item.country)
        {
            group_counter.count_key(std::make_pair((std::string_view)*item.country, convert_sex(item.sex)), count);
        }
        else
        {
            group_counter.count_key(std::make_pair(""sv, convert_sex(item.sex)), count);
        }
    }
};

template<class I>
struct count_by_field<I, std::pair<f_country, f_status>, std::pair<std::string_view, std::string_view>>
{
    void operator()(const I &item,
                    GroupCounterList<std::pair<std::string_view, std::string_view>> &group_counter, std::size_t count)
    {
        if (item.country)
        {
            group_counter.count_key(std::make_pair((std::string_view)*item.country, convert_account_status(item.status)), count);
        }
        else
        {
            group_counter.count_key(std::make_pair(""sv, convert_account_status(item.status)), count);
        }
    }
};

struct is_suitable_group
{
    bool operator()(const DB::Group &group, f_sex, const Value &value) const
    {
        return group.sex == std::get<Account::Sex>(value);
    }

    bool operator()(const DB::Group &group, f_status, const Value &value) const
    {
        return group.status == std::get<Account::Status>(value);
    }

    bool operator()(const DB::Group &group, f_city, const Value &value) const
    {
        return group.city && group.city == std::get<std::string_view>(value);
    }

    bool operator()(const DB::Group &group, f_country, const Value &value) const
    {
        return group.country && group.country == std::get<std::string_view>(value);
    }

    template<class T>
    bool operator()(const DB::Group &group, T, const Value &value) const
    {
        return true;
    }
};

template<class T>
void add_json_document_member(rapidjson::Document &document, rapidjson::Value &group, const T &key, const T &value)
{
    if (value != ""sv)
    {
        group.AddMember(rapidjson::StringRef(key.data(), key.size()), rapidjson::StringRef(value.data(), value.size()), document.GetAllocator());
    }
}

template<>
void add_json_document_member<std::pair<std::string_view, std::string_view>>(rapidjson::Document &document, rapidjson::Value &group,
                                                                             const std::pair<std::string_view, std::string_view> &key,
                                                                             const std::pair<std::string_view, std::string_view> &value)
{
    if (value.first != ""sv)
    {
        group.AddMember(rapidjson::StringRef(key.first.data(), key.first.size()), rapidjson::StringRef(value.first.data(), value.first.size()), document.GetAllocator());
    }
    group.AddMember(rapidjson::StringRef(key.second.data(), key.second.size()), rapidjson::StringRef(value.second.data(), value.second.size()), document.GetAllocator());
}

template<class T>
class JSONResult
{
public:
    JSONResult(std::size_t limit, const T &key)
        : _group_array(rapidjson::kArrayType)
        , _limit(limit)
        , _key(key)
    {
        _document.SetObject();
    }

    bool is_full() const
    {
        return _group_array.Size() == _limit;
    }

    void add_group(const T &value, uint32_t counter)
    {
        rapidjson::Value group(rapidjson::kObjectType);
        add_json_document_member(_document, group, _key, value);
        group.AddMember("count", counter, _document.GetAllocator());

        _group_array.PushBack(std::move(group), _document.GetAllocator());
    }

    const char *get_json()
    {
        _document.AddMember("groups", std::move(_group_array), _document.GetAllocator());

        rapidjson::Writer writer(_buffer);
        _document.Accept(writer);

        return _buffer.GetString();
    }

private:
    rapidjson::Document _document;
    rapidjson::Value _group_array;
    rapidjson::StringBuffer _buffer;
    std::size_t _limit;
    T _key;
};

template<>
struct RequestHandler<GroupAccounts>
{
    static bool is_suitable_account(const GroupAccounts &request, const Account &account)
    {
        bool is_suitable = true;
        for (auto &filter : request.filter)
        {
            is_suitable = std::visit([&account, &filter](auto &&field, auto &&method)
            {
                return t_check<std::decay_t<decltype(field)>, std::decay_t<decltype(method)>>()(account, filter.value);
            }, filter.field, filter.method);

            if (!is_suitable)
            {
                break;
            }
        }

        return is_suitable;
    }

    static void handle(DB &db, GroupAccounts &request, HttpServer::HttpResponse &response)
    {
        std::visit([&](auto &&group_key)
        {
            using group_key_type = std::decay_t<decltype(group_key)>;
            using group_key_result_type = typename GroupKeyTrait<group_key_type>::type;
            GroupCounterList<group_key_result_type> group_counter;

            if (request.likes_filter.field.index() != 0)
            {
                t_select<f_likes, m_eq>()(db, request.likes_filter.value, [&](auto &&range)
                {
                    auto begin = range.first;
                    auto end = range.second;
                    for (; begin != end; ++begin)
                    {
                        auto &account = begin->account();
                        if (is_suitable_account(request, account))
                        {
                            count_by_field<Account, group_key_type, group_key_result_type>()(account, group_counter, 1);
                        }
                    }
                });
            }
            else if (!std::holds_alternative<f_interests>(request.group_key)
                     && request.interests_filter.field.index() == 0)
            {
                for (auto &group : db.group_index.group_list)
                {
                    bool is_suitable = true;
                    for (auto &filter : request.filter)
                    {
                        is_suitable = std::visit([&group, &filter](auto &&field)
                        {
                            return is_suitable_group()(group, field, filter.value);
                        }, filter.field);
                        if (!is_suitable)
                        {
                            break;
                        }
                    }

                    if (is_suitable)
                    {
                        if (request.birth_year_filter.field.index() != 0)
                        {
                            auto count = group.birth_year_count[std::get<uint16_t>(request.birth_year_filter.value)];
                            if (count != 0)
                            {
                                count_by_field<DB::Group, group_key_type, group_key_result_type>()(group, group_counter, count);
                            }
                        }
                        else if (request.joined_year_filter.field.index() != 0)
                        {
                            auto count = group.joined_year_count[std::get<uint16_t>(request.joined_year_filter.value)];
                            if (count != 0)
                            {
                                count_by_field<DB::Group, group_key_type, group_key_result_type>()(group, group_counter, count);
                            }
                        }
                        else if (group.count != 0)
                        {
                            count_by_field<DB::Group, group_key_type, group_key_result_type>()(group, group_counter, group.count);
                        }
                    }
                }
            }

            group_counter.sort(request.limit, request.is_large_first);

            JSONResult<group_key_result_type> result(request.limit, GroupKeyTrait<group_key_type>::key_name());
            for (auto &group : group_counter.group_counter)
            {
                if (result.is_full())
                {
                    break;
                }

                result.add_group(group.key, group.count);
            }

            response.result(boost::beast::http::status::ok);
            response.body() = result.get_json();
            response.prepare_payload();
        }, request.group_key);
    }
};
