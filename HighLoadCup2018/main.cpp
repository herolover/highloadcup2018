//#include <boost/asio.hpp>
#include <boost/multi_index_container.hpp>

#include <string>
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>

#include <rapidjson/filereadstream.h>

//namespace mi = boost::multi_index;

#include "Account.h"
#include "AccountLoader.h"

int main()
{
    // *INDENT-OFF*
    //mi::multi_index_container<Account,
    //    mi::indexed_by<
    //        mi::ordered_unique<
    //            mi::identity<Account>>>> account_list;
    // *INDENT-ON*

    std::vector<Account> accounts;
    AccountLoader reader_handler([&](Account &&account)
    {
        accounts.push_back(std::move(account));
    });

    for (int i = 1; i <= 3; ++i)
    {
        std::string filename = "accounts_" + std::to_string(i) + ".json";

        rj::GenericReader<rj::UTF8<>, rj::UTF16<>> reader;
        std::FILE *fp = std::fopen(filename.c_str(), "r");
        char buffer[1024];
        rj::FileReadStream file_stream(fp, buffer, 1024);
        reader.Parse(file_stream, reader_handler);
        std::fclose(fp);
    }

    std::cout << "Finished: " << accounts.size() << std::endl;

    return 0;
}