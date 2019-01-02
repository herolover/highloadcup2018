#include "Account.h"
#include "DBLoader.h"
#include "DB.h"

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

#include <rapidjson/filereadstream.h>

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

    auto now_time = std::time(nullptr);
    auto range = FieldQuery<DB::like_tag>::contains(db, u8"401");

    std::cout << "Count: " << timer.elapsed_seconds() * 1000 << std::endl;

    for (auto &id : range)
    {
        std::cout << id << ", ";
    }
    //for (auto it = range.first; it != range.second; ++it)
    //{
    //    //std::cout << it->id << " " << (it->city ? *it->city: "null") << ", ";
    //    //std::cout << it->id << " " << it->birth << " " << it->birth_year << ", ";
    //    //phones.insert(it->phone);
    //}


    return 0;
}