#include "Account.h"
#include "DBLoader.h"
#include "DB.h"
#include "FieldQuery.h"
#include "Split.h"

#include "FieldQuery/Sex.h"
#include "FieldQuery/Email.h"
#include "FieldQuery/Status.h"
#include "FieldQuery/FirstName.h"
#include "FieldQuery/SecondName.h"
#include "FieldQuery/Phone.h"
#include "FieldQuery/Country.h"
#include "FieldQuery/City.h"
#include "FieldQuery/Birth.h"
#include "FieldQuery/Premium.h"
#include "FieldQuery/Interest.h"
#include "FieldQuery/Like.h"

#include "PerformanceTimer.h"

#include <string>
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string_view>

#include <rapidjson/filereadstream.h>

using namespace std::literals;

template<class Range>
std::vector<uint32_t> convert_range_to_list(const Range &range)
{
    std::vector<uint32_t> result;
    for (auto it = range.first; it != range.second; ++it)
    {
        result.push_back(it->id);
    }

    return result;
}

template<class Iter, class Range>
Iter filter_by_range(Iter begin, Iter end, const Range &range)
{
    return std::remove_if(begin, end, [&](uint32_t id)
    {
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->id == id)
            {
                return false;
            }
        }

        return false;
    });
}

template<class Iter>
Iter filter_by_list(Iter begin, Iter end, const std::vector<uint32_t> &id_list)
{
    return std::remove_if(begin, end, [&](uint32_t id)
    {
        return std::find(id_list.begin(), id_list.end(), id) == id_list.end();
    });
}

