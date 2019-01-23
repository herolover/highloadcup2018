#pragma once

#include "../RequestHandler.h"
#include "../AccountParser.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/memorystream.h>

template<>
struct RequestHandler<UpdateAccount>
{
    static void handle(DB &db, UpdateAccount &request, HttpServer::HttpResponse &response)
    {
        bool is_valid = false;
        thread_local AccountParser parser(db, [&db, &request, &is_valid](Account &&account)
        {
            account.id = request.account_id;
            account.interest_mask = db.get_interest_mask(account.interest_list);
            is_valid = db.update_account(std::move(account));
        }, AccountParserState::ACCOUNT_KEY);
        thread_local rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;

        parser.reset(AccountParserState::ACCOUNT_KEY);
        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
        if (!is_parsed)
        {
            parser.reset(AccountParserState::ACCOUNT_KEY);
        }
        if (is_parsed && is_valid)
        {
            response.result(boost::beast::http::status::accepted);
            response.body() = "{}";
        }
        else
        {
            response.result(boost::beast::http::status::bad_request);
        }
        response.prepare_payload();
    }
};
