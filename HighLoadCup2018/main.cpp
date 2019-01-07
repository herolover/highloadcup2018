#include "DBLoader.h"
#include "DB.h"
#include "UrlDecoding.h"

#include "HttpRequestParser.h"
#include "HttpServer.h"

#include "RequestHandler.h"
#include "RequestHandler/FilterAccounts.h"

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

    std::cout << "Loaded: " << db.account.size() << " " << timer.reset() * 1000 << std::endl;

    //DB::RangeList list;
    //boost::fusion::vector<> list;
    //auto list2 = boost::fusion::push_back(list, std::make_optional(birth::year(db, "1990")));
    //auto list2 = birth::filter(db, "year", "1990", list);
    //boost::fusion::for_each(list2, [](auto &&range)
    //{
    //    if (range.has_value())
    //    {
    //        std::cout << "range: " << range->first->get_id() << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "no value" << std::endl;
    //    }
    //});

    boost::asio::io_context io_context;
    HttpServer http_server(io_context, 80);
    http_server.start([&db](HttpServer::HttpRequest &request, HttpServer::HttpResponse &response)
    {
        auto decoded_target = decode_url(std::string_view(request.target().data(), request.target().size()));
        auto parsed_request = parse_http_request(request, std::string_view(decoded_target.data(), decoded_target.size()));
        std::visit([&db, &response](auto &&request)
        {
            RequestHandler<std::remove_reference_t<decltype(request)>>::handle(db, request, response);
        }, parsed_request);
    });

    io_context.run();

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
    //auto range = FieldQuery<DB::like_tag>::reverse_contains(db, u8"26566");
    //std::vector<uint32_t> result;
    //std::cout << "Result: " << filter(db, "/accounts/filter/?sname_null=0&query_id=2160&limit=18&sex_eq=m", result) << std::endl;

    //auto range1 = FieldQuery<DB::second_name_tag>::null(db, "0");
    //auto range2 = FieldQuery<DB::sex_tag>::eq(db, "m");

    //boost::fusion::vector vec2 = boost::fusion::make_vector(range1, range1);
    //boost::fusion::for_each(vec2, [](auto &it)
    //{
    //    std::cout << it.first->id << " " << *it.first->first_name << std::endl;
    //});

    auto elapsed_seconds = timer.elapsed_seconds() * 1000;

    std::cout << "Count: " << elapsed_seconds << std::endl;

    //for (auto &id : result)
    //{
    //    std::cout << id << ", ";
    //}
    //for (auto &it = range.first; it != range.second; ++it)
    {
        //std::cout << it->id << " " << (it->first_name ? *it->first_name : "null") << ", ";
        //std::cout << it->id << " " << (it->city ? *it->city : "null") << ", ";
        //std::cout << it->get_id() << " " << *it->get_email() << ", ";
        //std::cout << it->id << " " << it->birth << " " << it->birth_year << ", ";
        //phones.insert(it->phone);
    }

    std::cout << "Finish" << std::endl;

    return 0;
}
