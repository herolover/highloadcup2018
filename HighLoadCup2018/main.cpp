#include "DBLoader.h"
#include "DB.h"
#include "UrlDecoding.h"

#include "HttpRequestParser.h"
#include "HttpServer.h"

#include "RequestHandler.h"
#include "RequestHandler/BadRequest.h"
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

    for (int i = 1; i <= 130; ++i)
    {
        std::string filename = "../data/real_data/data/accounts_" + std::to_string(i) + ".json";

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
        auto parsed_request = parse_http_request(request, std::string_view(decoded_target.data(), decoded_target.size()));
        std::visit([&db, &response](auto &&request)
        {
            RequestHandler<std::remove_reference_t<decltype(request)>>::handle(db, request, response);
        }, parsed_request);
    });

    io_context.run();

    return 0;
}
