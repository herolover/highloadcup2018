﻿#include "Account.h"
#include "DBLoader.h"
#include "DB.h"

#include "FieldQuery/Sex.h"
#include "FieldQuery/Email.h"
#include "FieldQuery/Status.h"
#include "FieldQuery/FirstName.h"

#include "Split.h"

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

    for (int i = 1; i <= 1; ++i)
    {
        std::string filename = "accounts_" + std::to_string(i) + ".json";

        rj::GenericReader<rj::UTF8<>, rj::UTF16<>> reader;
        std::FILE *fp = std::fopen(filename.c_str(), "r");
        char buffer[1024];
        rj::FileReadStream file_stream(fp, buffer, 1024);
        reader.Parse(file_stream, db_loader);
        std::fclose(fp);
    }

    std::cout << "Finished: " << db.account.size() << " " << timer.elapsed_seconds() * 1000 << std::endl;
    std::cout << "first name size: " << db.first_name_size() << std::endl;
    std::cout << "second name size: " << db.second_name_size() << std::endl;
    std::cout << "country size: " << db.country_size() << std::endl;
    std::cout << "city size: " << db.city_size() << std::endl;
    std::cout << "interest size: " << db.interest_size() << std::endl;

    timer.reset();

    auto range = FieldQuery<DB::status_tag>::neq(db, L"заняты");
    for (auto it = range.first; it != range.second; ++it)
    {
        std::cout << it->id << ", ";
    }

    std::cout << "Count: " << " " << timer.elapsed_seconds() * 1000 << std::endl;

    return 0;
}