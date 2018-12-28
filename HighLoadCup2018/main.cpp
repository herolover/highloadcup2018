//#include <boost/asio.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/bimap.hpp>

#include <string>
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>

#include <rapidjson/filereadstream.h>

//namespace mi = boost::multi_index;

#include "Account.h"
#include "DBLoader.h"
#include "DB.h"

#include "PerformanceTimer.h"

int main()
{
    // *INDENT-OFF*
    //mi::multi_index_container<Account,
    //    mi::indexed_by<
    //        mi::ordered_unique<
    //            mi::identity<Account>>>> account_list;
    // *INDENT-ON*

    DB db;

    DBLoader db_loader(db);

    PerformanceTimer timer;

    for (int i = 1; i <= 3; ++i)
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

    return 0;
}