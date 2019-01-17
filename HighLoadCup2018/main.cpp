#include "DBLoader.h"
#include "DB.h"
#include "UrlDecoding.h"

#include "HttpRequestParser.h"
#include "HttpServer.h"

#include "RequestHandler.h"
#include "RequestHandler/BadRequest.h"
#include "RequestHandler/FilterAccounts.h"
#include "RequestHandler/GroupAccounts.h"
#include "RequestHandler/RecommendForAccount.h"
#include "RequestHandler/SuggestForAccount.h"

#include "PerformanceTimer.h"

#include <string>
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <thread>

#include <rapidjson/filereadstream.h>

using namespace std::literals;

int main(int argc, char *argv[])
{
    DB db;
    DBLoader db_loader(db);

    PerformanceTimer timer;

    std::string data(argv[1]);
    std::size_t limit = 0;
    std::from_chars(argv[2], argv[2] + std::strlen(argv[2]), limit);

    for (std::size_t i = 1; i <= limit; ++i)
    {
        std::string filename = "../data/" + data + "/data/accounts_" + std::to_string(i) + ".json";

        rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;
        std::FILE *fp = std::fopen(filename.c_str(), "r");
        char buffer[1024];
        rj::FileReadStream file_stream(fp, buffer, 1024);
        reader.Parse(file_stream, db_loader);
        std::fclose(fp);
    }

    db.build_indicies();

    std::cout << "Loaded: " << db.account.size() << " " << timer.reset() * 1000 << std::endl;

    boost::asio::io_context io_context;
    HttpServer http_server(io_context, 80);
    http_server.start([&db](HttpServer::HttpRequest &request, HttpServer::HttpResponse &response)
    {
        auto decoded_target = decode_url(std::string_view(request.target().data(), request.target().size()));
        auto parsed_request = parse_http_request(db, request, std::string_view(decoded_target.data(), decoded_target.size()));
        std::visit([&db, &response](auto &&request)
        {
            RequestHandler<std::decay_t<decltype(request)>>::handle(db, request, response);
        }, parsed_request);
    });

    auto t1 = std::thread([&io_context]()
    {
        io_context.run();

        std::cout << "io_context 1 finished" << std::endl;
    });
    auto t2 = std::thread([&io_context]()
    {
        io_context.run();

        std::cout << "io_context 2 finished" << std::endl;
    });
    auto t3 = std::thread([&io_context]()
    {
        io_context.run();

        std::cout << "io_context 3 finished" << std::endl;
    });

    io_context.run();

    std::cout << "io_context 0 finished" << std::endl;

    t3.join();
    t2.join();
    t1.join();

    std::cout << "all threads are joined" << std::endl;

    return 0;
}