bool filter(DB &db, const std::string_view &target, std::vector<uint32_t> &result)
{
    auto begin = result.begin();
    auto end = result.end();

    bool is_inited = false;
    auto process_range = [&](auto &&range)
    {
        if (is_inited)
        {
            end = filter_by_range(begin, end, range);
        }
        else
        {
            is_inited = true;

            result = convert_range_to_list(range);
            begin = result.begin();
            end = result.end();
        }
    };

    auto process_list = [&](auto &&list)
    {
        if (is_inited)
        {
            end = filter_by_list(begin, end, list);
        }
        else
        {
            is_inited = true;

            result = list;
            begin = result.begin();
            end = result.end();
        }
    };

    auto target_parts = split(target, '?');
    auto query_params = split(target_parts[1], '&');
    for (auto &query_param : query_params)
    {
        auto key_value = split(query_param, '=');
        if (key_value[0] == "query_id"sv || key_value[0] == "limit"sv)
        {
            continue;
        }

        auto field_method = split(key_value[0], '_');
        if (field_method.size() != 2)
        {
            return false;
        }

        auto &field = field_method[0];
        auto &method = field_method[1];
        auto &value = key_value[1];

        if (field == "sex"sv)
        {
            if (method == "eq"sv)
            {
                process_range(FieldQuery<DB::sex_tag>::eq(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "email"sv)
        {
            if (method == "domain"sv)
            {
                process_range(FieldQuery<DB::email_tag>::domain(db, value));
            }
            else if (method == "lt"sv)
            {
                process_range(FieldQuery<DB::email_tag>::lt(db, value));
            }
            else if (method == "gt"sv)
            {
                process_range(FieldQuery<DB::email_tag>::gt(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "status"sv)
        {
            if (method == "eq"sv)
            {
                process_range(FieldQuery<DB::status_tag>::eq(db, value));
            }
            else if (method == "neq"sv)
            {
                process_range(FieldQuery<DB::status_tag>::neq(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "fname"sv)
        {
            if (method == "eq")
            {
                process_range(FieldQuery<DB::first_name_tag>::eq(db, value));
            }
            else if (method == "any")
            {
                process_range(FieldQuery<DB::first_name_tag>::any(db, value));
            }
            else if (method == "null")
            {
                process_range(FieldQuery<DB::first_name_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "sname"sv)
        {
            if (method == "eq"sv)
            {
                process_range(FieldQuery<DB::second_name_tag>::eq(db, value));
            }
            else if (method == "starts"sv)
            {
                process_range(FieldQuery<DB::second_name_tag>::starts(db, value));
            }
            else if (method == "null"sv)
            {
                process_range(FieldQuery<DB::second_name_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "phone"sv)
        {
            if (method == "code"sv)
            {
                process_range(FieldQuery<DB::phone_tag>::code(db, value));
            }
            else if (method == "null"sv)
            {
                process_range(FieldQuery<DB::phone_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "country"sv)
        {
            if (method == "eq"sv)
            {
                process_range(FieldQuery<DB::country_tag>::eq(db, value));
            }
            else if (method == "null"sv)
            {
                process_range(FieldQuery<DB::country_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "city"sv)
        {
            if (method == "eq"sv)
            {
                process_range(FieldQuery<DB::city_tag>::eq(db, value));
            }
            else if (method == "any"sv)
            {
                //process_range(FieldQuery<DB::city_tag>::any(db, value));
            }
            else if (method == "null"sv)
            {
                process_range(FieldQuery<DB::city_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "birth"sv)
        {
            if (method == "lt"sv)
            {
                process_range(FieldQuery<DB::birth_tag>::lt(db, value));
            }
            else if (method == "gt"sv)
            {
                process_range(FieldQuery<DB::birth_tag>::gt(db, value));
            }
            else if (method == "year"sv)
            {
                process_range(FieldQuery<DB::birth_tag>::year(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "interests"sv)
        {
            if (method == "contains"sv)
            {
                //process_list(FieldQuery<DB::interest_tag>::contains(db, value));
            }
            else if (method == "any"sv)
            {
                //process_list(FieldQuery<DB::interest_tag>::any(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "likes"sv)
        {
            if (method == "contains"sv)
            {
                process_list(FieldQuery<DB::like_tag>::contains(db, value));
            }
            else
            {
                return false;
            }
        }
        else if (field == "premium"sv)
        {
            if (method == "now"sv)
            {
                process_range(FieldQuery<DB::premium_tag>::now(db, value));
            }
            else if (method == "null"sv)
            {
                process_range(FieldQuery<DB::premium_tag>::null(db, value));
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

int main()
{
    DB db;

    DBLoader db_loader(db);

    PerformanceTimer timer;

    for (int i = 1; i <= 3; ++i)
    {
        std::string filename = "accounts_" + std::to_string(i) + ".json";

        rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;
        std::FILE *fp = std::fopen(filename.c_str(), "r");
        char buffer[1024];
        rj::FileReadStream file_stream(fp, buffer, 1024);
        reader.Parse(file_stream, db_loader);
        std::fclose(fp);
    }

    std::cout << "Finished: " << db.account.size() << " " << timer.reset() * 1000 << std::endl;

    db.build_indicies();

    std::cout << "Finished: " << db.account.size() << " " << timer.reset() * 1000 << std::endl;
    std::cout << "first name size: " << Account::first_name_t::size() << std::endl;
    std::cout << "second name size: " << Account::second_name_t::size() << std::endl;
    std::cout << "country size: " << Account::country_t::size() << std::endl;
    std::cout << "city size: " << Account::city_t::size() << std::endl;
    std::cout << "interest size: " << Account::interest_t::size() << std::endl;

    //auto &index = db.account.get<DB::joined_tag>();
    //int i = 0;
    //for (auto it = index.rbegin(); i < 10; ++i, ++it)
    //{
    //    std::cout << it->id << " " << it->joined << ", ";
    //}

    auto range = FieldQuery<DB::interest_tag>::any(db, u8"Поцелуи,Симпсоны");
    //std::vector<uint32_t> result;
    //std::cout << "Result: " << filter(db, "/accounts/filter/?sname_null=0&query_id=2160&limit=18&sex_eq=m", result) << std::endl;

    std::cout << "Count: " << timer.elapsed_seconds() * 1000 << std::endl;

    //for (auto &id : result)
    //{
    //    std::cout << id << ", ";
    //}
    for (auto it = range.first; it != range.second; ++it)
    {
        //std::cout << it->id << " " << (it->first_name ? *it->first_name : "null") << ", ";
        std::cout << it->id << " " << *it->email << ", ";
        //std::cout << it->id << " " << it->birth << " " << it->birth_year << ", ";
        //phones.insert(it->phone);
    }

    std::cout << std::endl;

    return 0;
}