﻿#include "Account.h"
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

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_vector.hpp>

using namespace std::literals;

bool filter(DB &db, const std::string_view &target, std::vector<uint32_t> &result)
{
    auto begin = result.begin();
    auto end = result.end();

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
                (FieldQuery<DB::sex_tag>::eq(db, value));
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
                (FieldQuery<DB::email_tag>::domain(db, value));
            }
            else if (method == "lt"sv)
            {
                (FieldQuery<DB::email_tag>::lt(db, value));
            }
            else if (method == "gt"sv)
            {
                (FieldQuery<DB::email_tag>::gt(db, value));
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
                (FieldQuery<DB::status_tag>::eq(db, value));
            }
            else if (method == "neq"sv)
            {
                (FieldQuery<DB::status_tag>::neq(db, value));
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
                //process_range(FieldQuery<DB::first_name_tag>::eq(db, value));
            }
            else if (method == "any")
            {
                //process_range(FieldQuery<DB::first_name_tag>::any(db, value));
            }
            else if (method == "null")
            {
                //process_range(FieldQuery<DB::first_name_tag>::null(db, value));
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
                (FieldQuery<DB::second_name_tag>::eq(db, value));
            }
            else if (method == "starts"sv)
            {
                (FieldQuery<DB::second_name_tag>::starts(db, value));
            }
            else if (method == "null"sv)
            {
                (FieldQuery<DB::second_name_tag>::null(db, value));
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
                (FieldQuery<DB::phone_tag>::code(db, value));
            }
            else if (method == "null"sv)
            {
                (FieldQuery<DB::phone_tag>::null(db, value));
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
                (FieldQuery<DB::country_tag>::eq(db, value));
            }
            else if (method == "null"sv)
            {
                (FieldQuery<DB::country_tag>::null(db, value));
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
                (FieldQuery<DB::city_tag>::eq(db, value));
            }
            else if (method == "any"sv)
            {
                //process_range(FieldQuery<DB::city_tag>::any(db, value));
            }
            else if (method == "null"sv)
            {
                (FieldQuery<DB::city_tag>::null(db, value));
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
                (FieldQuery<DB::birth_tag>::lt(db, value));
            }
            else if (method == "gt"sv)
            {
                (FieldQuery<DB::birth_tag>::gt(db, value));
            }
            else if (method == "year"sv)
            {
                (FieldQuery<DB::birth_tag>::year(db, value));
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
                (FieldQuery<DB::like_tag>::contains(db, value));
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
                (FieldQuery<DB::premium_tag>::now(db, value));
            }
            else if (method == "null"sv)
            {
                (FieldQuery<DB::premium_tag>::null(db, value));
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

    db.build_indicies();

    std::cout << "Finished: " << db.account.size() << " " << timer.reset() * 1000 << std::endl;
    std::cout << "first name size: " << Account::first_name_t::size() << std::endl;
    std::cout << "second name size: " << Account::second_name_t::size() << std::endl;
    std::cout << "country size: " << Account::country_t::size() << std::endl;
    std::cout << "city size: " << Account::city_t::size() << std::endl;
    std::cout << "interest size: " << Account::interest_t::size() << std::endl;

    //auto &index = db.account.get<DB::id_tag>();
    //for (auto &interest : index.find(24911)->interest)
    //{
    //    std::cout << *interest << ", ";
    //}

    //std::cout << std::endl;

    //for (auto &interest : index.find(21214)->interest)
    //{
    //    std::cout << *interest << ", ";
    //}

    //for (auto &account : db.interest[u8"Чудак"sv])
    //{
    //    if (account.id == 24911 || account.id == 21214)
    //    {
    //        std::cout << account.id << ",";
    //    }
    //}
    //std::cout << std::endl;
    //for (auto &account : db.interest[u8"Общение"sv])
    //{
    //    if (account.id == 24911 || account.id == 21214)
    //    {
    //        std::cout << account.id << ",";
    //    }
    //}
    //int i = 0;
    //for (auto it = index.rbegin(); i < 10; ++i, ++it)
    //{
    //    std::cout << it->id << " " << it->joined << ", ";
    //}

    timer.reset();
    //auto range = FieldQuery<DB::first_name_tag>::reverse_any(db, u8"Егор,Антон");
    //auto range = FieldQuery<DB::city_tag>::any(db, u8"Белово,Забург,Лейпориж");
    //auto range = FieldQuery<DB::interest_tag>::reverse_contains(db, u8"Чудак,Общение"sv);
    //auto range = FieldQuery<DB::interest_tag>::reverse_any(db, u8"Симпсоны");
    auto range = FieldQuery<DB::like_tag>::reverse_contains(db, u8"26566");
    //std::vector<uint32_t> result;
    //std::cout << "Result: " << filter(db, "/accounts/filter/?sname_null=0&query_id=2160&limit=18&sex_eq=m", result) << std::endl;

    auto range1 = FieldQuery<DB::second_name_tag>::null(db, "0");
    auto range2 = FieldQuery<DB::sex_tag>::eq(db, "m");

    boost::fusion::vector vec2 = boost::fusion::make_vector(range1, range1);
    boost::fusion::for_each(vec2, [](auto &it)
    {
        std::cout << it.first->id << " " << *it.first->first_name << std::endl;
    });

    auto elapsed_seconds = timer.elapsed_seconds() * 1000;

    std::cout << "Count: " << elapsed_seconds << std::endl;

    //for (auto &id : result)
    //{
    //    std::cout << id << ", ";
    //}
    for (auto &it = range.first; it != range.second; ++it)
    {
        //std::cout << it->id << " " << (it->first_name ? *it->first_name : "null") << ", ";
        //std::cout << it->id << " " << (it->city ? *it->city : "null") << ", ";
        std::cout << it->get_id() << " " << *it->get_email() << ", ";
        //std::cout << it->id << " " << it->birth << " " << it->birth_year << ", ";
        //phones.insert(it->phone);
    }

    std::cout << "Finish" << std::endl;

    return 0;
}